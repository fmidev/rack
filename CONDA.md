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
conda install proj
conda install hdf5
# conda install libpng=1.6
conda install libpng
conda install geotiff

# Check the installed libraries - including their dependencies. 
conda list --explicit

#
conda info

# For compiling Rack, the environment does not have to be activated. 
conda deactivate
```

Ensure the location of the environment, for example with ``conda info``.
Typically, the environment is in path ``$USER/.conda/envs/rack-install``


