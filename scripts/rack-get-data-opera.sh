#!/bin/bash

# Retrieve EUMETNET OPERA single-site volume radar data from the ORD S3 bucket.
#
# Bucket:   openradar-24h  (CloudFerro S3, not Amazon — but aws-compatible)
# Endpoint: https://s3.waw3-1.cloudferro.com/
# S3 path:  s3://openradar-24h/YYYY/MM/DD/CC/site/PVOL/
# Data:     24-hour rolling archive; ODIM HDF5, CC BY 4.0
# Docs:     https://github.com/EUMETNET/openradardata-documentation/
# Stations: https://github.com/EUMETNET/openradardata-validator/blob/main/src/openradardata_validator/stations/OPERA_RADARS.csv
#
# Usage:
#   rack-get-data-opera.sh [DATE_SPEC [CC/SITE...]]
#   DATE_SPEC: argument for GNU 'date --date', e.g. "1 day ago", "yesterday",
#              "2 hours ago", "2026-10-14".  Default: "1 day ago".
#              If the first argument contains '/', it is treated as CC/SITE
#              and the default date is used.
#   CC/SITE:  2-letter ISO country code and ODIM site code, e.g. NO/nohur, FI/fivih.
#             Default: the predefined TEST_SITES list below.
#   HOUR:     target UTC hour; the file closest to this hour is chosen. Default: 12.
#   OUTDIR:   local cache root. Default: cache
#
# Tool selection:
#   This bucket uses CloudFerro S3 (s3.waw3-1.cloudferro.com), not Amazon S3,
#   but the S3 API is compatible.  'aws' with --endpoint-url and --no-sign-request
#   is the most capable tool: it lists directories natively and downloads cleanly.
#   It is preferred here because filenames inside PVOL/ are not known in advance
#   and must be discovered by listing.  wget/curl are used as download fallback
#   if aws is not installed; listing then falls back to curl + S3 XML parsing.

S3_BUCKET='openradar-24h'
S3_ENDPOINT='https://s3.waw3-1.cloudferro.com'

# Predefined test set: 2-letter ISO country code / ODIM site code
# (only a few sites; expand as needed)
TEST_SITES=(
    'NO/nohur'   # Hurum, Norway
    'FI/fivih'   # Vihti, Finland
    'DE/deboo'   # Boostedt, Germany
)

CACHE=${OUTDIR:-'cache'}
HOUR=${HOUR:-12}

