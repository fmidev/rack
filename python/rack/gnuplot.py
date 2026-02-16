#import os
import inspect
from typing import Any, List, Union
from pathlib import Path

#import rack.command
import rack.prog
import rack.log
logger = rack.log.logger.getChild(Path(__file__).stem)

# NEW
from enum import Enum

# Todo: move to rack.prog modules
class Literal():
    def __init__(self, name:str, value=None):
        self.name = name
        if not value is None:
            self.value = value
    
    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name

#class KeyWord(Literal):
#    pass

class Expr(Literal):
    pass


"""
class Terminal_OLD:
    PNG = KeyWord("png")
    SVG = KeyWord("svg")
    # SVG = KeyWord("svg")

class Datafile:
    SEPARATOR  = KeyWord("separator") 
    WHITESPACE = KeyWord("whitespace") 

class Format:
    X = KeyWord("x") 
    Y = KeyWord("y") 

class Data:
    TIME = KeyWord("time") 
"""

class GnuPlotFormatter(rack.prog.Formatter):
    
    #def __init__(self, name_format='{name}', key_format ='{key}', value_format='{value}', value_separator=':', 
    #             value_assign='=', param_separator=',', params_format='{params}', cmd_assign=' ', cmd_separator=' '):

    def __init__(self, value_separator=',', param_separator=' '):
        super().__init__(cmd_separator=';\n', value_assign=' ', value_separator=value_separator, param_separator=param_separator)
        #self.VALUE_ASSIGN=' '
        
    def fmt_value(self, value:str) -> str :
        #if isinstance(value, (Enum,Expr)):
        #    value=value.name
        
        if isinstance(value, Enum):
            value=value.value
        elif isinstance(value, Expr):
            value=value.name
        # elif isinstance(value, KeyWord):
        #    logger.warning("KeyWord deprecating...  value: %s", value.name)
        #    value=value.name
        elif isinstance(value, str):
            value=f'"{value}"'
        #return super().fmt_value(value)
        else:
            value = str(value)
        return value

        
class Mika:
    def __init__(self, a=1, b="dko", **global_opts):
        print(a, b)


class Terminal(Enum):
    PNG = "png"
    PDF = "pdf"
    SVG = "svg"

class Datafile(Enum):
    SEPARATOR  = "separator" 
    WHITESPACE = "whitespace" 

class Format(Enum):
    X = "x" 
    Y = "y" 

class Data(Enum):
    TIME = "time"

class Tics(Literal):
    OUT = "out"
    NOMIRROR = "nomirror"
    XTICS = "xtics"
    YTICS = "ytics"

class Key(Enum):
    """Gnuplot legend positioning keywords
    """
    INSIDE   = "inside"
    OUTSIDE  = "outside"
    LEFT     = "left"
    RIGHT    = "right"
    TOP      = "top"
    BOTTOM   = "bottom"
    BOXED    = "boxed"
    BELOW    = "below"
    ABOVE    = "above"  

# Chat 
class KeywordExpr:
    def __init__(self, *parts: Enum):
        self.parts = parts

    def __str__(self):
        return " ".join(p.value for p in self.parts)

    @classmethod
    def from_values(cls, enum_cls, values):
        return cls(*(enum_cls(v) for v in values))
    
class Range:
    def __init__(self, *args):
        self.args = args

    def __str__(self):
        if isinstance(self.args[0], (tuple, list)):
            args = list(self.args[0])
        else:
            args = self.args
        return f"[{':'.join(map(str, args))}]"
        #return f"[{':'.join(map(str, args))} {type(self.args[0])}]"

