import argparse
import re
import os
import datetime as dt

from abc import ABC, abstractmethod

import rack.log
logger = rack.log.logger.getChild(os.path.splitext(os.path.basename(__file__))[0])


# --- helper: our safe UTC-aware strftime ---
def utc_strftime(dtobj, fmt):
    if "%s" in fmt:
        if dtobj.tzinfo is None:
            epoch_seconds = int((dtobj - dt.datetime(1970, 1, 1)).total_seconds())
        else:
            epoch_seconds = int(dtobj.timestamp())
        fmt = fmt.replace("%s", str(epoch_seconds))
    return dtobj.strftime(fmt)

# --- shared abstract base class ---
class Token(ABC):
    @abstractmethod
    def resolve(self, data):
        pass


class Literal(Token):
    def __init__(self, text):
        self.text = text

    def resolve(self, data):
        return self.text

    def __repr__(self):
        return f"({self.text})"
        #return f"Literal({self.text!r})"


# --- variable object ---
class Stringlet(Token):
    def __init__(self, key, filters=None):
        self.key = key
        self.filters = filters or []

    def __repr__(self):
        #return f"Var({self.key!r}, {self.filters!r})"
        return f"{self.key}{self.filters!r}"

    def apply_filters(self, value):
        """Apply chained filters to the value (same logic as before)."""
        for filt in self.filters:
            if not filt:
                continue

            if ":" in filt:
                name, arg = filt.split(":", 1)
                if name == "strftime" and hasattr(value, "strftime"):
                    value = utc_strftime(value, arg)
                    continue

            if hasattr(value, "strftime") and filt.startswith("%"):
                value = utc_strftime(value, filt)
                continue

            if filt[0] in ".0123456789eEfFgG":
                try:
                    value = format(value, filt)
                    continue
                except Exception:
                    pass

            if filt == "upper" and isinstance(value, str):
                value = value.upper()
                continue
            if filt == "lower" and isinstance(value, str):
                value = value.lower()
                continue
            if filt == "abs":
                try:
                    value = abs(value)
                    continue
                except Exception:
                    pass
            if filt == "basename" and isinstance(value, str):
                value = os.path.basename(value)
                continue
        return value

    def resolve(self, data):
        value = data.get(self.key, "")
        return str(self.apply_filters(value))


# --- tokenizer ---
def parse_template(template: str):
    """Split template into tokens (Literals and Stringlets."""
    tokens = []
    pos = 0
    for m in re.finditer(r"\{([^{}]+)\}", template):
        # literal text before the match
        if m.start() > pos:
            tokens.append(Literal(template[pos:m.start()]))
        expr = m.group(1)
        parts = [p.strip() for p in expr.split("|")]
        key, filters = parts[0], parts[1:]
        tokens.append(Stringlet(key, filters))
        pos = m.end()
    # trailing literal
    if pos < len(template):
        tokens.append(Literal(template[pos:]))
    return tokens


# --- formatter ---
def render_template(template: str, data: dict):
    """Render parsed template using given data."""
    return tokens_tostring(parse_template(template), data)

def tokens_tostring(tokens: list, data: dict):
    """Render parsed template using given data."""
    return "".join(t.resolve(data) for t in tokens)


    """
    parts = []
    for t in parse_template(template):
        if isinstance(t, Stringlet):
            parts.append(t.resolve(data))
        else:
            parts.append(t)
    return "".join(parts)
    """ 

def main():

    global logger

    parser = argparse.ArgumentParser(description="Demonstrate stringlet templating")

    parser.add_argument(
        "-t", "--template",
        default='Hello {name}, value is {value|.2f}, time is {time|%Y-%B-%d (%A)|lower} and we say {text|upper}!',
        # default="Hello {name}, value is {value|.2f}, time is {time|%Y-%m-%d %H:%M} and we say {text|upper}!",
        help="string containing variables")

    rack.log.add_parameters(parser)
    
    args = parser.parse_args()
    
    rack.log.handle_parameters(args)

    data = {
        "name": "Ada",
        "value": 3.14159,
        "time": dt.datetime(2025, 11, 3, 14, 25, tzinfo=dt.timezone.utc),
        "text": "hello world",
    }

    tokens = parse_template(args.template)
    
    logger.info(f"TEMPLATE: {args.template}")
    logger.info(f"TOKENS:  {tokens} ")
    rendered = render_template(args.template, data)
    logger.info(f"RENDERED: {rendered} ")


if __name__ == "__main__":
    main()