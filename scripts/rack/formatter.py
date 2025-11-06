import re, os
from datetime import datetime

def smart_format(template: str, data: dict) -> str:
    """
    Template formatter with chained filters.
    Supports syntax like:
        {name|upper}
        {value|abs|.2f}
        {time|strftime:%Y-%m-%d %H:%M}
        {path|basename|upper}
    """

    def apply_one_filter(value, filt):
        """Apply a single filter string to value."""
        filt = filt.strip()

        # --- named filter with argument (e.g. strftime:%Y-%m-%d) ---
        if ":" in filt:
            name, arg = filt.split(":", 1)
            name, arg = name.strip(), arg.strip()

            if name == "strftime" and hasattr(value, "strftime"):
                return value.strftime(arg)

            # You could add more name:arg filters here.
            return value  # ignore unknowns

        # --- datetime auto-detect ---
        if hasattr(value, "strftime") and filt.startswith("%"):
            return value.strftime(filt)

        # --- numeric format (.2f, e, etc.) ---
        if filt and filt[0] in ".0123456789eEfFgG":
            try:
                return format(value, filt)
            except Exception:
                pass

        # --- string operations ---
        if isinstance(value, str):
            if filt == "upper": return value.upper()
            if filt == "lower": return value.lower()
            if filt == "title": return value.title()
            if filt == "basename": return os.path.basename(value)
            if filt == "dirname": return os.path.dirname(value)
            if filt == "stem": return os.path.splitext(os.path.basename(value))[0]

        # --- generic filters ---
        if filt == "len":
            try:
                return len(value)
            except Exception:
                pass
        if filt == "abs":
            try:
                return abs(value)
            except Exception:
                pass

        # --- fallback ---
        try:
            return format(value, filt)
        except Exception:
            return value

    def apply_filters(value, filters):
        """Apply a sequence of filters to a value."""
        for f in filters:
            value = apply_one_filter(value, f)
        return value

    def repl(match):
        expr = match.group(1)
        parts = [p.strip() for p in expr.split("|")]
        var, filters = parts[0], parts[1:]

        value = data.get(var, "")
        if filters:
            value = apply_filters(value, filters)
        return str(value)

    return re.sub(r"\{([^{}]+)\}", repl, template)





if __name__ == "__main__":
    data = {
        "Mika": 1,
        "name": "Ada",
        "value": -3.1415926,
        "time": datetime(2025, 11, 3, 14, 25),
        "path": "/home/ada/data/results.txt",
        "seq": [1, 2, 3, 4],
    }

    template = (
        "Hello {name|upper}!\n"
        "Time: {time|strftime:%Y-%m-%d %H:%M}\n"
        "Rounded: {value|abs|.2f}\n"
        "File: {path|basename|upper}\n"
        "Number of items: {seq|len|.0f}\n"
    )

    print(smart_format(template, data))
