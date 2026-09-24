# AGENTS.md

Guidance for AI coding agents working on **Rack**, a C++ command-line program for processing
weather radar data (ODIM-HDF5 in; HDF5/PNG/GeoTIFF/TXT/SVG out). User documentation:
<https://fmidev.github.io/rack/>. Human-oriented setup notes are in [README.md](README.md),
[src/INSTALL.md](src/INSTALL.md), [VSCODE.md](VSCODE.md), and [CONDA.md](CONDA.md).

## Current development focus

1. **User interfaces**: command/configuration files (`.cnf`, JSON), help
   output, and the Python front end in [python/rack](python/rack). No major changes to CLI at this point.
2. **Code structure and readability**: making the code easier to navigate and change without
   changing behavior.
3. **Python bindings (exploratory)**: a better Python interface to Rack. See
   [Python bindings](#python-bindings-open-design-question) before starting on this.

Treat work outside these areas (radar algorithms, image operators, projections) as out of scope
unless the task explicitly asks for it. When a readability change would also change CLI behavior,
default values, or output, stop and say so. Rack runs in operational pipelines, so changes
visible to users need a deliberate decision.

## Repository map

| Path | Contents |
|------|----------|
| [src/main](src/main) | Rack application: command modules (`andre.cpp`, `cartesian*.cpp`, `products.cpp`, `graphics*.cpp`, `image-ops.cpp`, `fileio*.cpp`, `commands.cpp`), `RackContext` / resources, `main.cpp`, Doxygen pages (`*.dox`) |
| [src/drain](src/drain) | General-purpose library, reusable outside Rack: variables, logging, trees, XML/JSON/SVG, image processing |
| [src/drain/prog](src/drain/prog) | **CLI framework**: `Command`, `CommandBank`, `CommandInstaller`, `CommandSection`, `Program`, `Context`, script parsing |
| [src/drain/util](src/drain/util) | Utilities, including `PythonConverter` (`PythonUtils.*`) used for the Python export |
| [src/data](src/data), [src/hi5](src/hi5), [src/radar](src/radar), [src/product](src/product), [src/andre](src/andre) | ODIM metadata, HDF5 I/O, radar geometry, meteorological products, anomaly detection/removal operators |
| [python/rack](python/rack) | Python package: subprocess-based wrapper and helper tools (statistics, composites, maps, gnuplot) |
| [python/rack/core.py](python/rack/core.py) | **Generated** Python API for all Rack commands (see below) |
| [python/rack/tests](python/rack/tests) | Python unit tests (`unittest`) |
| [demo](demo) | Example shell scripts and `.cnf` files. These double as Doxygen example sources (`EXAMPLE_PATH`) |
| [scripts](scripts) | Operational shell/Python helper scripts |
| [palette](palette) | Color palettes (`palette-<QUANTITY>.txt`) |
| [docs](docs) | **Generated** Doxygen HTML published as GitHub Pages. Do not edit by hand |
| [tests](tests) | Test Makefile. `tests/python` is a symlink to `../python` |

Stale or suspicious items to know about. Don't build on them, and ask before deleting them:
- [scripts/rack](scripts/rack) is an older, diverged copy of the Python package. The current one is
  [python/rack](python/rack).
- [scripts/rack-wrapper.py](scripts/rack-wrapper.py) is a dangling symlink to `rack.py`.
- [python/rack/plot_common.py](python/rack/plot_common.py) is empty.

## Build and run

Dependencies: g++, make, HDF5, PNG, PROJ (6+), TIFF, GeoTIFF, OpenMP (see [VSCODE.md](VSCODE.md)
or [CONDA.md](CONDA.md)).

```bash
make configure        # once: writes src/install-rack.cnf (not in git)
make -j$(nproc)       # builds src/rack (objects in src/Build/)
make OPTS="-O0 -g3"   # debug build
make compdb           # compile_commands.json via bear
make cppcheck         # static analysis
```

The root [Makefile](Makefile) delegates to [src/Makefile](src/Makefile). The code is compiled as
**`-std=gnu++11`**, so don't use C++14 or later features unless the standard is raised on purpose
(and all Docker targets under `src/Dockerfile.*` still build). A full build takes several minutes.
Prefer incremental builds, and run `make` after C++ changes before claiming they work.

Smoke-test the CLI after changes:

```bash
src/rack --help
src/rack --help <command>        # e.g. --help cCreate
src/rack --version
```

## Testing

- Python: `make test-python` (runs `python3 -m unittest discover -s rack/tests -v` inside
  `python/`). Single module: `cd python && python3 -m unittest -v rack.tests.test_typical`.
  Many tests call the `rack` executable, so it must be built and on `PATH`.
- C++: there is no unit-test suite. [demo/](demo) scripts (for example
  [demo/make-tests.sh](demo/make-tests.sh)) run end-to-end and need radar sample data, which is
  not in the repo.
- When refactoring C++ without a test harness, show that behavior is unchanged by comparing
  `rack --help` output and the output of representative command lines before and after the change.
  Say explicitly when you could not verify something.

## How the CLI works

Every CLI option is a C++ command class registered in a `CommandBank`:

- A command derives from `drain::BasicCommand`, `drain::SimpleCommand<T>` (one parameter), or
  `drain::BeanCommand<OP>` (wraps an operator "bean"). Parameters are declared with
  `getParameters().link("name", member = default, "unit/range")`. Commands that have parameters
  need a copy constructor that calls `copyStruct(...)`, because commands are cloned per thread
  or context.
- The option name comes from the class name (`CmdPython` → `--python`). Modules add a one-letter
  **prefix** through `drain::CommandInstaller<'x', Section>` / `drain::CommandModule`:
  `a` AnDRe (anomaly detection), `c` Cartesian/compositing, `p` polar products, `i` image ops,
  `g` graphics. For example, `AnDReCommand<BioMetOp>` becomes `--aBiomet`.
- Parameters are positional and comma-separated (`--aBiomet 0dBZ,1000m,5dBZ,500m`) or named
  (`--aBiomet reflMax=5dBZ`). Sub-values use `:`. Commands run **in order**, and state is carried
  in `RackContext` (for example `--select` affects the next product command).
- Command files: `--execFile file.cnf` reads one command per line, where `#` at line start begins
  a comment (`CommandBank::readFileTXT`). JSON command files are experimental
  (`CommandBank::readFile`). `--script` / `--exec` / `--execScript` define and run scripts that
  repeat for each input.
- Help text comes from the command description and parameter metadata. Keep them accurate:
  they feed `--help`, the Doxygen docs, and the generated Python API.

### UI principles for new or changed commands

- Backward compatibility: don't rename or remove options or parameters, or change defaults,
  without an explicit decision. Prefer adding an alias or a deprecation message.
- Keep naming consistent with neighboring commands (camelCase, module prefix, units in parameter
  metadata).
- Errors should name the command and the bad value (`mout.fail(...)` / `mout.error(...)`).
  Avoid `exit()` inside command `exec()` in new code, because it also blocks in-process use from
  Python.
- After changing any command signature, regenerate the Python API (below) and update the related
  `*.dox` pages and demos.

## Python package

- [python/rack/core.py](python/rack/core.py) is **generated** from the C++ command bank. Don't edit
  it by hand. Regenerate it with `make release` (which runs `rack --python rack/core.py` in
  `python/`), then check the diff.
- `rack.prog.Register` / `rack.command.Command` / `CommandSequence` build Rack command lines.
  Execution happens as a subprocess (`rack.process`).
- Tools such as `rack.statistics` and `rack.composite` use `argparse` with JSON config support
  (`rack.config`: precedence CLI > JSON config > defaults). Keep new tools consistent with this.
- Use Python 3 with the standard library, and keep third-party dependencies optional.

## Python bindings: open design question

Today Python talks to Rack by **generating command lines and running the binary**. "Bindings" can
mean two different things, and the choice changes the work a lot:

1. **Improve the subprocess wrapper** (typed signatures, better errors, config round-tripping).
   Low risk, works with any installed `rack` binary.
2. **In-process bindings** (for example pybind11 around `CommandBank` / `Program`, or around
   specific `drain`/product classes). This needs a shared library build target and the removal of
   blockers that are known to exist: process-global singletons (`drain::getCommandBank()`, static
   flags such as `DetectorOp::SUPPORT_UNIVERSAL`), `exit()` calls in commands, logging written
   straight to stderr, and the gnu++11 baseline, which recent binding libraries may not support.

Don't start implementing option 2 without agreeing on the scope with the user. Structural
refactors that remove the blockers above are worthwhile either way and fit the readability focus.

## Code conventions

- Match the surrounding style: tabs for indentation in C++, `CamelCase` classes, `camelCase`
  members and commands, Doxygen comments (`///`, `/** */`).
- Headers use `#include <drain/...>` for the library and quoted paths relative to `src/` for Rack
  code (`"data/ODIM.h"`).
- Logging: `drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);` with `mout.debug/info/note/warn/
  fail/error`. Don't write to `std::cout` for diagnostics, because stdout can carry data output.
- New files carry the MIT license header used in existing sources.
- Large blocks of commented-out code are common. When refactoring, you may remove dead code that
  you have confirmed is unused, but do it in separate commits from functional changes.
- Keep diffs focused. The code base is large (about 600 C++ files, 160k lines), so avoid sweeping
  reformatting.

## Versioning and commits

- The version is `RACK_VERSION` in [src/main/rack.h](src/main/rack.h). Don't bump it unless asked.
- Don't commit generated artifacts (`src/Build/`, `src/rack`, `src/install-rack.cnf`,
  `compile_commands.json`). Regenerate `docs/` only when asked.
- Commit only when the user asks.
