
import unittest
#import datetime as dt

import rack.prog
import rack.composite
import argparse

import logging
#logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
logging.basicConfig(format='%(levelname)s:\t %(message)s')
logger = logging.getLogger() 
logger.setLevel(logging.INFO)

class TestComposer(unittest.TestCase):
    """
        Note: command line arguments of 'unittest' confuse or corrupt the initialisation
        of rack.prog.RackModule 
    """

    fmtCLI = rack.prog.RackFormatter(params_format="'{params}'", cmd_separator=" \\\n\t")

    def test_basic(self):
        logger.warning("How!")
        parser:argparse.ArgumentParser = rack.composite.build_parser()
        dir(parser)
        logger.warning("How!")
        logger.warning(parser)
        try:
            known_args,unknown_args  = parser.parse_known_args() #parser.parse_args()
        except Exception as e:
            logger.error(e)

    def test_empty(self):

        composer = rack.prog.Composer(rack.composite)
        composer.set(INFILE='volume.h5')
        composer.set(OUTFILE='out.h5')
        prog = composer.get_prog()        
        self.assertTrue(isinstance(prog, rack.prog.CommandSequence))
        prog:rack.prog.CommandSequence = prog
        print (prog.to_string(self.fmtCLI))
        print (prog.to_list(self.fmtCLI))
        print (composer.get_module_cmd_line())

    def test_simple(self):

        cmdline = rack.prog.Composer(rack.composite)
        cmdline.set(INDIR='/tmp/', OUTFILE='out.h5')
        cmdline.set(SCHEME='TILED')
        cmdline.set(INFILE='volume.h5')
        prog:rack.prog.CommandSequence = cmdline.get_prog()
        fmt = rack.prog.RackFormatter(params_format="'{params}'", cmd_separator=" \\\n\t")
        print (prog.to_string(fmt))
        self.assertEqual(0, 0)  



if __name__ == "__main__":
    unittest.main()
