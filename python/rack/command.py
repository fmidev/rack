import pathlib
import logging
from typing import Any, List

import rack.base
from rack.formatter import Formatter, ParamFormatter

#from rack.prog import logger
logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)
#logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
#logger = logging.getLogger(pathlib.Path(__file__).stem) 
#logger.setLevel(logging.INFO)

#from rack.prog import logger




class Command:
    """A command, for example, a command line argument. It has a name and optionally, a list of parameters (arguments)."""

    PRIMARY_SEP   = ","
    SECONDARY_SEP = ":"

    # def __init__(self, name, args=None, specific_args={}):
    def __init__(self, name, args=None, explicit_args=None): #None
        """This defines the command parameter list, including default values.

        Works in two modes:

        Mode 1: args is a list of values and explicit_args is a dict
            [value1, value2, ...], {key1: value1, key2: value2, ...}

        Mode 2: args is a dict and explicit_args is a list of keys
            {key1: value1, key2: value2, ...}, [key1, key2, ...]

        Parameters
        ----------

        name - command name
        args - either a dict or a list
        specific_args - either a dict or a list. If dict, it contains the explicitly given arguments. If list, it contains the keys of the explicitly given arguments.

        """

        self.name = name.strip(" -\t\n")
        #if self.name == 'verbose':
            # logger.debug("enchanced logging")
            # logger.setLevel("DEBUG")
            # logger.warning(f"ARGS: {args}, EXPLICIT: {explicit_args}")
            # logger.warning(f"command: args={args}")

        self.args = {}

        # rename "unnamed_args" ?
        self.ordered_args_count = 0

        # If a command is implict, its name (keyword) will not be displayed on command line. Typical for input file args.
        self.implicit = False

        # ParamFormatter (if command specific)
        self.fmt = None

        if self.name == "":
            # logger.warning("Empty command name, setting implicit to True")
            self.name = "#"
            self.implicit = True
            return

        #if type(args) == str:
        if not args:
            args = {}

        if isinstance(args, str):
            # split argument to list
            args = args.strip().split(self.PRIMARY_SEP) # note: default value in PRIMARY_SEP

        if isinstance(args, (tuple, list)):
            self.ordered_args_count = len(args)
            args = dict(enumerate(args)) #{k,v for enumerate(args)}

        # At this point, args is a dict
        self.args.update(args)

        if explicit_args:
            if isinstance(explicit_args, dict):
                self.args.update(explicit_args)
                # Explicit argument keys given by the last set_args()
                self.expl_keys = list(explicit_args.keys())  # dict of explicitly given args
            elif isinstance(explicit_args, (set, tuple, list)):
                self.expl_keys = list(explicit_args)
            else:
                raise KeyError(f"could not parse explicit args {explicit_args} (type={type(explicit_args)})")
        else:
            self.expl_keys = []



    def set_args(self, *args, **kwargs):
        """ Assign values (ordered arguments and keyword arguments).

            The command parameters - the so called signature of the command - 
            must be defined first.
        """
        #logger.warning(f"  args: {args}")
        #logger.warning(f"kwargs: {kwargs}")

        keys = self.args.keys()

        #if self.name == 'verbose':
        #    logger.warning("enchanced logging")
        #    logger.setLevel("DEBUG")

        # 1. assign ordered args
        assigned_keys = []
        if args:
            if len(args) > len(self.args):
                keys = list(keys)
                raise KeyError(f"Too many arguments ({args}) for {keys}")
            for k,v in zip(keys, args):
                self.args[k] = v
                assigned_keys.append(k)
            self.ordered_args_count = len(args)

        # 2. assign keyword args
        self.expl_keys = []
        for (k,v) in kwargs.items():
            if k in keys:
                if k in assigned_keys:
                    logger.warning(f"Overriding ordered arg ({self.args[k]}) with keyword arg ({k}={v})")
                else:
                    self.expl_keys.append(k)
                self.args[k] = v
                 # This is not needed, because we already have specific_args in __init__ and it is used to determine expl_keys. So, we can just use specific_args to determine expl_keys, and not update expl_keys here.

            else:
                keys = list(keys)
                raise ValueError(f"Unknown argument keyword {k}, use: {keys})")



    # def set_separators(self, primary:str=",", secondary:str=":"):
    def set_separators(self, primary:str=",", secondary:str=None):
        """Separators: primary for parameters and secondary for parameters of the parameters"""
        self.PRIMARY_SEP   = primary
        if secondary is None:
            for c in (',', ':'):
                if primary != c:
                    secondary = c
                    break
        self.SECONDARY_SEP = secondary
        if (primary == secondary):
            logger.error(f"setSeparators illegal equality primary:{primary} == secondary:{secondary}")
        self.fmt = ParamFormatter(value_separator=secondary, param_separator=primary)
        #logger.warning(f"assigning separators in a formatter for {self.name}")

    def set_implicit(self, value:bool = True):
        """If a command is implict, its name (keyword) will not be displayed on command line. 

        Typical for input file args.
        """
        self.implicit = value

    def __str__(self):
        return " ".join(self.to_tuple()) #"'"


    def get_name(self, fmt:Formatter=Formatter()) -> str:
        if self.name and not self.implicit:
            return fmt.fmt_name(self.name)
        else:
            return None

    def get_key_list(self, count=-1) -> list:
        """
            If flexible>0, retrieve also N=(flexible) non-explicit keys,

        """
        if self.args:
            key_list = list(range(0,self.ordered_args_count))
            key_list.extend(self.expl_keys)
            if (len(key_list)==0): # and (count!=0):
                # Show at least one arg.
                # key_list = [0]
                key_list = list(self.args.keys())
                if (count > 0):
                    key_list = key_list[0:count]
            return key_list
        else:
            return None

    def get_args(self, fmt:Formatter=None) -> str:

        if self.fmt:
            if fmt:
                logger.debug(f"{self.name}: overriding with own format: {self.fmt}")
            fmt = self.fmt
        else:
            if not fmt:
                # logger.debug(f"{self.name}: using default format: {fmt}")
                fmt = Formatter()

        # Ensure at least 1 explicit argument key (i.e. the first one)
        key_list = self.get_key_list() #count=1)
        if key_list:
            #key_list = list(range(0,self.ordered_args_count))
            #key_list.extend(self.expl_keys)
            #if len(key_list) == 0:
                # Show at least one arg.
                # key_list = [0]
            #    key_list = list(self.args.keys())
            #    key_list = [key_list[0]]
            return fmt.fmt_params(self.args, key_list)
        else:
            return None


    def to_string(self, fmt:Formatter=Formatter()) -> str:

        name   = self.get_name(fmt)
        argStr = self.get_args(fmt)

        if name and (argStr is None):
            return name
        elif name:
            return f"{name}{fmt.CMD_ASSIGN}{argStr}"
        else:
            return argStr


    def to_tuple(self, fmt:Formatter=Formatter()) -> tuple:

        name = self.get_name(fmt)
        args = self.get_args(fmt)

        if name and args:
            return (name, args)
        elif name:
            return (name,)
        elif args:
            return (args,)
        else:
            return tuple()


    # This and next related?
    def fmt(self, val: Any, key: str = "") -> str:
        """Format a single argument or option for output.
        Default: just convert to string. Override in subclasses."""
        return str(val)

    """
    def _encode_value(self,v):
        # Format tuple as 'a:b', leave scalars untouched.
        #if isinstance(v, tuple) and len(v) == 2:
        if isinstance(v, tuple):
            return f"{v[0]}{self.SECONDARY_SEP}{v[1]}"  #extend for >2
        return v
    """


