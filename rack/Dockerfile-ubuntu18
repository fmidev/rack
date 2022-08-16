# Main level Dockerfile
# Markus Peura  fmi.fi

# Install environment & dependencies for rack

FROM ubuntu:18.04
RUN apt-get update && apt-get -y install g++ git make libproj-dev libhdf5-dev libtiff-dev libgeotiff-dev libpng-dev 

# Prepare for rack build

# RUN git clone https://github.com/fmidev/rack.git
COPY . /rack
# Override default
COPY docker/install-rack.cnf /rack

# Build: after this the binary is in rack/Release/rack and also copied to /usr/local/bin
RUN cd /rack && ./build.sh clean && ./build.sh

RUN apt -y install libhdf5-100 libproj12 libpng16-16 libtiff5 libgeotiff2 libgomp1
# libgomp1: Open MP 

# Remove build dependencies
RUN apt -y remove make libproj-dev libhdf5-dev libtiff-dev libgeotiff-dev libpng-dev

