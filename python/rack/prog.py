import inspect
import pathlib
from typing import Any, List, Dict, Union

#from collections import OrderedDict
import rack.base


logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)

class Formatter:

    NAME_FORMAT='{name}'
    KEY_FORMAT ='{key}'
    VALUE_FORMAT='{value}'
    VALUE_SEPARATOR=':'
    VALUE_ASSIGN='='
    PARAM_SEPARATOR=','
    PARAMS_FORMAT='{params}'
    CMD_ASSIGN=' ' 
    CMD_SEPARATOR=' '

    def __init__(self, name_format='{name}', key_format ='{key}', value_format='{value}', value_separator=':', 
                 value_assign='=', param_separator=',', params_format='{params}', cmd_assign=' ', cmd_separator=' '):
        self.NAME_FORMAT  = name_format
        self.KEY_FORMAT   = key_format
        self.VALUE_FORMAT = value_format
        self.VALUE_SEPARATOR = value_separator
        self.VALUE_ASSIGN = value_assign
        self.PARAM_SEPARATOR = param_separator
        self.PARAMS_FORMAT   = params_format
        self.CMD_ASSIGN      = cmd_assign
        self.CMD_SEPARATOR   = cmd_separator

    def fmt_name(self, name:str)->str :
        return self.NAME_FORMAT.format(name=name)

    def fmt_value(self, value:str)->str :
        if isinstance(value, (tuple,list)):
        #if type(value) in (tuple,list):
            value = [str(v) for v in value]
            value = self.VALUE_SEPARATOR.join(value)

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



class Command:

    PRIMARY_SEP   = ","
    SECONDARY_SEP = ":"


    def __init__(self, name, args=None, explicit_args={}):
        """This defines the command parameter list, including default values.

        Works in two modes:
        
        Mode 1: args is a list of values and explicit_args is a dict
        Mode 2: args is a dict and explicit_args is a list of keys
        
        Parameters
        ----------

        name - command name
        args - either a dict or a list

        """
        
        self.name = name.strip(" -\t\n")
        if self.name == 'verbose':
            logger.debug("enchanced logging")
            logger.setLevel("DEBUG")
            #logger.warning(f"ARGS: {args}, EXPLICIT: {explicit_args}")
            #logger.warning(f"command: args={args}")
        
        self.ordered_args_count = 0

        if type(args) == str:
            # split to list
            args = args.strip().split(self.PRIMARY_SEP) # note: default value in PRIMARY_SEP
            
        #if type(args) == list):
        if isinstance(args, (tuple, list)):
            self.ordered_args_count = len(args)
            args = dict(enumerate(args)) #{k,v for enumerate(args)}

        # At this point, args should be a dict

        if args:
            self.args = args.copy()
        else:
            self.args = {}

        if (explicit_args):
            if isinstance(explicit_args, dict):
                self.args.update(explicit_args)
                # Explicit argument keys given by the last set_args()
                self.expl_keys = explicit_args.keys()  # dict of explicitly given args
            elif isinstance(explicit_args, (set, tuple, list)):
                self.expl_keys = list(explicit_args)
        else:
            self.expl_keys = []
        # If a command is implict, its name (keyword) will not be displayed on command line. Typical for input file args.
        self.implicit = False

    def set_args(self, *args, **kwargs):
        """ Set ordered arguments and keyword arguments.
        """
        #logger.warning(f"  args: {args}")
        #logger.warning(f"kwargs: {kwargs}")

        keys = self.args.keys()

        if self.name == 'verbose':
            logger.warning("enchances logging")
            logger.setLevel("DEBUG")

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
                    logger.warning(f"overriding ordered arg ({self.args[k]}) with keyword arg ({k}={v})")
                self.args[k] = v
                self.expl_keys.append(k)
                
            else:
                keys = list(keys)
                raise ValueError(f"Unknown argument keyword {k}, use: {keys})")
        

        
 
    def set_separators(self, primary:str=",", secondary:str=":"):
        self.PRIMARY_SEP   = primary
        if secondary is None:
            for c in (',', ':'):
                if primary != c:
                    secondary = c
                    break
        self.SECONDARY_SEP = secondary
        if (primary == secondary):
            logger.error(f"setSeparators illegal equality primary:{primary} == secondary:{secondary}")

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


    def get_args(self, fmt:Formatter=Formatter()) -> str:
        if self.args:
            key_list = list(range(0,self.ordered_args_count))
            key_list.extend(self.expl_keys)
            if len(key_list) == 0:
                key_list = [0]
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
    

    def to_tuple_OLD(self, quote=None, compact=True) -> tuple: # , prefixed=True
        """Returns a singleton or a double token
        
        This is the 'official' rendering of a command - other formatting uses the tuple as input.

        Within command arguments, only 'explicit' ones - having non-default values - will be returned.
        However, if all the argument values are defaults, the first one will be shown. 
        """    
        
        #if prefixed:
        name = self.get_prefixed_name()

        if name and not self.implicit:
            result = [name]
        else:
            result = []

        keys = list(self.args.keys())
        args = []

        if self.implicit and (len(keys) == 1):
            return ( str(self._encode_value(self.args[keys[0]])), )

        for i in range(0,self.ordered_args_count):
            args.append(str(self._encode_value(self.args[keys[i]])))

        index = 0
        for k in self.expl_keys:
            if type(k) == int:
                args.append(self._encode_value(self.args[k]))
            else:
                if compact and (index == keys.index(k)):
                    args.append(str(self._encode_value(self.args[k])))
                else:
                    args.append(f"{k}={self._encode_value(self.args[k])}")
                    compact = False # cannot continue impicit keys for ordered
            index += 1
        
        if quote is None:
            quote = CommandSequence.QUOTE

        if self.args:
            # Notice: args may be empty, then append an empty string.
            # If all the arguments have default values (args is empty), display still an empty arg.
            if args:
                args = self.PRIMARY_SEP.join(args)
            else:
                # "modal argument": use first parameter value.
                args = self.args[keys[0]] 
            # result.append(quote + self.PRIMARY_SEP.join(args) + quote)
            args = f"{quote}{args}{quote}"
            #if self.implicit:
            #    return (args,)
            result.append(args)

        return tuple(result)

    # This and next related?
    def fmt(self, val: Any, key: str = "") -> str:
        """Format a single argument or option for output.
        Default: just convert to string. Override in subclasses."""
        return str(val)

    def _encode_value(self,v):
        """Format tuple as 'a:b', leave scalars untouched."""
        #if isinstance(v, tuple) and len(v) == 2:
        if isinstance(v, tuple):
            return f"{v[0]}{self.SECONDARY_SEP}{v[1]}"  #extend for >2
        return v




