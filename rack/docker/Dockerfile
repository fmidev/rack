
# Install environment & dependencies for rack

FROM ubuntu:18.04

RUN apt-get update && apt-get -y install g++ git make libproj-dev libhdf5-dev libtiff-dev libgeotiff-dev libpng-dev 




# Prepare for rack build

RUN git clone https://github.com/fmidev/rack.git

COPY install-rack.cnf rack/rack


# RUN cp ../install-rack.cnf rack/ && cd rack && ./build.sh

RUN cd rack/rack && ./build.sh







# Complete, finalize


COPY run.sh .
RUN ls -ltr 

WORKDIR rack
CMD ["./run.sh","Hello, world!"]