# --- Argument parsing ---
# If first arg contains '/' it is already a CC/SITE, so use the default date.
if [[ $# -eq 0 || "${1:-}" == */* ]]; then
    DATE_SPEC='1 day ago'
else
    DATE_SPEC="$1"
    shift
fi

if [ $# -gt 0 ]; then
    SITES=( "$@" )
else
    SITES=( "${TEST_SITES[@]}" )
fi

# Resolve the target date from the date spec (GNU date --date syntax)
TIMESTAMP=$(date --utc --date="$DATE_SPEC" '+%Y%m%d') || {
    echo "ERROR: unrecognised DATE_SPEC '${DATE_SPEC}'" >&2
    exit 1
}
YEAR=${TIMESTAMP:0:4}
MONTH=${TIMESTAMP:4:2}
DAY=${TIMESTAMP:6:2}
HHMM=$(printf '%02d00' "$HOUR")

echo "# Date: ${YEAR}-${MONTH}-${DAY}, target hour: ${HHMM:0:2}:${HHMM:2:2} UTC"

# --- list_files CC SITE YEAR MONTH DAY ---
# Prints S3 keys (full paths) of all volume files for this site on this date.
# The object-type subdirectory (PVOL, SCAN, …) varies per radar and is NOT
# hardcoded: the function lists everything under CC/SITE/ so both PVOL and
# SCAN radars are handled uniformly.
list_files() {

    local CC=$1 SITE=$2 YEAR=$3 MONTH=$4 DAY=$5
    
    local DATASET_NAME=${0} # Strip Extension
    DATASET_NAME=${DATASET_NAME%.*}  # Strip Extension
    DATASET_NAME=${DATASET_NAME##*rack-get-} # Strip leading part, leave "data-XXXX"

    # local PREFIX="${YEAR}/${MONTH}/${DAY}/${CC}/${SITE}"    
    local PREFIX="${DATASET_NAME}/${YEAR}-${MONTH}-${DAY}/${SITE}"
    
    if command -v aws &>/dev/null; then
        aws s3 ls "s3://${S3_BUCKET}/${PREFIX}/" \
            --endpoint-url "$S3_ENDPOINT" --no-sign-request \
            --recursive \
            | awk '{print $NF}'
    else
        # S3 XML listing via HTTPS — no delimiter, so all objects under the
        # site prefix are returned regardless of object-type subdir (PVOL/SCAN/…)
        curl --silent \
            "${S3_ENDPOINT}/${S3_BUCKET}?prefix=${PREFIX}/&max-keys=1000" \
            | grep -oP '(?<=<Key>)[^<]+'
    fi
}

# --- download_one CC SITE S3_KEY ---
# S3_KEY is the full object key as returned by list_files (includes PVOL/ or SCAN/ etc.)
download_one() {
    local CC=$1 SITE=$2 S3_KEY=$3
    local FILENAME=$(basename "$S3_KEY")
    local OBJTYPE=$(echo "$S3_KEY" | grep -oP '[^/]+(?=/[^/]+$)')  # dir just above filename
    local DIR="${CACHE}/$(dirname "$S3_KEY")"
    mkdir -v --parents "$DIR"
    local DST="${DIR}/${FILENAME}"

    echo "# s3://${S3_BUCKET}/${S3_KEY}"
    echo "#   -> ${DST}"

    if command -v aws &>/dev/null; then
        aws s3 cp "s3://${S3_BUCKET}/${S3_KEY}" "$DST" \
            --endpoint-url "$S3_ENDPOINT" --no-sign-request
    elif command -v wget &>/dev/null; then
        wget --no-verbose --continue \
             --output-document="$DST" \
             "${S3_ENDPOINT}/${S3_BUCKET}/${S3_KEY}"
    elif command -v curl &>/dev/null; then
        curl --silent --show-error --location --continue-at - \
             --output "$DST" "${S3_ENDPOINT}/${S3_BUCKET}/${S3_KEY}"
    else
        echo "ERROR: none of aws, wget, curl found — cannot download" >&2
        return 1
    fi

    update_conf "$CC" "$SITE" "$DST" "$OBJTYPE"
}

# --- update_conf CC SITE DST_FILE ---
# Creates a per-radar conf directory on first encounter, populates it with ODIM
# source metadata using rack, and creates an FMI-convention named symlink so
# the file integrates with the rest of the test infrastructure.
#
# FMI naming: ${TIMESTAMP}_radar.polar.${NOD}.h5
#   where TIMESTAMP = YYYYMMDDHHmm and NOD = ODIM site identifier (same as SITE
#   in the S3 path, but confirmed from the file's own /what/source attribute).
#
# The conf directory (cache/conf/CC/SITE/) is keyed by the S3 CC+site path,
# which maps 1:1 to the WMO/NOD radar identity.  rack is used to extract the
# canonical ODIM source string on first encounter; subsequent downloads reuse
# the cached mapping without re-reading the file.
update_conf() {
    local CC=$1 SITE=$2 DST=$3 OBJTYPE=${4:-''}
    local CONF_DIR="${CACHE}/conf/${CC}/${SITE}"

    if [ ! -f "${CONF_DIR}/conf" ]; then
        mkdir -v --parents "$CONF_DIR"

        # rack reads the file and parses /what/source into the status map.
        # ${SRC} = canonical preferred source code (e.g. "NOD:fivih");
        # ${NOD}, ${WMO}, ${RAD}, ${PLC}, ${CMT} are the individual fields.
        # No shell-side parsing needed — rack's SourceODIM does it all.
        if command -v rack &>/dev/null; then
            rack "$DST" --echo 'SRC=${SRC}\nNOD=${NOD}\nWMO=${WMO}\nRAD=${RAD}\nPLC=${PLC}\n' \
                2>/dev/null > "${CONF_DIR}/conf"
        else
            # No rack: fall back to the site code from the S3 path
            printf 'NOD=%s\nCC=%s\n' "$SITE" "$CC" > "${CONF_DIR}/conf"
        fi
        echo "# conf: ${CONF_DIR}/conf"
        cat "${CONF_DIR}/conf"
    fi

    # Read the cached NOD code (canonical ODIM identifier for local naming)
    local NOD
    NOD=$(grep '^NOD=' "${CONF_DIR}/conf" 2>/dev/null | cut -d= -f2)
    NOD=${NOD:-$SITE}

    # Extract timestamp from the original filename.
    # OPERA PVOL filenames carry a timestamp; common patterns:
    #   14-digit: YYYYMMDDHHmmSS (e.g. WMO GTS: T_PAVO..._20261016120000.h5)
    #   T-format: YYYYMMDDTHHmm  (e.g. ODIM-style: 20261016T1200)
    local FILENAME DST_DIR TS HHMM_FILE
    FILENAME=$(basename "$DST")
    DST_DIR=$(dirname "$DST")

    TS=$(echo "$FILENAME" | grep -oP '\d{14}' | head -1)
    TS=${TS:0:12}  # keep YYYYMMDDHHmm, drop seconds

    if [ -z "$TS" ]; then
        TS=$(echo "$FILENAME" | grep -oP '\d{8}T\d{4}' | head -1 | tr -d 'T')
    fi

    if [ -z "$TS" ]; then
        echo "# Warning: could not extract timestamp from '${FILENAME}' — skipping symlink" >&2
        return
    fi

    # Create symlink with FMI naming convention (relative, so it survives moves)
    local LINK_NAME="${TS}_radar.polar.${NOD}.h5"
    local LINK_PATH="${DST_DIR}/${LINK_NAME}"
    if [ ! -e "$LINK_PATH" ]; then
        ln -vs "$FILENAME" "$LINK_PATH"
    fi
}

# --- download_site CC/SITE ---
download_site() {
    local CC=${1%%/*}    # everything before the first /
    local SITE=${1##*/}  # everything after the last /

    echo "# Listing ${CC}/${SITE} for ${YEAR}-${MONTH}-${DAY}..."
    local ALL_KEYS
    ALL_KEYS=$(list_files "$CC" "$SITE" "$YEAR" "$MONTH" "$DAY")

    if [ -z "$ALL_KEYS" ]; then
        echo "# No files found for ${CC}/${SITE} on ${YEAR}-${MONTH}-${DAY}" >&2
        return 1
    fi

    # Keep only DBZH files to avoid downloading every quantity separately.
    # Each radar delivers multiple quantities (TH, VRADH, …) as individual files;
    # one DBZH volume per timestamp is enough for basic testing.
    local DBZH_KEYS
    DBZH_KEYS=$(echo "$ALL_KEYS" | grep 'DBZH')
    ALL_KEYS=${DBZH_KEYS:-$ALL_KEYS}  # fall back to all if no DBZH match

    # Pick the key whose name is closest to the target hour.
    # Filename format: site@YYYYMMDDTHHmm@…  — match on the 2-digit hour after 'T'.
    local TARGET_KEY
    TARGET_KEY=$(echo "$ALL_KEYS" | grep "T${HHMM:0:2}" | head -1)
    TARGET_KEY=${TARGET_KEY:-$(echo "$ALL_KEYS" | head -1)}  # fallback: first in list

    download_one "$CC" "$SITE" "$TARGET_KEY"
}

# --- Main loop ---
for CC_SITE in "${SITES[@]}"; do
    download_site "$CC_SITE"
done
