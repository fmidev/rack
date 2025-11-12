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

    def to_string(self) -> str:
        parts = [self.name]
        for a in self.args:
            parts.append(self.fmt(a))
        for k, v in self.kwargs.items():
            parts.append(str(k))
            parts.append(self.fmt(v, k))
        return " ".join(parts)


class CommandSequence:
    """Base class for a sequence of commands."""
    def __init__(self):
        self.commands: List[Command] = []

    def add(self, cmd: Command):
        self.commands.append(cmd)

    def to_list(self) -> list:
        return [cmd.to_string() for cmd in self.commands]

    def to_string(self, joiner="\n") -> str:
        return joiner.join(cmd.to_string() for cmd in self.commands)



# --- GnuPlot-specific layer ---

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

