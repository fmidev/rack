#!/usr/bin/env python3
from __future__ import annotations

import argparse
#import dataclasses
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple

from matplotlib import lines

import rack.config
import rack.process
#import rack.style
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
MAKE_EXTENSIONS = {"hlp"}

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
    #tests = {}

    #def __init__(self):
    #    self.tests = {}


@dataclass
class CliConf(BaseConf):
    workdir: Optional[Path] = None
    prepend_args: List[str] = field(default_factory=list)
    append_args: List[str] = field(default_factory=list)
    replacements: List[ReplaceRule] = field(default_factory=list)
    validations: List[ValidationRule] = field(default_factory=list)
    shell: bool = False
    # Filename (under LINK_FILE_DIR) to dump the assembled script to, for \linkfile{...}
    linkFile: Optional[str] = None
    # Output file validation, keyed by output path (relative to workdir). See TESTS.md
    # ("Output file testing") and validate_tests() below.
    tests: Optional[Dict[str, Any]] = None

    def apply_replacements(self, s: str) -> str:
        for r in self.replacements:
            s = r.apply(s)
        return s

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
    tests: Dict[str,Any] = None
    # Last \include argument, if <filename>.py
    include_py_sh:   str = ""
    include_rack_sh: str = ""
    workdir: Optional[Path] = None
    # Lines that should be injected before the snippet (optional)
    prepend_code: List[str] = field(default_factory=list)
    append_code:  List[str] = field(default_factory=list)
    
    replacements: List[ReplaceRule] = field(default_factory=list)

    #def __init__(self):
    #    self.tests = {}

    def apply_replacements(self, s: str) -> str:
        for r in self.replacements:
            s = r.apply(s)
        return s
    
    def reset(self):
        self.include_py_sh = None
        self.include_rack_sh = None
        self.prepend_code: None
        self.append_code: None


# ----------------------------
# Parsing helpers
# ----------------------------

# Recognizes \example and \include commands
SINGLE_LINE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>(example|include|section|subsection)))\s*(?:(?P<arg>.+))?$")

# Recognizes \linkfile{...} (see ALIASES in Doxyfile): a link-only sibling of \include,
# naming the same kind of generated/example file but not embedding it inline.
LINKFILE_CMD_RE = re.compile(r"^\\linkfile\{(?P<arg>[^}]*)\}\s*$")

# KEY_VALUE_RE = re.compile(r"^\s*(?:(?P<key>([a-zA-Z][a-zA-Z0-9_]*)))\s*=\s*(?:(?P<value>.+))$")
# Accept either a plain command line, or a line prefixed with "$ "
# CLI_LINE_RE = re.compile(r"^\s*(?:\$\s+)?(?P<cmd>.+?)\s*$")

# Recognizes code block: \code ... \endcode
# CODE_START_RE = re.compile(r"^\\code(?:\{(?P<arg>[^}]*)\})?\s*$")
# CODE_END_RE = re.compile(r"^\\endcode\s*$")
# SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>[^}]*))?\s*$")
# SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>[a-zA-Z0-9_]+))\s*(?:(?P<arg>.+))?$")
# EXEC_START_RE = re.compile(r"^\\~exec\s*$")
# BLOCK_START_RE = re.compile(r"^\\~(?:(?P<kind>[a-z]+))\s*$")
# BLOCK_START_RE = re.compile(r"^\\~(?:(?P<kind>[a-z]+))(?:\{(?P<arg>[^}]*)\})?\s*$")
# BLOCK_START_RE = re.compile(r"^\\(?:(?:~(?P<user>conf|exec|stop))|(?P<code>code))(?:\{(?P<arg>[^}]*)\})?\s*$")
BLOCK_START_RE = re.compile(r"^\\(?P<key>code|~conf|~exec|~stop)(?:\{(?P<arg>[^}]*)\})?\s*$")
# BLOCK_START_RE = re.compile(r"^\\(?:~(?P<kind>[a-z]+))|(?P<kind2>code)(?:\{(?P<arg>[^}]*)\})?\s*$")
BLOCK_END_RE = re.compile(r"^\\~\s*$")
CODE_END_RE = re.compile(r"^\\endcode\s*$")

# Recognizes conf block: \~cliconf or \~pyconf ... \endcode
# Deprecating
# CONF_START_RE = re.compile(r"^\\~(?P<kind>cliconf|pyconf)\s*$")
# CONF_END_RE = re.compile(r"^\\~\s*$")


