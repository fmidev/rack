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
    """ Protocol for Rack modules. A Rack module is a Python module that defines a set of commands 
    and a way to compose them into a command sequence. 
    
    It should have the following methods:
    - compose_command(args) -> CommandSequence: takes the parsed arguments and returns a CommandSequence
    - build_parser() -> argparse.ArgumentParser: returns an ArgumentParser for the module-specific arguments.
    """
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

    def set(self, strict=True,**argv): # strict=False): # todo strict=True: only allow keys defined in parser
        if not self.args:
            raise RuntimeError("args undefined")
        v = vars(self.args)
        for k in argv:
            if k not in v:
                msg = f"Unknown argument '{k}'." # It is not defined in the parser."
                if strict:
                    raise ValueError(msg)
                else:
                    logger.warning(msg)
        v.update(argv)
        #vars(self.args).update(argv)

    def get_prog(self) -> List[Command]:
        return self.module.compose_command(self.args)
    
    def get_defaults(self):
        return {a.dest: a.default for a in self.parser._actions if a.dest != 'help'}  

    def get_module_cmd_line(self, separator=" ") -> str:
        """ Return module-specific command line
        """
        test = rack.args.args_to_list(self.parser, self.args)
        #test = rack.args.args_to_cli(self.parser, self.args, separator=" XX ")
        logger.warning(f"Command line: {test}")
        
        line = rack.args.args_to_cli(self.parser, self.args, separator=separator)
        logger.debug(f"Command line: {line}")
        logger.debug(self.module.__str__)
        return line
    
    def get_module_name(self) -> str:
        return self.module.__name__

def run_module(module):
    """Drive a RackModule as a standalone program.

    Handles the common main() flow shared by all profile plot modules:
    parse args, compose the rack command sequence, print and/or execute it,
    then run gnuplot if a script was generated.

    Usage in any conforming module's main():
        def main():
            rack.cmdline.run_module(sys.modules[__name__])
    """
    import subprocess
    import sys
    import rack.log
    import rack.args
    from rack.plot_common import load_config, export_defaults_to_json

    parser = module.build_parser()
    rack.log.add_parameters(parser)

    # Apply JSON config if --config was given (before full parse)
    known_args, _ = parser.parse_known_args()
    if getattr(known_args, 'config', None):
        parser.set_defaults(**load_config(known_args.config))

    args = parser.parse_args()

    if getattr(args, 'export_config', None):
        export_defaults_to_json(parser, args, args.export_config)
        sys.exit(0)

    if getattr(args, 'test', False):
        logger.info("Running tests..")
        sys.exit(0)

    prog = module.compose_command(args)

    if getattr(args, 'print', None):
        sep = args.print.replace(r'\t', '\t').replace(r'\n', '\n')
        logger.info("# Rack cmd line:")
        fmt = RackFormatter(params_format="'{params}'", cmd_separator=sep)
        print(prog.to_string(fmt))

    if getattr(args, 'exec', False):
        logger.info("# Executing Rack...")
        fmt = RackFormatter(params_format="'{params}'")
        logger.debug(prog.to_string(fmt))
        fmt = RackFormatter()
        cmd = prog.to_token_list(fmt)
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if result.returncode != 0:
            if result.stdout:
                logger.info(f"stdout:\n{result.stdout.rstrip()}")
            if result.stderr:
                logger.warning(f"stderr:\n{result.stderr.rstrip()}")
            logger.error(f"Command exited with code {result.returncode}")

        if getattr(args, 'gnuplot_script', None):
            cmd = ["gnuplot", args.gnuplot_script]
            logger.info(f"Executing GnuPlot script: {cmd}")
            subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    line = rack.args.args_to_cli(parser, args)
    logger.debug(f"Python command line args: {line}")


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
