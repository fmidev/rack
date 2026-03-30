#!/usr/bin/env python3
from __future__ import annotations

import argparse
#import dataclasses
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile

from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, List, Optional, Tuple

import rack.config
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
class CliConf:
    workdir: Optional[Path] = None
    prepend_args: List[str] = field(default_factory=list)
    append_args: List[str] = field(default_factory=list)
    replacements: List[ReplaceRule] = field(default_factory=list)
    validations: List[ValidationRule] = field(default_factory=list)

    def apply_replacements(self, s: str) -> str:
        for r in self.replacements:
            s = r.apply(s)
        return s


@dataclass
class PyConf:
    
    # This Python(3.x) interpreter
    python: str = sys.executable
    # For example rack.composer
    # module: str = "" 

    imports: List[str] = None
    # Last \include argument, if <filename>.py
    include_py_sh:   str = ""
    include_rack_sh: str = ""
    workdir: Optional[Path] = None
    # Lines that should be injected before the snippet (optional)
    prepend_code: List[str] = field(default_factory=list)
    append_code:  List[str] = field(default_factory=list)
    
    replacements: List[ReplaceRule] = field(default_factory=list)
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
SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>(example|include)))\s*(?:(?P<arg>.+))?$")

#KEY_VALUE_RE = re.compile(r"^\s*(?:(?P<key>([a-zA-Z][a-zA-Z0-9_]*)))\s*=\s*(?:(?P<value>.+))$")
# Accept either a plain command line, or a line prefixed with "$ "
CLI_LINE_RE = re.compile(r"^\s*(?:\$\s+)?(?P<cmd>.+?)\s*$")

# Recognizes code block: \code ... \endcode
#CODE_START_RE = re.compile(r"^\\code(?:\{(?P<arg>[^}]*)\})?\s*$")
#CODE_END_RE = re.compile(r"^\\endcode\s*$")

# SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>[^}]*))?\s*$")
# SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>[a-zA-Z0-9_]+))\s*(?:(?P<arg>.+))?$")

# EXEC_START_RE = re.compile(r"^\\~exec\s*$")
# BLOCK_START_RE = re.compile(r"^\\~(?:(?P<kind>[a-z]+))\s*$")
# BLOCK_START_RE = re.compile(r"^\\~(?:(?P<kind>[a-z]+))(?:\{(?P<arg>[^}]*)\})?\s*$")
#BLOCK_START_RE = re.compile(r"^\\(?:(?:~(?P<user>conf|exec|stop))|(?P<code>code))(?:\{(?P<arg>[^}]*)\})?\s*$")
BLOCK_START_RE = re.compile(r"^\\(?P<key>code|~conf|~exec|~stop)(?:\{(?P<arg>[^}]*)\})?\s*$")
#BLOCK_START_RE = re.compile(r"^\\(?:~(?P<kind>[a-z]+))|(?P<kind2>code)(?:\{(?P<arg>[^}]*)\})?\s*$")
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


