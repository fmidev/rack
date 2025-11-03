import sys # sys.stderr
from pathlib import Path

def test():
    pass

def load_config(filename):
    """Load JSON config if it exists."""
    path = Path(filename)
    if not path.is_file():
        print(f"⚠️  File not found: {filename}", file=sys.stderr)
        return {}
    with open(path, "r") as f:
        return json.load(f)


def copy_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, not removing them
    """
    return [conf[i] for i in keys]

def extract_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, removing them
    """
    return [conf.pop(i) for i in keys]


