#import os
import inspect
from typing import Any, List, Union
from pathlib import Path

#import rack.command
import rack.prog
import rack.log
logger = rack.log.logger.getChild(Path(__file__).stem)

class Literal():
    def __init__(self, name:str, value=None):
        self.name = name
        if not value is None:
            self.value = value
    
    def __str__(self):
        return self.name

class KeyWord(Literal):
    pass

class Expr(Literal):
    pass

class Terminal:
    PNG = KeyWord("png")
    SVG = KeyWord("svg")
    #SVG = KeyWord("svg")

class Datafile:
    SEPARATOR  = KeyWord("separator") 
    WHITESPACE = KeyWord("whitespace") 

class Format:
    X = KeyWord("x") 
    Y = KeyWord("y") 

class Data:
    TIME = KeyWord("time") 


class GnuPlotFormatter(rack.prog.Formatter):
    
    #def __init__(self, name_format='{name}', key_format='{key}', value_format='{value}', value_assign='=', param_separator=',', params_format='{params}', cmd_assign=' ', cmd_separator=' '):
    #    super().__init__(name_format, key_format, value_format, value_assign, param_separator, params_format, cmd_assign, cmd_separator)

    def __init__(self):
        self.VALUE_ASSIGN=' '
        self.VALUE_SEPARATOR=','
        self.PARAM_SEPARATOR=' '
        self.CMD_SEPARATOR=';\n'

    def fmt_value(self, value:str) -> str :
        if isinstance(value, (KeyWord,Expr)):
            value=value.name
        elif isinstance(value, str):
            value=f'"{value}"'
        return super().fmt_value(value)

        


