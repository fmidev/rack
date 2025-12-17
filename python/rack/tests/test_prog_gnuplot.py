import unittest
import rack.gnuplot as gp
#
import logging
logger = logging.getLogger(__name__)    

class TestCommand(unittest.TestCase):

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
    conf = gp.Registry()


    # Test set commands and their formatting

    def test_set(self):
        """
        Ensure that Registry hosting a ConfSequence can be created.
        """
        seq = gp.ConfSequence()
        reg = gp.Registry(seq)
        self.assertIsNotNone(reg)

    def test_set_terminal(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.terminal(gp.Terminal.PNG, size=(800, 600))
        expected_str = "set terminal png size 800,600"
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)

    def test_set_output(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.output("my-file.png")
        expected_str = 'set output "my-file.png"'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  

    def test_set_datafile(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.datafile(gp.Datafile.SEPARATOR, gp.Datafile.WHITESPACE)
        expected_str = 'set datafile separator whitespace'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  

    def test_set_xdata(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.xdata(gp.Data.TIME)
        expected_str = 'set xdata time'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  

    def test_set_timefmt(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.timefmt("%s")
        expected_str = 'set timefmt "%s"'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  

    def test_set_format_x(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.format_x("%H:%M")
        expected_str = 'set format x "%H:%M"'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)

    def test_set_grid(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.grid()
        expected_str = 'set grid'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  

    def test_set_title(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.title("statistics 00min 20140525-1200")
        expected_str = 'set title "statistics 00min 20140525-1200"'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)

    def test_set_xlabel(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.xlabel("TIME START REL")
        expected_str = 'set xlabel "TIME START REL"'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)

    def test_set_ylabel(self):
        """
        Ensure that 'set' command is formatted correctly.
        """
        cmd = self.conf.ylabel("ELANGLE")
        expected_str = 'set ylabel "ELANGLE"'
        self.assertEqual(cmd.to_string(gp.ConfSequence.fmt), expected_str)  


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
        print(expected_str)
        cmd = self.conf.plot(*args, **kwargs)
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
        self.check_plot_command('plot sin(x) with lines,cos(x) with lines,tan(x) with lines',
                             "sin(x)", "cos(x)", "tan(x)", style=gp.Style.LINES)
    
    # plot.plot("sin(x)", "cos(x)", "tan(x)", style=Style.LINES)

    def test_plot_entries(self):
        """
        Check formatting of multiple plot entries.
        """

        e1 = self.conf.plot_entry(expr="random(x)", style=gp.Style.LINES_DOTS)
        #expected_str = 'random(x) with linesdots'
        #self.assertEqual(e1.to_string(gp.PlotSequence.fmt), expected_str)

        e2 = self.conf.plot_entry(filename="my_file.dat", style=gp.Style.DOTS, color="brown")
        #expected_str = '"my_file.dat" with dots color brown'
        #self.assertEqual(e2.to_string(gp.PlotSequence.fmt), expected_str)   
 
        cmd = self.conf.plot(e1, e2)
        expected_str = 'plot random(x) with linesdots,"my_file.dat" with dots color "brown"'
        self.assertEqual(cmd.to_string(gp.PlotSequence.fmt), expected_str)  


    def test_plot_expr_and_datafile_fail(self):
        """
        Ensure that 'plot' command does not allow both expr and filename.
        """
        self.assertRaises(KeyError, self.conf.plot, expr="random(x)", filename="my_file.dat")  



if __name__ == "__main__":
    
    prog_conf = gp.ConfSequence()

    conf = gp.Registry(prog_conf)
    cmd = conf.terminal(gp.Terminal.PNG, size=(800, 600))
    print(cmd.to_string())
    #unittest.main() 