import inspect
import pathlib
from typing import Any, List, Dict, Union

#from collections import OrderedDict
import rack.base


logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)



class Command:

    PRIMARY_SEP   = ","
    SECONDARY_SEP = ":"

    #def __init__(self, name, args):

    def FOO__init__(self, name, args={}):
        self.name = name
        self.expl_args = args  # dict of explicitly given args

    def __init__(self, name, args={}, explicit_args={}):
        self.name = name
        self.default_args = args
        self.expl_args = explicit_args  # dict of explicitly given args

    def setArgs(self, *args, **kwargs):
        logger.warning(f"  args: {args}")
        logger.warning(f"kwargs: {kwargs}")

        if args:
            for k,v in zip(self.expl_args.keys(), args):
                self.expl_args[k] = v

        self.expl_args.update(kwargs)
        #self.args = {}
        #self.args.update(args)
        
        #self.expl_args = (kwargs)

    def setSeparators(self, primary:str=",", secondary:str=":"):
        self.PRIMARY_SEP   = primary
        self.SECONDARY_SEP = secondary
        if (primary == secondary):
            logger.error(f"setSeparators illegal equality primary:{primary} == secondary:{secondary}")

    def fmt(self, val: Any, key: str = "") -> str:
        """Format a single argument or option for output.
        Default: just convert to string. Override in subclasses."""
        return str(val)

    def __str__(self):
        return " ".join(self.to_tuple("'"))
        #name = self.get_prefixed_name()
        #args = []
        #for k, v in self.args.items():
        #    args.append(f"{k}={self._encode_value(v)}")
        #return f"{name} {self.PRIMARY_SEP.join(args)}"

    def get_prefixed_name(self) -> str:
        l = len(self.name)
        if l==0:
            # Default command (--inputFile ...) works like this - needs no key.
            return ""
        elif l==1:
            return f"-{self.name}"
        else:
            return f"--{self.name}"

    
        
    def to_tuple(self, quote:str="") -> tuple:
        """Returns a singleton or a double token"""    
        
        name = self.get_prefixed_name()
        if name:
            result = [name]
        else:
            result = []
        
        if self.expl_args:
            args = []
            for k, v in self.expl_args.items():
                args.append(f"{k}={self._encode_value(v)}")
                #args.append(f"{k}={quote}{self._encode_value(v)}{quote}")
            result.append(quote+self.PRIMARY_SEP.join(args)+quote)

        return tuple(result)

    def _encode_value(self,v):
        """Format tuple as 'a:b', leave scalars untouched."""
        if isinstance(v, tuple) and len(v) == 2:
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
        explicit = {}

        for k, v in local_vars.items():
            if k != "self":
                args[k]=v
                if not Register._is_default(func, k, v):
                    explicit[k]=v


        cmd = Command(caller_name, args=args, explicit_args=explicit)
        if (separator):
            cmd.setSeparators(separator)
        if self.cmdSequence:
            self.cmdSequence.add(cmd)
            # return ?

        return cmd
        #return Command(caller_name, explicit)
    


class CommandSequence:

    CmdClass = Command

    """Base class for a sequence of commands - 'programs'."""
    def __init__(self, cmdClass=Command, programName:str=""):
        self.CmdClass = cmdClass
        self.commands: List[Command] = []
        self.programName = programName

    def add(self, cmd: Command, args={}):
        if type(cmd) == self.CmdClass:
            self.commands.append(cmd)
        elif type(cmd) == str:
            # logger.warning(f"adding string: {cmd}")
            self.commands.append(Command(str(cmd).strip(" \t-"), args))
        else:
            raise ValueError(f"Unsupported arg type for cmd='{cmd}':", + type(cmd))

    def to_list(self) -> list:
        """Produces a list suited to be joined with newline char, for example"""
        if self.programName:
            result = [self.programName]
        else:
            result = []
        for cmd in self.commands:
            result.append(str(cmd))
        return result
        #return [str(cmd) for cmd in self.commands]
        #return [cmd.to_string() for cmd in self.commands]
    
    def to_token_list(self) -> list:
        """Produces a list compatible with subprocess calls"""
        if self.programName:
            result = [self.programName]
        else:
            result = []
        for cmd in self.commands:
            result.extend(cmd.to_tuple()) 
        return result
            
        #return [cmd.to_string() for cmd in self.commands]


    def to_string(self, joiner:str=" ", quote:str="") -> str:
        """
        
        Joiner can also be '\\\n\t' for example.
        """
        return joiner.join(self.to_list())






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