@dataclass
class Block:
    kind: str                 # "cli" or "py" (later: "cpp")
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
    filepath=f"out/{filename}"
    module_name = composer.get_module_name()
    logger.info(f"")
    logger.info(f"{Emoji.DISC}  writing {filepath}")
    python = Path(sys.executable).name
    option_separator=" \\\n  "
    cmd = f'{python} -m {module_name} {option_separator}'
    cmd += composer.get_module_cmd_line(separator=option_separator) + " --print ' \\\\n  ' # or --exec"
    logger.info(f"Example (python): " + Style(Color.CYAN).str(cmd))
    cmd += '\n' # Ensure newline
    with open(filepath, mode='w') as file:
        file.write(cmd)
        

def save_example_rack(composer:rack.cmdline.Composer, filename: str, dirname="out"):
    prog = composer.get_prog()
    fmt = rack.cmdline.RackFormatter(
        params_format="'{params}'", 
        cmd_separator=" \\\n\t"
        ) 
    cmd = prog.to_string(fmt)
    
    filepath = Path(dirname, filename)  #  f"out/{filename}"
    logger.info(f"{Emoji.DISC}  writing {filepath}")
    # styleExample = Style(Color.CYAN)
    logger.info(f"Example (rack cmd line): " + Style(Color.CYAN).str(cmd))
    cmd += '\n' # Ensure newline
    with open(filepath, mode='w') as file:
        file.write(cmd)
        #file.write('\n')

# TODO: use rack.process.handle...
@staticmethod
def dump_subprocess_output(
        result: subprocess.CompletedProcess[str],
        styleStdErr = Style(Color.YELLOW, Effect.DIM),
        styleStdOut = Style(Color.LIGHT_GRAY, Effect.ITALIC)
        ):
    print(f"--- stdout ---")
    styleStdOut.sprint(result.stdout)
    print(f"--- stderr ---")
    styleStdErr.sprint(result.stderr)

"""
def run_shell_system(script:str) -> None:
    logger.info(f"{Emoji.SCRIPT.value} executing shell script: {script}")
    result = os.system(script)
    if result:
        logger.error(f"execution of '{script}' failed with return code {result}")
        exit(1) 
    logger.info(Emoji.SUCCESS + " Success!")
"""

# Todo: In dox files, allow to specify whether to run in shell mode (shell=True) or as a direct command (shell=False).
# Special handling of output redirection, e.g. "ls > out.txt" should work in shell mode, but not in direct command mode.
def run_shell_subprocess(script:str, shell:bool=False) -> None:

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
        print(f"script line: '{code}'")
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
    if script and (cliconf.prepend_args or cliconf.append_args):
        tokens = shlex.split(script[0])
        if cliconf.prepend_args:
            tokens = [tokens[0], cliconf.prepend_args, *tokens[1:]]
            cliconf.prepend_args = ""
        code = " ".join(tokens)
        if cliconf.append_args:
            code += " " + cliconf.append_args
            cliconf.append_args = ""
        script[0] = code

    logger.info(f"{Emoji.RUN.value} Executing shell script: {script}")

    if cliconf.linkFile:
        # \linkfile{...} serves the .txt-suffixed copy directly (see ALIASES in Doxyfile) -
        # it is the only file written here, not a twin alongside an extension-true original.
        linkpath = Path(LINK_FILE_DIR, cliconf.linkFile + ".txt")
        logger.info(f"{Emoji.DISC.value}  writing {linkpath} (for \\linkfile)")
        linkpath.write_text("\n".join(script) + "\n", encoding="utf-8")

    # A fresh capture per block: truncate once, then each subprocess call appends.
    if cliconf.stdoutToFile:
        Path(cliconf.stdoutToFile).write_text("", encoding="utf-8")
    if cliconf.stderrToFile:
        Path(cliconf.stderrToFile).write_text("", encoding="utf-8")

    for line in script:
        logger.info(f"{Emoji.RUN.value} Executing line: {line}")
        rack.process.run(line, logger=logger, shell=True, cwd=cliconf.workdir,
                          stdout_file=cliconf.stdoutToFile, stderr_file=cliconf.stderrToFile)

    validate_tests(cliconf.tests, cliconf.workdir)


