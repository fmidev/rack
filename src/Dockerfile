# Dockerfile for quay.io, based on Dockerfile.ubuntu20
# Markus Peura  fmi.fi

# Install environment & dependencies for rack

FROM ubuntu:20.04 AS builder

#ARG CONF_DIR=.

# Set proxy and test it
#COPY $CONF_DIR/apt-proxy.conf /etc/apt/apt.conf.d/proxy.conf
#RUN cat /etc/apt/apt.conf.d/proxy.conf
#RUN set | fgrep proxy || echo "No proxy env variables set"

COPY . /rack
COPY install-rack-docker-ubuntu20.cnf /rack/install-rack.cnf

# Build: after this the binary is in rack/Release/rack and also copied to /usr/local/bin
WORKDIR /rack

RUN apt-get update && \
    apt-get -y install \
        g++ \
        make \
        libproj-dev \
        libhdf5-dev \
        libpng-dev \
        libtiff-dev \
        libgeotiff-dev \
        libgomp1 && \
    ./build.sh && ./install.sh



# Restart, and install only run-time dependencies.
FROM ubuntu:20.04

RUN apt update && \
    	apt -y install \
       	libgomp1 \
        libhdf5-103 \
	libproj15 \
        libpng16-16 \
        libtiff5 \
	libgeotiff5 \
    	&& apt clean


COPY --from=builder /usr/local/bin /usr/local/bin


RUN ls /usr/local/bin/rack*

CMD ["rack","--version"]


