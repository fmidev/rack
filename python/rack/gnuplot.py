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

    def __repr__(self):
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

    def __init__(self, param_separator=' '):
        self.VALUE_ASSIGN=' '
        self.VALUE_SEPARATOR=','
        self.PARAM_SEPARATOR=param_separator
        self.CMD_SEPARATOR=';\n'

    def fmt_value(self, value:str) -> str :
        if isinstance(value, (KeyWord,Expr)):
            value=value.name
        elif isinstance(value, str):
            value=f'"{value}"'
        return super().fmt_value(value)

        
class Mika:
    def __init__(self, a=1, b="dko", **global_opts):
        print(a, b)


class Registry(rack.prog.Register):
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


    @rack.prog.copy_signature_from(Mika.__init__)
    def mika(self, *plots, **plot_kwargs):
        if plot_kwargs:
            plots += (Mika(**plot_kwargs), )
        return plots

    def plot(self, *items: Union[str, dict], expr=None, filename=None, style=None, **global_opts):
        """Create a plot command with multiple items.
        Each item can be a string (expression) or a dict with 'expr' or 'file' key and options.
        Global options apply to the entire plot command.

        Parameters

        """
        if (style):
            global_opts["with"] = style

        m = Mika(a=5, b="87")
        self.mika(a=6, b="s")
        
        #logger.warning(str(entry))
        #logger.warning(items)
        #logger.warning(global_opts)
        
        cmd = None

        # Multiple plot dicts
        segments = []
        
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
        #logger.warning("Coming up: ENTRIES!")
        #logger.warning(cmd.args)
        if self.cmdSequence:
            self.cmdSequence.add(cmd)
        return cmd

    

class Style:
    """ In gunplot, after "with" keyword
    """

    LINES = KeyWord("lines")
    DOTS  = KeyWord("dots")
    LINES_DOTS = KeyWord("linesdots")


class ConfSequence(rack.prog.CommandSequence):
    
    def to_string(self, fmt = GnuPlotFormatter(param_separator=' ')):
        return super().to_string(fmt)

class PlotSequence(rack.prog.CommandSequence):
    
    def to_string(self, fmt = GnuPlotFormatter(param_separator=',')):
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
