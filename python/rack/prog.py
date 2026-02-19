import argparse
import inspect
import pathlib
from typing import Any, List, Dict, Union
import numbers

#from collections import OrderedDict
import rack.base
from rack.formatter import Formatter, ParamFormatter


logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)

import inspect
from functools import wraps

def copy_signature_from(source):
    """ UNUSED?
    Copy signature from another callable (except 'self' in methods)."""

    def decorator(target):
        sig = inspect.signature(source)
        params = list(sig.parameters.values())
        
        # Remove 'self' if it exists in the source signature
        if params and params[0].name == "self":
            params = params[1:]
        
        # Add *args at beginning if target already has it
        target_sig = inspect.signature(target)
        new_params = list(target_sig.parameters.values())
        
        # Keep the original *args, but append the copied kwargs
        new_params += params
        #new_params.extend(params)
        
        new_sig = inspect.Signature(parameters=new_params)
        # new_sig = inspect.Signature(new_params)
        target.__signature__ = new_sig
        
        return target
    return decorator


class Command:
    """A command, for example, a command line argument. It has a name and optionally, a list of parameters (arguments)."""

    PRIMARY_SEP   = ","
    SECONDARY_SEP = ":"
    
    # def __init__(self, name, args=None, specific_args={}):
    def __init__(self, name, args=None, specific_args=None): #None
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
            # split to list
            args = args.strip().split(self.PRIMARY_SEP) # note: default value in PRIMARY_SEP
            
        if isinstance(args, (tuple, list)):
            self.ordered_args_count = len(args)
            args = dict(enumerate(args)) #{k,v for enumerate(args)}

        # At this point, args is a dict
        self.args.update(args)

        if specific_args:
            if isinstance(specific_args, dict):
                self.args.update(specific_args)
                # Explicit argument keys given by the last set_args()
                self.expl_keys = list(specific_args.keys())  # dict of explicitly given args
            elif isinstance(specific_args, (set, tuple, list)):
                self.expl_keys = list(specific_args)
        else:
            self.expl_keys = []
        


    def set_args(self, *args, **kwargs):
        """ Set ordered arguments and keyword arguments.
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


    def get_args(self, fmt:Formatter=None) -> str:
        
        if self.fmt:
            if fmt:
                logger.debug(f"{self.name}: overriding with own format: {self.fmt}")
            fmt = self.fmt
        else:
            if not fmt:
                # logger.debug(f"{self.name}: using default format: {fmt}")
                fmt = Formatter()

 
        if self.args:
            key_list = list(range(0,self.ordered_args_count))
            key_list.extend(self.expl_keys)
            if len(key_list) == 0:
                # Show at least one arg.
                # key_list = [0]
                key_list = list(self.args.keys())
            return fmt.fmt_params(self.args, key_list)
        else:
            return None
        

    def to_string(self, fmt:Formatter=Formatter()) -> str:

        name = self.get_name(fmt)
        args = self.get_args(fmt)
        
        if name and (args is None):
            return name
        elif name:
            return f"{name}{fmt.CMD_ASSIGN}{args}"
        else:
            return args


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


class Register:
    """A command register, for example, for a specific program or a specific group of commands."""

    def __init__(self, cmdSequence=None):
        """ Create instance of a command Register.

        If a commandSequence is give, every call adds a command to it. 
        """
        self.cmdSequence = cmdSequence

    def _is_default(func, name, value):
        sig = inspect.signature(func)
        param = sig.parameters[name]
        return ( param.default is not inspect._empty and value == param.default )


    def make_cmd(self, local_vars:dict, separator:str="") -> Command:
        """ Creates a command. The name will be the caller function
        
        Each explicitly given argument will be stored if its value differs from the default one.
        """
        local_vars.pop('self')
       
        # typelist = [type(v) for v in local_vars.values()]
        # Detect caller function. Then, myFunction(...) implies cmd name "myFunction"
        caller_name = inspect.stack()[1].function
        func = getattr(self, caller_name)
        
        # Function signature
        sig = inspect.signature(func)
        # Detect if default values are numeric, for possible "covering" with a single sequence argument.
        numerics = [isinstance(v.default, numbers.Number) for v in sig.parameters.values()]
        logger.debug(f"{caller_name}: args numeric: {numerics}")


        if caller_name == "xxcBBox":
            for i in inspect.stack()[1]:
                logger.error(i)
            # for i in dir(func):
            #    logger.warning(i) #, getattr(func, i))   
            # TODO: check if 1st arg could "cover" all the args.
            # For example 1) len is 
            # 1) len(arg0) == len(params)
            # 2) type(arg0) in [list,tuple,set]
            # 4) types of arg0[*] are all the same
            # 5) types of N first params are all the same
            # 3) type(params) = type(arg[0]..) match
   
        args = {}
        explicit_args = {}

        
        # Extract only explicit arguments
        FIRST=True
        cmd = None
        for k,v in local_vars.items():
            #if k != "self":
            args[k]=v
            if not Register._is_default(func, k, v):
                #logger.warning(f"{caller_name}: explicit {k}={v}")
                if FIRST and (type(v) in (tuple,list,set)):
                    # Expanding sequence arg to all args, if sizes match and all are numeric 
                    arg_numerics = [isinstance(i, numbers.Number) for i in v]
                    if (len(numerics) == len(arg_numerics)):
                        similar_sig = {(t1==t2) for (t1,t2) in zip(numerics, arg_numerics)}
                        if (True in similar_sig) and not (False in similar_sig): 
                            logger.info(f"Expanding sequence to args : {caller_name} {v}")
                            #cmd = Command(caller_name, args=v)
                            args=v
                            break
                explicit_args[k]=v
            FIRST=False
        
        logger.warning(f"{caller_name}: args={args}, explicit_args={explicit_args}")
        if cmd == None:
            cmd = Command(caller_name, args=args, specific_args=explicit_args)
        
        if (separator):
            cmd.set_separators(separator)

        if self.cmdSequence:
            self.cmdSequence.add(cmd)

        return cmd
    

    def import_commands(self, commands:Dict[str,Any], separator:str=""):
        """ Creates multiple commands from a dictionary of command names and arguments.
        """

        for k,v in commands.items():

            func = getattr(self, k)   # resolves Registry.terminal, Registry.output, ...
            # logger.warning(func.__name__)
            # logger.warning(dir(func))
            
            cmd = None
            args_list = []
            kwargs_dict = {}
            
            if isinstance(v, (list, tuple)):
                logger.warning("type(v) = %s", type(v))
                l = len(v)
                if l > 0:
                    last_elem = v[l-1]
                    if isinstance(last_elem, dict):
                        args_list = v[0:l-1]
                        kwargs_dict = last_elem
                    else:
                        args_list = v
                v = None
            elif isinstance(v, set):
                args_list = list(v)
                v = None
            elif isinstance(v, dict):
                kwargs_dict = v
                v = None
            # else: v is a single value (e.g., str, int, bool, ...)

            if not (v is None):
                logger.warning(f"calling {func.__name__} with v={v} args={args_list}, kwargs={kwargs_dict}")
                cmd = func(v, *args_list, **kwargs_dict)
            else:
                logger.warning("calling %s with args=%s, kwargs=%s", func.__name__, args_list, kwargs_dict)
                cmd = func(*args_list, **kwargs_dict)
            #logger.warning(cmd.to_string(gp.ConfSequence.fmt))

            #if self.cmdSequence:
            #    self.cmdSequence.add(cmd)
    
    @classmethod
    def publish_func(self, func: callable, parser: argparse.ArgumentParser = None, key: str = None, name_mapper = None):
        """ Export function arguments to an argparse parser. The function name will be the command name,
            and the arguments will be the command arguments.
        
        :param self: Description
        :param func: Description
        :param key: Description
        :param parser: Description
        :type parser: argparse.ArgumentParser
        :param name_mapper: Description
        """
        
        logger.info(f"Exporting args for {func.__name__}")
        
        if isinstance(func, str):
            caller_name = func
            func = getattr(self, caller_name)

        if not inspect.isfunction(func):
            raise TypeError(f"Expected a function, got {type(func)}")

        # Function signature
        sig = inspect.signature(func)
        params = list(sig.parameters.values())
        # Remove 'self' if it exists in the source signature
        if params and params[0].name == "self":
            params = params[1:]

        # Function name 
        if parser:
            if key:
                caller_name = key
            else:
                caller_name = func.__name__
                # This is not done:
                # if name_mapper:
                #    caller_name = name_mapper(caller_name)
            keys = ",".join((p.name for p in sig.parameters.values()))
            parser.add_argument(f"--{caller_name}", metavar=f"<{keys}>", help=f"execute {func.__name__} with given arguments" )


        def camel_to_upper_underscore(name: str) -> str:
            # Insert underscore between lowercase-to-uppercase transitions
            s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
            # Insert underscore between lower/number-to-uppercase transitions
            s2 = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1)
            return s2.upper()
        
        if name_mapper == True:
            logger.warning(f"No name mapper given for {func.__name__}, using default (camel to upper-underscore)")
            name_mapper = camel_to_upper_underscore

        if not name_mapper:
            return

        for v in params:
            name = v.name
            logger.debug(f"{func.__name__}: arg {v.name} default: {v.default}")
            if name_mapper:
                logger.debug(f"mapping {v.name} to {name_mapper(v.name)}")
                name = name_mapper(v.name)
                #v.name = name_mapper(v.name)
            
            if parser:
                args = {"dest":v.name, "type":str, #"nargs":argparse.REMAINDER,
                        "help": f"equals --{caller_name} {v.name}=..."}

                if v.default is not inspect._empty:
                    args["default"] = v.default
                    # args["default"] = str(v.default)
                    default_str = ""
                    if isinstance(v.default, str) and v.default.strip() != "":
                        default_str = f" default: '{v.default}'"
                    elif v.default is not None:
                        default_str = f" default: {v.default}"

                    if default_str:
                        args["help"] += default_str
                        args["metavar"] = f"<{type(v.default).__name__}>"
                        #args["default"] = v.default

                    #f" default: {v.default}" if v.default is not None else ""
                    parser.add_argument(f"--{name}", **args)
                else:
                    parser.add_argument(f"--{name}", required=True, **args)
            #else:
            #   logger.warning(f"No parser given for {func.__name__}, skipping argument registration for {name}")
        
    

    # @classmethod
    def handle_published_cmd_args(self, args: argparse.Namespace, cmd_func: callable):
        """
        Docstring for handle_exploded_command
        
        :param args: Namespace of arguments, typically from argparse. The keys should match the parameter names of cmd_func.
        :param cmd_func: The function to call with the arguments. Typically, this is a method of the Rack class, e.g., Rack.select or Rack.pPseudoRhi.
        :param exec: If True, the command will be executed (i.e., cmd_func will be called with the arguments).
        :type exec: bool
        :return: If exec is True, returns the result of cmd_func. Otherwise, returns a dict of arguments that would be passed to cmd_func.
        """
    
        sig = inspect.signature(cmd_func)
        params = list(sig.parameters.values())

        var_args = vars(args)
        # logger.warning(f"Handling exploded command {cmd_func.__name__}")
        # logger.warning(f"Handling exploded command {cmd_func.__name__} with args: {var_args}") # too verbose...

        # First, create the command with default args, then override with explicit args, and finally parse free args if any.
        cmd = cmd_func(self)
        # logger.warning(f"Building command {cmd_func.__name__}: {cmd.to_string()}")

        pos_args = []
        kw_args = {}
        if not cmd.fmt:
            cmd.set_separators()        
        cmd.fmt.parse_args(var_args.get(cmd_func.__name__, ""), pos_args, kw_args)
        #cmd.set_args(*pos_args, **kw_args)
        #
        logger.debug(f"Initial (private) args {cmd_func.__name__}: {pos_args} {kw_args} ")

        # explicit args. Notice that they are given as --cmd_func arg=value, so they are in var_args with key cmd_func and value "arg=value,..."
        for v in params:
            #name = v.name
            if v.name == "self":
                continue

            if v.name in var_args and var_args[v.name] is not None:
                value = var_args[v.name]
                # default is studied "locally"
                if value != v.default:
                    logger.debug(f"Argument {v.name} has value {value} different from default {v.default}, including in command")
                    kw_args[v.name] = str(value).strip().replace(cmd.fmt.PARAM_SEPARATOR, cmd.fmt.VALUE_SEPARATOR) 
                    #.strip('"').strip("'") # TODO: strip quotes if any, for example, value.strip('"').strip("'") or similar
                    #logger.info(f"{cmd_func.__name__}: adding argument {v.name}={value}")
            else:
                logger.warning(f"Argument {v.name} not found in args or is None, skipping for {cmd_func.__name__}")
        
        cmd.set_args(*pos_args, **kw_args)
        logger.debug(f"Status of {cmd_func.__name__}: {cmd.to_string()}")

        # if exec:
        # logger.warning(f"Executing '{cmd_func.__name__}' with explicit args: {kw_args}")
        # cmd = cmd_func(self, free_args, **dict_args)

        return cmd


class CommandSequence:
    """A sequence of commands, for example, a program or a script."""

    CmdClass = Command

    fmt = Formatter()

    # For arguments. Helps in toggling within-argument quotes
    QUOTE = "'"

    """Base class for a sequence of commands - 'programs'."""
    def __init__(self, cmdClass=Command, programName:str="", quote="'"):
        self.CmdClass = cmdClass
        self.commands: List[Command] = []
        self.programName = programName
        self.QUOTE = quote

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

        Typically, this format is suited to command line use.
        
        fmt.CMD_SEPARATOR can also be '\\\n\t' for example.
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




class RackFormatter(Formatter):
    """A formatter for Rack commands (command line arguments). Overrides some of the default formatting rules."""
    
    def fmt_name(self, name:str) -> str :
        l = len(name)
        if l==0: # Default command (--inputFile ...) works like this - needs no key.
            return ""
        elif l==1:
            return f"-{name}"
        else:
            return f"--{name}"
        #return self.NAME_FORMAT.format(name=name)

    #def fmt_params(self, arg_dict={}, key_list=None)->str :
    #    param_list = self.get_param_lst(arg_dict, key_list)
    #    params = self.PARAM_SEPARATOR.join(param_list)
    #    return self.PARAMS_FORMAT.format(params=params)

    def get_param_lst(self, args={}, key_list=None)->list:
        """ Return a list of strings of key-value entries. By default, 
            in this base class implementation, such entry 
            is "key=value", or only the value if the key is numeric. 
        """
        if key_list == None:
            # Return all
            return [self.fmt_param(v,k) for k,v in args.items()]
        else: 
            # Note: key_list can be empty.
            result = []
            key_map = dict(enumerate(args.keys())) # list(arg_dict.keys())))
            
            compact = True
            
            for (key, key_orig) in zip(key_list, args.keys()):
                if isinstance(key, (int, float)):
                    key = key_map[key]
                    #result.append(self.fmt_param(args[key_map[k]], None))
                value = args[key]
                if (key == key_orig) and compact:
                    key = None
                else:
                    compact = False
                result.append(self.fmt_param(value,key))
            
            """
            if len(key_map) == 1:
                key0 = key_map[0]
            else:
                key0 = None

            for k in key_list:
                if isinstance(k, (int, float)):
                    result.append(self.fmt_param(args[key_map[k]], None))
                elif k==key0:
                    result.append(self.fmt_value(args[k]))  
                else:
                    result.append(self.fmt_param(args[k],k))
                key0 = None                        
            """

            return result


def main():

    
    class MyRegister(Register):

        def sample(
            self,
            iStep: int = 10,
            jStep: int = 0,
            i: tuple = (-1, 1),
            j: tuple = (-1, 1),
            commentChar: str = "#",
            skipVoid: bool = False,
            handleVoid = None,
        ):
            return self.make_cmd(locals())


    reg = MyRegister()
    cmd = reg.sample(i=(-2, 5), iStep=1)
    cmd.PRIMARY_SEP='|'
    cmd.SECONDARY_SEP='/'
    print(cmd)   # assuming MyCommand.__str__ formats tuples as "a:b" and joins with commas
    
    prog = CommandSequence(Command)
    prog.add(cmd)

    cmd = Command("unary")
    prog.add(cmd)
    cmd = Command("anon", "value", {"key": "value"})
    prog.add(cmd)
    cmd = Command("anon", ["first", "second", "third"])
    prog.add(cmd)
    cmd = Command("anon", ["first", "second", "third"])
    cmd.set_args("a","b")
    prog.add(cmd)

    cmd = Command("normal", {"key": "value", "key2": "value2"} )
    cmd.set_args(key2="b2")
    prog.add(cmd)

    #print(cmd)

    #print(prog.to_string())
    #print(prog)
    print(prog.to_list())
    print(prog.to_list(fmt=Formatter(cmd_separator="\n")))
    #print(prog.to_token_list())

    def test(a:int=-1, b:str="?", *args, **kwargs):
        print (f"a={a}, b={b} | {args} | {kwargs}")

    test("Mika")
    test("Mika","mäki", "the third")
    test("Mika","mäki", key2="the value")
    test("Mika", key2="the value")
    test(b="the value")
    test(b="the value", key3="thirdo")

if __name__ == "__main__":
    main()
