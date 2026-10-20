#!/bin/bash

# Retrieve open single-radar HDF5 volume data from the FMI open data AWS S3 bucket.
#
# Bucket:   s3://fmi-opendata-radar-volume-hdf5  (public, no credentials required)
# Web view: http://fmi-opendata-radar-volume-hdf5.s3-website-eu-west-1.amazonaws.com/
# S3 HTTPS: https://fmi-opendata-radar-volume-hdf5.s3.eu-west-1.amazonaws.com/
# Instructions: https://en.ilmatieteenlaitos.fi/radar-data-on-aws-s3
#
# Usage:
#   rack-get-data-fmi.sh [VOLUME]
#   VOLUME=202610140015_fivih_PVOL.h5 rack-get-data-fmi.sh
#   rack-get-data-fmi.sh all      # download the predefined test set
#
# OUTDIR can be set externally; default is cache/YYYY/MM/DD
#
# Tool selection:
#   wget and curl are pre-installed on virtually all Linux distros and work
#   directly with the HTTPS URL — best for portability.
#   The 'aws' CLI (aws s3 cp/ls --no-sign-request) is more flexible: it can
#   list bucket contents without parsing XML and handles path discovery cleanly,
#   but requires a separate installation (pip install awscli, or distro package).
#   This script auto-detects 'aws' and falls back to wget/curl.

BUCKET='fmi-opendata-radar-volume-hdf5'
REGION='eu-west-1'
S3_BASE="https://${BUCKET}.s3.${REGION}.amazonaws.com"
S3_URI="s3://${BUCKET}"

# Predefined test set: a handful of sites at a fixed timestamp for reproducibility.
# One file per site is enough for basic pipeline testing.
TEST_VOLUMES=(
    '202610140015_fivih_PVOL.h5'
    '202610140015_fikor_PVOL.h5'
    '202610140015_fivan_PVOL.h5'
)

# --- download_volume ---
# Download one volume file and rename it to the FMI local convention.
# Args: VOLUME (filename as it appears in the directory 202610140015_radar.polar.fivih.h5)
# Former logic: VOLUME (filename as it appears in the bucket, e.g. 202610140015_fivih_PVOL.h5)
download_volume() {
    local VOLUME=${1##*/}  # strip any leading path — filename only

    # Split filename on _ and . delimiters into an array.
    # E.g. '202610140015_fivih_PVOL.h5' -> (202610140015 fivih PVOL h5)
    # local METADATA=( ${VOLUME//[_.]/ } )
    # local TIMESTAMP=${METADATA[0]}   # 12-digit YYYYMMDDHHmm
    # local SITE=${METADATA[1]}        # e.g. fivih
    local METADATA=( ${VOLUME//_/ } )
    local TIMESTAMP=${METADATA[0]}   # 12-digit YYYYMMDDHHmm
    local PRODUCT=${METADATA[1]%.*}  # Strip Extension, resulting radar.polar.fivih
    # echo PRODUCT=${PRODUCT}  
    local SITE=${PRODUCT##*.}  # Strip Extension, resulting radar.polar.fivih
    # METADATA[2] is object type (PVOL etc.), unused here

    # Extract date components from timestamp
    local YEAR=${TIMESTAMP:0:4}
    local MONTH=${TIMESTAMP:4:2}
    local DAY=${TIMESTAMP:6:2}

    local DATASET_NAME=${0} # Strip Extension
    DATASET_NAME=${DATASET_NAME%.*}  # Strip Extension
    #DATASET_NAME=${DATASET_NAME##*/} # Strip Extension
    DATASET_NAME=${DATASET_NAME##*rack-get-} # Strip leading part, leave "data-XXXX"
    local DIR=${OUTDIR:-"cache/${DATASET_NAME}/${YEAR}-${MONTH}-${DAY}/${SITE}"}
    mkdir -v --parents "${DIR}"

    # FMI local naming convention: TIMESTAMP_radar.polar.SITE.h5
    #local DST="${DIR}/${TIMESTAMP}_radar.polar.${SITE}.h5"
    local DST="${DIR}/${VOLUME}"

    # S3 object key: YYYY/MM/DD/site/filename
    # local S3_KEY="${YEAR}/${MONTH}/${DAY}/${SITE}/${VOLUME}"
    local S3_KEY="${YEAR}/${MONTH}/${DAY}/${SITE}/${TIMESTAMP}_${SITE}_PVOL.h5"

    # echo "# ${S3_URI}/${S3_KEY}  ->  ${DST}"

    local cmd
    if command -v aws &>/dev/null; then
        cmd=( aws s3 cp "${S3_URI}/${S3_KEY}" "${DST}" --no-sign-request )
    elif command -v wget &>/dev/null; then
        cmd=( wget --no-verbose --continue --output-document="${DST}" "${S3_BASE}/${S3_KEY}" )
    elif command -v curl &>/dev/null; then
        cmd=( curl --silent --show-error --location --continue-at - \
                   --output "${DST}" "${S3_BASE}/${S3_KEY}" )
    else
        echo "ERROR: none of aws, wget, curl found — cannot download" >&2
        return 1
    fi

    echo "# ${cmd[*]}"
    "${cmd[@]}"
}

# --- list_site ---
# List available volume files for a given site and date.
# Args: SITE YEAR MONTH DAY
# Useful for discovery; requires aws or falls back to S3 XML listing via curl/wget.
list_site() {
    local SITE=$1 YEAR=$2 MONTH=$3 DAY=$4
    local PREFIX="${YEAR}/${MONTH}/${DAY}/${SITE}/"
    if command -v aws &>/dev/null; then
        aws s3 ls "${S3_URI}/${PREFIX}" --no-sign-request
    else
        # S3 XML listing via HTTPS query parameter; parse out Key elements
        local LIST_URL="${S3_BASE}/?prefix=${PREFIX}"
        if command -v curl &>/dev/null; then
            curl --silent "${LIST_URL}"
        else
            wget --quiet --output-document=- "${LIST_URL}"
        fi | grep -oP '(?<=<Key>)[^<]+'
    fi
}

# --- main ---

CMD=${1:-''}

case "$CMD" in
    all)
        for VOL in "${TEST_VOLUMES[@]}"; do
            download_volume "$VOL"
        done
        ;;
    list)
        # Example: rack-get-data-fmi.sh list fivih 2026 10 14
        list_site "${2:-fivih}" "${3:-2026}" "${4:-10}" "${5:-14}"
        ;;
    '')
        # Default: download the first test volume
        download_volume "${TEST_VOLUMES[0]}"
        ;;
    *)
        # Treat the argument as an explicit volume filename
	# download_volume "$CMD"
	for i in $*; do
            download_volume "$i"
	done
        ;;
esac
