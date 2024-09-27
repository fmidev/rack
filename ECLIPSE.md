Notes on Eclipse settings
=========================

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

C++ build, environment (variables)
```
Defined symbols (-D) :
USE_GEOTIFF_${USE_GEOTIFF}
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


Author
------

Markus Peura (first.last@fmi.fi)