class CommandSequence:
    """A sequence of commands, for example, a program or a script."""

    CmdClass = Command

    #fmt = Formatter()

    # For arguments. Helps in toggling within-argument quotes
    QUOTE = "'"

    """Base class for a sequence of commands - 'programs'."""
    def __init__(self, cmdClass=Command, programName:str="", quote="'"):
        self.CmdClass = cmdClass
        self.commands: List[Command] = []
        self.programName = programName
        self.QUOTE = quote
        self.fmt = None

    def get_secondary_quote(self) -> str:
        if self.QUOTE == '"':
            return "'"
        else:
            return '"'

    def add(self, cmd: Command, cmd_args:dict={}):
        t = type(cmd)
        #if t == self.CmdClass: Great!
        if isinstance(cmd, Command):
            self.commands.append(cmd)
            if cmd_args:
                logger.warning(f"Parameter override for {cmd.name}: {cmd_args} (vs. {cmd.args})")
        elif t in {str,list}:
            logger.warning(f"adding free command: {cmd} args={cmd_args}")
            # check if without --, so explicit (inputFile?)
            # Notice: this _defines_ a command, so further param keys will raise warnings/errors.
            # TODO: handling coud take place in Command(...)
            self.commands.append(Command(cmd, cmd_args))
            """
            if type(cmd_args) == str:
                cmd_args = cmd_args.strip().split(',')
            if type(cmd_args) == list:
                cmd_args = dict(enumerate(cmd_args))
            self.commands.append(Command(str(cmd).strip(" \t-"), explicit_args=cmd_args))
            """
        else:
            raise TypeError(f"Unsupported arg type for cmd='{cmd}':" + str(type(cmd)))
            #raise TypeError(f"Unsupported arg type for cmd='{cmd}'..")

    def clear(self):
        self.commands = []

    #def to_list(self, fmt:Formatter=Formatter()) -> list:
    def to_list(self, fmt:Formatter=None) -> list:
        """Produces a list suited to be joined with newline char, for example. """

        if not fmt:
            fmt = self.fmt

        if self.programName:
            result = [self.programName]
        else:
            result = []

        for cmd in self.commands:
            result.append(cmd.to_string(fmt)) # quote 

        return result


    def to_token_list(self, fmt:Formatter=None) -> list:

        if not fmt:
            fmt = self.fmt

        if self.programName:
            result = [self.programName]
        else:
            result = []

        for cmd in self.commands:
            result.extend(cmd.to_tuple(fmt)) # quote 
        return result

        #return [cmd.to_string() for cmd in self.commands]


    def to_string(self, fmt:Formatter=None) -> str:
        """ Compose a single string of commands.

        This format is suited to command line use.
        String produced by this command is also a valid parameter for
        Python shell invocation command os.system(...)

        For examples in documentation and tutorials, command lines
        are more readable if they on separate lines. Then, consider
        fmt.CMD_SEPARATOR='\\\n\t' for example.
        """
        #prog = [str(k) for k in ]
        if not fmt:
            fmt = self.fmt

        return fmt.CMD_SEPARATOR.join(self.to_list(fmt))



        """Produces a list compatible with subprocess calls

            Notice that the default quote here is empty string, "".
            Substituting None implies default quotes, CommandSequence.QUOTE.
        """

    def to_script(self, filename: str):
        with open(filename, "w", encoding="utf-8") as f:
            # TODO Formatter
            f.write(self.to_string() + "\n")

    #def to_debug(self, fmt:Formatter=Formatter()) -> list:
    def to_debug(self) -> list:
        if self.programName:
            result = [self.programName]
        else:
            result = []

        for cmd in self.commands:
            # result.append((cmd.name, cmd.args, cmd.expl_keys))
            result.append(f"{cmd.name}, {cmd.args}, {cmd.expl_keys}")
            #result.append(cmd.to_string(fmt)) # quote 

        return "\n".join(result)

    def to_python(self, prefix='') -> list:
        """ Create Python API code.
            Returning a list instead of a string allows later smart formatting. 
        """
        result = []
        fmt = ParamFormatter()
        for cmd in self.commands:
            # result.append((cmd.name, cmd.args, cmd.expl_keys))
            #result.append(f"{cmd.name}, {cmd.args}, {cmd.expl_keys}")
            #result.append(cmd.to_string(fmt)) # quote 
            #for k in cmd.expl_keys:
            #    s += k+'='+cmd.args[k]
            print(cmd.get_args(fmt))
            args = []
            for k in cmd.expl_keys:
                v = cmd.args[k]
                if isinstance(v, str):
                    args.append(f"{k}='{v}'")
                else:
                    args.append(f"{k}={v}")
            #args = [f'{k}={cmd.args[k]}' for k in cmd.expl_keys]  
            if cmd.get_name():
                result.append(prefix+cmd.get_name()+'(' + ",".join(args) +')')
            else:
                result.append(prefix+cmd.name+'(' + ",".join(args) +')')

        return "\n".join(result)