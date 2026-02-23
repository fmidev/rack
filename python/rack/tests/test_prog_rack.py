import unittest
import rack.prog
import rack.core


class TestCommand(unittest.TestCase):

    fmt = rack.prog.RackFormatter(params_format="'{params}'")

    def test_command_plain(self):
        """
        Ensure leading double hyphen.        
        """
        cmd = rack.prog.Command("help")
        expected_str = "--help"
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    """
    --select, -s <path>,<quantity>,<elangle>,<count>,<order>,<prf>,<timespan> 
        def select(self,
            path:str='',
            quantity:str='',
            elangle:list=[-90,90],
            count:int=4095,
            order:str='DATA:MIN',
            prf:str='ANY',
            timespan:list=[0,0]):

    Mode 1: args is a list of values and explicit_args is a dict
    Mode 2: args is a dict and explicit_args is a list of keys
    """

    def test_command_single_arg(self):
        """
        Plain argument appears as is?
        """
        cmd = rack.prog.Command("select", "/dataset1:5")
        expected_str = "--select '/dataset1:5'"
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    def test_command_multiple_args(self):
        """
        Plain arguments are joined correctly?        
        """
        cmd = rack.prog.Command("select", ["/dataset1:5", "elangle=0:15.0", "prf=ANY"])
        expected_str = "--select '/dataset1:5,elangle=0:15.0,prf=ANY'"
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    def test_command_dict(self):
        """
        All the dict arguments appear, in order. 
        """
        cmd = rack.prog.Command("select", {"quantity":'DBZH', "elangle":(0.5,10.0), "prf": "ANY" })
        expected_str = "--select 'DBZH,0.5:10.0,ANY'" # Note colon separator for range
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    def test_command_values_and_kwargs(self):
        """
        Plain argument first, then dict argument values in order?
        """
        cmd = rack.prog.Command("select", "/dataset1:5", {"quantity":'DBZH', "elangle":(0.5,10.0), "prf": "ANY" })
        expected_str = "--select '/dataset1:5,DBZH,0.5:10.0,ANY'" # Note colon separator for range
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    def test_command_kwargs_and_keys(self):
        """
        Given a dict of arguments and a list of keys, only respective values appear, in that order.
        """
        cmd = rack.prog.Command("select", {"quantity":'DBZH', "elangle":(0.5,10.0), "prf": "ANY" }, ["elangle", "prf"])
        expected_str = "--select 'elangle=0.5:10.0,prf=ANY'" # Note colon separator for range
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    
    def test_prog_pseudo_rhi(self):
        """
        Given a dict of arguments and a list of keys, only respective values appear, in that order.
        """
        fmt = rack.prog.RackFormatter(params_format="'{params}'")
        rackProg = rack.prog.CommandSequence(programName='rack', quote="'")
        Rack = rack.core.Rack(rackProg)
        Rack.inputFile('volume.h5')
        Rack.pPseudoRhi(168, range=[-50000,250000], height=[0,15000])
        Rack.outputFile('pseudo-rhi.h5')
        cmds = rackProg.to_list(fmt)
        self.assertEqual(cmds, ['rack',
                                "'volume.h5'",
                                "--pPseudoRhi '168,-50000:250000,0:15000'",
                                "--outputFile 'pseudo-rhi.h5'"])



    """
    def test_command_dict_explicit(self):
        cmd = rack.prog.Command("verbose", {"level":"NOTE", "imageLevel":"WARNING"}, ["level", "imageLevel"])
        expected_str = "--verbose 'NOTE,WARNING'"
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    def test_command_dict(self):
        cmd = rack.prog.Command("verbose", {"level":"NOTE", "imageLevel":"WARNING"}, ["level", "imageLevel"])
        expected_str = "--verbose 'NOTE,WARNING'"
        self.assertEqual(cmd.to_string(self.fmt), expected_str)

    def test_command_to_string(self):
        cmd = rack.prog.Command("myprog", ["-a", "value1", "--flag"], {"opt1": "val1", "opt2": "val2"})
        expected_str = "myprog -a value1 --flag --opt1 val1 --opt2 val2"
        self.assertEqual(cmd.to_string(), expected_str)


    def test_command_only_opts(self):
        cmd = rack.prog.Command("optprog", opts={"debug": None, "output": "file.txt"})
        expected_str = "optprog --debug --output file.txt"
        self.assertEqual(cmd.to_string(), expected_str)

    def test_command_only_args(self):
        cmd = rack.prog.Command("argprog", args=["input.txt", "output.txt"])
        expected_str = "argprog input.txt output.txt"
        self.assertEqual(cmd.to_string(), expected_str)
    """
