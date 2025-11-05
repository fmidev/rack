import string
import os
from datetime import datetime


class SmartFormatter(string.Formatter):
    """
    Extended formatter supporting filters after '|':
        {time|%Y-%m-%d %H:%M}
        {value|.2f}
        {text|upper}
        {path|basename}
        {seq|len}
    """

    def get_field(self, field_name, args, kwargs):
        # Custom field parsing: handle "|" filter syntax first
        if "|" in field_name:
            var, filt = field_name.split("|", 1)
            var = var.strip()
            filt = filt.strip()
            value = kwargs.get(var, None)
            return self.apply_filter(value, filt), var
        # Fallback to normal behaviour
        return super().get_field(field_name, args, kwargs)

    def apply_filter(self, value, filt):
        # --- datetime formatting ---
        if hasattr(value, "strftime") and filt.startswith("%"):
            return value.strftime(filt)

        # --- numeric format (.2f etc.) ---
        if filt and filt[0] in ".0123456789eEfFgG":
            try:
                return format(value, filt)
            except Exception:
                pass

        # --- string transformations ---
        if isinstance(value, str):
            if filt == "upper": return value.upper()
            if filt == "lower": return value.lower()
            if filt == "title": return value.title()
            if filt == "basename": return os.path.basename(value)
            if filt == "dirname": return os.path.dirname(value)
            if filt == "stem": return os.path.splitext(os.path.basename(value))[0]

        # --- sequence filters ---
        if filt == "len":
            try:
                return str(len(value))
            except Exception:
                pass

        # --- fallback ---
        try:
            return format(value, filt)
        except Exception:
            return str(value)

        
# Example usage
if __name__ == "__main__":
    
    fmt = SmartFormatter()
    data = {
        "name": "Ada",
        "value": 3.1415926,
        "time": datetime(2025, 11, 3, 14, 25),
        "path": "/home/ada/data/results.txt",
        "seq": [1, 2, 3]
    }

    template = (
        "Hello {name|upper}!\n"
        "Time: {time|%Y-%m-%d %H %M}\n"
        "Value: {value|.2f}\n"
        "File: {path|basename} (dir: {path|dirname})\n"
        "Number of items: {seq|len}"
    )


    
    print(fmt.format("Hello {name|upper}!\n", **data))
    print(fmt.format("Time: {time|%Y-%m-%d %H %M}\n", **data))
    #print(fmt.format("Time: {time|%Y-%m-%d %H:%M}\n", time = datetime(2025, 11, 3, 14, 25) ))
    print(fmt.format(template, **data))
