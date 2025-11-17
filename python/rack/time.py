
import datetime as dt

def parse_step(s, fmt="") -> int:
    """
    Convert step 's' to seconds.
    s: int, float, or string
    fmt: "", "%s", "%M", "%H", "%M:%S", "%H:%M:%S", etc.
    """
    # 1) No format → treat s as seconds
    if not fmt or fmt == "%s":
        return int(s)

    # 2) Convert input to string for parsing
    ss = str(s).strip()

    # 3) Supported formats (extend as needed)
    if fmt == "%M":
        return int(ss) * 60

    if fmt == "%H":
        return int(ss) * 3600

    if fmt == "%H:%M":
        h, m = ss.split(":")
        return int(h) * 3600 + int(m) * 60

    if fmt == "%M:%S":
        m, sec = ss.split(":")
        return int(m)*60 + int(sec)

    if fmt == "%H:%M:%S":
        h, m, sec = ss.split(":")
        return int(h)*3600 + int(m)*60 + int(sec)

    raise ValueError(f"Unsupported fmt '{fmt}'")


# --- helper: our safe UTC-aware strftime ---
def utc_strftime(dtobj, fmt:str) -> str:

    if "%s" in fmt:
        # "Prefilter": pick %s, and change it/them to numeric value (unix seconds).
        if dtobj.tzinfo is None:
            epoch_seconds = int((dtobj - dt.datetime(1970, 1, 1)).total_seconds())
        else:
            epoch_seconds = int(dtobj.timestamp())
        fmt = fmt.replace("%s", str(epoch_seconds))
    return dtobj.strftime(fmt)


def datetime_floor(t: dt.datetime, s, fmt=""):
    step = parse_step(s, fmt)
    seconds = t.hour*3600 + t.minute*60 + t.second
    newsec = (seconds // step) * step
    return t.replace(hour=0, minute=0, second=0, microsecond=0) + dt.timedelta(seconds=newsec)


def datetime_round(t: dt.datetime, s, fmt=""):
    step = parse_step(s, fmt)
    seconds = t.hour*3600 + t.minute*60 + t.second
    rounded = int((seconds + step/2) // step) * step
    return t.replace(hour=0, minute=0, second=0, microsecond=0) + dt.timedelta(seconds=rounded)


def datetime_ceil(t: dt.datetime, s, fmt=""):
    step = parse_step(s, fmt)
    seconds = t.hour*3600 + t.minute*60 + t.second
    ceiled = ((seconds + step - 1) // step) * step
    return t.replace(hour=0, minute=0, second=0, microsecond=0) + dt.timedelta(seconds=ceiled)

# def truncate_datetime(t: dt.datetime, s, fmt="", mode="floor"):
def datetime_truncate(t: dt.datetime, mode, s, fmt=""):
    """
    Universal truncator.

    mode:
      - "floor"
      - "round"
      - "ceil"
      - a callable(t, s, fmt)
    """
    if callable(mode):
        return mode(t, s, fmt)

    mode = mode.lower()

    if mode == "floor":
        return datetime_floor(t, s, fmt)

    if mode == "round":
        return datetime_round(t, s, fmt)

    if mode == "ceil":
        return datetime_ceil(t, s, fmt)

    raise ValueError(f"Unknown mode '{mode}' — expected floor, round, ceil or callable")


def datetime_floor2(t: dt.datetime, s: int) -> dt.datetime:
    """Truncate datetime to nearest lower multiple of s seconds."""
    # Seconds since the day's start
    seconds = t.hour * 3600 + t.minute * 60 + t.second

    # Truncate
    new_seconds = (seconds // s) * s

    # Return new datetime
    return t.replace(hour=0, minute=0, second=0, microsecond=0) + dt.timedelta(seconds=new_seconds)

def datetime_round2(t: dt.datetime, s: int) -> dt.datetime:
    """Round datetime to nearest multiple of s seconds."""
    seconds = t.hour * 3600 + t.minute * 60 + t.second
    rounded = int((seconds + s/2) // s) * s
    return t.replace(hour=0, minute=0, second=0, microsecond=0) + dt.timedelta(seconds=rounded)

def datetime_ceil2(t: dt.datetime, s: int) -> dt.datetime:
    seconds = t.hour * 3600 + t.minute * 60 + t.second
    ceiled = ((seconds + s - 1) // s) * s
    return t.replace(hour=0, minute=0, second=0, microsecond=0) + dt.timedelta(seconds=ceiled)

