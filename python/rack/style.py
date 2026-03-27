


import logging

class VT100:

    RED = "\033[01;31m"
    GREEN = "\033[01;32m"
    YELLOW = "\033[01;33m"
    BLUE = "\033[01;34m"
    MAGENTA = "\033[01;35m"
    CYAN = "\033[01;36m"
    GRAY = "\033[01;37m"
    
    ITALIC = "\033[01;3m"
    BOLD = "\033[01;1m"
    DIM = "\033[01;2m"
    REVERSE = "\033[01;7m"
    UNDERLINE = "\033[01;4m"
    DOUBLE_UNDERLINE = "\033[01;21m"
    OVERLINE = "\033[01;9m"

    END = "\033[00m"
    
class Style:

    class LogFormatter(logging.Formatter):

        COLORS = {
            logging.DEBUG: (VT100.CYAN,VT100.DIM),
            logging.INFO: VT100.GREEN,
            logging.WARNING: (VT100.YELLOW,VT100.BOLD),
            logging.ERROR: (VT100.RED,VT100.BOLD),
            logging.CRITICAL: VT100.MAGENTA,
        }

        def format(self, record):
            message = super().format(record)
            color = self.COLORS.get(record.levelno, "")
            if color:
                if isinstance(color, (tuple,list)):
                    return "".join(color) + message + VT100.END 
                else:
                   return color + message + VT100.END 
            else: 
                return message


    @staticmethod
    def color(text, code):
        return code + str(text) + VT100.END

    @staticmethod
    def cyan(text):
        return VT100.color(text, VT100.CYAN)

    @staticmethod
    def red(text):
        return VT100.color(text, VT100.RED)
    
    @staticmethod
    def cprint(*args, color="", file=None, sep=" ", end="\n"):
        text = sep.join(str(a) for a in args)
        print(color + text + VT100.END, file=file, end=end)

def main():

    logger = logging.getLogger("demo")
    logger.setLevel(logging.DEBUG)

    handler = logging.StreamHandler()
    handler.setFormatter(Style.LogFormatter("%(levelname)s: %(message)s"))

    logger.addHandler(handler)

    logger.debug("All fine")
    logger.info("Hello")
    logger.warning("More serious")
    logger.error("Something failed")

    import sys
    Style.cprint("Hello,", "world", color=VT100.CYAN)
    Style.cprint("Error:", "bad input", color=VT100.RED, file=sys.stderr)


if __name__ == "__main__":
    main()
