import subprocess
import logging

import rack.prog
import rack.cmdline
import rack.style
# logger = logging.getLogger(__name__)
from rack.style import Style, Color, Effect

def run(prog: rack.prog.CommandSequence,  
        description: str = "Running subprocess",
        logger: logging.Logger = logging.getLogger(__name__),
        shell = False,
        ):
    # logger.info(prog.to_string(fmt))
    #logger.debug(f"Command:\n{prog.to_string(fmt)}")
    logger.debug(f"Command:\n{prog}")

    if isinstance(prog, rack.prog.CommandSequence):
        # Plain format (no quotes for subprocess.run())
        fmt = rack.cmdline.RackFormatter(params_format="{params}")
        prog = prog.to_token_list(fmt)

    result = subprocess.run(prog, 
                            stdout=subprocess.PIPE, 
                            stderr=subprocess.PIPE, 
                            text=True,
                            shell=shell,
                            )
    
    try:
        logger.debug(f"Command: return code {result.returncode}")
        handle_result(result, description=description, logger=logger)
    except Exception as e:
        logger.error(f"Error while executing command sequence: {e}")
        raise e
    
def handle_result(result: subprocess.CompletedProcess, 
                  description:str = "Executed subprocess", 
                  logger: logging.Logger = logging.getLogger(__name__),
                  styleStdErr=Style(Color.RED, Effect.BOLD),
                  styleStdOut = Style(Color.LIGHT_GRAY, Effect.ITALIC),
                  ):
    if result.returncode != 0:
        if result.stdout:
           #logger.info(f"stdout:\n" + styleStdOut.str(result.stdout))
            logger.info(f"stdout:\n{result.stdout.rstrip()}")
        if result.stderr:
            #logger.warning(f"stdout:\n" + styleStdErr.str(result.stdout))
            logger.warning(f"stderr:\n{result.stderr.rstrip()}")
        logger.warning(f"Failed: \n{description}")
        logger.error(f"Command exited with code {result.returncode}")
        raise subprocess.CalledProcessError(result.returncode, result.args, output=result.stdout, stderr=result.stderr) 
        #exit(result.returncode)