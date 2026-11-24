#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple

#from matplotlib import lines

import rack.config
import rack.process
from rack.style import *

import logging
#logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
#logging.basicConfig(format='%(levelname)s:\t %(message)s')
logger = logging.getLogger(__file__) 
logger.setLevel(logging.INFO)

handler = logging.StreamHandler()
handler.setFormatter(rack.style.LogFormatter("%(levelname)s: %(message)s"))
logger.addHandler(handler)
logger.propagate = False

"""
logger = logging.getLogger("demo")
logger.setLevel(logging.DEBUG)
handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter("%(levelname)s: %(message)s"))
logger.addHandler(handler)
"""

# Extensions of \include targets that should be built via `make` (demo/Makefile)
MAKE_EXTENSIONS = {"hlp", "hlp.md"}

# Set from --on-mismatch in main(). Governs what happens when a content check (image
# similarity or text diff) whose test-spec "on_fail" is "error" (the default) fails:
# "ask" pauses and prompts the user (see _resolve_mismatch_action()); "quit"/"continue"/
# "update" apply that action straight away, without asking. Checks with an explicit
# on_fail of "warn"/"ignore" (e.g. a deliberate meta-test) are never affected by this -
# they always just log, regardless of --on-mismatch.
DEFAULT_MISMATCH_ACTION: str = "ask"

# Where linkFile-captured scripts are written, so Doxygen's EXAMPLE_PATH/HTML_EXTRA_FILES
# (see Doxyfile) can serve them as plain \linkfile{...} download links.
LINK_FILE_DIR = Path("..", "tests", "out")

# Default configuration for external commands used by doxymoron.
# Override individual keys by passing --config file.json (JSON format).
# Example override file:
#   {
#     "auto_svg_to_png": true,
#     "svg_converters": [
#       {"cmd": "inkscape", "call_syntax": "{cmd} -o {outfile} {infile}", "enabled": true},
#       {"cmd": "convert",  "call_syntax": "{cmd} {infile} {outfile}",   "enabled": true}
#     ]
#   }
# call_syntax may reference {cmd}, {infile}, {outfile}.
# When cmd is a compound like "python3.12 -m some.module", only its first word is used for
# the PATH availability check; the full string is substituted into {cmd}.
DOXYMORON_CONF: Dict[str, Any] = {
    # Set to False to disable automatic SVG-to-PNG conversion after shell blocks.
    "auto_svg_to_png": True,
    # Directories to scan for newly created .svg files after each shell block,
    # relative to the working directory (cliconf/pyconf workdir, or cwd if unset).
    "svg_scan_dirs": [".", "out"],
    # Converters are tried in order; the first one whose binary is on PATH and
    # whose call succeeds is used.
    "svg_converters": [
        {
            "cmd": "inkscape",
            "call_syntax": "{cmd} -o {outfile} {infile}",
            "enabled": True,
        },
        {
            "cmd": "convert",
            "call_syntax": "{cmd} {infile} {outfile}",
            "enabled": True,
        },
    ],
}

@dataclass
class ReplaceRule:
    src: str
    dst: str

    def apply(self, s: str) -> str:
        return s.replace(self.src, self.dst)


@dataclass
class ValidationRule:
    # Exactly one of these should be set
    expect_exists: Optional[str] = None              # path
    diff_pair: Optional[Tuple[str, str]] = None      # (produced, expected)
    pngdiff_pair: Optional[Tuple[str, str]] = None   # (produced, expected) - stub


@dataclass
class BaseConf:
    stdoutToFile: Optional[Path] = None
    stderrToFile: Optional[Path] = None

    # unused / deprecating
    replacements: List[ReplaceRule] = field(default_factory=list)
    def apply_replacements(self, s: str) -> str:
        for r in self.replacements:
            s = r.apply(s)
        return s

@dataclass
class CliConf(BaseConf):
    workdir: Optional[Path] = None
    # Text inserted into the first command line of the next \code/\~exec{.sh} block, right
    # after the program name (prepend_args) or at the very end (append_args) - exactly as
    # if it had been written there in the .dox file. Either a plain string, or a list of
    # strings (one option per array entry, for readability) that gets joined with a space -
    # see _args_to_str().
    prepend_args: List[str] = field(default_factory=list)
    append_args: List[str] = field(default_factory=list)
    #replacements: List[ReplaceRule] = field(default_factory=list)
    validations: List[ValidationRule] = field(default_factory=list)
    shell: bool = False
    # Filename (under LINK_FILE_DIR) to dump the assembled script to, for \linkfile{...}
    linkFile: Optional[str] = None
    # Output file validation, keyed by output path (relative to workdir). See TESTS.md
    # ("Output file testing") and validate_tests() below. Two special, non-dict values:
    # False skips the upcoming \code/\~exec block (or \include'd exec.sh) entirely - it is
    # never run - for a block that is pure illustration, possibly not even valid syntax
    # (e.g. "..." placeholders); {} (or omitting "tests") runs it normally with no output
    # file checks afterward.
    tests: Optional[Dict[str, Any]] = None


    def reset(self):
        self.prepend_args = []
        self.append_args = []
        self.stdoutToFile = None
        self.stderrToFile = None
        self.shell = False
        self.linkFile = None
        self.tests = None

@dataclass
class PyConf(BaseConf):
    
    # This Python(3.x) interpreter
    python: str = sys.executable
    # For example rack.composer
    # module: str = "" 

    imports: List[str] = None
    # False skips the upcoming \code/\~exec{.py} block entirely (see CliConf.tests for the
    # same convention on the shell side); a dict with a "rack.sh" entry additionally runs
    # the generated rack.sh afterward and checks its returncode (see run_py_block()).
    tests: Dict[str,Any] = None
    # Last \include argument, if <filename>.py
    include_py_sh:   str = ""
    include_rack_sh: str = ""
    # True when the above came from \linkfile{...} rather than \include — triggers
    # creation of a "name.txt -> name" symlink after the file is written.
    py_sh_needs_txt_link:   bool = False
    rack_sh_needs_txt_link: bool = False
    # Filename (under LINK_FILE_DIR) to dump the upcoming \~exec{.py}/\code{.py} block's own
    # illustrative source to - set by \linkfile{name.py}/\include name.py (see main()). Unlike
    # include_py_sh/include_rack_sh (the composer-generated command line), this is the source
    # as written in the .dox file: imports + prepend_code + the block itself, but not
    # append_code or the save_example_*() calls appended for include_py_sh/include_rack_sh,
    # which are internal plumbing rather than documented example code. Always gets a
    # "name.py.txt" symlink (see run_py_block()), since the whole point of \linkfile is a
    # download link even when the block itself is hidden (\~exec, not \code).
    linkFile: Optional[str] = None
    workdir: Optional[Path] = None
    # Lines that should be injected before the snippet (optional)
    prepend_code: List[str] = field(default_factory=list)
    append_code:  List[str] = field(default_factory=list)

    replacements: List[ReplaceRule] = field(default_factory=list)

    #def __init__(self):
    #    self.tests = {}

    #def apply_replacements(self, s: str) -> str:
    #    for r in self.replacements:
    #        s = r.apply(s)
    #    return s

    def reset(self):
        self.include_py_sh = None
        self.include_rack_sh = None
        self.py_sh_needs_txt_link = False
        self.rack_sh_needs_txt_link = False
        self.linkFile = None
        self.prepend_code: None
        self.append_code: None