def validate_tests(tests: Optional[Dict[str, Any]], workdir: Optional[Path]) -> None:
    """
    Validates output files declared in a "tests" conf entry (see TESTS.md, "Output file
    testing"). Each key is an output path, relative to workdir; its value is either a
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


def _resolve_mismatch_action(produced: Path, reference: Path, summary_lines: List[str], allow_view: bool) -> str:
    """Decides what to do about a failed content check (image similarity or text diff):
    "quit", "continue" (skip it), or "update" (overwrite reference with produced).

    Honours --on-mismatch (DEFAULT_MISMATCH_ACTION): if it is "ask" (the default),
    prompts the user interactively - printing *summary_lines* first, then offering to
    view the two files side by side (images only, via _display_images()) in addition to
    quit/continue/update. Any other --on-mismatch value is returned directly, without
    prompting.
    """
    if DEFAULT_MISMATCH_ACTION != "ask":
        logger.info(f"{Emoji.INFO.value}  Mismatch for {produced}: --on-mismatch={DEFAULT_MISMATCH_ACTION}")
        return DEFAULT_MISMATCH_ACTION

    view_opt = ", [v]iew" if allow_view else ""
    prompt = f"[q]uit testing, [c]ontinue{view_opt}, [u]pdate reference > "
    while True:
        print(f"\n{Emoji.WARNING.value} Content check failed: {produced} vs {reference}")
        for line in summary_lines:
            print(f"  {line}")
        try:
            choice = input(prompt).strip().lower()
        except EOFError:
            print()
            logger.warning(f"{Emoji.STOP.value}  No more input (EOF); treating as quit")
            return "quit"
        if allow_view and choice == "v":
            _display_images(produced, reference)
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
        shutil.copyfile(produced, reference)
        logger.info(f"{Emoji.SUCCESS.value}  Reference updated: {reference} <- {produced}")
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
    logger.info(f"{Emoji.MAGNIFIER.value}  Testing {produced}"
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

            

    # logger.info(f"Executing Python code from line {block.start_line}:\n" + "\n".join(f"  {line}" for line in block.content))

    code = "\n".join(code_lines) + "\n"

    with tempfile.TemporaryDirectory(prefix="doxpy_") as td:
        
        tdpath = Path(td)
        script = tdpath / "snippet.py"
        script.write_text(code, encoding="utf-8")
        #script.writelines(code_lines)
        
        logger.debug(f"Running PYTHON script: {script}")
        styleWrapper = Style(Effect.DIM, Effect.ITALIC)    
        logger.info(f"{Emoji.SCRIPT.value} script:\n  " + styleWrapper.str("\n  ".join(code_lines)))

        #cp = subprocess.run(['python3', script_file], 
        default_args = {
            "cwd": pyconf.workdir,
            "stdout": subprocess.PIPE,
            "stderr": subprocess.PIPE,
            "text": True,
        }

        # todo use rack.process.run
        result = subprocess.run(['python3', str(script)], **default_args)

        # todo use rack.process.run
        if result.returncode == 0:
            dump_subprocess_output(result)
            logger.info(Emoji.SUCCESS + " Success!")
        else:
            dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
            raise AssertionError(
                f"Python snippet failed (line {block.start_line})\n"
            )

    #    if pyconf.include_rack_sh and pyconf.tests and ('rack.sh' in pyconf.tests):
    if pyconf.tests:
        # logger.warning("TEST! " + pyconf.include_rack_sh)
        for (k,v) in pyconf.tests.items():
            logger.warning(f"{k} = {v}")
            if k == "rack.sh":
                if pyconf.include_rack_sh:
                    logger.warning("Executing {pyconf.include_rack_sh}")
                    filepath = Path("out", pyconf.include_rack_sh)
                    os.chmod(filepath, 0o755) # Ensure it's executable
                    #result = subprocess.run(['ls',str(filepath)], **default_args)
                    #result = subprocess.run([str(filepath)], **default_args)
                    result = subprocess.run(['bash', str(filepath)], **default_args)
                    if 'returncode' in v and result.returncode != v['returncode']:
                        logger.error(f"Executing '{filepath}' returned {result.returncode}")
                        dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
                        raise AssertionError(f"Executed '{filepath}', return code={result.returncode}")
                    dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
                else:
                    logger.warning("shell test '{k}' requested but no script (pyconf.include_rack_sh)")


def main() -> int:

    ap = argparse.ArgumentParser(description="Run and validate \\code blocks inside a Doxygen .dox file.")
    ap.add_argument("docfile", type=Path)
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--debug", action="store_true")
    ap.add_argument("--on-mismatch", choices=["ask", "quit", "continue", "update"], default="ask",
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
    ap.add_argument("--scan-start", type=int, default=None, metavar="LINE",
                     help="Only execute \\code/\\~exec blocks (and \\include'd exec.sh scripts "
                          "or make targets) that start at or after this line number of the "
                          ".dox file; ones before it are skipped. \\~conf blocks are always "
                          "parsed regardless of range, so cliconf/pyconf state stays correct "
                          "for whatever block you do run. Handy for re-running one test in the "
                          "middle of a long .dox file without re-running everything before it. "
                          "Line numbers shift whenever the file is edited above this point, so "
                          "treat the value as a throwaway, one-off number, not a stored setting.")
    ap.add_argument("--scan-end", type=int, default=None, metavar="LINE",
                     help="Only execute \\code/\\~exec blocks (and \\include'd exec.sh scripts "
                          "or make targets) that start at or before this line number. See "
                          "--scan-start.")
    args = ap.parse_args()

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
            logger.info(Emoji.SCRIPT.value + styleWrapper.str(f" {key.upper()}: {obj}"))

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
            logger.info(f"Parsing conf[{block.arg}] on line {block.start_line}")
            conf = rack.config.parse(block.content, dst, rack.config.PARSE_ERROR)
            logger.debug(json.JSONEncoder(indent=2).encode(conf))
            logger.info(f"{Emoji.SUCCESS.value} parsed")
            # print(json.str(), file=sys.stderr)

        elif key in {"example", "include", "linkfile"}:
            # Handle example command, e.g. by running the example script and capturing its output

            logger.debug(f"[{key}] {obj}")

            p = str(obj)
            # \linkfile{...} serves its target as a browser-renderable .txt copy (see ALIASES
            # in Doxyfile), so the file that gets generated must carry that suffix too.
            generated_name = p + ".txt" if key == "linkfile" else p
            if p.endswith('py.sh'):
                # Do not run the script, but remember it for later use in a Python snippet.
                pyconf.include_py_sh = generated_name
            elif p.endswith('rack.sh'):
                # Do not run the script, but remember it for later use in a Python snippet.
                pyconf.include_rack_sh = generated_name
            elif p.endswith('exec.sh'):
                # No path search. Script path should include directory (relative ok)
                if in_scan_range(line_no):
                    logger.info(f"{Emoji.RUN.value}  running {p}")
                    lines = Path(p).read_text(encoding="utf-8").splitlines()
                    run_shell_lines(lines, cliconf)
                    cliconf.reset()
                else:
                    logger.debug(f"Skipping include {p} at line {line_no} (outside --scan-start/--scan-end)")
            elif Path(p).suffix.lstrip(".") in MAKE_EXTENSIONS:
                # Delegate to demo/Makefile, e.g. "cExtract.hlp" -> `make cExtract.hlp`
                if in_scan_range(line_no):
                    logger.info(f"{Emoji.RUN.value}  make {p}")
                    rack.process.run(["make", p], description=f"make {p}", logger=logger)
                else:
                    logger.debug(f"Skipping make {p} at line {line_no} (outside --scan-start/--scan-end)")
            
        elif key == 'code' or key == "~exec":
            block: Block = obj
            if block.arg == '.py':
                if in_scan_range(block.start_line):
                    logger.info(f"{Emoji.SCRIPT.value}  Handling Python code from line {block.start_line}:")
                    run_py_block(block, pyconf) # Todo: flag to also run the generated command line.
                else:
                    logger.debug(f"Skipping Python code at line {block.start_line} (outside --scan-start/--scan-end)")
                pyconf.reset()
            elif block.arg == '.sh':
                if in_scan_range(block.start_line):
                    logger.info(f"{Emoji.SCRIPT.value}  Handling shell code from line {block.start_line}")
                    run_shell_lines(block.content, cliconf)
                else:
                    logger.debug(f"Skipping shell code at line {block.start_line} (outside --scan-start/--scan-end)")
                cliconf.reset()
            elif block.arg == None:
                logger.warning(f"Skipping code{{}} at line {block.start_line}")
            else:
                logger.warning(f"unknown code arg='{block.arg}' at line {block.start_line}")
            
        #elif key == "~exec":
        #    block: Block = obj 
        #    if block.arg == '.sh':
        #        run_shell(block, cliconf)
        #        cliconf.reset()
        #    else:
        #        logger.warning(f"Skipping {key}[{block.arg}] at line {block.start_line}")
        #    continue
        else:
            raise RuntimeError(f"Unknown key: {key}")

    if args.verbose:
        print(f"[OK ] processed {total_blocks} code block(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())