class Registry(rack.prog.Register):
    """ Collection of GnuPlot 'set' and 'plot' commands.
    """

    # Helper to create 'set' commands

    def comment(self, *args):
        #cmd = rack.prog.Command("# ", [Literal(text)])
        cmd = rack.prog.Command("# ", [Literal(' '.join([str(a) for a in args]))])
        if self.cmdSequence:
            self.cmdSequence.add(cmd)
        return cmd
    

    def make_set_cmd(self, locs: dict):

        # Detect caller function automatically
        caller_name = inspect.stack()[1].function

        # Initialize with caller name
        args = [Literal(caller_name.replace('_',' '))]

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

        # see be below .. 
        if self.cmdSequence:
            self.cmdSequence.add(cmd)
        
        #logger.warning(cmd.to_tuple())
        return cmd
    
    def set(self, *args): #, **opts):
        # General 'set' command support
        # args = [Literal(' '.join([str(a) for a in args]))]
        # cmd = rack.prog.Command("set", Literal(' '.join([str(a) for a in args])))
        cmd = rack.prog.Command("set", [Literal(' '.join([str(a) for a in args]))])
        if self.cmdSequence:
            self.cmdSequence.add(cmd)
        return cmd
        #return self.make_set_cmd(locals()) 
    
    
    def terminal(self, terminal_type: Terminal | str = Terminal.PNG, **opts):
        terminal_type = Terminal(terminal_type) 
        if "size" in opts:
            size = opts.pop("size")
            if isinstance(size, tuple):
                size = ",".join(str(s) for s in size)   
            # size = Literal(f"size {size[0]},{size[1]}")
            #else: # if size is not None:
            size = Literal(f"size {size}")
        
        #opts["size"] = size

        return self.make_set_cmd(locals())

    def output(self, filename: str):
        return self.make_set_cmd(locals())

    def title(self, title: str):
        return self.make_set_cmd(locals())

    
    def datafile(self, key: Datafile|str, value: Datafile|str, **opts) :
        key = Datafile(key)
        value = Datafile(value)
        return self.make_set_cmd(locals()) 
    
    def grid(self, *args): #, **opts):
        return self.set("grid", *args)
        #args = Literal(' '.join([str(a) for a in args]))
        #return self.make_set_cmd(locals()) 
    
    def xdata(self, mode: Data|str, **opts) : 
        mode = Data(mode)
        return self.make_set_cmd(locals()) 
    
    def xlabel(self, label: str, **opts) : 
        return self.make_set_cmd(locals()) 
    
    def ylabel(self, label: str, **opts) : 
        return self.make_set_cmd(locals()) 


    def xrange(self, *args) :
        args = Range(*args)
        return self.make_set_cmd(locals())
    
    def yrange(self, *args) :
        args = Range(*args)
        return self.make_set_cmd(locals())

    def timefmt(self, fmt: str) : 
        return self.make_set_cmd(locals()) 

    def format_x(self, fmt: str) : 
        #fmt = Format(fmt)
        return self.make_set_cmd(locals()) 
        #return self.make_set_cmd(locals()) 
    
    def format_y(self, fmt: str) : 
        #fmt = Format(fmt)
        return self.make_set_cmd(locals()) 
        
    def label(self, text: str, **opts) : 
        return self.make_set_cmd(locals()) 
    
    def key(self, position: Key|str, **opts) : 
        #def key(self, position, **opts) : 
        if isinstance(position, str):
            position = position.strip().split(' ')
        if isinstance(position, Enum):
            position = [position.value]

        position = [Key(p).value for p in position]
        position = Literal(' '.join(position))
        return self.make_set_cmd(locals()) 


    def multiplot(self, rows: int = 1, cols: int = 1, title: str = "") : 
        # cmds = [GnuPlotCommand("set", "multiplot", f"layout {rows},{cols}")]
        # if title:
        #    cmds.append(GnuPlotCommand("set", "title", title))
        return self.make_set_cmd(locals()) #  cmds
    
    #def unset_multiplot(self) : 
    #    return self.make_set_cmd(locals()) 

    """
    def tics(self, *args: Tics|str, **opts) :
        tics = []
        for arg in args:
            if isinstance(arg, str):
                arg = arg.strip().split(' ')
            if isinstance(arg, Enum):
                arg = [arg.value]

            tics.extend(Tics(a).value for a in arg)
        
        tics_expr = Literal(' '.join(tics))
        return self.make_set_cmd({"arg": tics_expr})  
    """

    def unset(self, what: str) :
        what = Literal(what)
        return self.make_set_cmd(locals()) 
        

    # Plot command support

    class plot_entry():

        src=None
        opts = {}

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


        def __str__(self):
            fmt = GnuPlotFormatter()
            src  = fmt.fmt_value(self.src) # Quote strings, but pass expressions as such
            opts = fmt.fmt_params(self.opts)
            return f"{src} {opts}"


        def is_set(self):
            return (self.src != None)


    #@rack.prog.copy_signature_from(Mika.__init__)
    #def mika(self, *plots, **plot_kwargs):
    #    if plot_kwargs:
    #        plots += (Mika(**plot_kwargs), )
    #    return plots

    def plot(self, *items: Union[str, dict], expr=None, filename=None, style=None, **global_opts):
        """Create a plot command with multiple items.
        Each item can be a string (expression) or a dict with 'expr' or 'file' key and options.
        Global options apply to the entire plot command.

        Parameters

        """
        if (style):
            global_opts["with"] = style

        #m = Mika(a=5, b="87")
        #self.mika(a=6, b="s")
        
        #logger.warning(str(entry))
        #logger.warning(items)
        #logger.warning(global_opts)
        
        cmd = None

        # Multiple plot dicts
        segments = []
        
        # Try to create a single entry from direct parameters
        entry = self.plot_entry(expr=expr, filename=filename, style=style, **global_opts)
        if entry.is_set():
            segments = [entry]
            if items:
                logger.error("Ambiguous parameters: both item list and a directly defined plot entry")
        else:
            for item in items:
                #logger.info(f"item={item} ({type(item)})")
                if isinstance(item, self.plot_entry):
                    segments.append(item)
                elif isinstance(item, str):
                    segments.append(self.plot_entry(item, style=style))
                else:
                    raise TypeError("Each plot item must be a string or dict with 'expr'")
        
        cmd = rack.prog.Command("plot", segments) #  there are no super_global_opts 
        if self.cmdSequence:
            self.cmdSequence.add(cmd)
        return cmd

