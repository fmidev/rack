# Markus.Peura@fmi.fi

prefix=/opt

USE_GEOTIFF=NO

# 
#VERSION := 

help:
	@ X="`cat Makefile Makefile-doxygen | grep '^[a-z\-\.]\+:' | cut -d' ' -f1 | tr ':' '|' `"; echo make \[ $$X \]


release: #unpack
	@ echo "Using GeoTIFF: ${USE_GEOTIFF}"
	tar xzf make.tgz 
	cd Release; make CCFLAGS="${CCFLAGS}" LDFLAGS="${LDFLAGS}" USE_GEOTIFF="${USE_GEOTIFF}" -k all


install:
	@echo 'Quick test:'
	Release/rack --help > /dev/null
	mkdir -v --parents ${prefix}/bin
	cp -vi Release/rack ${prefix}/bin/rack-$(shell Release/rack --version | head -1) && cd ${prefix}/bin; ln -s ./rack-$(shell Release/rack --version | head -1) ./rack || ln -si ./rack-$(shell Release/rack --version | head -1) ./rack
# alpine Linux 'ln' does not support -i


make.tgz: Release/
	tar czf $@ `find Release/ -name makefile -or -name '*.d' -or -name '*.mk' `

#tests:
#	cd demo; ./make-tests.sh

# BASE_IMAGE=alpine
BASE_IMAGE=ubuntu18

# make rack7.20_ubuntu18
docker: Dockerfile.${BASE_IMAGE}
	docker build . --file $<  --tag rack$(shell Release/rack --version | head -1 | tr -d '[:space:]')_${BASE_IMAGE}


clean:
	rm -fv Release/rack Release/*/*.o Release/drain/*/*.o 

include Makefile-doxygen



