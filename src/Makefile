# Markus.Peura@fmi.fi

prefix=/opt

#USE_GEOTIFF=NO
USE_GEOTIFF=YES

# Change to acrual Rack version?
VERSION=$(shell fgrep __RACK_VERSION__ main/rack.h | cut -d' ' -f 3 | tr -d '"' )

help:
#	@ X="`cat Makefile Makefile-doxygen | grep '^[a-z.-]\+:' | cut -d' ' -f1 | tr ':' '|' `" && echo make \[ $$X \]
	@ X="`cat Makefile Makefile-doxygen | grep '^[a-z.-]\+:' | cut -d':' -f1 | tr ':' '|' `" && echo make \[ $$X \]

# New
build: 
	USE_GEOTIFF=${USE_GEOTIFF} ./build.sh

cppcheck:
	cppcheck --language=c++ --error-exitcode=1 --quiet */*.{h,cpp} drain/*/*.{h,cpp} 

install:
	./install.sh

BASE_IMAGE=ubuntu20
# BASE_IMAGE=alpine
# BASE_IMAGE=ubuntu18
# BASE_IMAGE=ubuntu20
# make rack7.20_ubuntu18


HOSTNAME := $(shell hostname)
DOCKER_CONF_DIR=docker-conf-${HOSTNAME}
# DOCKER_CONF_FILE=${DOCKER_CONF_DIR}/config.json

${DOCKER_CONF_DIR}:
	@ mkdir -v --parents $@
	@ echo "See also the default conf: ~/.docker/config.json"

${DOCKER_CONF_DIR}/config.json ${DOCKER_CONF_DIR}/apt-proxy.conf: ${DOCKER_CONF_DIR}
	@ echo "Creating empty '$@'."
	@ touch $@
	@ echo "$${http_proxy}"
	@ echo "$${https_proxy}"
# If proxy settings not empty, store them in config.json
	@ test "$${http_proxy}" && test "$${https_proxy}" && cat docker-conf/$(@F).tpl | envsubst > $@ || echo ok


docker-image: Dockerfile.${BASE_IMAGE} ${DOCKER_CONF_DIR}/config.json  ${DOCKER_CONF_DIR}/apt-proxy.conf
# Note: config precedes 'build' cmd
	@ echo "Cleaning local ./Build"
	ls -d Build/ && rm -rf ./Build/*
	@ echo "Docker main configuration file (read through --config ARG)"
	@ file ${DOCKER_CONF_DIR}/config.json
	@ echo "Proxy configuration applied by Dockerfile (read through --build-arg CONF_DIR=...)"
	@ file ${DOCKER_CONF_DIR}/apt-proxy.conf
	@ echo
	docker --config ${DOCKER_CONF_DIR}  build . --build-arg CONF_DIR=${DOCKER_CONF_DIR} --file $<  --tag rack_${BASE_IMAGE}:${VERSION}


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
	ls -d Build/ && rm -vrf Build/*




