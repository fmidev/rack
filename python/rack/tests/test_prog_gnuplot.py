import unittest
import rack.gnuplot as gp
#
import logging
logger = logging.getLogger(__name__)    

class TestGnuPlot(unittest.TestCase):

    """
    set terminal png size 800,600;
    set output "my-file.png";
    set datafile separator whitespace;
    set xdata time;
    set timefmt "%s";
    set format x "%H:%M";
    set grid;
    set title "statistics 00min 20140525-1200";
    set xlabel "TIME START REL";
    set ylabel "ELANGLE"
    plot sin(x) ;
    plot sin(x) with lines ;
    plot sin(x) ;
    plot sin(x) with lines;
    plot "data.txt" with dots;
    plot sin(x) with lines,cos(x) with lines,tan(x) with lines;
    plot random(x) with linesdots;
    plot "my_file.dat" with dots color "brown";
    plot random(x) with linesdots,"my_file.dat" with dots color "brown"

    """

    #prog_conf = gp.ConfSequence()
    #conf = gp.Registry(prog_conf)
    reg = gp.Registry()


    # Test set commands and their formatting

    def test_set(self):
        """
        Ensure that Registry hosting a ConfSequence can be created.
        """
        seq = gp.ConfSequence()
        reg = gp.Registry(seq)
        self.assertIsNotNone(reg)

    from typing import Any, List, Union

    def check_set_command(self, expected_str, cmd: Union[str,callable], *args, **kwargs):
        """ Helper to check plot command formatting.
        """
        #logger.warning("Checking plot command with args=%s, kwargs=%s", args, kwargs)
        print(f'\n --> {expected_str}')
        if type(cmd) == str:
            cmd = getattr(self.reg, cmd)    
        if not callable(cmd):
            raise Exception(f"type(cmd) = {type(cmd)}, must be a function or a string")
        # cmd = getattr(self.conf, cmdName)(*args, **kwargs)
        cmd = cmd(*args, **kwargs)
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)

    def test_set_terminal(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        #logger.warning(self._testMethodName)
        self.check_set_command("set terminal png", self.reg.terminal, gp.Terminal.PNG)
        self.check_set_command("set terminal png size 640,400", "terminal", gp.Terminal.PNG, size="640,400")
        self.check_set_command("set terminal png size 800,600", "terminal", gp.Terminal.PNG, size=(800, 600))
        #self.check_set_command("set terminal png size 800,600", "terminal", gp.Terminal.PNG, size=[800, 600])
        # OLD STYLE:
        # cmd = self.conf.terminal(gp.Terminal.PNG, size=(800, 600))
        # expected_str = "set terminal png size 800,600"
        # self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)

    def test_set_output(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        self.check_set_command('set output "my-file.png"', "output", "my-file.png")

    def test_set_datafile(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        self.check_set_command('set datafile separator whitespace', self.reg.datafile, gp.Datafile.SEPARATOR, gp.Datafile.WHITESPACE)

    def test_set_xdata(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        #cmd = self.conf.xdata(gp.Data.TIME)
        #expected_str = 'set xdata time'
        #self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  
        self.check_set_command('set xdata time', self.reg.xdata, gp.Data.TIME)

    def test_set_timefmt(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        # cmd = self.conf.timefmt("%s")
        # expected_str = 'set timefmt "%s"'
        # self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  
        self.check_set_command('set timefmt "%s"', self.reg.timefmt, "%s")

    def test_set_format_x(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        # cmd = self.conf.format_x("%H:%M")
        # expected_str = 'set format x "%H:%M"'
        # self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)
        self.check_set_command('set format x "%H:%M"', self.reg.format_x, "%H:%M")

    def test_set_grid(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        # cmd = self.conf.grid()
        # expected_str = 'set grid'
        # self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  
        self.check_set_command('set grid', self.reg.grid)

    def test_set_title(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        # cmd = self.reg.title("statistics 00min 20140525-1200")
        # expected_str = 'set title "statistics 00min 20140525-1200"'
        # self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)
        self.check_set_command('set title "statistics 00min 20140525-1200"', self.reg.title, "statistics 00min 20140525-1200")

    def test_set_xlabel(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        #cmd = self.reg.xlabel("TIME START REL")
        #expected_str = 'set xlabel "TIME START REL"'
        #self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)
        self.check_set_command('set xlabel "TIME START REL"', self.reg.xlabel, "TIME START REL")

    def test_set_ylabel(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        #cmd = self.reg.ylabel("ELANGLE")
        #expected_str = 'set ylabel "ELANGLE"'
        #self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)
        self.check_set_command('set ylabel "ELANGLE"', self.reg.ylabel, "ELANGLE")

    def test_set_key(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        self.check_set_command('set key outside', self.reg.key, "outside")
        self.check_set_command('set key outside', self.reg.key, gp.Key.OUTSIDE)
        self.check_set_command('set key outside top', self.reg.key, [gp.Key.OUTSIDE, gp.Key.TOP])

    def test_set_from_conf(self):
        """
        Ensure that Registry can create 'set' commands from a configuration dictionary.
        If a value is a list and its last element is a dict, the dict is treated as keyword arguments.
        Similarly, if a value is a set, its elements are treated as positional arguments.
        """

        commands = {
            #"terminal": "png size 800,600",
            "terminal": [
                "png", 
                {"size": "800,600"}],
            "output": '"out.png"',
            "datafile": ["separator", "whitespace"],
            "xdata": "time",
            # "timefmt": '"%Y-%m-%dT%H:%M:%S"', # must match with above TIMESTAMP
            "timefmt": '"%s"', # must match with above TIMESTAMP
            # "format": 'x "%s"',
            # "format": 'x "%H:%M"',
            "format_x": '%H:%M',
            # "format": ["y", '%H:%M'],
            "grid": [],
            "title": '"Measured data (actual timestamp)"',
            "xlabel": '"Time"',
            "ylabel": '"Value"',
            "key": "inside left top",
            # "using": '2:3',
        }

        self.reg.import_commands(commands)

        return True

        """
        for k,v in commands.items():
            func = getattr(self.reg, k)   # resolves Registry.terminal, Registry.output, ...
            # logger.warning(func.__name__)
            # logger.warning(dir(func))
            
            cmd = None
            args_list = []
            kwargs_dict = {}
            
            if isinstance(v, (list, tuple)):
                logger.warning("type(v) = %s", type(v))
                l = len(v)
                if l > 0:
                    last_elem = v[l-1]
                    if isinstance(last_elem, dict):
                        args_list = v[0:l-1]
                        kwargs_dict = last_elem
                    else:
                        args_list = v
                v = None

            elif isinstance(v, set):
                args_list = list(v)
                v = None
            elif isinstance(v, dict):
                kwargs_dict = v
                v = None
            # else: v is a single value (e.g., str, int, bool, ...)

            if not (v is None):
                logger.warning(f"calling {func.__name__} with v={v} args={args_list}, kwargs={kwargs_dict}")
                cmd = func(v, *args_list, **kwargs_dict)
            else:
                logger.warning("calling %s with args=%s, kwargs=%s", func.__name__, args_list, kwargs_dict)
                cmd = func(*args_list, **kwargs_dict)
            logger.warning(cmd.to_string(gp.ConfSequence.fmt))
            # logger.warning(func(v))
            # cmds.add(func(v))
        """

    # Test plot commands and their formatting

    def test_plot(self):
        """
        Ensure that Registry hosting a PlotSequence can be created.
        """
        seq = gp.PlotSequence()
        reg = gp.Registry(seq)
        self.assertIsNotNone(reg)

    def check_plot_command(self, expected_str, *args, **kwargs):
        """ Helper to check plot command formatting.
        """
        #logger.warning("Checking plot command with args=%s, kwargs=%s", args, kwargs)
        print(f'\n --> {expected_str}')
        cmd = self.reg.plot(*args, **kwargs)
        self.assertEqual(cmd.to_string(gp.PlotSequence.fmt), expected_str)

    def test_plot_sin(self):
        self.check_plot_command('plot sin(x) ', "sin(x)") # TODO: trailing space?

    def test_plot_cos(self):
        self.check_plot_command('plot cos(x) with lines', "cos(x)", style=gp.Style.LINES)
        #self.assertEqual(cmd.to_string(gp.PlotSequence.fmt), expected_str)

    def test_plot_datafile(self):
        #cmd = self.conf.plot(filename="data.txt", style=gp.Style.DOTS)
        self.check_plot_command('plot "data.txt" with dots', filename="data.txt", style=gp.Style.DOTS)
        #self.assertEqual(cmd.to_string(gp.PlotSequence.fmt), expected_str)

    def test_plot_multiple(self):
        self.check_plot_command('plot sin(x) with lines,\n  cos(x) with lines,\n  tan(x) with lines',
                             "sin(x)", "cos(x)", "tan(x)", style=gp.Style.LINES)
    
    # plot.plot("sin(x)", "cos(x)", "tan(x)", style=Style.LINES)

    def test_plot_entries(self):
        """
        Check formatting of multiple plot entries.
        """

        e1 = self.reg.plot_entry(expr="random(x)", style=gp.Style.LINES_DOTS)
        #expected_str = 'random(x) with linesdots'
        #self.assertEqual(e1.to_string(gp.PlotSequence.fmt), expected_str)

        e2 = self.reg.plot_entry(filename="my_file.dat", style=gp.Style.DOTS, color="brown")
        #expected_str = '"my_file.dat" with dots color brown'
        #self.assertEqual(e2.to_string(gp.PlotSequence.fmt), expected_str)   
 
        cmd = self.reg.plot(e1, e2)
        expected_str = 'plot random(x) with linesdots,\n  "my_file.dat" with dots color "brown"'
        self.assertEqual(cmd.to_string(gp.PlotSequence.fmt), expected_str)  


    def test_plot_expr_and_datafile_fail(self):
        """
        Ensure that 'plot' command does not allow both expr and filename.
        """
        self.assertRaises(KeyError, self.reg.plot, expr="random(x)", filename="my_file.dat")  



if __name__ == "__main__":
    
    prog_conf = gp.ConfSequence()

    #conf = gp.Registry(prog_conf)
    #cmd = conf.terminal(gp.Terminal.PNG, size=(800, 600))
    #print(cmd.to_string())

    # test_set_from_conf 
    tester = TestGnuPlot()
    # tester.te st_set_from_conf()
    tester.check_set_command('set key outside top', tester.reg.key, [gp.Key.OUTSIDE, gp.Key.TOP])


    #unittest.main() 