class Register:

    def __init__(self, cmdSequence=None):
        """ Create instance of a command Register.

        If a commandSequence is give, every call adds a command to it. 
        """
        self.cmdSequence = cmdSequence

    def _is_default(func, name, value):
        sig = inspect.signature(func)
        param = sig.parameters[name]
        return ( param.default is not inspect._empty and value == param.default )


    def make_cmd(self, local_vars, separator:str=""):
        """ Creates a command. The name will be the caller function
        
        Each explicitly given argument will be stored if its value differs from the default one.
        """
        # Detect caller function. Then, myFunction(...) implies cmd name "myFunction"
        caller_name = inspect.stack()[1].function
        func = getattr(self, caller_name)
        #if caller_name == "verbose":
            #logger.warning(f"MAKE_CMD: locals: {local_vars}")

        args = {}
        explicit_args = {}

        
        # Extract only explicit arguments
        for k, v in local_vars.items():
            if k != "self":
                args[k]=v
                if not Register._is_default(func, k, v):
                    explicit_args[k]=v


        cmd = Command(caller_name, args=args, explicit_args=explicit_args)
        if (separator):
            cmd.set_separators(separator)
        if self.cmdSequence:
            self.cmdSequence.add(cmd)

        return cmd
    


class CommandSequence:

    CmdClass = Command

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

    
    def to_list(self, fmt:Formatter=Formatter()) -> list:
        """Produces a list suited to be joined with newline char, for example. """
        
        if self.programName:
            result = [self.programName]
        else:
            result = []

        for cmd in self.commands:
            result.append(cmd.to_string(fmt)) # quote 
        
        return result


    def to_token_list(self, fmt:Formatter=Formatter()) -> list:
        
        if self.programName:
            result = [self.programName]
        else:
            result = []

        for cmd in self.commands:
            result.extend(cmd.to_tuple(fmt)) # quote 
        return result

        #return [cmd.to_string() for cmd in self.commands]


    def to_string(self, fmt:Formatter=Formatter()) -> str:
        """ Compose a single string of commands.

        Typically, this format is suited to command line use.
        
        fmt.CMD_SEPARATOR can also be '\\\n\t' for example.
        """
        #prog = [str(k) for k in ]
        return fmt.CMD_SEPARATOR.join(self.to_list(fmt))



        """Produces a list compatible with subprocess calls
        
            Notice that the default quote here is empty string, "".
            Substituting None implies default quotes, CommandSequence.QUOTE.
        """

    def to_script(self, filename: str):
        with open(filename, "w", encoding="utf-8") as f:
            # TODO Formatter
            f.write(self.to_string() + "\n")
    
    def to_debug(self, fmt:Formatter=Formatter()) -> list:
        
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