class LineStyle:
    
    linetype: int
    linewidth: int
    linecolor: str
    pointtype: int
    pointsize: int
    fillstyle: str
    

    def __init__(self, index: int):
        self.index = index

    def __str__(self):
        return f"linetype {self.index}" 
    

class Style(Enum):
    """ In gunplot, after "with" keyword
    """

    LINES = "lines"
    DOTS  = "dots"
    LINES_DOTS = "linesdots"
    RGBIMAGE = "rgbimage"
    LINESPOINTS = "linespoints"
    POINTS = "points"


class ConfSequence(rack.prog.CommandSequence):
    
    fmt = GnuPlotFormatter(param_separator=' ', value_separator=',')

    def to_string(self, fmt = None):
        if not fmt:
            fmt = self.fmt
        return super().to_string(fmt)

class PlotSequence(rack.prog.CommandSequence):
    
    fmt = GnuPlotFormatter(param_separator=',\n  ')

    def to_string(self, fmt = None):
        if not fmt:
            fmt = self.fmt
        return super().to_string(fmt)


def main():

    
    prog_conf = ConfSequence()

    conf = Registry(prog_conf)
    #reg.terminal(GnuPlot.Terminal.PNG, size=(800, 600))
    conf.terminal(Terminal.PNG, size=(800, 600))
    conf.output("my-file.png")
    #reg.set("terminal", GnuPlot.Terminal.PNG, size=(800, 600))

    conf.datafile(Datafile.SEPARATOR, Datafile.WHITESPACE)
    #reg.datafile(Datafile.SEPARATOR, char=Datafile.WHITESPACE)
    conf.xdata(Data.TIME)
    conf.timefmt("%s")
    # reg.format_x(Format.X, "%H:%M")
    conf.format_x("%H:%M")
    conf.grid()
    conf.title("statistics 00min 20140525-1200")
    conf.xlabel("TIME START REL")
    conf.ylabel("ELANGLE")

    prog_plot = PlotSequence()
    plot = Registry(prog_plot)
    plot.plot("sin(x)")
    plot.plot("sin(x) with lines")
    plot.plot(expr="sin(x)")
    plot.plot(expr="sin(x)", style=Style.LINES)
    plot.plot(filename="data.txt", style=Style.DOTS)
    plot.plot("sin(x)", "cos(x)", "tan(x)", style=Style.LINES)

    e1 = plot.plot_entry(expr="random(x)", style=Style.LINES_DOTS)
    plot.plot(e1)
    e2 = plot.plot_entry(filename="my_file.dat", style=Style.DOTS, color="brown")
    #reg.plot(["cos(x)"])
    plot.plot(e2)
    plot.plot(e1,e2)

    #print(prog_conf.to_debug())
    #print ("# -----")
    #print(prog_plot.to_debug())

    # Save to file
    #prog.to_script("example_plot.plt")
    #fmt = GnuPlotFormatter()

    print("# Generated GnuPlot script:\n")
    print(prog_conf.to_string())  # ";\n"
    #fmt.PARAM_SEPARATOR = ",\\\n\t"
    print(prog_plot.to_string())  # ";\n"



if __name__ == "__main__":
    main()
