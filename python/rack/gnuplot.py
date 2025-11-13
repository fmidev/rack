from typing import Any, List, Union

import rack.command



# --- Base Layer (already familiar) ---

class GnuPlotCommand(rack.command.Command):
    """GnuPlot-specific command, inherits from Command."""
    
    QUOTED_KEYS = {"output", "file", "timefmt", "format", "title", "label"}
    
    @staticmethod
    def static_fmt(val: Any, key: str = "") -> str:
        """Format any argument for gnuplot."""
        if isinstance(val, (tuple, list)):
            return ",".join(str(x) for x in val)
        if isinstance(val, str):
            needs_quotes = False
            if key in GnuPlotCommand.QUOTED_KEYS or val.endswith((".png", ".svg", ".dat", ".txt")) or "%" in val:
                needs_quotes = True
            if needs_quotes and not (val.startswith('"') or val.startswith("'")):
                return f'"{val}"'
        return str(val)

    def fmt(self, val: Any, key: str = "") -> str:
        return self.static_fmt(val, key)



class GnuPlotCommandSequence(rack.command.CommandSequence):
    """Sequence of GnuPlotCommands."""
    
    def add(self, cmd: GnuPlotCommand):
        if not isinstance(cmd, GnuPlotCommand):
            raise TypeError("Only GnuPlotCommand instances allowed")
        super().add(cmd)

    def to_script(self, filename: str):
        with open(filename, "w", encoding="utf-8") as f:
            f.write(self.to_string("\n") + "\n")





