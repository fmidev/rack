import argparse
import inspect
import pathlib
from typing import Any, List, Dict, Union
import numbers

#from collections import OrderedDict
from rack.command import Command
import rack.base
import rack.args

from rack.formatter import Formatter, ParamFormatter
from rack.command import CommandSequence

logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)
from typing import Protocol



class RackModule(Protocol):
    def compose_command(args) -> CommandSequence:
        ...

    def build_parser() -> argparse.ArgumentParser:
        ...


class Composer():
    """ Relies that a module has the following commands:
        - build_parser()
        - compose_command(self.args)
        where args are obtained as self.args = parser.parse_args()

    """

    module = None
    parser = None
    args = None

    def __init__(self, module:RackModule):
        self.module = module
        self.parser:argparse.ArgumentParser = module.build_parser()
        
        known_args, unknown_args = self.parser.parse_known_args()
        self.args = known_args
        #logger.warning(known_args)
        logger.debug(f"unknown_args: {unknown_args}")

    def set(self, **argv):
        if not self.args:
            raise RuntimeError("args undefined")
        vars(self.args).update(argv)

    def get_prog(self) -> List[Command]:
        return self.module.compose_command(self.args)
    
    def get_defaults(self):
        return {a.dest: a.default for a in self.parser._actions if a.dest != 'help'}  

    def get_module_cmd_line(self, separator=" ") -> str:
        """ Return module-specific command line
        """
        line = rack.args.args_to_cli(self.parser, self.args, separator=separator)
        logger.debug(f"Command line: {line}")
        logger.debug(self.module.__str__)
        return line
    
    def get_module_name(self) -> str:
        return self.module.__name__

class RackFormatter(Formatter):
    """A formatter for Rack commands (command line arguments). Overrides some of the default formatting rules."""
    
    def fmt_name(self, name:str) -> str :
        l = len(name)
        if l==0: # Default command (--inputFile ...) works like this - needs no key.
            return ""
        elif l==1:
            return f"-{name}"
        else:
            return f"--{name}"
        #return self.NAME_FORMAT.format(name=name)

    #def fmt_params(self, arg_dict={}, key_list=None)->str :
    #    param_list = self.get_param_lst(arg_dict, key_list)
    #    params = self.PARAM_SEPARATOR.join(param_list)
    #    return self.PARAMS_FORMAT.format(params=params)

    def get_param_lst(self, args={}, key_list=None)->list:
        """ Return a list of strings of key-value entries. By default, 
            in this base class implementation, such entry 
            is "key=value", or only the value if the key is numeric. 
        """
        if key_list == None:
            # Return all
            return [self.fmt_param(v,k) for k,v in args.items()]
        else: 
            # Note: key_list can be empty.
            result = []
            key_map = dict(enumerate(args.keys())) # list(arg_dict.keys())))
            
            compact = True
            
            for (key, key_orig) in zip(key_list, args.keys()):
                if isinstance(key, (int, float)):
                    key = key_map[key]
                    #result.append(self.fmt_param(args[key_map[k]], None))
                value = args[key]
                if (key == key_orig) and compact:
                    key = None
                else:
                    compact = False
                result.append(self.fmt_param(value,key))
            
            """
            if len(key_map) == 1:
                key0 = key_map[0]
            else:
                key0 = None

            for k in key_list:
                if isinstance(k, (int, float)):
                    result.append(self.fmt_param(args[key_map[k]], None))
                elif k==key0:
                    result.append(self.fmt_value(args[k]))  
                else:
                    result.append(self.fmt_param(args[k],k))
                key0 = None                        
            """

            return result

#import rack.composite

def main():

    #bd = Composer(rack.composite)
    #print(bd)
    
    class MyRegister(rack.prog.Register):

        def sample(
            self,
            iStep: int = 10,
            jStep: int = 0,
            i: tuple = (-1, 1),
            j: tuple = (-1, 1),
            commentChar: str = "#",
            skipVoid: bool = False,
            handleVoid = None,
        ):
            return self.make_cmd(locals())


    reg = MyRegister()
    cmd = reg.sample(i=(-2, 5), iStep=1)
    cmd.PRIMARY_SEP='|'
    cmd.SECONDARY_SEP='/'
    print(cmd)   # assuming MyCommand.__str__ formats tuples as "a:b" and joins with commas
    
    prog = CommandSequence(Command)
    prog.add(cmd)

    cmd = Command("unary")
    prog.add(cmd)
    cmd = Command("anon", "value", {"key": "value"})
    prog.add(cmd)
    cmd = Command("anon", ["first", "second", "third"])
    prog.add(cmd)
    cmd = Command("anon", ["first", "second", "third"])
    cmd.set_args("a","b")
    prog.add(cmd)

    cmd = Command("normal", {"key": "value", "key2": "value2"} )
    cmd.set_args(key2="b2")
    prog.add(cmd)

    #print(cmd)

    #print(prog.to_string())
    #print(prog)
    print(prog.to_list())
    print(prog.to_list(fmt=Formatter(cmd_separator="\n")))
    #print(prog.to_token_list())

    def test(a:int=-1, b:str="?", *args, **kwargs):
        print (f"a={a}, b={b} | {args} | {kwargs}")

    test("Mika")
    test("Mika","mäki", "the third")
    test("Mika","mäki", key2="the value")
    test("Mika", key2="the value")
    test(b="the value")
    test(b="the value", key3="thirdo")

if __name__ == "__main__":
    main()
