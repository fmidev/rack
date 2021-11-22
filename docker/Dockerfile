
FROM ubuntu

COPY .  rack
WORKDIR rack

# RUN ls -ltr /usr/bin
# RUN apt-get -c apt.conf update
# RUN apt-get update &&

#RUN apt-get update && apt-get -y install g++ git make libproj-dev libhdf5-dev libtiff-dev libgeotiff-dev libpng-dev
RUN apt-get update && apt-get -y install libproj-dev libhdf5-dev libtiff-dev libgeotiff-dev libpng-dev 

RUN ls -ltr 
ls -ltrd /usr/include/{,x86_64-linux-gnu/}{tiff,hdf5,png,proj}*

# RUN git clone https://github.com/fmidev/rack.git


#RUN cp -f ./install-rack.cnf rack/
#WORKDIR rack/rack
#RUN ./configure.sh

RUN cat install-rack.cnf

# RUN ./build.sh

CMD ["./run.sh","Hello, world!"]



