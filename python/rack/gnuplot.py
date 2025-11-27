#import os
import inspect
from typing import Any, List, Union
from pathlib import Path

#import rack.command
import rack.prog
import rack.log
logger = rack.log.logger.getChild(Path(__file__).stem)

class KeyWord():
    def __init__(self, name:str, value=None):
        self.name = name
        if not value is None:
            self.value = value


class Terminal:
    PNG = KeyWord("png")
    SVG = KeyWord("svg")
    #SVG = KeyWord("svg")



class GnuPlotFormatter(rack.prog.Formatter):
    
    #def __init__(self, name_format='{name}', key_format='{key}', value_format='{value}', value_assign='=', param_separator=',', params_format='{params}', cmd_assign=' ', cmd_separator=' '):
    #    super().__init__(name_format, key_format, value_format, value_assign, param_separator, params_format, cmd_assign, cmd_separator)

    def __init__(self):
        self.VALUE_ASSIGN=' '
        self.VALUE_SEPARATOR=','
        self.PARAM_SEPARATOR=' '
        self.CMD_SEPARATOR=';\n'

    def fmt_value(self, value:str) -> str :
        if isinstance(value, KeyWord):
            value=value.name
        elif isinstance(value, str):
            value=f'"{value}"'
        return super().fmt_value(value)

    def fmt_name_UNUSED(self, name:str) -> str :
        l = len(name)
        if l==0: # Default command (--inputFile ...) works like this - needs no key.
            return ""
        elif l==1:
            return f"-{name}"
        else:
            return f"--{name}"
        


