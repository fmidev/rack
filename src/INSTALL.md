Installation
=================

**Rack** is a C++ based program for weather radar data processing. It supports
quality control of measurement volumes, product generation and compositing, for
example. The program works on command line; there is no graphical interface.


Prerequisites
-------------

Three libraries are required:

* Hdf5   (libhdf5.so)	HDF5 data files
* Png    (libpng.so)	png image files
* Proj4  (libproj.so)	geographic projections  

Optionally:

* GeoTIFF (libtiff.so, libgeotiff.so)  geolocated image files (output)

In many systems, the general libraries can be installed for example
as follows:

```bash
  sudo apt-cache search libhdf-dev 
```
If those version are not found, try searching with a part of the name, like
```bash
  apt-cache search libhdf
```

To check if the libraries are already installed, try utilities like
```bash
  find /usr/include -name '*hdf5*'
```
Other recommended utilities are `locate` and `pkg-config`.





Downloading
-----------

* Latest version: https://github.com/fmidev/rack
* Old version:    http://baltrad.fmi.fi/software/rack/download.html

Command:

```bash
git clone https://github.com/fmidev/rack.git
```


Compilation
-----------

After donwloading the code, move to rack/src/ directory and:

1. Run `./configure.sh`  
   Edit the resulting install-rack.cnf if needed.
   
2. Run `./build.sh`  
   The command unzips make.zip to Release/ directory and calls 'make all' there. 
   If you encounter compilation problems after installing several versions,
   issue `./build.sh clean` and rerun `./build.sh`.

3. Run `./install.sh`  
   In success, the script will also prompt for copying utility scripts in
   the same target directory

If `hdf5` or `sz` libraries raise errors upon invocation of Rack, ensure that
both libraries are available, for example with

```
  export LD_RUN_PATH=$LD_RUN_PATH:/var/opt/lib
```


Note on Eclipse settings
------------------------

Rack has been developed using *Eclipse IDE*, with the following settings.

### Prequisites

Install standard C++ tools and the libraries required by Rack. For example:
```
apt install git make g++ 
apt install 
```

Download (clone) the Rack software in a temporary directory, for example:
```
clone https://github.com/fmidev/rack.git ~/rack-git
```

### All configurations

* Create project "rack": C++ Managed Build, Executable, Empty
* Exit Eclipse
* Move code to Eclipse project dir. For example: `mv ~/rack-git/* ~/rack-git/.git ~/eclipse-workspace/rack/`
* Restart Eclipse and refresh (F5) "rack" project dir.

* Exclude "examples" and "demo" dirs from build.

**C++ linker**, libraries: 

* hdf5
* proj
* png 
* tiff, gtiff (optional)

### "Debug" configuration:

C++ Build/Settings/C++ Compiler/Includes: Include paths
```
${workspace_loc:}
```

C++ linker, library paths:
```
${workspace_loc:drain/Debug}
```

### "Release" configuration:

C++ build, environment (variables) or C++ preprocessor:
```
Defined symbols (-D) :
USE_GEOTIFF_${USE_GEOTIFF}
VT100
```



C++ compiler, include paths:
```
$$hdfroot/include
$$projroot/include
```

C/C++ Build, Environment:

Create variables:
|CCFLAGS|-std=gnu++11 -fopenmp -I/usr/include/geotiff -I/usr/include/hdf5/serial   -I/usr/include/libpng12   -I/usr/include/x86_64-linux-gnu  -I/usr/include/geotiff|
|LDFLAGS|-std=gnu++11 -fopenmp -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5 -lproj -lpng12 -L/usr/lib/x86_64-linux-gnu -ltiff  -lgeotiff|
|USE_GEOTIFF|YES| 

C++ compiler, misc options:
```
$$CCFLAGS
```

C++ linker, misc options:
```
$$LDFLAGS
```

C++ linker, library search paths:
```
$$hdfroot/lib
$$projroot/lib
```

Documentation
-------------

Online code documentation containing examples of usage is on https://fmidev.github.io/rack/

In addition, a PDF manual may be shipped with this file, or can be generated with

```bash
  cd demo
  ./make-tests.sh
  cd ..
  make doc      # For html version
  make doc-pdf  # For rack-doc.pdf
```

Contact
-------

Markus Peura (First.Last@fmi.fi)