# ----------------------------
# Parsing helpers
# ----------------------------

# Recognizes \example and \include etc. commands
SINGLE_LINE_CMD_RE = re.compile(r"^\\(?P<cmd>example|include|section|subsection)(?:\{[^}]*\})?\s*(?P<arg>.+)?$")

# Recognizes \linkfile{...} (see ALIASES in Doxyfile): a link-only sibling of \include,
# naming the same kind of generated/example file but not embedding it inline.
LINKFILE_CMD_RE = re.compile(r"^\\linkfile\{(?P<arg>[^}]*)\}\s*$")

# Recognizes code block: \code ... \endcode and extensions like \~conf{...} ... \~. The kind is captured in "kind" (code, ~conf, ~exec, ~stop).
# BLOCK_START_RE = re.compile(r"^\\~(?:(?P<kind>[a-z]+))(?:\{(?P<arg>[^}]*)\})?\s*$")
# BLOCK_START_RE = re.compile(r"^\\(?:(?:~(?P<user>conf|exec|stop))|(?P<code>code))(?:\{(?P<arg>[^}]*)\})?\s*$")
BLOCK_START_RE = re.compile(r"^\\(?P<key>code|~conf|~exec|~stop)(?:\{(?P<arg>[^}]*)\})?\s*$")
BLOCK_END_RE = re.compile(r"^\\~\s*$")
CODE_END_RE = re.compile(r"^\\endcode\s*$")


@dataclass
class Block:
    kind: str               # "cli" or "py" (later: "cpp")
    arg: Optional[str]      # content inside \code{...}
    content: List[str]
    start_line: int


"""
TODO: run also \\ example, if conf["run_example"] = True;
"""


def scan_dox(path: Path) -> Iterable[Tuple[str, object, int]]:
    """
    Low level scanning of a .dox file. Yields commands and blocks as they are encountered.
    Does not interpret the content of blocks, except for simple commands like \\example.

    Yields (key, obj, line_no) - line_no is the 1-based line where the command/block starts
    (same value as obj.start_line for a Block, also given separately for commands that are
    just plain strings):
      ("cliconf", List[str])  - raw lines inside config
      ("pyconf",  List[str])
      ("block",   Block)
    """

    lines = path.read_text(encoding="utf-8").splitlines()

    # Maximum lines, to detect non-ending block.
    doc_lines = len(lines)

    i = 0
    while i < doc_lines:
        
        line = lines[i]
        # line = line.strip()

        # Detect single-line command, like \include or \example
        m = SINGLE_LINE_CMD_RE.match(line.strip()) # keep strip here
        if m:
            cmd = m.group("cmd")
            arg = m.group("arg")
            # Future option: \example and \include files can be tested (implies support of Confs)
            if cmd in ("example", "include", "section") and arg:
                yield (cmd, arg.strip(), i + 1)
                logger.debug(f"line {i}: handling  {cmd}({arg})")
            else:
                logger.debug(f"line {i}: bypassing {cmd}({arg})")
            i += 1
            continue

        # Detect \linkfile{...}
        m = LINKFILE_CMD_RE.match(line.strip())
        if m:
            arg = m.group("arg")
            yield ("linkfile", arg.strip(), i + 1)
            logger.debug(f"line {i}: handling  linkfile({arg})")
            i += 1
            continue

        # Detect \~<kind>{arg}  \~ block  
        m = BLOCK_START_RE.match(line.strip())
        if m:

            key = m.group("key")
            if key=='~stop':
                logger.warning(f"{Emoji.STOP.value}  Stopped by {key} on line: {i}")
                exit(0)
                #raise InterruptedError(f"stopped by {key} on line: {i}")

            END_RE = BLOCK_END_RE
            if key == "code":
                END_RE = CODE_END_RE

            arg = m.group("arg")
            logger.debug(f"Line {i}: {key}[{arg}]")
            start_line = i + 1
            buf = []
            i += 1
            line_buf=''
            # NEW 2026/10
            #lines[i] = lines[i].strip() # remove whitespace
            #while i < doc_lines and not END_RE.match(lines[i].strip()):
            while i < doc_lines and not END_RE.match(lines[i].strip()):
                if lines[i].startswith('\\'):
                    logger.error(lines[i])
                    raise ValueError(f"Line inside a block starts with backslash '\\' near line {start_line}")
                if lines[i].endswith('\\'):
                    # TODO: lines[i] is used raw here - indented continuation lines (common
                    # for multi-line \code{.sh} commands) keep their leading whitespace, and
                    # rstrip('\\') only removes the backslash, not the space before it, so the
                    # trailing ' ' appended below doubles up. Should be
                    line_buf += lines[i].strip().rstrip('\\').rstrip() + ' '
                    # (or equivalent) to collapse this to a single space per joined line.
                    #line_buf += lines[i].rstrip('\\')+' '
                elif line_buf:
                    # Last line concatenated with '\' 
                    buf.append(line_buf+lines[i].strip())
                    line_buf=''
                else:                    
                    buf.append(lines[i])
                i += 1
            if i >= doc_lines: # len(lines):
                raise ValueError(f"Unterminated '{key}' block starting near line {start_line}")
            yield (key, Block(kind=key, arg=arg, content=buf, start_line=start_line), start_line)
            i += 1
            continue

        i += 1


