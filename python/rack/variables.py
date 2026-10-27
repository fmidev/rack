"""Reverse-matching of Python str.format-style syntax templates.

Forward direction (already built in to Python):
    path = syntax.format(**variables)         e.g.  "./mapcache/EPSG:3067/..."

Reverse direction (provided here):
    variables = match_syntax(syntax, path)    e.g.  {"CRS": "EPSG:3067", ...}

The same forward/reverse pattern appears in rack.statistics (extract_metadata):
forward  – a SEPARATOR-joined format string drives the rack command output format
reverse  – zip(keys, line.split(SEPARATOR)) reconstructs the variable dict
The rack.statistics variant works because its separator never appears inside a
value; this module handles the general {VAR} case via regex.
"""

import re


def match_syntax(syntax: str, value: str) -> dict | None:
    """Extract variable values from `value` using a format-string `syntax`.

    Converts each ``{VAR}`` placeholder in `syntax` to a named regex capture
    group ``(?P<VAR>.*?)`` and matches against `value` with `re.fullmatch`.
    Lazy quantifiers plus fullmatch let the regex engine correctly assign
    multi-character separators (e.g. ``_LAYERS=``) even when values contain
    the separator character (e.g. BBOX with commas).

    Returns a ``{var_name: matched_str}`` dict, or ``None`` if `syntax` does
    not match `value`.

    Example::

        syntax = "./mapcache/{CRS}/BBOX={BBOX}_SIZE={WIDTH},{HEIGHT}.png"
        match_syntax(syntax, "./mapcache/EPSG:3067/BBOX=1,2,3,4_SIZE=800,600.png")
        # → {"CRS": "EPSG:3067", "BBOX": "1,2,3,4", "WIDTH": "800", "HEIGHT": "600"}
    """
    # re.split on {VAR} tokens gives alternating [literal, varname, literal, ...]
    parts = re.split(r'\{(\w+)\}', syntax)

    regex = ''
    for i, part in enumerate(parts):
        if i % 2 == 0:          # literal segment
            regex += re.escape(part)
        else:                   # variable name
            regex += f'(?P<{part}>.*?)'

    m = re.fullmatch(regex, value)
    return m.groupdict() if m else None
