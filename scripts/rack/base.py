import sys # sys.stderr
from pathlib import Path

def test():
    pass



def copy_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, not removing them
    """
    return [conf[i] for i in keys]

def extract_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, removing them
    """
    return [conf.pop(i) for i in keys]