def ensure_map(composer:rack.cmdline.Composer):
    """ Check BBOX, PROJ and SIZE

        TODO: new rack.maps 
    """
    prog = composer.get_prog()
    logger.info(f"Debugging geo.py: " + prog.__str__())
    logger.info(f"BBOX: {composer.args.BBOX}")
    logger.info(f"PROJ: {composer.args.PROJ}")
    logger.info(f"SIZE: {composer.args.SIZE}") 

    # terrestris.de/osm/service?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&LAYERS=OSM-WMS&
    # STYLES=&CRS=EPSG:3035&BBOX=2500000,1500000,6500000,5500000&WIDTH=800&HEIGHT=600&FORMAT=image/png
    # https://kartta.paikkatietoikkuna.fi/arcgis/rest/services/MASTO/MASTO_ortokuva_2021/ImageServer",
    rack.maps.get(
        mapCache="./mapcache/{mapServer}/{CRS}/BBOX={BBOX}_LAYERS={layers}_SIZE={WIDTH},{HEIGHT}_STYLES={styles}.png",
        #cache="maps/my-current-map.png",
        #server="terrestris",
        mapServer="mundialis",
        mapLayers=["OSM-WMS"],
        BBOX=composer.args.BBOX,
        PROJ=composer.args.PROJ,
        SIZE=composer.args.SIZE,
    )

    """
    fmt = rack.cmdline.RackFormatter(
        params_format="'{params}'", 
        cmd_separator=" \\\n\t"
        ) 
    cmd = prog.to_string(fmt)
    logger.info(f"Debug geo.py: " + cmd)   
    """
    
def save_example_py(composer: rack.cmdline.Composer, filename: str):
    filepath = f"out/{filename}"
    module_name = composer.get_module_name()
    # logger.info(f"")
    logger.info(f"{Emoji.DISC}  writing {filepath}")
    python = Path(sys.executable).name
    option_separator=" \\\n  "
    cmd = f'{python} -m {module_name} {option_separator}'
    cmd += composer.get_module_cmd_line(separator=option_separator) + " --print ' \\\\n  ' # or --exec"
    # logger.info(f"Example (python): " + Style(Color.CYAN).str(cmd))
    cmd += '\n' # Ensure newline
    with open(filepath, mode='w') as file:
        file.write(cmd)
        

def save_example_rack(composer:rack.cmdline.Composer, filename: str, dirname="out"):
    prog = composer.get_prog()
    fmt = rack.cmdline.RackFormatter(
        params_format="'{params}'", 
        cmd_separator=" \\\n\t"
        ) 
    cmd = prog.to_string(fmt) + '\n' # Ensure newline
    # cmd += '\n' # Ensure newline
        
    filepath = Path(dirname, filename)  #  f"out/{filename}"
    logger.info(f"{Emoji.DISC}  writing {filepath}")
    #logger.info(f"Example (rack cmd line): " + Style(Color.CYAN).str(cmd))
    with open(filepath, mode='w') as file:
        file.write(cmd)

# TODO: use rack.process.handle...
@staticmethod
def dump_subprocess_output(
        result: subprocess.CompletedProcess[str],
        styleStdErr = Style(Color.YELLOW, Effect.DIM),
        styleStdOut = Style(Color.LIGHT_GRAY, Effect.ITALIC)
        ):
    print(f"--- stdout ---")
    #styleStdOut.sprint(result.stdout)
    print(result.stdout)
    print(f"--- ------ ---")
    print(f"--- stderr ---")
    #styleStdErr.sprint(result.stderr)
    print(result.stderr)
    print(f"--- ------ ---")



# Todo: In dox files, allow to specify whether to run in shell mode (shell=True) or as a direct command (shell=False).
# Special handling of output redirection, e.g. "ls > out.txt" should work in shell mode, but not in direct command mode.
def run_shell_subprocess_FOO(script:str, shell:bool=False) -> None:

    #logger.info(f"{Emoji.SCRIPT.value} executing shell script: {script}")
    
    default_args = {
        "stdout": subprocess.PIPE,
        "stderr": subprocess.PIPE,
        "text": True,
        #"capture_output": True,
        #"shell": shell, 
    }

    result = None

    #script += "-o foo.h5 --format 'BBOX={BBOX}, PROJ={what:EPSG}, SIZE={where:xsize}x{where:ysize}\n' -o '{outputFileBase}.cnf'"

    # Note: shlex.split does not handle backslashes well, so we do it ourselves above.
    if shell:
        # In shell mode, we pass the whole script as a single string to subprocess.run with shell=True.
        # This allows for more complex shell features, but also requires careful handling of the script content.
        result = subprocess.run(script, shell=True, **default_args)
    else:
        tokens = shlex.split(script)

        # Security check:
        if not (tokens[0] in ["rack", "echo", "inkscape", "convert"]): # add to conf?
            logger.error(f"Refusing to execute unknown script: {tokens}")
            raise RuntimeError(f"Refusing to execute unknown script: {tokens}")
        
        result = subprocess.run(tokens, **default_args)

    if result.returncode != 0:
        logger.error(f"execution of '{script}' failed with return code {result.returncode}")
        dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
        exit(1) 

    logger.info(Emoji.SUCCESS + " Executed with success!")    




def _find_svg_outputs(script_lines: List[str]) -> List[str]:
    """Return .svg filenames found in shell script lines (any token ending with '.svg').

    Rack only writes SVG files, never reads them, so every .svg token in a command
    line is an output candidate worth converting.
    """
    seen: set = set()
    result: List[str] = []
    for line in script_lines:
        try:
            tokens = shlex.split(line, comments=True)
        except ValueError:
            tokens = line.split()
        for tok in tokens:
            if tok.lower().endswith(".svg") and tok not in seen:
                seen.add(tok)
                result.append(tok)
    return result


def _ensure_txt_symlink(symlink_path: Path, target_name: str) -> None:
    """Create (or replace) a relative symlink at *symlink_path* pointing to *target_name*.

    *target_name* must be a bare filename (no directory component); the symlink sits in
    the same directory, so the link is relative and survives directory moves.
    Doxygen's \\linkfile{file.sh} alias serves 'file.sh.txt' for browser download;
    this symlink makes that '.txt' path resolve to the real '.sh' file without
    keeping two copies.
    """
    symlink_path.parent.mkdir(parents=True, exist_ok=True)
    if symlink_path.is_symlink() or symlink_path.exists():
        symlink_path.unlink()
    symlink_path.symlink_to(target_name)
    logger.info(f"{Emoji.ROBOT.value}  Symlinking: {symlink_path.name} -> {target_name}")


