

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

Hint: take a look at the `Dockerfile`(s) in [rack/src](.) directory for applicable library versions.


Download (clone) the Rack software in a temporary directory, for example:
```
clone https://github.com/fmidev/rack.git ~/rack-git
```

Initial set-up
--------------

Here, it will be assumed that `$ECLIPSE_DIR` is the workspace directory used by Eclipse – typically `~/eclipse-workspace`.

* Create: New Project named `rack` (C++ managed build, executable, empty)
* Create: Source Folder `src`
* Exit Eclipse
* In shell, *remove* the just created subdirectory `$ECLIPSE_DIR/rack/src` .
* Move the downloaded Rack code to the workspace dir. For example: `mv ~/rack-git/* ~/rack-git/.git $ECLIPSE_DIR/rack`
* Now directory `$ECLIPSE_DIR/rack/src` should appear again. 
* Restart Eclipse and refresh (F5) project directory `rack`.

Main settings
-------------

In all the following settings, apply them globally (ie. for both Debug and Release) by selecting `Configuration: All configurations` in applicable dialog boxes.

Create variables
................


'-std=gnu++11 -fopenmp'

CCFLAGS -I/usr/include/geotiff -I/usr/include/hdf5/serial   -I/usr/include/libpng12
LDFLAGS -lproj -lpng12 -ltiff -lgeotiff -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5

|CCFLAGS|-std=gnu++11 -fopenmp -I/usr/include/geotiff -I/usr/include/hdf5/serial   -I/usr/include/libpng12   -I/usr/include/x86_64-linux-gnu  -I/usr/include/geotiff|
|LDFLAGS|-std=gnu++11 -fopenmp -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5 -lproj -lpng12 -L/usr/lib/x86_64-linux-gnu -ltiff  -lgeotiff|
|USE_GEOTIFF|YES| 



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

Exclude appropriate dirs 
------------------------

Exclude examples and tests. For directory `examples`, select Properties (with right mouse button) and click `Exclude from build`.
