
FROM ubuntu

COPY .  rack
WORKDIR rack

# RUN ls -ltr /usr/bin
# RUN apt-get -c apt.conf update
# RUN apt-get update &&

RUN apt-get update && apt-get -y install g++ git make libproj-dev libhdf5-dev libtiff-dev libgeotiff-dev libpng-dev 

RUN ls -ltr 

# RUN git clone https://github.com/fmidev/rack.git

WORKDIR rack
RUN git clone https://github.com/fmidev/rack.git


#RUN mv install-rack.cnf rack/ && cd rack && ./build.sh

WORKDIR rack/rack
RUN ./configure.sh

RUN cat install-rack.cnf

# RUN ./build.sh


#RUN tar xvzf rack.tgz

#RUN ls -ltr rack/
#RUN ls -ltr rack/Release



# RUN apt-get -y install libtiff-dev
# RUN apt-get -y install 

WORKDIR rack
CMD ["./run.sh","Hello, world!"]

# RUN gcc -o myapp main.c
# CMD ["./myapp"]