def _convert_svg_to_png(svg_path: Path, workdir: Optional[Path] = None) -> bool:
    """Convert *svg_path* to a same-stem .png using DOXYMORON_CONF['svg_converters'].

    Tries each enabled converter in order, checking PATH availability via shutil.which
    before attempting the call. call_syntax supports {cmd}, {infile}, {outfile}.
    Returns True on the first successful conversion, False if all converters fail.
    """
    converters = DOXYMORON_CONF.get("svg_converters", [])
    # Naming convention: append .png to the full SVG filename.
    # file.svg → file.svg.png  (keeps the .svg stem intact)
    # Use absolute paths so the converter works regardless of its cwd.
    abs_svg = svg_path.resolve()
    abs_png = abs_svg.with_name(abs_svg.name + ".png")
    # Run with cwd = svg's own directory so relative image references inside the SVG
    # (e.g. <image href="./map.png"/>) resolve correctly.
    svg_dir = str(abs_svg.parent)
    for entry in converters:
        if not entry.get("enabled", True):
            continue
        cmd_bin: str = entry.get("cmd", "")
        call_syntax: str = entry.get("call_syntax", "")
        if not call_syntax:
            logger.debug(f"SVG converter entry has no call_syntax; skipping: {entry}")
            continue
        binary = cmd_bin.split()[0] if cmd_bin else ""
        if binary and not shutil.which(binary):
            logger.debug(f"SVG converter not on PATH: {binary}")
            continue
        cmd_str = call_syntax.format(
            cmd=cmd_bin,
            infile=shlex.quote(str(abs_svg)),
            outfile=shlex.quote(str(abs_png)),
        )
        logger.debug(f"{Emoji.ROBOT.value} SVG->PNG: {cmd_str}")
        result = subprocess.run(cmd_str, shell=True, capture_output=True, text=True,
                                cwd=svg_dir)
        if result.returncode == 0 and abs_png.exists():
            logger.info(f"{Emoji.ROBOT.value} Converted: {abs_svg.name} -> {abs_png.name}")
            return True
        if result.returncode == 0:
            logger.warning(f"Converter '{binary}' returned rc=0 but output not found: {abs_png}")
        else:
            logger.warning(f"Converter '{binary}' failed (rc={result.returncode}): "
                           f"{result.stderr.strip()}")
    logger.warning(f"No SVG converter succeeded for {abs_svg}")
    return False


def _convert_new_svgs(base: Path, since: float, workdir: Optional[Path] = None) -> None:
    """Convert every .svg file created/modified at or after *since* (epoch seconds).

    Scans directories listed in DOXYMORON_CONF['svg_scan_dirs'] relative to *base*.
    This is more reliable than token-scanning the script because it catches SVG paths
    that result from flag combinations (e.g. rack's --outputPrefix + --outputFile).
    """
    scan_dirs = DOXYMORON_CONF.get("svg_scan_dirs", [".", "out"])
    for rel in scan_dirs:
        d = base / rel
        if not d.is_dir():
            continue
        for svg_path in sorted(d.glob("*.svg")):
            try:
                mtime = svg_path.stat().st_mtime
            except OSError:
                continue
            if mtime >= since:
                _convert_svg_to_png(svg_path, workdir)


def _args_to_str(value: Any) -> str:
    """Normalizes a prepend_args/append_args conf value to a single string: a list/tuple
    is joined with a space (handy in JSON to spell out several options one per array
    entry, for readability, instead of one long string), a plain string is used as-is.
    """
    if isinstance(value, (list, tuple)):
        return " ".join(str(v) for v in value)
    return value


def run_shell_lines(lines: list, cliconf: CliConf) -> None:
    """
    Shared shell-execution engine, used for \\code{.sh}/\\~exec{.sh} blocks and
    \\include'd shell scripts (*_exec.sh). Strips '#' comments, joins
    backslash-continued lines into full command lines, applies
    cliconf.prepend_args/append_args to the first command line only (consumed
    once), and runs each resulting command line as a subprocess.
    """

    script = []
    pending_code = None
    for line in lines:
        code = line.split("#", 1)
        code = code[0].strip()
        #print(f"script line: '{code}'")
        if code:
            if pending_code is not None:
                code = f"{pending_code} {code}"
            if code.endswith("\\"):
                pending_code = code[:-1].rstrip()
                continue
            script.append(code)
            pending_code = None

    if pending_code is not None:
        script.append(pending_code)

    # prepend_args/append_args apply to the first command line only, consumed once.
    # Inserted as raw text (not re-tokenized/re-quoted): each is itself a shell-syntax
    # fragment, and splitting the line with shlex then rejoining with plain spaces would
    # strip any quoting it finds along the way (e.g. turning --script "--pCappi 500 --cAdd"
    # into --script --pCappi 500 --cAdd, leaking "500 --cAdd" as separate arguments). The
    # goal is exactly as if the argument string had been written at that position inside
    # the \~exec block itself.
    if script and (cliconf.prepend_args or cliconf.append_args):
        if cliconf.prepend_args:
            prog, _, rest = script[0].partition(" ")
            script[0] = f"{prog} {_args_to_str(cliconf.prepend_args)} {rest}".rstrip()
            cliconf.prepend_args = ""
        if cliconf.append_args:
            script[0] = f"{script[0]} {_args_to_str(cliconf.append_args)}"
            cliconf.append_args = ""

    logger.info(f"{Emoji.OPEN_DIRECTORY.value} Working directory: {cliconf.workdir or Path.cwd()}")
    logger.info(f"{Emoji.RUN.value}  Executing shell script: {script}")
    t_before = time.time()

    if cliconf.linkFile:
        # Write the script with its natural name, then add a .txt symlink so that
        # Doxygen's \linkfile{...} alias can serve "file.sh.txt" for browser download.
        linkpath = Path(LINK_FILE_DIR, cliconf.linkFile)
        logger.info(f"{Emoji.DISC.value}  writing {linkpath} (for \\linkfile)")
        linkpath.write_text("\n".join(script) + "\n", encoding="utf-8")
        _ensure_txt_symlink(linkpath.with_name(cliconf.linkFile + ".txt"), cliconf.linkFile)

    # A fresh capture per block: truncate once, then each subprocess call appends.
    if cliconf.stdoutToFile:
        Path(cliconf.stdoutToFile).write_text("", encoding="utf-8")
    if cliconf.stderrToFile:
        Path(cliconf.stderrToFile).write_text("", encoding="utf-8")

    for line in script:
        logger.debug(f"{Emoji.RUN.value} Executing line: {line}")
        rack.process.run(line, logger=logger, shell=True, cwd=cliconf.workdir,
                          stdout_file=cliconf.stdoutToFile, stderr_file=cliconf.stderrToFile)

    # Convert before validating: a "tests" entry for "*.svg.png" (the naming convention
    # _convert_svg_to_png() produces) names a file that only exists after this runs.
    if DOXYMORON_CONF.get("auto_svg_to_png", True):
        base = Path(cliconf.workdir) if cliconf.workdir else Path()
        _convert_new_svgs(base, since=t_before, workdir=cliconf.workdir)

    validate_tests(cliconf.tests, cliconf.workdir)


