


import logging

"""
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
"""

class Color:

    DEFAULT = 39 # checked

    BLACK = 30
    RED 	 = 31
    GREEN 	 = 32
    YELLOW 	 = 33
    BLUE 	 = 34
    MAGENTA  = 35
    CYAN 	 = 36
    LIGHT_GRAY 	 = 37

    DARK_GRAY 	 = 90
    LIGHT_RED 	 = 91
    LIGHT_GREEN  = 92
    LIGHT_YELLOW = 93
    LIGHT_BLUE 	 = 94
    LIGHT_MAGENTA= 95
    LIGHT_CYAN 	 = 96
    WHITE        = 97


    #value = DEFAULT

    def __init__(self, value=DEFAULT):
        self.value = int(value)
    
    def __str__(self):
        return "\033[01;"+str(self.value)+"m"
    
    #def __
    
    
class Effect:

    # DEFAULT = 0 check
    RESET = 0
    BOLD = 1
    DIM = 2
    ITALIC = 3
    UNDERLINE = 4
    REVERSE = 7
    STRIKE = 9
    DOUBLE_UNDERLINE = 21

    #values = set() 
 
    def add(self,*args):
        for arg in args:
            if isinstance(arg, (tuple,set,list)):
                self.add(*arg)
            else:
                self.values.add(arg)
    
    def __str__(self):
        return "\033[01;"+";".join([str(v) for v in self.values])+"m"
    
    def __init__(self, *args):
        self.values = set() 
        self.add(*args)

class EmojiOld:

    CHECK = "✅"
    CHECK2 = "☑️"
    CHECK3 = "✔️"
    NOTE = "☝️"
    WARNING = "⚠️"
    WRONG = "⛔"
    FORBIDDEN = "🚫"
    INFO = "ℹ️"
    LOG = "📋" 
    SYNTAX = "🈲"
    BOMB = "💣"
    EXPLOSION = "💥"
    SCRIPT = "📜"
    EXECUTE = "⚙️"
    TOOL = "🛠️"
    GRAPH = "📊"
    HOURGLASS = "⏳"
    TIMING = "⏱️"
    QUESTION ="❓"
    EXCLAMATION ="❗"
    # _MARKS ="❔⁉️❕"
    
    WORLD = "🌍"
    CONNECTION = "🛜"
    CHARACTERS = "🔡"
    LOCK = "🔒"
    # _LOCKS = "🔒🔐🔑🔓"
    BUG = "🪲" 
    RESTART = "🔄" 
    RECYCLE = "♻️"
    BOX = "📦"
    CONTROL = "🛂"

from enum import Enum


class Emoji(str, Enum):

    # Status / Results
    SUCCESS = "✅"
    SUCCESS_ALT = "✔️"
    CHECKBOX = "☑️"
    FAILURE = "⛔"
    FORBIDDEN = "🚫"

    # Info / Messages
    INFO = "ℹ️"
    NOTE = "☝️"
    WARNING = "⚠️ "
    ERROR = "❗"
    QUESTION = "❓"

    # Actions / Process
    RUN = "⚙️"
    SCRIPT = "📜"
    TOOL = "🛠️"
    RESTART = "🔄"
    RECYCLE = "♻️"
    STOP = "⏹️"

    # Time / Progress
    WAIT = "⏳"
    TIMER = "⏱️"

    # Technical / Dev
    BUG = "🪲"
    LOG = "📋"
    DATA = "📊"
    PACKAGE = "📦"
    SYNTAX = "🈲"
    CHARS = "🔡"

    # Network / System
    NETWORK = "🛜"
    WORLD = "🌍"
    LOCK = "🔒"
    CONTROL = "🛂"

    # Events / Emphasis
    BOMB = "💣"
    EXPLOSION = "💥"
    THINKING = "🤔"

    # Misc
    TRIANGLE = "📐"




