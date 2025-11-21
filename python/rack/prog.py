import inspect
import pathlib
from typing import Any, List, Dict, Union

#from collections import OrderedDict
import rack.base


logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)



class Command:

    PRIMARY_SEP = ","
    SECONDARY_SEP = ":"

    #def __init__(self, name, args):

    def __init__(self, name, args={}):
        self.name = name
        self.args = args  # dict of explicitly given args

    """Base class for any command-like DSL."""
    def OLD__init__(self, name: str, *args, **kwargs):
        self.name = name
        self.args = list(args)
        self.kwargs = kwargs

    def fmt(self, val: Any, key: str = "") -> str:
        """Format a single argument or option for output.
        Default: just convert to string. Override in subclasses."""
        return str(val)

    def __str__(self):
        # Format "i=2:7,iStep=1"
        parts = []
        for k, v in self.args.items():
            parts.append(f"{k}={self._encode_value(v)}")
        return f"{self.name} {self.PRIMARY_SEP.join(parts)}"

    def to_string_default(self):
        return self.__str__()

    def to_string(self) -> str:
        parts = [self.name]
        for a in self.args:
            parts.append(self.fmt(a))
        logger.warning(parts)
        # Deactivated...
        # for k, v in self.kwargs.items():
        #    parts.append(str(k))
        #    parts.append(self.fmt(v, k))
        return " ".join(parts)
    
    def to_tuple(self, separator=None) -> tuple:
        args = []
        args.extend(self.args)
        for k, v in self.kwargs.items():
            args.append(f"{k}={v}")
        if (args):
            if type(separator) is str:
                return (self.name, separator.join(args))
            else:
                return (self.name, args)
        else:
            return (self.name, )

    def to_tuple(self):
        return self.__str__()

    def _encode_value(self,v):
        """Format tuple as 'a:b', leave scalars untouched."""
        if isinstance(v, tuple) and len(v) == 2:
            return f"{v[0]}{self.SECONDARY_SEP}{v[1]}"  #extend for >2
        return v




class Register:

    def _is_default(func, name, value):
        sig = inspect.signature(func)
        param = sig.parameters[name]
        return ( param.default is not inspect._empty and value == param.default )


    def _make_cmd(self, local_vars):
        # Detect caller function automatically
        caller_name = inspect.stack()[1].function
        func = getattr(self, caller_name)

        # Extract only explicit arguments
        explicit = {
            k: v
            for k, v in local_vars.items()
            if k != "self" and not Register._is_default(func, k, v)
        }

        return Command(caller_name, explicit)
    


class CommandSequence:

    CmdClass = Command

    """Base class for a sequence of commands - 'programs'."""
    def __init__(self, cmdClass=Command):
        self.CmdClass = cmdClass
        self.commands: List[Command] = []

    def add(self, cmd: Command, args={}):
        if type(cmd) == self.CmdClass:
            self.commands.append(cmd)
        elif type(cmd) == str:
            self.commands.append(self.CmdClass(str, args))
        else:
            raise ValueError(f"Unsupported arg type for cmd='{cmd}':", + type(cmd))

    def to_list(self) -> list:
        return [cmd.to_string() for cmd in self.commands]
    
    def to_token_list(self, separator=None) -> list:
        cmds = []
        for cmd in self.commands:
            cmds.append(cmd) #separator))
            # cmds.extend(cmd.to_tuple()) #separator))
        return cmds
            
        #return [cmd.to_string() for cmd in self.commands]


    def to_string(self, joiner="\n") -> str:
        return joiner.join(cmd.to_string() for cmd in self.commands)






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