def validate_tests(tests: Optional[Dict[str, Any]], workdir: Optional[Path]) -> None:
    """
    Validates output files declared in a "tests" conf entry (see TESTS.md, "Output file
    testing"). Only reached for a block that was actually run - "tests": false (don't run
    this block at all) is handled earlier in main() (see skip_exec_reason()) and never
    gets here. Each key is an output path, relative to workdir; its value is either a
    single spec, or a list of specs to check the same file against several things (e.g.
    two different reference images) - a JSON object cannot repeat a key, so a list is how
    one output file gets more than one check. Each spec is one of:
      - True                : the file must exist
      - int                 : exact file size, in bytes
      - [min, max]          : file size, in bytes, must fall within this (inclusive) range
      - float (0.0 .. 1.0)  : image similarity to reference/<basename> (PNG/TIFF only)
      - dict                : any of {"exists": bool, "filename": str, "size": int|[min,max],
                               "similarity": float, "on_fail": "error"|"warn"|"ignore",
                               "markers": ..., "marker_mismatch_cost": float, "raw": bool},
                               combined. The last three are passed through to
                               rack.imagecheck for PNG/TIFF content checks (see its
                               check_pair() docstring).

    The reference file defaults to "reference/<basename>" (relative to workdir),
    overridable per-entry via spec["filename"]. "on_fail" (default "error") controls what
    a failed spec does: raise (aborting the run), log a warning, or log at info level -
    e.g. for a "meta test" that is expected to fail (checking a near-miss reference on
    purpose) without aborting the rest of the run.

    Content comparison is currently implemented for plain text (.txt) via `diff` and for
    PNG/TIFF (including GeoTIFF pixel content - see rack.imagecheck for its gain/offset
    and nodata/undetect marker handling) via rack.imagecheck. Other formats (HDF5,
    GeoTIFF *geographic* metadata, SVG/XML - see TESTS.md) are only checked for
    existence/size for now.

    For a spec whose "on_fail" is "error" (the default), a failed PNG/TIFF or .txt content
    check does not immediately raise: instead --on-mismatch (see the DEFAULT_MISMATCH_ACTION
    module flag, set from main()'s --on-mismatch) decides what happens. Its default, "ask",
    pauses and asks the user to quit, continue (skip this error), view the two images side
    by side (PNG/TIFF only, via ImageMagick's montage/display), or update the reference file
    with the produced one; "quit"/"continue"/"update" apply that action automatically,
    without asking - see _resolve_mismatch_action() and _apply_mismatch_action(). A spec
    with an explicit on_fail of "warn"/"ignore" (e.g. a deliberate meta-test) is unaffected
    by --on-mismatch and always just logs, as before.
    """
    if not tests:
        return

    base = Path(workdir) if workdir else Path(".")

    for rel_path, raw_spec in tests.items():
        specs = raw_spec if (isinstance(raw_spec, list) and raw_spec
                              and all(isinstance(s, dict) for s in raw_spec)) else [raw_spec]
        for spec in specs:
            _validate_one(base, rel_path, spec)


def _display_images(produced: Path, reference: Path) -> None:
    """Show *produced* and *reference* side by side, labeled, via ImageMagick's
    `montage`/`display` (see _resolve_mismatch_action()). Blocks until the viewer is
    closed (press 'q' in the `display` window).

    The montage is also written to a file under the system temp dir and its path is
    logged, since `display` is an X11 app: if no window reaches the screen (no X
    forwarding, Wayland-only session, etc.), it fails silently and this is the only
    way to actually see the comparison.
    """
    label_produced = f"produced: {produced}"
    label_reference = f"reference: {reference}"
    panel_path = Path(tempfile.gettempdir()) / f"doxymoron-compare-{os.getpid()}.png"
    montage_cmd = (
        "montage -tile 2x1 -geometry +4+4 -frame 5 -pointsize 16 "
        f"-label {shlex.quote(label_produced)} {shlex.quote(str(produced))} "
        f"-label {shlex.quote(label_reference)} {shlex.quote(str(reference))} "
        "-title 'press q to exit viewing' "
        f"{shlex.quote(str(panel_path))}"
    )
    try:
        subprocess.run(montage_cmd, shell=True, check=False)
    except FileNotFoundError:
        logger.error("ImageMagick ('montage') not found; cannot view images")
        return
    logger.info(f"{Emoji.MAGNIFIER.value}  Comparison panel saved to {panel_path} "
                f"(open it manually if no viewer window appears)")
    try:
        subprocess.run(["display", str(panel_path)], check=False)
    except FileNotFoundError:
        logger.error("ImageMagick ('display') not found; open the panel above manually")


def _display_single_image(path: Path) -> None:
    """Show *path* alone via ImageMagick's `display` (see _resolve_mismatch_action()) -
    used when there is no reference yet to compare against, so there is nothing to pair
    it with. Blocks until the viewer is closed (press 'q' in the `display` window).
    """
    logger.info(f"{Emoji.MAGNIFIER.value}  Viewing {path} (open it manually if no viewer window appears)")
    try:
        subprocess.run(["display", "-title", f"produced: {path}  (press q to exit viewing)", str(path)],
                        check=False)
    except FileNotFoundError:
        logger.error(f"ImageMagick ('display') not found; open {path} manually")


def _resolve_mismatch_action(produced: Path, reference: Path, summary_lines: List[str], allow_view: bool,
                              reference_exists: bool = True) -> str:
    """Decides what to do about a failed content check (image similarity or text diff):
    "quit", "continue" (skip it), or "update" (write produced over reference).

    Honours --on-mismatch (DEFAULT_MISMATCH_ACTION): if it is "ask" (the default),
    prompts the user interactively - printing *summary_lines* first, then offering to
    view the file(s) (images only) in addition to quit/continue/update. Any other
    --on-mismatch value is returned directly, without prompting.

    reference_exists=False is for the "reference file is simply missing yet" case (a new
    output never checked before), not a content mismatch: the prompt and the view action
    adapt accordingly - just *produced* is shown (via _display_single_image(), no pair to
    view via _display_images()), and "update" means accepting *produced* as the new
    reference rather than overwriting an existing one (see _apply_mismatch_action()).
    """
    if DEFAULT_MISMATCH_ACTION != "ask":
        logger.info(f"{Emoji.INFO.value}  Mismatch for {produced}: --on-mismatch={DEFAULT_MISMATCH_ACTION}")
        return DEFAULT_MISMATCH_ACTION

    view_opt = ", [v]iew" if allow_view else ""
    update_label = "[u]pdate reference" if reference_exists else "[u]se as new reference"
    prompt = f"[q]uit testing, [c]ontinue{view_opt}, {update_label} > "
    header = (f"Content check failed: {produced} vs {reference}" if reference_exists
              else f"No reference file yet for {produced}")
    while True:
        print(f"\n{Emoji.WARNING.value} {header}")
        for line in summary_lines:
            print(f"  {line}")
        try:
            choice = input(prompt).strip().lower()
        except EOFError:
            print()
            logger.warning(f"{Emoji.STOP.value}  No more input (EOF); treating as quit")
            return "quit"
        if allow_view and choice == "v":
            if reference_exists:
                _display_images(produced, reference)
            else:
                _display_single_image(produced)
            continue
        if choice in ("q", "c", "u"):
            return {"q": "quit", "c": "continue", "u": "update"}[choice]
        print(f"Please answer one of: q, c{', v' if allow_view else ''}, u")