class Style:

    #color = Color()
    #effect = Effect()
    RESET = "\033[00m"

    def __init__(self, *args):
        self._color = Color()
        self._effects = Effect()
        self._emojis = list()
        self.modify(self, args)

    @staticmethod
    def _isColor(i:int):
        return (i>30) and (i<40)

    def modify(self, *args):
        """ Changes the color and/or effects of this object. 
        """
        for arg in args:
            if isinstance(arg, (tuple,set,list)):
                self.modify(*arg)
            elif isinstance(arg, Style):
                self._color.value = arg._color.value
                self._effects.values.update(arg._effects.values)
            elif isinstance(arg, Color):
                self._color.value = arg.value
            elif isinstance(arg, Effect):
                self._effects.values.update(arg.values)
            elif isinstance(arg, int):
                if self._isColor(arg):
                    self._color.value = arg
                else: # todo check value <-> attr
                    self._effects.values.add(arg)
            elif isinstance(arg, str):
                self._emojis.append(arg)   
            else:
                raise KeyError(f"unsupported Color or Effect: {arg}")
                #self.value.add(arg)
    
    def copy(self):
        """  Returns a modified copy
        """
        return Style(self._color, self._effects)
    
    def color(self, arg):
        """  Returns a modified copy
        """
        s = self.copy()
        if isinstance(arg, Color):
            s._color.value = arg.value
        elif isinstance(arg, int) and self._isColor(arg):
            s._color.value = arg
        else:
            raise KeyError(f"unsupported Color: {arg}")
        return s
    
    def effect(self, *args):
        """  Returns a modified copy
        """    
        s = self.copy()
        s.modify(*args) # lazy
        """
        if isinstance(arg, Effect):
            s._effects.values.update(arg.values)
        elif isinstance(arg, int) and not self._is_color(arg):
            s._effects.values.add(arg)
        else:
            raise KeyError(f"unsupported Effect: {arg}")
        """
        return s    

    def emojis(self, *args):
        s = self.copy()
        s.modify(*args) # lazy
        

    def __str__(self):
        result = []
        if self._color.value != Color.DEFAULT:
            result.append(self._color.value)
        result.extend(self._effects.values)
        s = "\033[01;"+";".join([str(i) for i in result])+"m"
        if self._emojis:
            return "".join(self._emojis) + ' ' + s
        else:
            return s

    #@staticmethod
    def str(self, *args, file=None):
        result = [self.__str__()]
        result.extend([str(arg) for arg in args])
        #result = [self.__str__()].extend(result)
        result.append(Style.RESET)   
        return "".join(result)

    #@staticmethod
    def sprint(self, *args, file=None):
        #result = [str(arg) for arg in args]
        #result.append(Style.RESET)   
        print(self.str(*args), file=file)
        #print("".join(result), file=file)

    def black(self):
        return self.color(Color.BLACK)

    def red(self):
        return self.color(Color.RED)

    def green(self):
        return self.color(Color.GREEN)

    def yellow(self):
        return self.color(Color.YELLOW)

    def blue(self):
        return self.color(Color.BLUE)

    def magenta(self):
        return self.color(Color.MAGENTA)

    def cyan(self):
        return self.color(Color.CYAN)

    def white(self):
        return self.color(Color.WHITE)

    def bold(self):
        return self.effect(Effect.BOLD)

    def dim(self):
        return self.effect(Effect.DIM)

    def italic(self):
        return self.effect(Effect.ITALIC)

    def underline(self):
        return self.effect(Effect.UNDERLINE)

    def reverse(self):
        return self.effect(Effect.REVERSE)

    def strike(self):
        return self.effect(Effect.STRIKE)
    
    """
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
    """

class LogFormatter(logging.Formatter):

    # , Emoji.BUG Emoji.WARNING, 
    STYLES = {
        logging.DEBUG:   Style(Color.LIGHT_GRAY, Effect.DIM),
        #logging.INFO:    Style(Color(Color.GREEN)),
        logging.INFO:    Style(Color(Color.LIGHT_GRAY)),
        logging.WARNING: Style(Color.YELLOW, Effect.BOLD),
        logging.ERROR:   Style(Color(Color.RED), Effect(Effect.BOLD)),
        logging.CRITICAL:Style(Color.MAGENTA),
    }

    """ Option: separately 
    EMOJIS = {
        logging.DEBUG: Emoji.BUG,
        logging.INFO: Emoji.INFO,
        logging.WARNING: Emoji.WARNING,
        logging.ERROR: Emoji.ERROR,
        logging.CRITICAL: Emoji.BOMB,
    }
    """


    def format(self, record):
        message = super().format(record)
        style = LogFormatter.STYLES.get(record.levelno, Style(Color.YELLOW))
        if style:
            return Style.str(style, message) 
        else: 
            return message

def main():

    logger = logging.getLogger("demo")
    logger.setLevel(logging.DEBUG)

    handler = logging.StreamHandler()
    handler.setFormatter(LogFormatter("%(levelname)s: %(message)s"))

    logger.addHandler(handler)

    logger.warning("More serious")

    #return 0
    logger.debug("All fine")
    logger.info("Hello")
    logger.error("Something failed")

    #import sys
 
    color = Color(Color.RED)
    Style().sprint(color,"red!")

    effect = Effect(Effect.ITALIC, Effect.UNDERLINE)
    Style().sprint(effect,"italic, underline")

    style = Style(color, effect)
    style.sprint("red + (italic, underline)")
    print("default")
    print(Style.str(style,"red + (italic, underline)"))
    print("default")
    print(Style.str(style.color(Color.MAGENTA),"red + (italic, underline)"))
    print("default")

    print(Style(Color.CYAN, Effect.DIM)._color.value)
    print(Style(Color(Color.GREEN))._color.value)

    Style(Color.RED).cyan().dim().underline().sprint("Complex...", 122)
    style.sprint("whats wronk?")

if __name__ == "__main__":
    main()