# --- Static Namespace Layer (for autocompletion) ---
class GnuPlot:
    """Namespace for gnuplot DSL."""

    # --- constants ---
    class Terminal:
        PNG = "png"
        SVG = "svg"
        QT = "qt"

    class Datafile:
        WHITESPACE = "whitespace"
        COMMA = "comma"

    # --- Set commands ---
    class Set:
        @staticmethod
        def terminal(terminal_type: str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", "terminal", terminal_type, **opts)

        @staticmethod
        def output(filename: str) -> GnuPlotCommand:
            return GnuPlotCommand("set", "output", filename)

        @staticmethod
        def datafile(text: str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", datafile=text, **opts)

        @staticmethod
        def grid(arg:str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", "grid", **opts)

        @staticmethod
        def xdata(arg: str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", xdata=arg, **opts)

        @staticmethod
        def xlabel(arg: str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", xlabel=arg, **opts)

        @staticmethod
        def ylabel(arg: str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", ylabel=arg, **opts)

        #@staticmethod
        #def xdata(mode: str) -> GnuPlotCommand:
        #    return GnuPlotCommand("set", "xdata", mode)

        @staticmethod
        def timefmt(fmt: str) -> GnuPlotCommand:
            return GnuPlotCommand("set", "timefmt", fmt)

        @staticmethod
        def format_x(fmt: str) -> GnuPlotCommand:
            return GnuPlotCommand("set", "format x", fmt)
            #return GnuPlotCommand("set", "format", "x", fmt)

        @staticmethod
        def format_y(fmt: str) -> GnuPlotCommand:
            return GnuPlotCommand("set", "format y", fmt)

        @staticmethod
        def title(text: str) -> GnuPlotCommand:
            return GnuPlotCommand("set", title=text)

        @staticmethod
        def label(text: str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", label=text, **opts)

        @staticmethod
        def multiplot(rows: int, cols: int, title: str = None) -> List[GnuPlotCommand]:
            cmds = [GnuPlotCommand("set", "multiplot", f"layout {rows},{cols}")]
            if title:
                cmds.append(GnuPlotCommand("set", "title", title))
            return cmds

        @staticmethod
        def unset_multiplot() -> GnuPlotCommand:
            return GnuPlotCommand("unset", "multiplot")


    # --- Plot commands ---
    class Plot:

        @staticmethod
        def plot(*items: Union[str, dict], **global_opts) -> GnuPlotCommand:
            """Create a plot command with multiple items.
            Each item can be a string (expression) or a dict with 'expr' or 'file' key and options.
            Global options apply to the entire plot command.
            """

            segments = []

            def build_segment(expr: str, **opts):
                opts = {('with' if k == 'with_' else k): v for k, v in opts.items()}
                parts = [expr]
                for k, v in opts.items():
                    parts.append(str(k))
                    parts.append(GnuPlotCommand.static_fmt(v, k))
                return " ".join(parts)

            # Single string expression
            if len(items) == 1 and isinstance(items[0], str):
                return GnuPlotCommand("plot", items[0], **global_opts)

            # Multiple plot dicts
            for item in items:
                """ 
                if isinstance(item, dict) and "expr" in item:
                    expr = item["expr"]
                    opts = {k: v for k, v in item.items() if k != "expr"}
                    segments.append(build_segment(expr, **opts))
                """
                if isinstance(item, dict):
                    if "expr" in item:
                        src = item["expr"]                      # unquoted
                    elif "file" in item:
                        src = f'"{item["file"]}"'               # quoted
                    else:
                        raise ValueError("Each plot item must include either 'expr' or 'file'")

                    opts = {k: v for k, v in item.items() if k not in ("expr", "file")}
                    segments.append(build_segment(src, **opts))

                elif isinstance(item, str):
                    segments.append(item)
                else:
                    raise TypeError("Each plot item must be a string or dict with 'expr'")
            
            return GnuPlotCommand("plot", ",\\\n     ".join(segments))

        @staticmethod
        def replot(**opts) -> GnuPlotCommand:
            return GnuPlotCommand("replot", **opts)

        
    # Group under top-level for autocomplete: GnuPlot.set.output(...)
    set  = Set()
    plot = Plot()

# --- Example usage ---

def main():

    
    cmds = GnuPlotCommandSequence()
    cmds.add(GnuPlot.set.terminal(GnuPlot.Terminal.PNG, size=(800, 600)))
    cmds.add(GnuPlot.set.output("plot_output.png"))


    # titles and labels
    cmds.add(GnuPlot.set.title("Combined Sine and Cosine"))
    cmds.add(GnuPlot.set.label("t=0", at=(0, 0)))

    # multiplot layout
    for c in GnuPlot.set.multiplot(2, 1):
        cmds.add(c)

    # xdata and format
    cmds.add(GnuPlot.set.xdata("time"))
    cmds.add(GnuPlot.set.timefmt("%s"))
    cmds.add(GnuPlot.set.format_x("%H:%M"))

    # multiple plots in one line
    cmds.add(GnuPlot.plot.plot(
        {"expr": "sin(x)", "title": "Sine", "with_": "lines"},
        {"expr": "cos(x)", "title": "Cosine", "with_": "linespoints"}
    ))

    #unset multiplot
    cmds.add(GnuPlot.set.unset_multiplot())

    """
    cmds.add(GnuPlot.set.datafile_separator(GnuPlot.Datafile.WHITESPACE))
    cmds.add(GnuPlot.set.xdata("time"))
    cmds.add(GnuPlot.set.timefmt("%s"))
    cmds.add(GnuPlot.set.format_x("%H:%M"))
    
    # Add plot command
    #cmds.add(GnuPlot.plot.plot("sin(x)", title="Sine", with_="lines", linewidth=2))
    cmds.add(GnuPlot.plot.plot(expr="cos(x)", title="Cosine", with_="lines", linewidth=2))
    cmds.add(GnuPlot.plot.plot(
      {"expr": "sin(x)", "title": "Sine", "with_": "lines"},
      {"expr": "cos(x)", "title": "Cosine", "with_": "lines"}
    ))
    """
  
    # Save to file
    cmds.to_script("example_plot.plt")

    print("Generated GnuPlot script:\n")
    print(cmds.to_string(";\n"))



if __name__ == "__main__":
    main()
