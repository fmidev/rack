# Markus.Peura@fmi.fi

prefix=/opt

USE_GEOTIFF=NO

# Change to acrual Rack version?
VERSION=$(shell fgrep __RACK_VERSION__ main/rack.h | cut -d' ' -f 3 | tr -d '"' )

help:
#	@ X="`cat Makefile Makefile-doxygen | grep '^[a-z.-]\+:' | cut -d' ' -f1 | tr ':' '|' `" && echo make \[ $$X \]
	@ X="`cat Makefile Makefile-doxygen | grep '^[a-z.-]\+:' | cut -d':' -f1 | tr ':' '|' `" && echo make \[ $$X \]

# New
all: 
	@ echo "Using GeoTIFF: ${USE_GEOTIFF}"
	./build.sh

# Old, eclipse-imitating scheme
release: make.tgz
	@ echo "Using GeoTIFF: ${USE_GEOTIFF}"
	tar xzf make.tgz 
	cd Release; make CCFLAGS="${CCFLAGS}" LDFLAGS="${LDFLAGS}" USE_GEOTIFF="${USE_GEOTIFF}" -k all

# For eclipse based 
make.tgz: Release/
	tar czf $@ `find Release/ -name makefile -or -name '*.d' -or -name '*.mk' `

cppcheck:
	cppcheck --language=c++ --error-exitcode=1 --quiet */*.{h,cpp} drain/*/*.{h,cpp} 


install:
	./install.sh
#	@echo 'Quick test:'
#	Release/rack --help > /dev/null
#	mkdir -v --parents ${prefix}/bin
#	cp -vi Release/rack ${prefix}/bin/rack-$(shell Release/rack --version | head -1) && cd ${prefix}/bin; ln -s ./rack-$(shell Release/rack --version | head -1) ./rack || ln -si ./rack-$(shell Release/rack --version | head -1) ./rack
# alpine Linux 'ln' does not support -i



#tests:
#	cd demo; ./make-tests.sh

BASE_IMAGE=ubuntu20
# BASE_IMAGE=alpine
# BASE_IMAGE=ubuntu18
# BASE_IMAGE=ubuntu20
# make rack7.20_ubuntu18

#HOSTNAME := $(shell hostname)
#apt-proxy-${HOSTNAME}.conf:
#	@ echo "Creating empty $@"
#	@ touch $@

#apt-proxy.conf: apt-proxy-${HOSTNAME}.conf
#	@ echo -n "Linking: "
#	@ ln -svf $< $@

#docker-config-${HOSTNAME}.json:
#	@ echo "Creating empty $@"
#	@ touch $@
#	@ echo "See also: ~/.docker/config.json"

#docker-config.json: docker-config-${HOSTNAME}.json
#	@ echo -n "Linking: "
#	@ ln -svf $< $@



HOSTNAME := $(shell hostname)
DOCKER_CONF_DIR=docker-conf-${HOSTNAME}
DOCKER_CONF_FILE=${DOCKER_CONF_DIR}/config.json
#${DOCKER_CONF_FILE}:
${DOCKER_CONF_DIR}:
	@ mkdir -v --parents $@
#@ mkdir -v --parents $(@D)
#@ touch $@
#	@ echo "Created dir '$@'."
	@ echo "See also the default conf: ~/.docker/config.json"
#	@ echo "Trying to set automatically:"
#	@ cat docker-conf/apt-proxy.conf.tpl | envsubst ${DOCKER_CONF_DIR}/config.json
#	@ cat docker-conf/config.json.tpl    | envsubst ${DOCKER_CONF_DIR}/config.json

${DOCKER_CONF_DIR}/config.json ${DOCKER_CONF_DIR}/apt-proxy.conf: ${DOCKER_CONF_DIR}
	@ echo "Creating empty '$@'."
	@ touch $@
	@ echo "$${http_proxy}"
	@ echo "$${https_proxy}"
	@ test "$${http_proxy}" && test "$${https_proxy}" && cat docker-conf/$(@F).tpl | envsubst > $@ || echo ok


docker-image: Dockerfile.${BASE_IMAGE} ${DOCKER_CONF_DIR}/config.json  ${DOCKER_CONF_DIR}/apt-proxy.conf
# Note: config precedes 'build' cmd
	@ echo "Cleaning local ./Build"
	rm -rf ./Build/*
	@ echo "Note: Docker conf files:"
	@ file ${DOCKER_CONF_DIR}/apt-proxy.conf
	@ file ${DOCKER_CONF_DIR}/config.json
	@ echo
#@ echo "Note: apt-get proxy conf file: apt-proxy.conf"
#@ file apt-proxy.conf
	docker --config ${DOCKER_CONF_DIR}  build . --build-arg CONF_DIR=${DOCKER_CONF_DIR} --file $<  --tag rack_${BASE_IMAGE}:${VERSION}
#docker build . --file $<  --tag rack$(shell Release/rack --version | head -1 | tr -d '[:space:]')_${BASE_IMAGE}

docker-image-test: # docker
	docker image history "rack_${BASE_IMAGE}:${VERSION}" || make docker-image
	docker run -t "rack_${BASE_IMAGE}:${VERSION}" rack --version


docker-image-push: docker
	docker tag "rack_${BASE_IMAGE}:${VERSION}" "quay.io/fmi/rack:${VERSION}"
	docker login quay.io
	docker push "quay.io/fmi/rack:${VERSION}"

docker-clean:
	docker system prune -f

clean:
	rm -fv Release/rack Release/*/*.o Release/drain/*/*.o 
	ls -d Build/ && rm -fv Build/*
#include Makefile-doxygen



