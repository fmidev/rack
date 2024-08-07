# Main level Dockerfile for ubuntu20
# Markus Peura  fmi.fi

# Install environment & dependencies for rack

FROM ubuntu:20.04 AS builder

ARG CONF_DIR=.

# Set proxy and test it
COPY $CONF_DIR/apt-proxy.conf /etc/apt/apt.conf.d/proxy.conf
RUN cat /etc/apt/apt.conf.d/proxy.conf
RUN set | fgrep proxy || echo "No proxy env variables set"
# RUN set | fgrep proxy && ping -c 3 -w 5  wwwcache.fmi.fi

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

# ./build.sh clean && \


#RUN ls /usr/local/bin/rack*

FROM ubuntu:20.04

RUN apt update && \
    	apt -y install \
       	libgomp1 \
#       libhdf5-100 \
        libhdf5-103 \
	libproj15 \
#       libproj12 \
        libpng16-16 \
        libtiff5 \
	libgeotiff5 \
#       libgeotiff2 \
    	&& apt clean

# RUN ls /usr/local/bin/rack*

COPY --from=builder /usr/local/bin /usr/local/bin

RUN ls /usr/local/bin/rack*

CMD ["rack","--version"]