def _apply_mismatch_action(action: str, produced: Path, reference: Path) -> None:
    """Carries out the "quit"/"continue"/"update" verdict from _resolve_mismatch_action()."""
    if action == "quit":
        logger.warning(f"{Emoji.STOP.value}  Testing aborted at {produced}")
        sys.exit(1)
    elif action == "update":
        reference.parent.mkdir(parents=True, exist_ok=True)
        existed = reference.exists()
        shutil.copyfile(produced, reference)
        verb = "updated" if existed else "created"
        logger.info(f"{Emoji.SUCCESS.value}  Reference {verb}: {reference} <- {produced}")
    else:  # "continue"
        logger.info(f"{Emoji.WARNING.value}  Skipping failed check for {produced} (continue)")


def _validate_one(base: Path, rel_path: str, spec: Any) -> None:
    """Runs a single test spec for one file - see validate_tests()."""

    if isinstance(spec, bool):
        spec = {"exists": spec}
    elif isinstance(spec, (int, list, tuple)):
        spec = {"size": spec}
    elif isinstance(spec, float):
        spec = {"similarity": spec}
    elif not isinstance(spec, dict):
        raise ValueError(f"Unsupported test spec for '{rel_path}': {spec!r}")

    on_fail = spec.get("on_fail", "error")
    failed = False

    def report(message: str) -> None:
        nonlocal failed
        failed = True
        if on_fail == "error":
            raise AssertionError(message)
        elif on_fail == "warn":
            logger.warning(f"Test failed (on_fail=warn): {message}")
        else:
            logger.info(f"Test failed (on_fail=ignore): {message}")

    produced = base / rel_path
    logger.info(f"{Emoji.MAGNIFIER.value} Testing {produced}"
                + (f" vs {spec['filename']}" if "filename" in spec else ""))

    should_exist = spec.get("exists", True)
    exists = produced.exists()
    if should_exist and not exists:
        report(f"Test failed: expected output file not found: {produced}")
        return
    if not should_exist:
        if exists:
            report(f"Test failed: file should not exist: {produced}")
        return

    if "size" in spec:
        actual_size = produced.stat().st_size
        size_spec = spec["size"]
        if isinstance(size_spec, (list, tuple)):
            lo, hi = size_spec
            if not (lo <= actual_size <= hi):
                report(f"Test failed: {produced} size {actual_size} not in range [{lo}, {hi}]")
        elif actual_size != size_spec:
            report(f"Test failed: {produced} size {actual_size} != expected {size_spec}")

    reference = base / spec.get("filename", f"reference/{produced.name}")
    suffix = produced.suffix.lower()

    if suffix in (".png", ".tif", ".tiff"):
        if not reference.exists():
            if on_fail == "error":
                action = _resolve_mismatch_action(produced, reference, [], allow_view=True,
                                                   reference_exists=False)
                _apply_mismatch_action(action, produced, reference)
            else:
                # Explicit on_fail=warn/ignore (e.g. a deliberate meta-test): just log, never ask.
                logger.warning(f"No reference file {reference} for {produced}; skipping content check")
        else:
            import rack.imagecheck
            check_spec = {
                "size": True,
                "pixels_equal_min": spec.get("similarity", 0.95),
                "on_fail": on_fail,
            }
            for key in ("markers", "marker_mismatch_cost", "raw"):
                if key in spec:
                    check_spec[key] = spec[key]
            if on_fail == "error":
                # Don't let a failure raise/abort yet - decide what to do first.
                result = rack.imagecheck.run_checks([str(produced), str(reference)], check_spec, on_fail="ignore")
                if not result.passed:
                    stats_str = "  ".join(f"{k}={v}" for k, v in result.stats.items())
                    summary_lines = list(result.failures) + ([stats_str] if stats_str else [])
                    action = _resolve_mismatch_action(produced, reference, summary_lines, allow_view=True)
                    _apply_mismatch_action(action, produced, reference)
            else:
                # Explicit on_fail=warn/ignore (e.g. a deliberate meta-test): just log, never ask.
                result = rack.imagecheck.run_checks([str(produced), str(reference)], check_spec)
                failed = failed or not result.passed
    elif suffix == ".txt":
        if not reference.exists():
            if on_fail == "error":
                action = _resolve_mismatch_action(produced, reference, [], allow_view=False,
                                                   reference_exists=False)
                _apply_mismatch_action(action, produced, reference)
            else:
                # Explicit on_fail=warn/ignore (e.g. a deliberate meta-test): just log, never ask.
                logger.warning(f"No reference file {reference} for {produced}; skipping content check")
        else:
            diff = subprocess.run(["diff", str(reference), str(produced)],
                                   capture_output=True, text=True)
            if diff.returncode != 0:
                if on_fail == "error":
                    logger.warning(f"{produced} differs from {reference}:\n{diff.stdout}")
                    action = _resolve_mismatch_action(produced, reference, [], allow_view=False)
                    _apply_mismatch_action(action, produced, reference)
                else:
                    # Explicit on_fail=warn/ignore (e.g. a deliberate meta-test): just log, never ask.
                    report(f"Test failed: {produced} differs from {reference}:\n{diff.stdout}")
    else:
        logger.debug(f"No content comparator for suffix '{suffix}'; only existence/size checked for {produced}")

    if not failed:
        logger.info(f"{Emoji.SUCCESS.value}  Test passed: {produced}")


