
Using an IDE in developing Rack
===============================

Programming work is efficient when using an
*intergrated development environment*. 
Rack has been developed using *Eclipse IDE* with the settings described below.

This information serves only as an example and will probably not apply directly
to other IDEs.

The basic idea is to pack build options for C++ compiler and linker into 
variables `$CCFLAGS` and `$LDFLAGS`, respectively. This supports easier export
to installation environments.

Prequisites
-----------

Install standard C++ development tools. For example:
```
apt install git make g++
```

Install the libraries required by Rack. For example:
```
apt install libhdf5-dev libpng-dev libproj-dev libtiff-dev libgeotiff-dev libgomp1 
```

Hints:
# Take a look Dockerfiles in ./src for applicable library versions


Download (clone) the Rack software in a temporary directory, for example:
```
clone https://github.com/fmidev/rack.git ~/rack-git
```

Importing the code to Eclipse
-----------------------------

Here, it will be assumed that `$ECLIPSE_DIR` is the workspace directory used by Eclipse – typically `~/eclipse-workspace`.

* Create: New Project "rack": C++ Managed Build, Executable, Empty
* Create: Source Folder "src"
* Exit Eclipse
* In shell, *remove* the just created subdirectory `$ECLIPSE_DIR/rack/src` .
* Move the downloaded Rack code to Eclipse project dir. For example: `mv ~/rack-git/* ~/rack-git/.git $ECLIPSE_DIR/rack`
* Now directory `$ECLIPSE_DIR/rack/src` should appear again. 
* Restart Eclipse and refresh (F5) "rack" project dir.

Important settings in Eclipse
-----------------------------

* Exclude examples and tests. For directory `examples`, select Properties (with right mouse button) and click `Exclude from build`.

In all the following settings, apply them globally (ie. for both Debug and Release) by selecting `Configuration: All configurations` in applicable dialog boxes.

'-std=gnu++11 -fopenmp'

Variable (example)
CCFLAGS -I/usr/include/geotiff -I/usr/include/hdf5/serial   -I/usr/include/libpng12
LDFLAGS -lproj -lpng12 -ltiff -lgeotiff -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5


Settings
========

In all the following settings, apply them for
Configuration: All configurations


C++ Compiler 
------------

Top level command settings: `Expert settings: Command line pattern`:
Add `CCFLAGS` variable as the first command argument:
```
${COMMAND} ${CCFLAGS} ...
```

C++ Linker
----------

Top level command settings: `Expert settings: Command line pattern`:
Add `LDFLAGS` variable in the **end**:
```
${COMMAND} ...   ${LDFLAGS}
```
