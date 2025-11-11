from typing import Any, List, Dict, Union

# --- Base classes (generic command structure) ---

class Command:
    def __init__(self, name: str, *args, **options):
        self.name = name
        self.args = list(args)
        self.options = options

    def to_string(self) -> str:
        parts = [self.name]
        parts += [str(a) for a in self.args]
        for key, val in self.options.items():
            if val is True:
                parts.append(f"--{key}")
            elif val is not None:
                parts.append(f"--{key}={val}")
        return " ".join(parts)


class CommandSequence:
    def __init__(self):
        self.commands: List[Command] = []

    def add(self, name: str, *args, **options):
        self.commands.append(Command(name, *args, **options))

    def to_string(self, joiner=";\n") -> str:
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