def run_py_block(block: Block, pyconf: PyConf) -> None:
    
    # Apply replacements to python snippet text (often handy for file names).
    #code_lines = [pyconf.apply_replacements(l) for l in block.content]
    
    code_lines = []

    if pyconf.imports:

        if isinstance(pyconf.imports, str):
            pyconf.imports = pyconf.imports.split(",")

        # remove duplicates
        pyconf.imports = set(pyconf.imports)
        # Utils for saving docs
        pyconf.imports.add("rack.doxymoron")
        #if isinstance(pyconf.imports, (list,tuple)):
        for i in pyconf.imports:
            code_lines.append(f"import {i}")

    #logger.info(f"{Emoji.RUN.value}  Executing Python code from line {block.start_line}:")

    #styleCode    = Style(Color.DEFAULT, Effect.BOLD)
    #styleWrapper = Style(Effect.DIM, Effect.ITALIC)
    if pyconf.prepend_code:
       code_lines.extend(pyconf.prepend_code)
       #styleWrapper.sprint("\n".join(f"  {line}" for line in pyconf.prepend_code))

    # Snapshot for \linkfile{*.py}: imports + prepend_code + the snippet itself, i.e. what
    # was actually written in the .dox file - not append_code or the save_example_*()
    # calls added below, which are internal plumbing rather than documented example code.
    visible_lines = code_lines + list(block.content)

    code_lines.append("# -------------- start snippet ")
    code_lines.extend(block.content)
    code_lines.append("# -------------- end snippet ")
    #styleCode.sprint("\n".join(f"  {line}" for line in block.content))

    if pyconf.append_code:
        code_lines.extend(pyconf.append_code)
        #styleWrapper.sprint("\n".join(f"  {line}" for line in pyconf.append_code))

    if pyconf.include_py_sh:
        code_lines.append(
            f"rack.doxymoron.save_example_py(composer, '{pyconf.include_py_sh}')")

    logger.info("rack.doxymoron.ensure_map dropped from here")
    #code_lines.append("rack.doxymoron.ensure_map(composer)")

    if pyconf.include_rack_sh:
        code_lines.append(
            f"rack.doxymoron.save_example_rack(composer, '{pyconf.include_rack_sh}', 'out')")

    if pyconf.linkFile:
        linkpath = Path(LINK_FILE_DIR, pyconf.linkFile)
        logger.info(f"{Emoji.DISC.value}  writing {linkpath} (for \\linkfile)")
        linkpath.write_text("\n".join(visible_lines) + "\n", encoding="utf-8")
        _ensure_txt_symlink(linkpath.with_name(pyconf.linkFile + ".txt"), pyconf.linkFile)

    # logger.info(f"Executing Python code from line {block.start_line}:\n" + "\n".join(f"  {line}" for line in block.content))

    code = "\n".join(code_lines) + "\n"

    with tempfile.TemporaryDirectory(prefix="doxpy_") as td:
        
        tdpath = Path(td)
        script = tdpath / "snippet.py"
        script.write_text(code, encoding="utf-8")
        #script.writelines(code_lines)
        
        logger.debug(f"Running PYTHON script: {script}")
        logger.info(f"{Emoji.OPEN_DIRECTORY.value}  Working directory: {pyconf.workdir or Path.cwd()}")
        styleWrapper = Style(Effect.DIM, Effect.ITALIC)
        logger.info(f"{Emoji.SCRIPT.value} script:\n  " + styleWrapper.str("\n  ".join(code_lines)))

        #cp = subprocess.run(['python3', script_file], 
        default_args = {
            "cwd": pyconf.workdir,
            "stdout": subprocess.PIPE,
            "stderr": subprocess.PIPE,
            "text": True,
        }

        # todo use rack.process.run ?
        python = Path(sys.executable).name
        #result = subprocess.run(['python3', str(script)], **default_args)
        result = subprocess.run([python, str(script)], **default_args)

        # todo use rack.process.run
        if result.returncode == 0:
            dump_subprocess_output(result)
            logger.info(Emoji.SUCCESS + " Success!")
        else:
            dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
            raise AssertionError(
                f"Python snippet failed (line {block.start_line})\n"
            )

    out_base = Path(pyconf.workdir or ".") / "out"
    if pyconf.include_rack_sh and pyconf.rack_sh_needs_txt_link:
        _ensure_txt_symlink(out_base / (pyconf.include_rack_sh + ".txt"), pyconf.include_rack_sh)
    if pyconf.include_py_sh and pyconf.py_sh_needs_txt_link:
        _ensure_txt_symlink(out_base / (pyconf.include_py_sh + ".txt"), pyconf.include_py_sh)

    #    if pyconf.include_rack_sh and pyconf.tests and ('rack.sh' in pyconf.tests):
    if pyconf.tests:
        # logger.warning("TEST! " + pyconf.include_rack_sh)
        for (k,v) in pyconf.tests.items():
            # logger.warning(f"{k} = {v}")
            if k == "rack.sh":
                if pyconf.include_rack_sh:
                    logger.info(f"Test requested: {k}, executing {pyconf.include_rack_sh}")
                    filepath = Path("out", pyconf.include_rack_sh)
                    os.chmod(filepath, 0o755) # Ensure it's executable
                    #result = subprocess.run(['ls',str(filepath)], **default_args)
                    #result = subprocess.run([str(filepath)], **default_args)
                    t_before = time.time()
                    result = subprocess.run(['bash', str(filepath)], **default_args)
                    if 'returncode' in v and result.returncode != v['returncode']:
                        logger.error(f"Executing '{filepath}' returned {result.returncode}")
                        dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
                        raise AssertionError(f"Executed '{filepath}', return code={result.returncode}")
                    dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
                    if DOXYMORON_CONF.get("auto_svg_to_png", True):
                        base = Path(pyconf.workdir) if pyconf.workdir else Path()
                        _convert_new_svgs(base, since=t_before, workdir=pyconf.workdir)
                else:
                    logger.warning("shell test '{k}' requested but no script (pyconf.include_rack_sh)")


