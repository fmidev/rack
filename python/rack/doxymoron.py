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
# import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, List, Optional, Tuple


    
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
    python: str = sys.executable
    workdir: Optional[Path] = None
    replacements: List[ReplaceRule] = field(default_factory=list)
    # Lines that should be injected before the snippet (optional)
    prepend_code: List[str] = field(default_factory=list)
    # How python emits CLI commands to stdout
    emit_prefix: str = "$ "

    def apply_replacements(self, s: str) -> str:
        for r in self.replacements:
            s = r.apply(s)
        return s

# ----------------------------
# Parsing helpers
# ----------------------------

CONF_START_RE = re.compile(r"^\\~(?P<kind>cliconf|pyconf)\s*$")
CONF_END_RE = re.compile(r"^\\~\s*$")

CODE_START_RE = re.compile(r"^\\code(?:\{(?P<brace>[^}]*)\})?\s*$")
CODE_END_RE = re.compile(r"^\\endcode\s*$")

# SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>[^}]*))?\s*$")
# SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>[a-zA-Z0-9_]+))\s*(?:(?P<arg>.+))?$")
SIMPLE_CMD_RE = re.compile(r"^\\(?:(?P<cmd>(example|include)))\s*(?:(?P<arg>.+))?$")

# Accept either a plain command line, or a line prefixed with "$ "
CLI_LINE_RE = re.compile(r"^\s*(?:\$\s+)?(?P<cmd>.+?)\s*$")

@dataclass
class Block:
    kind: str                 # "cli" or "py" (later: "cpp")
    brace: Optional[str]      # content inside \code{...}
    content: List[str]
    start_line: int

def scan_dox(path: Path) -> Iterable[Tuple[str, object]]:
    """
    Yields:
      ("cliconf", List[str])  - raw lines inside config
      ("pyconf",  List[str])
      ("block",   Block)
    """

    """ 
    text = path.read_text(encoding="utf-8")
    try:
        conf = json.loads(text)
        return [("json", conf)] 
    except json.JSONDecodeError:
        pass  # Not a JSON file, proceed with normal Doxygen parsing    
    """

    lines = path.read_text(encoding="utf-8").splitlines()
    doc_lines = len(lines)
    i = 0
    while i < doc_lines:
        
        line = lines[i]
        # line = line.strip()

        m = SIMPLE_CMD_RE.match(line.strip()) # keep here
        if m:
            kind = m.group("cmd")
            arg = m.group("arg")
            if kind in ("example", "include") and arg:
                yield (kind, arg.strip())
            print(f"[DEBUG] line {i}: cmd: {kind} arg={arg}")
            i += 1
            continue

        m = CONF_START_RE.match(line.strip()) # keep here
        if m:
            kind = m.group("kind")
            buf = []
            i += 1
            
            # Scan until end of config block
            while i < doc_lines and not CONF_END_RE.match(lines[i].strip()):
                buf.append(lines[i])
                i += 1
            
            if i >= doc_lines:
                raise ValueError(f"Unterminated config block \\~{kind} starting near line {i}")
            
            try:
                conf = json.loads("\n".join(buf))
                print(f"[DEBUG] Parsed JSON config for {kind} at lines {i-len(buf)}-{i}")
                yield (kind, conf)
            except json.JSONDecodeError:
                yield (kind, buf)  # yield raw lines if not valid JSON
                #raise ValueError(f"Invalid JSON in config block \\~{kind} starting near line {i}")
            #yield (kind, conf)
            i += 1
            continue

        m = CODE_START_RE.match(line.strip())
        if m:
            brace = m.group("brace")
            start_line = i + 1
            buf = []
            i += 1
            while i < doc_lines and not CODE_END_RE.match(lines[i].strip()):
                buf.append(lines[i])
                i += 1
            if i >= doc_lines: # len(lines):
                raise ValueError(f"Unterminated \\code block starting near line {start_line}")
            # Determine type
            kind = "cli"
            if brace and ".py" in brace:
                kind = "py"
            yield ("block", Block(kind=kind, brace=brace, content=buf, start_line=start_line))
            i += 1
            continue

        i += 1


def main() -> int:

    ap = argparse.ArgumentParser(description="Run and validate \\code blocks inside a Doxygen .dox file.")
    ap.add_argument("docfile", type=Path)
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    cliconf = CliConf()
    pyconf = PyConf()

    conf = {}

    total_blocks = 0
    for kind, obj in scan_dox(args.docfile):

        if 'head' in conf:
            print("[HEAD] " + "\n[HEAD] ".join(conf['head']))

        if kind == "jsonUNUSED":
            # This is a special case for when the entire .dox file is just a JSON config blob
            # (instead of Doxygen markup). We can directly parse it into our conf objects.
            conf_dict = obj
            if "cliconf" in conf_dict:
                cliconf = CliConf(**conf_dict["cliconf"])
                if args.verbose:
                    print("[CFG] loaded cliconf from JSON")
            if "pyconf" in conf_dict:
                pyconf = PyConf(**conf_dict["pyconf"])
                if args.verbose:
                    print("[CFG] loaded pyconf from JSON")
            continue
        elif kind in {"example", "include"}:
            # Handle example command, e.g. by running the example script and capturing its output
            print(f"[{kind.upper()}] {obj}")
            conf[kind] = obj
            # You could implement logic here to run the example and capture CLI commands it emits
        #elif kind == "include":
        #    print(f"[INCLUDE] {obj}")
            # You could implement logic here to read the included file and process it as a separate .dox
        elif kind == "cliconf":
            #update_conf(cliconf, parse_kv_lines(obj))  # type: ignore[arg-type]
            if args.verbose:
                print("[CFG] updated cliconf:")
                print("\n".join(f"  {line}" for line in obj))
        elif kind == "pyconf":
            #update_conf(pyconf, parse_kv_lines(obj))    # type: ignore[arg-type]
            if args.verbose:
                print("[CFG] updated pyconf")
                if isinstance(obj, list):   
                    print("\n".join(f"  {line}" for line in obj))
                else:
                    print(json.dumps(obj, indent=2))
                    conf.update(obj)  # merge into main conf dict
        elif kind == "block":
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
                print(f"[PY ] Executing Python code from line {block.start_line}:\n" + "\n".join(f"  {line}" for line in block.content))
                #run_py_block(block, pyconf, cliconf, args.verbose)
            else:
                if args.verbose:
                    print(f"[SKIP] unknown block kind {block.kind} at line {block.start_line}")
        else:
            raise RuntimeError(kind)

    if args.verbose:
        print(f"[OK ] processed {total_blocks} code block(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())