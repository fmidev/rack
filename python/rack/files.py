#from __future__ import annotations
#import argparse
import logging
import sys

import rack.log

logger = rack.log.logger.getChild(__name__)
# logging.getLogger(__name__)

from pathlib import Path

class SmartFileManager:
    """
    Opens text files lazily (on first use), writes header if new,
    keeps them open for reuse, and closes all on exit.
    """
    def __init__(self, header_func=None, encoding="utf-8"):
        # header_func(file_path: Path) -> str | None
        self._files = {}
        self._header_func = header_func
        self._encoding = encoding

    def write(self, path, text):
        """Append text to the given file path (str or Path)."""
        path = Path(path)
        file_obj = self._files.get(path)

        if file_obj is None:  # first use
            is_new = not path.exists()
            f = path.open("a", encoding=self._encoding)
            if is_new and self._header_func:
                # could also use text arg somehow?
                header = self._header_func(path)  
                if header:
                    f.write(header)
                    if not header.endswith("\n"):
                        f.write("\n")
            self._files[path] = f
            file_obj = f

        file_obj.write(text)
        file_obj.flush()  # optional: ensure content visible immediately

    def close(self, path=None):
        """Close one or all open files."""
        if path is None:
            for f in self._files.values():
                logger.debug(f"Closing file {f.name}")
                f.close()
            self._files.clear()
        else:
            path = Path(path)
            f = self._files.pop(path, None)
            if f:
                logger.debug(f"Closing file {f.name}")
                f.close()

    def __del__(self):
        # auto cleanup on program exit
        self.close()

def main(argv):
    # Define header for new files
    def make_header(path):
        return f"# Data file: {path.name}\n# timestamp value comment"

    # Create the manager
    files = SmartFileManager(header_func=make_header)

    # Simulated loop visiting multiple files
    for name, val in [("data_A.txt", 10), ("data_B.txt", 5), ("data_A.txt", 15)]:
        line = f"2025-11-03T12:00 {val:6.2f} # example\n"
        files.write(name, line)

    # When done
    files.close()


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))