from typing import Any #, List, Dict, Union
import rack.command

# --- GnuPlot-specific layer ---

class GnuPlot:
    """Namespace for GnuPlot constants and helpers."""
    # Keywords
    terminal = "terminal"
    output = "output"
    datafile = "datafile"
    timefmt = "timefmt"
    format = "format"

    
    # Terminal types
    PNG = "png"
    SVG = "svg"
    QT = "qt"

    # Common separators
    WHITESPACE = "whitespace"
    COMMA = "comma"


class GnuPlotCommand(rack.command.Command):
    QUOTED_KEYS = {"output", "timefmt", "format", "file", "title"}  # automatic quoting
    TUPLE_JOIN_KEYS = {"size"}  # join tuples/lists with commas

    def _quote_if_needed(self, key: str, val: Any) -> str:
        """Apply quoting rules for known keys."""
        if isinstance(val, str):
            if key in self.QUOTED_KEYS and not (val.startswith('"') or val.startswith("'")):
                return f'"{val}"'
        return str(val)

    
    def _format_arg(self, val: Any, key_hint: str = None) -> str:
        """Handle tuple/list flattening and quoting."""
        if isinstance(val, (tuple, list)):
            joined = ",".join(str(v) for v in val)
            if key_hint in self.TUPLE_JOIN_KEYS:
                return joined
            else:
                return f'({joined}){key_hint}!!'
        return self._quote_if_needed(key_hint or "", val)

    
    def to_string(self) -> str:
        parts = [self.name]

        # positional arguments
        for a in self.args:
            parts.append(self._format_arg(a))

        # keyword arguments (gnuplot-style: no "--")
        for key, val in self.options.items():
            formatted = self._format_arg(val, key)
            parts.append(str(key))
            parts.append(formatted)

        return " ".join(parts)


class GnuPlotCommandSequence(rack.command.CommandSequence):
    def add(self, name: str, *args, **options):
        self.commands.append(GnuPlotCommand(name, *args, **options))


def main():

    # --- Example usage ---
    #cmds = CommandSequence()
    cmds = GnuPlotCommandSequence()
    # cmds.add("set", "terminal", GnuPlot.PNG, "size", (600, 400))
    cmds.add("set", "terminal", GnuPlot.PNG, size=(600, 400))
    cmds.add("set", "output", file="out.png")
    cmds.add("set", "datafile", "separator", GnuPlot.WHITESPACE)
    cmds.add("set", "xdata", "time")
    cmds.add("set", "timefmt", format="%s")
    cmds.add("set", "format", "x", format="%H:%M")
        
    cmds.add("set", "grid")
    cmds.add("plot", "sin(x)", title="Sine", with_="lines", linewidth=2)
    
    cmds.add("plot", file="foo.txt", title="Sine", with_="lines", linewidth=2)
    print(cmds.to_string())


if __name__ == "__main__":
    main()
