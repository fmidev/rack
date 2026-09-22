import unittest
#import datetime as dt

#import rack.typical

import logging

from rack import prog
import rack
#logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
logging.basicConfig(format='%(levelname)s:\t %(message)s')
logger = logging.getLogger() 
logger.setLevel(logging.INFO)

class TestTypical(unittest.TestCase):
    """
        Note: command line arguments of 'unittest' confuse or corrupt the initialisation
        of rack.prog.RackModule 
    """

    def diagnose_OLD(self, x):
        logger.info(f"diagnosing: *{x}*")
        logger.info(type(x))
        logger.info(x)
        logger.info(x.__repr__())
        logger.info(x.__str__())

    def test_identity(self):
        for i in [None, 42, 3.14, "hello"]:
            self.assertEqual(rack.typical(i), i)

    def test_str_to_tuple(self):
        """
        String to tuple of strings with separators
        """
        self.assertEqual(rack.typical("640,400", tuple),      ('640','400')) 
        self.assertEqual(rack.typical("640,400", tuple, ','), ('640','400')) 
        self.assertEqual(rack.typical("640:400", tuple, ':'), ('640','400')) 

    def test_str_to_list_with_separators(self):
        """
        String to tuple of strings with more complex separators
        """
        self.assertEqual(rack.typical("640:400",       [str],   ':'),    ['640', '400'] ) 
        self.assertEqual(rack.typical(" 640.0:400.0 ", [float], '[,: ]'), [640.0, 400.0]) 
        self.assertEqual(rack.typical(" 640.0,400.0 ", [float], '[,: ]'), [640.0, 400.0]) 
        self.assertEqual(rack.typical(" 640.0 400.0 ", [float], '[,: ]'), [640.0, 400.0]) 

    def test_str_to_tuple_float(self):
        """
        String to tuple of floats
        """
        #x = rack.typical("640,400", (float,))
        #self.diagnose(x)
        self.assertEqual(rack.typical("640,400", (float,)), (640.0, 400.0)) 

    def test_str_to_list_int(self):
        """
        String to 
        """
        self.assertEqual(rack.typical("640,400", [int]), [640, 400]) 

    def test_tuple_to_list(self):
        """
        Tuple to list
        """
        self.assertEqual(rack.typical(('640','400'), list), ['640','400']) 
        self.assertEqual(rack.typical(( 640 , 400 ), list), [ 640 , 400 ]) 
        #self.diagnose(rack.typical(('640','400'), [float]))
        self.assertEqual(rack.typical(('640','400'), [float]), [640.0,400.0]) 


    def test_list_to_set(self):
        """
        List to set
        """
        self.assertEqual(rack.typical(['640','400'], set), {'640','400'}) 


    def old_complex(self):
        """
        Test empty configuration
        """
        #x = "640,400"
        result = rack.typical("640,400", tuple, ',')
        correct = ('640','400')
        logger.info(f"Result: {result}, correct: {correct}")
        self.assertEqual(result, correct)        
        # self.assertTrue(isinstance(prog, rack.prog.CommandSequence))