class GnuReg(rack.prog.Register):
    """ Automatic Drain command set export
    """

    def make_set_cmd(self, locs):

        # Detect caller function automatically
        caller_name = inspect.stack()[1].function
        # func = getattr(self, caller_name)
        args = [KeyWord(caller_name)]

        for k, v in locs.items():
            if k == "self":
                pass
            elif k != "opts":
                #(key,value) = (k,v)
                logger.warning(f"detected: {k} = {v}")
                args.append(v)

        # cmd = GnuPlotSet(caller_name, value, locs["opts"])
        if "opts" in locs:
            cmd = rack.prog.Command("set", args, locs["opts"])
        else:
            cmd = rack.prog.Command("set", args)


        if self.cmdSequence:
            self.cmdSequence.add(cmd)
        
        return
    
    
    def terminal(self, terminal_type: KeyWord=Terminal.PNG, **opts):
        return self.make_set_cmd(locals())

    def output(self, filename: str):
        return self.make_set_cmd(locals())

    def title(self, title: str):
        return self.make_set_cmd(locals())

    
    def datafile(self, text: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", datafile=text, **opts)
    
    def grid(self, arg:str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "grid", **opts)
    
    def xdata(self, arg: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", xdata=arg, **opts)
    
    def xlabel(self, arg: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", xlabel=arg, **opts)
    
    def ylabel(self, arg: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", ylabel=arg, **opts)

    #
    #def xdata(self, mode: str) : #  GnuPlotCommand:
    #    return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "xdata", mode)

    
    def timefmt(self, fmt: str) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "timefmt", fmt)

    
    def format_x(self, fmt: str) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "format x", fmt)
        #return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "format", "x", fmt)

    
    def format_y(self, fmt: str) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "format y", fmt)

    
    #def title(self, text: str) : #  GnuPlotCommand:
    #    return self.make_set_cmd(locals()) #  GnuPlotCommand("set", title=text)

    
    def label(self, text: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", label=text, **opts)

    
    def key(self, text: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", key=text, **opts)

    
    def multiplot(self, rows: int, cols: int, title: str = None) : #  List[GnuPlotCommand]:
        #cmds = [GnuPlotCommand("set", "multiplot", f"layout {rows},{cols}")]
        #if title:
        #    cmds.append(GnuPlotCommand("set", "title", title))
        return self.make_set_cmd(locals()) #  cmds

    
    def unset_multiplot(self) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("unset", "multiplot")

# --- Base Layer (already familiar) ---

#class GnuPlotCommand(rack.command.Command):
class GnuPlotCommand(rack.prog.Command):
    """GnuPlot-specific command, inherits from Command."""
    
    QUOTED_KEYS = {"output", "file", "timefmt", "format", "title", "label", "xlabel", "ylabel"}
    
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


class GnuPlotCommandSequence(rack.prog.CommandSequence):
#class GnuPlotCommandSequence(rack.command.CommandSequence):
    """Sequence of GnuPlotCommands."""
    

    def addOld(self, cmd: GnuPlotCommand):
        if not isinstance(cmd, GnuPlotCommand):
            raise TypeError("Only GnuPlotCommand instances allowed")
        super().add(cmd)

    def to_script(self, filename: str):
        with open(filename, "w", encoding="utf-8") as f:
            #f.write(self.to_string("\n") + "\n")
            f.write(self.to_string() + "\n")

    # see also: create_gnuplot_script in rack/statistics.py
    def create_scriptFOO(self, files: list, settings=dict(), colums=(1,2)):

        log = logger.getChild("create_script")
        
        # Default configuration for gnuplot

        conf = {
            "terminal": "png size 800,600",
            "output": None,
            "datafile": "separator whitespace",
            "xdata": None,
            "timefmt": None,
            "format_x": None,
            "grid": None,
            "xlabel": None,
            "ylabel": None,
            "title": None,
            #"using": '2:3',
        }

        conf.update(settings)

        for (k,v) in conf.items():
            if not v is None:
                func = getattr(GnuPlot.set, k)   # resolves GnuPlot.set.format_x
                self.add(func(v))

        SEPARATOR='_'
        log.debug("adding input files")
        plots = []
        files.reverse()
        colums = ":".join([str(i) for i in colums])
        #selector = ":".join(selector)
        while files:
            f = files.pop()
            split_name = f.replace('/',SEPARATOR).split(SEPARATOR)
            # DEFUNCT title =  " ".join([split_name[i] for i in distinct_indices])
            title = "FOO"
            plots.append({"file": f, "using": colums, "with_": "lines", "title": title})

        self.add(GnuPlotCommand.plot.plot( *plots ))
        # log.debug("created gnuplot script with %d plot items", len(plots))

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
        def key(text: str, **opts) -> GnuPlotCommand:
            return GnuPlotCommand("set", key=text, **opts)

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
                    if k != "style":    
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

    
    prog = GnuPlotCommandSequence()

    reg = GnuReg(prog)
    #reg.terminal(GnuPlot.Terminal.PNG, size=(800, 600))
    reg.terminal(Terminal.PNG, size=(800, 600))
    reg.output("my-file.png")
    reg.title("Combined Sine and Cosine")
    #reg.set("terminal", GnuPlot.Terminal.PNG, size=(800, 600))

    reg.label("t=0", at=(0,0))
    #prog.add(GnuPlot.set.terminal(GnuPlot.Terminal.PNG, size=(800, 600)))
    #prog.add(GnuPlot.set.output("plot_output.png"))

    """

    # titles and labels
    prog.add(GnuPlot.set.title("Combined Sine and Cosine"))
    prog.add(GnuPlot.set.label("t=0", at=(0, 0)))

    # multiplot layout
    for c in GnuPlot.set.multiplot(2, 1):
        prog.add(c)

    # xdata and format
    prog.add(GnuPlot.set.xdata("time"))
    prog.add(GnuPlot.set.timefmt("%s"))
    prog.add(GnuPlot.set.format_x("%H:%M"))

    # multiple plots in one line
    prog.add(GnuPlot.plot.plot(
        {"expr": "sin(x)", "title": "Sine", "with_": "lines"},
        {"expr": "cos(x)", "title": "Cosine", "with_": "linespoints"}
    ))
    """

    #unset multiplot
    #prog.add(GnuPlot.set.unset_multiplot())

  
    # Save to file
    #prog.to_script("example_plot.plt")
    fmt = GnuPlotFormatter()

    print("Generated GnuPlot script:\n")
    print(prog.to_string(fmt))  # ";\n"



if __name__ == "__main__":
    main()
