from enum import Enum
import re, os
#from datetime import datetime
# datetime.timezone.utc
import datetime as dt

#from pathlib import Path
import pathlib
#from rack.prog import logger
import rack.base
logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)

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

        # Revised (and -> or, and first priority)
        if hasattr(value, "strftime") or filt.startswith("%"):

            if "%s" in filt:
                # compute epoch seconds respecting tzinfo
                if value.tzinfo is None:
                    # assume naive datetime is UTC
                    epoch_seconds = int((value - dt.datetime(1970, 1, 1)).total_seconds())
                else:
                    epoch_seconds = int(value.timestamp())
                filt = filt.replace("%s", str(epoch_seconds))
            
            return value.strftime(filt) # , dt.timezone.utc)
        
        # --- named filter with argument (e.g. strftime:%Y-%m-%d) ---
        if ":" in filt:
            name, arg = filt.split(":", 1)
            name, arg = name.strip(), arg.strip()

            if name == "strftime" and hasattr(value, "strftime"):
                return value.strftime(arg)

            # You could add more name:arg filters here.
            return value  # ignore unknowns

        # --- datetime auto-detect ---
        #if hasattr(value, "strftime") and filt.startswith("%"):
        #    return value.strftime(filt)

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

def detect_filter(value:str) -> tuple:
    return value.split('|', 1)
    #(key, filters) = value.split("|",1)


if __name__ == "__main__":
    data = {
        "Mika": 1,
        "name": "Ada",
        "value": -3.1415926,
        "time": dt.datetime(2025, 11, 3, 14, 25),
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


class ParamFormatter:

    """Formatting key=value pairs"""
    KEY_FORMAT ='{key}'
    VALUE_FORMAT='{value}'
    VALUE_SEPARATOR=':'
    VALUE_ASSIGN='='
    PARAM_SEPARATOR=','
    PARAMS_FORMAT='{params}'

    def __init__(self, key_format ='{key}', value_format='{value}', value_separator=':',
                 value_assign='=', param_separator=',', params_format='{params}'):
        self.KEY_FORMAT   = key_format
        self.VALUE_FORMAT = value_format
        self.VALUE_SEPARATOR = value_separator
        self.VALUE_ASSIGN = value_assign
        self.PARAM_SEPARATOR = param_separator
        self.PARAMS_FORMAT = params_format

    def fmt_value(self, value:str)->str :
        if isinstance(value, (tuple,list)):
            # if type(value) in (tuple,list):
            value = [str(v) for v in value]
            value = self.VALUE_SEPARATOR.join(value)

        if isinstance(value, Enum):
            value=value.value

        return self.VALUE_FORMAT.format(value=value)

    def fmt_param(self, value, key:str=None)->str :
        """
        Notice the order of parameters.
        """
        value = self.fmt_value(value)
        if (key is None) or (isinstance(key, (int, float))):
            return value
        else:
            key = self.KEY_FORMAT.format(key=key)
            return f"{key}{self.VALUE_ASSIGN}{value}"

    def get_param_lst(self, arg_dict={}, key_list=None)->list:
        """ Return a list of strings of key-value entries. By default, 
            in this base class implementation, such entry 
            is "key=value", or only the value if the key is numeric. 
        """
        if key_list == None:
            # Return all
            return [self.fmt_param(v,k) for k,v in arg_dict.items()]
        else: # key_list can be empty.
            result = []
            key_map = dict(enumerate(arg_dict.keys())) # list(arg_dict.keys())))
            for k in key_list:
                if isinstance(k, (int, float)):
                    result.append(self.fmt_param(arg_dict[key_map[k]], None))
                else:
                    result.append(self.fmt_param(arg_dict[k],k))
            return result

    # fmt strings also for this?
    def fmt_params(self, arg_dict={}, key_list=None)->str :
        param_list = self.get_param_lst(arg_dict, key_list)
        params = self.PARAM_SEPARATOR.join(param_list)
        return self.PARAMS_FORMAT.format(params=params)

    def parse_args(self, expr:str, args:list, kw_args:dict):

        """Parse a string of arguments and set them as command parameters. The string should be in the format "key=value,key2=value2,...". 
        """

        if expr:
            # arg_entries = args.split(self.PRIMARY_SEP)
            POSITIONAL = True
            for entry in expr.split(self.PARAM_SEPARATOR):
                if self.VALUE_ASSIGN in entry:
                    key, value = entry.split(self.VALUE_ASSIGN, 1)
                    kw_args[key.strip()] = value.strip()
                    # TODO: strip quotes if any, for example, value.strip('"').strip("'") or similar
                    POSITIONAL = False
                else:
                    args.append(entry.strip())
                    if not POSITIONAL:
                        logger.warning(f"Positional argument '{entry}' added after key-value arguments.")


#logger.warning(f"Argument '{entry}' does not contain '{self.fmt.VALUE_ASSIGN}', skipping.")



class Formatter(ParamFormatter):

    NAME_FORMAT='{name}'
    # KEY_FORMAT ='{key}'
    # VALUE_FORMAT='{value}'
    # VALUE_SEPARATOR=':'
    # VALUE_ASSIGN='='
    # PARAM_SEPARATOR=','
    # PARAMS_FORMAT='{params}'
    CMD_ASSIGN=' '
    CMD_SEPARATOR=' '

    def __init__(self, name_format='{name}', key_format ='{key}', value_format='{value}', value_separator=':',
        value_assign='=', param_separator=',', params_format='{params}', cmd_assign=' ', cmd_separator=' '):

        super().__init__(key_format=key_format, value_format=value_format, value_separator=value_separator,
            value_assign=value_assign, param_separator=param_separator, params_format=params_format)

        self.NAME_FORMAT  = name_format
        # self.KEY_FORMAT   = key_format
        # self.VALUE_FORMAT = value_format
        # self.VALUE_SEPARATOR = value_separator
        # self.VALUE_ASSIGN = value_assign
        # self.PARAM_SEPARATOR = param_separator
        # self.PARAMS_FORMAT   = params_format
        self.CMD_ASSIGN      = cmd_assign
        self.CMD_SEPARATOR   = cmd_separator

    def fmt_name(self, name:str)->str :
        return self.NAME_FORMAT.format(name=name)