class GnuReg(rack.prog.Register):
    """ Automatic Drain command set export
    """

    def make_set_cmd(self, locs: dict):

        # Detect caller function automatically
        caller_name = inspect.stack()[1].function
        # func = getattr(self, caller_name)

        # Initialize with caller name
        args = [KeyWord(caller_name.replace('_',' '))]

        for k, v in locs.items():
            if k == "self":
                pass
            elif k != "opts":
                #(key,value) = (k,v)
                logger.debug(f"detected setting: {k} = {v}")
                args.append(v)

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

    
    def datafile(self, key, value, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", datafile=text, **opts)
    
    def grid(self, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "grid", **opts)
    
    def xdata(self, mode: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", xdata=arg, **opts)
    
    def xlabel(self, label: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", xlabel=arg, **opts)
    
    def ylabel(self, label: str, **opts) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", ylabel=arg, **opts)

    
    def timefmt(self, fmt: str) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "timefmt", fmt)

    def format_x(self, fmt: str) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "format x", fmt)
        #return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "format", "x", fmt)
    
    def format_y(self, fmt: str) : #  GnuPlotCommand:
        return self.make_set_cmd(locals()) #  GnuPlotCommand("set", "format y", fmt)
        
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


    class plot_entry():

        src=None
        opts = {}

        def __str__(self):
            fmt = GnuPlotFormatter()
            opts = []
            """
            if self.opts:
                if 'style' in self.opts: # with ?
                    opts = [f"with={self.opts['style']}"]
                if 'with' in self.opts: # with ?
                    opts = [f"with={self.opts['with']}"]
                for (k,v) in self.opts:
                    opts.append(fmt.fmt_param(v,k))
                    #opts.append(f"{k} {v}")
            """
            #opts = " ".join(opts)
            opts =fmt.fmt_params(self.opts)
            return f"{self.src} {opts}"

        def __init__(self, expr=None, filename=None, style=None, **opts):
            if expr and filename:
                raise KeyError(f"both 'expr'={expr} and 'filename'={filename} assigned")
            elif expr:
                self.src=Expr(expr)
            elif filename:
                self.src=filename
            else:
                pass
                #raise KeyError("neither 'expr' nor 'filename' assigned")

            self.opts = {}
            if (style):
                self.opts["with"] = style
            elif opts:
                logger.warning(f"unset 'style' with options={opts}") 
           
            self.opts.update(opts)
            pass

        def is_set(self):
            return (self.src != None)



    def plot(self, *items: Union[str, dict], expr=None, filename=None, style=None, **global_opts):
        """Create a plot command with multiple items.
        Each item can be a string (expression) or a dict with 'expr' or 'file' key and options.
        Global options apply to the entire plot command.

        Parameters

        """
        if (style):
            global_opts["with"] = style

        entry = self.plot_entry(expr=expr, filename=filename, style=style, **global_opts)

        logger.warning(items)
        logger.warning(global_opts)
        
        cmd = None
        # Single string expression
        if len(items) == 1 and isinstance(items[0], str):
            cmd = rack.prog.Command("plot", [Expr(items[0])], **global_opts)
            #return GnuPlotCommand("plot", items[0], **global_opts)

        if (expr): #, Expr("toka")
            cmd = rack.prog.Command("plot", [Expr(expr)], global_opts)

        if (filename):
            cmd = rack.prog.Command("plot", filename, global_opts)

        if cmd:
            if self.cmdSequence:
                self.cmdSequence.add(cmd)
            return cmd


        def build_segment(expr: str, **opts):
            opts = {('with' if k == 'with_' else k): v for k, v in opts.items()}
            parts = [expr]
            for k, v in opts.items():
                if k != "style":    
                    parts.append(str(k))
                parts.append(rack.prog.Command(k, v))
                #parts.append(GnuPlotCommand.static_fmt(v, k))
            return " ".join(parts)


        # Multiple plot dicts
        segments = []
        for item in items:

            logger.info(f"item={item} ({type(item)})")
            #if isinstance(item, dict):
            if False:
                if "expr" in item:
                    src = item["expr"]                      # unquoted
                elif "file" in item:
                    src = f'"{item["file"]}"'               # quoted
                else:
                    raise ValueError("Each plot item must include either 'expr' or 'file'")

                opts = {k: v for k, v in item.items() if k not in ("expr", "file")}
                segments.append(build_segment(src, **opts))
            elif isinstance(item, self.plot_entry):
                segments.append(item)
            elif isinstance(item, str):
                segments.append(self.plot_entry(item, style=style))
                #segments.append(self.plot_entry(filename=item, style=style))
                #segments.append(item)
            else:
                raise TypeError("Each plot item must be a string or dict with 'expr'")
        
        #return rack.prog.Command("plot", ",\\\n     ".join(segments))
        #cmd = rack.prog.Command("plöt", ",\\\n     ".join(segments))
        cmd = rack.prog.Command("plotMANY", segments) #  there are no super_global_opts 
        if self.cmdSequence:
            self.cmdSequence.add(cmd)
        return cmd

    
# --- Static Namespace Layer (for autocompletion) ---
class GnuPlot:
    """Namespace for gnuplot DSL."""


    # --- Plot commands ---
    class Plot:

        @staticmethod
        def plot(*items: Union[str, dict], **global_opts):
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
                    #parts.append(GnuPlotCommand.static_fmt(v, k))
                return " ".join(parts)

            # Single string expression
            if len(items) == 1 and isinstance(items[0], str):
                return None #GnuPlotCommand("plot", items[0], **global_opts)

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
            
            return None # GnuPlotCommand("plot", ",\\\n     ".join(segments))

        
    # Group under top-level for autocomplete: GnuPlot.set.output(...)
    #set  = Set()
    plot = Plot()

# --- Example usage ---

class Style:
    """ In gunplot, after "with" keyword
    """

    LINES = KeyWord("lines")
    DOTS  = KeyWord("dots")
    LINES_DOTS = KeyWord("linesdots")


def main():

    
    prog = rack.prog.CommandSequence()

    reg = GnuReg(prog)
    #reg.terminal(GnuPlot.Terminal.PNG, size=(800, 600))
    reg.terminal(Terminal.PNG, size=(800, 600))
    reg.output("my-file.png")
    #reg.set("terminal", GnuPlot.Terminal.PNG, size=(800, 600))

    reg.datafile(Datafile.SEPARATOR, Datafile.WHITESPACE)
    #reg.datafile(Datafile.SEPARATOR, char=Datafile.WHITESPACE)
    reg.xdata(Data.TIME)
    reg.timefmt("%s")
    #reg.format_x(Format.X, "%H:%M")
    reg.format_x("%H:%M")
    reg.grid()
    reg.title("statistics 00min 20140525-1200")
    reg.xlabel("TIME START REL")
    reg.ylabel("ELANGLE")

    reg.plot("sin(x)")
    reg.plot("sin(x) with lines")
    reg.plot(expr="sin(x)")
    reg.plot(expr="sin(x)", style=Style.LINES)
    reg.plot(filename="data.txt", style=Style.DOTS)
    reg.plot("sin(x)", "cos(x)", "tan(x)", style=Style.LINES)

    e1 = reg.plot_entry(expr="random(x)", style=Style.LINES_DOTS)
    reg.plot(e1)
    e2 = reg.plot_entry(filename="my_file.dat", style=Style.DOTS)
    #reg.plot(["cos(x)"])
    reg.plot(e2)
    reg.plot(e1,e2)


    #reg.label("t=0", at=(0,0))
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

    print("# Generated GnuPlot script:\n")
    print(prog.to_string(fmt))  # ";\n"



if __name__ == "__main__":
    main()