def main() -> int:

    parser = argparse.ArgumentParser(description="Run and validate \\code blocks inside a Doxygen .dox file.")
    parser.add_argument("docfile", type=Path)
    parser.add_argument("-v", "--verbose", action="store_true")
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("--config", type=Path, default=None,
                    help="JSON config file to override DOXYMORON_CONF defaults "
                         "(auto_svg_to_png, svg_converters, …). "
                         "Top-level keys are merged (shallow update) into the defaults.")
    parser.add_argument("--on-mismatch", choices=["ask", "quit", "continue", "update"], default="ask",
                     help="What to do when a content check (image similarity, text diff) "
                          "whose test-spec on_fail is \"error\" (the default) fails: "
                          "\"ask\" (default) pauses and prompts interactively - quit/"
                          "continue/update, plus viewing the images side by side for "
                          "PNG/TIFF; \"quit\"/\"continue\"/\"update\" apply that action "
                          "straight away, without asking (\"update\" overwrites the "
                          "reference file with the produced one - handy to (re)populate "
                          "references from scratch, but use with care). Checks with an "
                          "explicit on_fail of \"warn\"/\"ignore\" (e.g. a deliberate "
                          "meta-test) are never affected by this - they always just log.")
    parser.add_argument("--scan-start", type=int, default=None, metavar="LINE",
                     help="Only execute \\code/\\~exec blocks (and \\include'd exec.sh scripts "
                          "or make targets) that start at or after this line number of the "
                          ".dox file; ones before it are skipped. \\~conf blocks are always "
                          "parsed regardless of range, so cliconf/pyconf state stays correct "
                          "for whatever block you do run. Handy for re-running one test in the "
                          "middle of a long .dox file without re-running everything before it. "
                          "Line numbers shift whenever the file is edited above this point, so "
                          "treat the value as a throwaway, one-off number, not a stored setting.")
    parser.add_argument("--scan-end", type=int, default=None, metavar="LINE",
                     help="Only execute \\code/\\~exec blocks (and \\include'd exec.sh scripts "
                          "or make targets) that start at or before this line number. See "
                          "--scan-start.")
    args = parser.parse_args()

    if args.config:
        overrides = rack.config.read(args.config)
        DOXYMORON_CONF.update(overrides)
        logger.info(f"Loaded config overrides from {args.config}: {list(overrides.keys())}")

    if args.debug:
        logger.setLevel(logging.DEBUG)
    elif args.verbose:
        logger.setLevel(logging.INFO)

    global DEFAULT_MISMATCH_ACTION
    DEFAULT_MISMATCH_ACTION = args.on_mismatch

    def in_scan_range(line_no: int) -> bool:
        if args.scan_start is not None and line_no < args.scan_start:
            return False
        if args.scan_end is not None and line_no > args.scan_end:
            return False
        return True

    def skip_exec_reason(tests: Any, line_no: int) -> Optional[str]:
        """None if a \\code/\\~exec block (or \\include'd exec.sh) should run, else why not:
        "tests": false means it's pure illustration - possibly not even valid syntax (e.g.
        "..." placeholders) - and must never be executed, regardless of --scan-start/--scan-end.
        """
        if tests is False:
            return "tests: false - not executed"
        if not in_scan_range(line_no):
            return "outside --scan-start/--scan-end"
        return None

    cliconf = CliConf()
    pyconf  = PyConf()

    styleDebug = Style(Color.LIGHT_GRAY, Effect.DIM)
    mainConf = {}

    total_blocks = 0
    for key, obj, line_no in scan_dox(args.docfile):

        if key in {"section", "subsection"}:
            # Simply echo the titles, to help visualize the structure of the document. 
            # No further processing is done for these commands.
            # logger.info(f"{key.upper()}: {obj}")
            styleWrapper = Style(Effect.DIM, Effect.ITALIC)

        elif key == '~conf':
            block: Block = obj 
            dst = mainConf
            if block.arg == '.sh':
                dst = cliconf
            elif block.arg == '.py':
                dst = pyconf
            elif block.arg == 'py':
                logger.warning(f"Deprecated conf type: {block.arg} at line {block.start_line}, use '.py' instead")
                dst = pyconf
            else:
                raise ValueError(f"Unknown conf type: {block.arg} at line {block.start_line}")
            #logger.info(obj)
            logger.info(f"{Emoji.PARSING.value} Parsing conf[{block.arg}] on line {block.start_line}")
            conf = rack.config.parse(block.content, dst, rack.config.PARSE_ERROR)
            # logger.debug(json.JSONEncoder(indent=2).encode(conf))
            logger.debug(f"{Emoji.SUCCESS.value} parsed")
            # print(json.str(), file=sys.stderr)

        elif key in {"example", "include", "linkfile"}:
            # Handle example command, e.g. by running the example script and capturing its output

            logger.debug(f"[{key}] {obj}")

            p = str(obj)
            if p.endswith('py.sh'):
                # Do not run the script, but remember it for later use in a Python snippet.
                pyconf.include_py_sh = p
                if key == "linkfile":
                    pyconf.py_sh_needs_txt_link = True
            elif p.endswith('rack.sh'):
                # Do not run the script, but remember it for later use in a Python snippet.
                pyconf.include_rack_sh = p
                if key == "linkfile":
                    pyconf.rack_sh_needs_txt_link = True
            elif p.endswith('.py'):
                # \linkfile{name.py} (or \include name.py): remember to dump the upcoming
                # \~exec{.py}/\code{.py} block's own illustrative source to this file - see
                # PyConf.linkFile and run_py_block().
                pyconf.linkFile = p
            elif p.endswith('exec.sh'):
                # No path search. Script path should include directory (relative ok)
                reason = skip_exec_reason(cliconf.tests, line_no)
                if reason is None:
                    logger.info(f"{Emoji.RUN.value}  running {p}")
                    lines = Path(p).read_text(encoding="utf-8").splitlines()
                    run_shell_lines(lines, cliconf)
                else:
                    logger.debug(f"Skipping include {p} at line {line_no} ({reason})")
                cliconf.reset()
            elif "".join(Path(p).suffixes).lstrip(".") in MAKE_EXTENSIONS:
                # Delegate to demo/Makefile, e.g. "cExtract.hlp" or "cBBox.hlp.md" -> `make <file>`
                if in_scan_range(line_no):
                    logger.info(f"{Emoji.RUN.value}  make {p}")
                    rack.process.run(["make", p], description=f"make {p}", logger=logger)
                else:
                    logger.debug(f"Skipping make {p} at line {line_no} (outside --scan-start/--scan-end)")
            
        elif key == 'code' or key == "~exec":
            block: Block = obj
            if block.arg == '.py':
                reason = skip_exec_reason(pyconf.tests, block.start_line)
                if reason is None:
                    logger.info(f"{Emoji.SCRIPT.value} Handling Python code from line {block.start_line}:")
                    run_py_block(block, pyconf) # Todo: flag to also run the generated command line.
                else:
                    logger.debug(f"Skipping Python code at line {block.start_line} ({reason})")
                pyconf.reset()
            elif block.arg == '.sh':
                reason = skip_exec_reason(cliconf.tests, block.start_line)
                if reason is None:
                    logger.info(f"{Emoji.SCRIPT.value} Handling shell code from line {block.start_line}")
                    run_shell_lines(block.content, cliconf)
                else:
                    logger.debug(f"Skipping shell code at line {block.start_line} ({reason})")
                cliconf.reset()
            elif block.arg == None:
                logger.warning(f"Skipping code{{}} at line {block.start_line}")
            else:
                logger.warning(f"unknown code arg='{block.arg}' at line {block.start_line}")
            
        else:
            raise RuntimeError(f"Unknown key: {key}")

    if args.verbose:
        print(f"[OK ] processed {total_blocks} code block(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())