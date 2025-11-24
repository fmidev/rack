import inspect
import pathlib
from typing import Any, List, Dict, Union

#from collections import OrderedDict
import rack.base


logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)



class Command:

    PRIMARY_SEP   = ","
    SECONDARY_SEP = ":"


    def __init__(self, name, args={}, explicit_args={}):
        """This defines the command parameter list, including default values.
        """
        
        self.name = name.strip(" -\t\n")
        
        self.expl_indices = 0

        if type(args) == str:
            args = args.strip().split(self.PRIMARY_SEP) # note: default value in PRIMARY_SEP
            

        if (type(args) == list):
            self.expl_indices = len(args)
            args = dict(enumerate(args)) #{k,v for enumerate(args)}

        if self.name == 'verbose':
            logger.debug("enchanced logging")
            logger.setLevel("DEBUG")
            logger.warning(f"ARGS: {args}, EXPLICIT: {explicit_args}")

        self.args = args.copy()
        self.args.update(explicit_args)
        # Explicit argument indices given by the last set_args()
        
        # Explicit argument keys given by the last set_args()
        self.expl_keys = explicit_args.keys()  # dict of explicitly given args
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
            self.expl_indices = len(args)

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
        return " ".join(self.to_tuple("'"))

    def get_prefixed_name(self) -> str:
        l = len(self.name)
        if l==0:
            # Default command (--inputFile ...) works like this - needs no key.
            return ""
        elif l==1:
            return f"-{self.name}"
        else:
            return f"--{self.name}"

    
    
    def to_tuple(self, quote=None, compact=True) -> tuple: # , prefixed=True
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

        for i in range(0,self.expl_indices):
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
        # Detect caller function automatically
        caller_name = inspect.stack()[1].function
        func = getattr(self, caller_name)

        # Extract only explicit arguments
        """
        explicit = {
            k: v
            for k, v in local_vars.items()
            if k != "self" and not Register._is_default(func, k, v)
        }
        """

        args = {}
        explicit_args = {}

        if caller_name == "verbose":
            logger.warning(f"MAKE_CMD: locals: {local_vars}")
        
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
            # return ?

        return cmd
        #return Command(caller_name, explicit)
    


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
        if t == self.CmdClass:
            self.commands.append(cmd)
            if cmd_args:
                logger.warning(f"Parameter override for {cmd.name}: {cmd_args} (vs. {cmd.args})")
        elif t in {str,list}:
            logger.warning(f"adding string: {cmd} args={cmd_args}")
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
            raise TypeError(f"Unsupported arg type for cmd='{cmd}':", + type(cmd))

    def to_list(self, quote=None) -> list:
        """Produces a list suited to be joined with newline char, for example"""

        if self.programName:
            result = [self.programName]
        else:
            result = []

        if quote is None:
            quote = self.QUOTE

        for cmd in self.commands:
            # TODO: quote...
            ##result.append(str(cmd))
            result.append(" ".join(cmd.to_tuple(quote)))
        return result
        #return [str(cmd) for cmd in self.commands]
        #return [cmd.to_string() for cmd in self.commands]
    
    def to_token_list(self, quote="") -> list:
        """Produces a list compatible with subprocess calls
        
            Notice that the default quote here is empty string, "".
            Substituting None implies default quotes, CommandSequence.QUOTE.
        """
        
        if self.programName:
            result = [self.programName]
        else:
            result = []

        if quote is None:
            quote = self.QUOTE

        for cmd in self.commands:
            result.extend(cmd.to_tuple(quote)) 
        return result
            
        #return [cmd.to_string() for cmd in self.commands]


    def to_string(self, joiner:str=" ", quote=None) -> str:
        """ Compose a single string of commands.

        Typically, this format is suited to command line use.
        
        Joiner can also be '\\\n\t' for example.
        """
        return joiner.join(self.to_list(quote))






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
            #return self._make_cmd(locals())
            return self._make_cmd(locals())


    reg = MyRegister()
    cmd = reg.sample(i=(-2, 5), iStep=1)
    cmd.PRIMARY_SEP='|'
    cmd.SECONDARY_SEP='/'
    print(cmd)   # assuming MyCommand.__str__ formats tuples as "a:b" and joins with commas
    prog = CommandSequence(Command)
    prog.add(cmd)
    print(prog.to_string())
    print(prog)
    print(prog.to_list())
    print(prog.to_token_list())




if __name__ == "__main__":
    main()
