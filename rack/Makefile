# Markus.Peura@fmi.fi

prefix=/opt

USE_GEOTIFF=NO

#VERSION := $(shell Release/rack --version | head -1)

help:
	@ X="`cat Makefile Makefile-doxygen | grep '^[a-z\-\.]\+:' | cut -d' ' -f1 | tr ':' '|' `"; echo make \[ $$X \]


release: #unpack
	@ echo "Using GeoTIFF: ${USE_GEOTIFF}"
	tar xzf make.tgz --keep-newer-files
	cd Release; make CCFLAGS="${CCFLAGS}" LDFLAGS="${LDFLAGS}" USE_GEOTIFF="${USE_GEOTIFF}" -k all

#unpack:
#	unzip -o make.zip
#	tar xvzf make.tgz

install:
	@echo 'Quick test:'
	Release/rack --help > /dev/null
	mkdir -v --parents ${prefix}/bin
	cp -vi Release/rack ${prefix}/bin/rack-$(shell Release/rack --version | head -1) && cd ${prefix}/bin; ln -si ./rack-$(shell Release/rack --version | head -1) ./rack

make.tgz: Release/
	tar czf $@ `find Release/ -name makefile -or -name '*.d' -or -name '*.mk' `


#tests:
#	cd demo; ./make-tests.sh

# make docker-ubuntu18
# docker-%: Dockerfile-%
docker: Dockerfile
#	docker build . --tag rack6_full:latest
#	echo LABEL=$* FILE1=$< TARGET=$@
	docker build . --tag rack6_ubuntu18:latest


clean:
	rm -fv Release/rack Release/*/*.o Release/drain/*/*.o 


#PROJECT_LOGO=demo/rack-logo-110x55.png
#PROJECT_NAME=' '
include Makefile-doxygen



