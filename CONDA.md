Installation using Conda (MiniConda)
====================================

Sometimes installing dependencies – HDF5, PNG, Proj, GeoTiff, etc. libraries – on system level is not possible or desired.
Then, one may use *virtual environment* created in a directory owned by the user.

The following instructions apply to [MiniConda](https://docs.anaconda.com/miniconda/), but other alternatives should work similarly.



Instructions (experimental)
---------------------------

```
# Create an environment (ie. a folder) for the dependencies. 
conda create -n rack-install

# Enter it
conda activate rack-install

# Add a code source suitable for C++ development
conda config --add channels conda-forge

# Main: install the required packages.
# conda install --yes libpng=1.6
conda install --yes proj
conda install --yes hdf5
conda install --yes libpng
conda install --yes geotiff

# Check the installed libraries - including their dependencies. 
conda list --explicit

# Check 
conda info

# For compiling Rack, the environment does not have to be activated. 
conda deactivate
```

Ensure the location of the environment, for example with ``conda info``
Typically, the environment is in path ``$USER/.conda/envs/rack-install`` .

Actual configuration is then done with
```
VENV_DIR=$USER/.conda/envs/rack-install [./configure.sh](./configure.sh)
```

After that, essential variables `CCFLAGS` and `LDFLAGS` should contain references to $VENV_DIR.
It is recommended to contain all the dependencies from the same source (system or Conda environment)
but mixed configurations may work, and look as follows, for example:
```
# Include paths
CCFLAGS='-std=gnu++11 -fopenmp   -I/usr/include -I/usr/include/libpng16 -I/home/fmi_ode/.conda/envs/rack-install/include'

# Library paths
LDFLAGS='-std=gnu++11 -fopenmp -L/usr/lib64 -lpng16 -L/home/user/.conda/envs/rack-install/lib -lproj -lhdf5 -lz -ltiff -lgeotiff'
```
Then, the build continues as follows, using `build.sh` as explained in [INSTALL.md](./INSTALL.md) .

Note. As *Rack* uses dynamic libraries, environment variable ```LD_LIBRARY_PATH```
should contain the path to locally installed libraries. This is typically done with something like:

```
export LD_LIBRARY_PATH
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$VENV_DIR/lib
```




After successful compilation of Rack, you can remove the virtual environment with ``conda ??? rack install`` 