def scan_dox(path: Path) -> Iterable[Tuple[str, object]]:
    """
    Yields:
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
        m = SIMPLE_CMD_RE.match(line.strip()) # keep strip here
        if m:
            cmd = m.group("cmd")
            arg = m.group("arg")
            # Future option: \example and \include files can be tested (implies support of Confs)
            if cmd in ("example", "include") and arg:
                yield (cmd, arg.strip())
                logger.debug(f"line {i}: handling  {cmd}({arg})")
            else:
                logger.debug(f"line {i}: bypassing {cmd}({arg})")
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
            while i < doc_lines and not END_RE.match(lines[i].strip()):
                if lines[i].startswith('\\'):
                    logger.error(lines[i])
                    raise ValueError(f"Line inside a block starts with backslash '\\' near line {start_line}")
                if lines[i].endswith('\\'):
                    line_buf += lines[i].rstrip('\\')+' '
                elif line_buf:
                    # Last line concatenated with '\' 
                    buf.append(line_buf+lines[i])
                    line_buf=''
                else:                    
                    buf.append(lines[i])
                i += 1
            if i >= doc_lines: # len(lines):
                raise ValueError(f"Unterminated '{key}' block starting near line {start_line}")
            yield (key, Block(kind=key, arg=arg, content=buf, start_line=start_line))
            i += 1
            continue

        i += 1



def save_example_py(composer:rack.cmdline.Composer, filename: str):
    #with open(file=Path.resolve("out", filename), mode='w') as file:
    styleExample = Style(Color.CYAN)
    filepath=f"out/{filename}"
    module_name = composer.get_module_name()
    logger.info(f"")
    logger.info(f"{Emoji.DISC}  writing {filepath}")
    cmd = f'{sys.executable} -m {module_name}  '
    cmd += composer.get_module_cmd_line() + " --print ' \\\\n  ' # or --exec"
    logger.info(f"Example: " + styleExample.str(cmd))
    cmd += '\n' # Ensure newline
    with open(filepath, mode='w') as file:
        file.write(cmd)
        

def save_example_rack(composer:rack.cmdline.Composer, filename: str):
    prog = composer.get_prog()
    fmt = rack.cmdline.RackFormatter(params_format="'{params}'") 
    cmd = prog.to_string(fmt)
    filepath=f"out/{filename}"
    logger.info(f"{Emoji.DISC}  writing {filepath}")
    #logger.info(f"Example: {cmd}")
    styleExample = Style(Color.CYAN)
    logger.info(f"Example: " + styleExample.str(cmd))
    cmd += '\n' # Ensure newline
    with open(filepath, mode='w') as file:
        file.write(cmd)
        #file.write('\n')

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
    
# , cliconf: CliConf, verbose: bool
def run_py_block(block: Block, pyconf: PyConf) -> None:
    
    # Apply replacements to python snippet text (often handy for file names).
    #code_lines = [pyconf.apply_replacements(l) for l in block.content]
    #if pyconf.prepend_code:
    #    code_lines = list(pyconf.prepend_code) + [""] + code_lines
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

    if pyconf.include_rack_sh:
        code_lines.append(
            f"rack.doxymoron.save_example_rack(composer, '{pyconf.include_rack_sh}')")

    # logger.info(f"Executing Python code from line {block.start_line}:\n" + "\n".join(f"  {line}" for line in block.content))

    code = "\n".join(code_lines) + "\n"

    with tempfile.TemporaryDirectory(prefix="doxpy_") as td:
        #script_file = "/tmp/snippet.py"
        #with open("/tmp/snippet.py", "w") as script:
        
        tdpath = Path(td)
        script = tdpath / "snippet.py"
        script.write_text(code, encoding="utf-8")
        #script.writelines(code_lines)
        
        logger.debug(f"Running PYTHON script: {script}")
        styleWrapper = Style(Effect.DIM, Effect.ITALIC)    
        logger.info(f"{Emoji.SCRIPT.value} script:\n  " + styleWrapper.str("\n  ".join(code_lines)))

        # logger.warning("Double check:")
        #with open(script, "r") as f:
        #    for line in f:
        #        print(line)

        #cp = subprocess.run(['python3', script_file], 
        default_args = {
            "cwd": pyconf.workdir,
            "stdout": subprocess.PIPE,
            "stderr": subprocess.PIPE,
            "text": True,
        }
        result = subprocess.run(['python3', str(script)], **default_args)
        
        if result.returncode == 0:
            dump_subprocess_output(result)
            logger.info(Emoji.SUCCESS + " Success!")
        else:
            dump_subprocess_output(result, styleStdErr=Style(Color.RED, Effect.BOLD))
            raise AssertionError(
                f"Python snippet failed (line {block.start_line})\n"
            )


def main() -> int:

    ap = argparse.ArgumentParser(description="Run and validate \\code blocks inside a Doxygen .dox file.")
    ap.add_argument("docfile", type=Path)
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--debug", action="store_true")
    args = ap.parse_args()

    if args.debug:
        logger.setLevel(logging.DEBUG)
    elif args.verbose:
        logger.setLevel(logging.INFO)

    cliconf = CliConf()
    pyconf = PyConf()

    styleDebug = Style(Color.LIGHT_GRAY, Effect.DIM)
    mainConf = {}

    total_blocks = 0
    for key, obj in scan_dox(args.docfile):

        #if 'head' in conf:
        #    print("[HEAD] " + "\n[HEAD] ".join(conf['head']))

        #print ('kind: ', kind)

        if key in {"example", "include"}:
            # Handle example command, e.g. by running the example script and capturing its output
            # print(f"[{key.upper()}] {obj}")
            
            logger.debug(f"[{key}] {obj}")
            #p = Path(obj)
            #if p.suffix == 'py':
            p = str(obj)
            if p.endswith('_py.sh'):
                pyconf.include_py_sh = p
            elif p.endswith('_rack.sh'):
                pyconf.include_rack_sh = p
            
            #mainConf[kind] = obj
            # You could implement logic here to run the example and capture CLI commands it emits
            #elif kind == "include":
            #    print(f"[INCLUDE] {obj}")
            # You could implement logic here to read the included file and process it as a separate .dox
        elif key == '~conf':
            block: Block = obj 
            dst = mainConf
            if block.arg == 'cli':
                dst = cliconf
            elif block.arg == 'py':
                dst = pyconf
            #logger.info(obj)
            conf = rack.config.parse(block.content, dst, rack.config.PARSE_ERROR)
            #logger.info(f"parsed conf({block.arg}): {conf}")            
            #logger.info(obj)
            logger.info(f"{Emoji.SUCCESS.value} Parsed conf[{block.arg}] on line {block.start_line}")
            #json.str() # dump(conf, sys.stderr, indent=2)
            #logger.debug(styleDebug.str(json.JSONEncoder(indent=2).encode(conf)))
            logger.debug(json.JSONEncoder(indent=2).encode(conf))
            #print(json.str(), file=sys.stderr)
            
            pass   
        elif key == 'code':
            block: Block = obj 
            #print(f"[PY ] block at line {block.start_line}")
            if block.arg == '.py':
                script = []
                if 'head' in mainConf: # ???
                    script.extend(mainConf['head'])
                script.extend(block.content)
                #logger.info(f"Executing Python code")
                logger.info(f"{Emoji.RUN.value}  Executing Python code from line {block.start_line}:")
                #logger.info(f"Executing Python code from line {block.start_line}:") #\n" + "\n".join(f"  {line}" for line in block.content))
                #gray = Style(Color.DEFAULT, Effect.ITALIC)
                #rack.style.Style.sprint(gray, "\n".join(f"  {line}" for line in block.content))
                run_py_block(block, pyconf)
                pyconf.reset()   
            else:
                logger.warning(f"unknown code arg='{block.arg}' at line {block.start_line}")
            pass
        elif key == "~exec":
            block: Block = obj 
            logger.warning(f"Handling special {block.kind}[{block.arg}]")
            #lines = 
            if block.kind == '~exec':
                logger.warning(f" {Emoji.EXPLOSION.value} executing: " + "\n".join(block.content))
                # TODO: backslash handling?
                # cmd = ["echo", "koe", "$PYTHONPATH", '"$PYTHONPATH"']
                # cmd = ["python3", "-m", "rack.composer", "-h"]
                # Has no effect
                env = os.environ
                my_env = {
                    "PATH": env["PATH"]+':'+'/usr/local/bin',
                    "PYTHONPATH":'/home/mpeura/eclipse-workspace/rack/python',
                    }
                my_env.update(env)
                for line in block.content:
                    cmd = []
                    for i in shlex.split(line):
                        if i[0]!='#':
                            cmd.append(i)
                        else:
                            break
                    logger.info(cmd)
                    result = subprocess.run(cmd, text=True, env=my_env, capture_output=True)
                    print("--- stdout --- \n", result.stdout)
                    print("--- stdout --- ")
                    if result.returncode != 0:
                        logger.error(f"execution of '{cmd[0]}' failed")
                        print("--- stderr --- ")
                        print(result.stderr)
                        print("--- stderr --- ")
                exit(1)
            else:
                 logger.error(f"Unknown block type '{block.kind}' under key: {key}")

            #print(f"Executing code from line {block.start_line}:\n" + "\n".join(f"  {line}" for line in block.content))
                
                #print("\n".join(f"  {line}" for line in obj))
        elif key == "~block":
            block: Block = obj  # type: ignore[assignment]
            total_blocks += 1
            if block.kind == "cli":
                print(f"[CLI] block at line {block.start_line}")
                #run_cli_block(block, cliconf, args.verbose)
            elif block.kind == "py":
                print(f"[PY ] block at line {block.start_line}")
                script = []
                if 'head' in conf:
                    script.extend(conf['head'])
                script.extend(block.content)
                #logger.info(f"Executing Python code from line {block.start_line}:\n" + "\n".join(f"  {line}" for line in block.content))
                run_py_block(block, pyconf)
            else:
                if args.verbose:
                    print(f"[SKIP] unknown block kind {block.kind} at line {block.start_line}")
        else:
            raise RuntimeError(f"Unknown key: {key}")

    if args.verbose:
        print(f"[OK ] processed {total_blocks} code block(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())