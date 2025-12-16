

import logging
logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
logger = logging.getLogger("rack") 
logger.setLevel(logging.INFO)


"""
dir (logging)
 'CRITICAL',
 'DEBUG',
 'ERROR',
 'FATAL',
 'INFO',
 'WARN',
 'WARNING',
"""



        
def copy_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, not removing them
    """
    return [conf[i] for i in keys]

def extract_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, removing them
    """
    return [conf.pop(i) for i in keys]



