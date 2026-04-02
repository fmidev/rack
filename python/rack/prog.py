import argparse
import inspect
import pathlib
from typing import Any, Dict, Union
import numbers

#from collections import OrderedDict
import rack.base
from   rack.command   import Command, CommandSequence
from   rack.formatter import Formatter
from   rack.style     import *

logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)

import inspect
#from functools import wraps

def copy_signature_from(source):
    """ UNUSED?
    Copy signature from another callable (except 'self' in methods)."""

    def decorator(target):
        sig = inspect.signature(source)
        params = list(sig.parameters.values())
        
        # Remove 'self' if it exists in the source signature
        if params and params[0].name == "self":
            params = params[1:]
        
        # Add *args at beginning if target already has it
        target_sig = inspect.signature(target)
        new_params = list(target_sig.parameters.values())
        
        # Keep the original *args, but append the copied kwargs
        new_params += params
        #new_params.extend(params)
        
        new_sig = inspect.Signature(parameters=new_params)
        # new_sig = inspect.Signature(new_params)
        target.__signature__ = new_sig
        
        return target
    return decorator


class Register:
    """A command register, for example, for a specific program or a specific group of commands."""

    def __init__(self, cmdSequence:CommandSequence=None):
        """ Create instance of a command Register.

        If a commandSequence is give, every call adds a command to it. 
        """
        self.cmdSequence:CommandSequence = cmdSequence

    def getCmdSequence(self) -> CommandSequence:
        return self.cmdSequence

    def is_default(func, key, value) -> bool:
        """ Return True, if a given value for this key is the default. 
        """
        sig = inspect.signature(func)
        param = sig.parameters[key]
        if param.default is inspect._empty:
            return False
        else:
            return value==param.default


    def make_cmd(self, local_vars:dict, separator:str="") -> Command:
        """ Creates a command. The name will be the caller function
        
        Each explicitly given argument will be stored if its value differs from the default one.
        See rack.core 
        Typical usage, inside a function that identifies with a command: cmd = self.make_cmd(locals())

        """
        
        local_vars.pop('self')
       
        # typelist = [type(v) for v in local_vars.values()]
        # Detect caller function. Then, myFunction(...) implies cmd name "myFunction"
        caller_name = inspect.stack()[1].function
        func = getattr(self, caller_name)
        
        # Function signature
        sig = inspect.signature(func)
        # Detect if default values are numeric, for possible "covering" with a single sequence argument.
        numerics = [isinstance(v.default, numbers.Number) for v in sig.parameters.values()]
        logger.debug(f"{caller_name}: args numeric: {numerics}")


        if caller_name == "xxcBBox":
            for i in inspect.stack()[1]:
                logger.error(i)
            # for i in dir(func):
            #    logger.warning(i) #, getattr(func, i))   
            # TODO: check if 1st arg could "cover" all the args.
            # For example 1) len is 
            # 1) len(arg0) == len(params)
            # 2) type(arg0) in [list,tuple,set]
            # 4) types of arg0[*] are all the same
            # 5) types of N first params are all the same
            # 3) type(params) = type(arg[0]..) match
   
        args = {}
        explicit_args = {}

        
        # Extract only explicit arguments
        FIRST=True
        cmd = None # Needed? See below
        for k,v in local_vars.items():
            #if k != "self":
            args[k]=v
            if not Register.is_default(func, k, v):
                #logger.warning(f"{caller_name}: explicit {k}={v}")
                if FIRST and (type(v) in (tuple,list,set)):
                    # Expanding sequence arg to all args, if sizes match and all are numeric 
                    arg_numerics = [isinstance(i, numbers.Number) for i in v]
                    if (len(numerics) == len(arg_numerics)):
                        similar_sig = {(t1==t2) for (t1,t2) in zip(numerics, arg_numerics)}
                        if (True in similar_sig) and not (False in similar_sig): 
                            #logger.warning(f"Skipping expanding sequence to args : {caller_name} {v}")
                            #logger.warning(f"SEPARATOR: {separator}")
                            style = Style(Color.CYAN, Effect.REVERSE)
                            #logger.warning(style.str(f"Expanding sequence to args : {caller_name} {v} (sep: '{separator}')"))
                            args=v
                            break
                explicit_args[k]=v
            FIRST=False
        
        logger.debug(f"{caller_name}: args={args}, explicit_args={explicit_args}")
        if cmd == None:
            cmd = Command(caller_name, args=args, explicit_args=explicit_args)
        
        if (separator):
            cmd.set_separators(separator)

        if self.cmdSequence:
            self.cmdSequence.add(cmd)

        return cmd
    

    def import_commands(self, commands:Dict[str,Any], separator:str=""):
        """ Creates multiple commands from a dictionary of command names and arguments.
        """

        for k,v in commands.items():

            func = getattr(self, k)   # resolves Registry.terminal, Registry.output, ...
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
            #logger.warning(cmd.to_string(gp.ConfSequence.fmt))

            #if self.cmdSequence:
            #    self.cmdSequence.add(cmd)
    
    @classmethod
    def publish_func(self, func: callable, parser: argparse.ArgumentParser = None, key: str = None, name_mapper = None):
        """ Exports, "explodes" function arguments to an argparse parser. The function name will be the command name,
            and the arguments will be the command arguments.
        
        :param self: Description
        :param func: Description
        :param key: Description
        :param parser: Description
        :type parser: argparse.ArgumentParser
        :param name_mapper: Description

        Consider --pPseudoRHI range=...,height=.... 
        Separate commands --RANGE... --HEIGHT=...
        """
        
        logger.info(f"Exporting args for {func.__name__}")
        
        if isinstance(func, str):
            caller_name = func
            func = getattr(self, caller_name)

        if not inspect.isfunction(func):
            raise TypeError(f"Expected a function, got {type(func)}")

        # Function signature
        sig = inspect.signature(func)
        params = list(sig.parameters.values())
        # Remove 'self' if it exists in the source signature
        if params and params[0].name == "self":
            params = params[1:]

        # Function name 
        if parser:
            if key:
                caller_name = key
            else:
                caller_name = func.__name__
                # This is not done:
                # if name_mapper:
                #    caller_name = name_mapper(caller_name)
            keys = ",".join((p.name for p in sig.parameters.values()))
            parser.add_argument(f"--{caller_name}", metavar=f"<{keys}>", help=f"execute {func.__name__} with given arguments" )


        def camel_to_upper_underscore(name: str) -> str:
            # Insert underscore between lowercase-to-uppercase transitions
            s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
            # Insert underscore between lower/number-to-uppercase transitions
            s2 = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1)
            return s2.upper()
        
        if name_mapper == True:
            logger.warning(f"No name mapper given for {func.__name__}, using default (camel to upper-underscore)")
            name_mapper = camel_to_upper_underscore

        if not name_mapper:
            return

        for v in params:
            name = v.name
            logger.debug(f"{func.__name__}: arg {v.name} default: {v.default}")
            if name_mapper:
                logger.debug(f"mapping {v.name} to {name_mapper(v.name)}")
                name = name_mapper(v.name)
                #v.name = name_mapper(v.name)
            
            if parser:
                args = {"dest":v.name, "type":str, #"nargs":argparse.REMAINDER,
                        "help": f"equals --{caller_name} {v.name}=..."}

                if v.default is not inspect._empty:
                    args["default"] = v.default
                    # args["default"] = str(v.default)
                    default_str = ""
                    if isinstance(v.default, str) and v.default.strip() != "":
                        default_str = f" default: '{v.default}'"
                    elif v.default is not None:
                        default_str = f" default: {v.default}"

                    if default_str:
                        args["help"] += default_str
                        args["metavar"] = f"<{type(v.default).__name__}>"
                        #args["default"] = v.default

                    #f" default: {v.default}" if v.default is not None else ""
                    parser.add_argument(f"--{name}", **args)
                else:
                    parser.add_argument(f"--{name}", required=True, **args)
            #else:
            #   logger.warning(f"No parser given for {func.__name__}, skipping argument registration for {name}")
        
    

    # @classmethod
    def handle_published_cmd_args(self, args: argparse.Namespace, cmd_func: callable) -> Command:
        """
        Docstring for handle_exploded_command
        
        :param args: Namespace of arguments, typically from argparse. The keys should match the parameter names of cmd_func.
        :param cmd_func: The function to call with the arguments. Typically, this is a method of the Rack class, e.g., Rack.select or Rack.pPseudoRhi.
        :param exec: If True, the command will be executed (i.e., cmd_func will be called with the arguments).
        :type exec: bool
        :return: If exec is True, returns the result of cmd_func. Otherwise, returns a dict of arguments that would be passed to cmd_func.
        """
    
        sig = inspect.signature(cmd_func)
        params = list(sig.parameters.values())

        var_args = vars(args)
        # logger.warning(f"Handling exploded command {cmd_func.__name__}")
        # logger.warning(f"Handling exploded command {cmd_func.__name__} with args: {var_args}") # too verbose...

        # First, create the command with default args, then override with explicit args, and finally parse free args if any.
        cmd = cmd_func(self)
        # logger.warning(f"Building command {cmd_func.__name__}: {cmd.to_string()}")

        pos_args = []
        kw_args = {}
        if not cmd.fmt:
            cmd.set_separators()        
        cmd.fmt.parse_args(var_args.get(cmd_func.__name__, ""), pos_args, kw_args)
        #cmd.set_args(*pos_args, **kw_args)
        #
        logger.debug(f"Initial (private) args {cmd_func.__name__}: {pos_args} {kw_args} ")

        # explicit args. Notice that they are given as --cmd_func arg=value, so they are in var_args with key cmd_func and value "arg=value,..."
        for v in params:
            #name = v.name
            if v.name == "self":
                continue

            if v.name in var_args and var_args[v.name] is not None:
                value = var_args[v.name]
                # default is studied "locally"
                if value != v.default:
                    logger.debug(f"Argument {v.name} has value {value} different from default {v.default}, including in command")
                    kw_args[v.name] = str(value).strip().replace(cmd.fmt.PARAM_SEPARATOR, cmd.fmt.VALUE_SEPARATOR) 
                    #.strip('"').strip("'") # TODO: strip quotes if any, for example, value.strip('"').strip("'") or similar
                    #logger.info(f"{cmd_func.__name__}: adding argument {v.name}={value}")
            else:
                logger.warning(f"Argument {v.name} not found in args or is None, skipping for {cmd_func.__name__}")
        
        cmd.set_args(*pos_args, **kw_args)
        logger.debug(f"Status of {cmd_func.__name__}: {cmd.to_string()}")

        # if exec:
        # logger.warning(f"Executing '{cmd_func.__name__}' with explicit args: {kw_args}")
        # cmd = cmd_func(self, free_args, **dict_args)

        return cmd


def main():

    # Todo: move these to test
    
    class MyRegister(Register):

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
