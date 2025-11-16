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
class Var(Token):
    def __init__(self, key, filters=None):
        self.key = key
        self.filters = filters or []

    def __repr__(self):
        #return f"Var({self.key!r}, {self.filters!r})"
        return f"{self.key}{self.filters!r}"

    def apply_filters(self, value):

        if (type(value) in {list,set}):
            return ",".join(str(v) for v in value)
        #if (type(value) is set):
        #    return ",".join(value)


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



class Stringlet:
    
    tokens = []

    def __init__(self, template: str = ""):
        if template:
            self.parse_template(template)
        else:
            self.tokens = []
    
    def __repr__(self):
        return f"Stringlet({self.tokens!r})"
    
    def __str__(self):
        return "".join(str(t) for t in self.tokens)

    def string(self, data: dict=None) -> str:
        """Render parsed template using given data."""
        if not self.tokens:
            return ""
        elif data is None:
            return self.__str__()
        else:
            return "".join(t.resolve(data) for t in self.tokens)


    # --- tokenizer ---
    def parse_template(self, template: str):
        """Split template into tokens (Literals and Vars."""
        self.tokens = []
        pos = 0
        for m in re.finditer(r"\{([^{}]+)\}", template):
            # literal text before the match
            if m.start() > pos:
                self.tokens.append(Literal(template[pos:m.start()]))
            expr = m.group(1)
            parts = [p.strip() for p in expr.split("|")]
            key, filters = parts[0], parts[1:]
            self.tokens.append(Var(key, filters))
            pos = m.end()
        # trailing literal
        if pos < len(template):
            self.tokens.append(Literal(template[pos:]))
        return self.tokens

    def get_vars(self):
        """Extract variables (key,filters) from token list."""
        keys = [k for k in self.tokens if isinstance(k, Var)]
        return keys  

    def get_var_keys(self): #tokens: list):
        """Extract variable keys (without filters) from token list."""
        keys = [v.key for v in self.get_vars(self.tokens)]
        return keys  

    def get_index(self, key: str, offset: int = 0) -> int:
        """
        Resolve a column specifier (name or number) to a Var index.

        key:  "humidity" or "3"
        tokens: list of Literal / Var objects
        offset: optional offset to add to the result, if spec was a string index

        Returns integer index (1-based, like column numbering),
        or raises KeyError / ValueError if not found.
        """
        # First, collect Vars only
        vars_only = self.get_vars() #[t for t in tokens if isinstance(t, Var)]

        # Try numeric spec (1-based)
        r = range(0, len(vars_only))

        if key.isdigit():
            idx = int(key)
            if r.start < idx <= r.stop:
                return idx
            raise ValueError(f"Column index {idx} out of range ({r})")
            #raise ValueError(f"Column index {idx} out of range (1..{len(vars_only)})")  

        # Try name lookup
        for i, var in enumerate(vars_only, start=0):
            if var.key == key:
                return i+offset #

        raise KeyError(f"No column named '{key}' found")





    # --- formatter ---
def render_template(template: str, data: dict) -> str:
    """Render parsed template using given data."""
    stringlet = Stringlet()
    tokens = stringlet.parse_template(template) 
    return stringlet.tokens_tostring(data)
    #return tokens_tostring(parse_template(template), data)



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

    stringlet = Stringlet()
    tokens = stringlet.parse_template(args.template)
    
    logger.info(f"TEMPLATE: {args.template}")
    logger.info(f"TOKENS:  {tokens} ")
    vars = stringlet.get_vars(tokens)
    logger.info(f"VARS: {vars} ") 
    rendered = render_template(args.template, data)
    logger.info(f"RENDERED: {rendered} ")




if __name__ == "__main__":
    main()