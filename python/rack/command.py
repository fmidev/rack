
import inspect # 

from typing import Any, List, Dict, Union

# --- Base classes (generic command structure) ---
from typing import Any, List, Dict

class Command:

    """Base class for any command-like DSL."""
    def __init__(self, name: str, *args, **kwargs):
        self.name = name
        self.args = list(args)
        self.kwargs = kwargs

    def fmt(self, val: Any, key: str = "") -> str:
        """Format a single argument or option for output.
        Default: just convert to string. Override in subclasses."""
        return str(val)

    #def fmt_key(self, key: str) -> str:
    #    return key

    def to_string(self) -> str:
        parts = [self.name]
        for a in self.args:
            parts.append(self.fmt(a))
        for k, v in self.kwargs.items():
            parts.append(str(k))
            parts.append(self.fmt(v, k))
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


class Registry:

    # ---- Extensions
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
            if k != "self" and not Registry._is_default(func, k, v)
        }

        return Command(caller_name, explicit)
    



class CommandSequence:
    """Base class for a sequence of commands - 'programs'."""
    def __init__(self):
        self.commands: List[Command] = []

    def add(self, cmd: Command):
        self.commands.append(cmd)

    def to_list(self) -> list:
        return [cmd.to_string() for cmd in self.commands]
    
    def to_token_list(self, separator=None) -> list:
        cmds = []
        for cmd in self.commands:
            cmds.extend(cmd.to_tuple(separator))
        return cmds
            
        #return [cmd.to_string() for cmd in self.commands]


    def to_string(self, joiner="\n") -> str:
        return joiner.join(cmd.to_string() for cmd in self.commands)




def main():

    # --- Example usage ---
    cmds = CommandSequence()
    """
    cmds = GnuPlotCommandSequence()
    cmds.add("set", "terminal", GnuPlot.PNG, "size", (600, 400))
    cmds.add("set", "output", file="out.png")
    cmds.add("set", "datafile", "separator", GnuPlot.WHITESPACE)
    cmds.add("set", "xdata", "time")
    cmds.add("set", "timefmt", "%s")
    cmds.add("set", "format", "x", "%H:%M")
    """
    print(cmds.to_string())


if __name__ == "__main__":
    main()

