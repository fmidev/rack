# Rack — GNU Make build system
# Build configuration is read from install-rack.cnf (run: make configure).
#
# Usage:
#   make              build the rack executable (default)
#   make configure    detect libraries and write install-rack.cnf
#   make clean        remove Build/ directory and the executable
#   make install      install to prefix defined in install-rack.cnf
#   make compdb       generate compile_commands.json for IDE IntelliSense (needs bear)
#   make cppcheck     run static analysis

SHELL   := /bin/bash
CC      := g++
OBJ_DIR := Build
TARGET  := ./rack
CONF    := install-rack.cnf

# Optimisation / debug flags — override on the command line, e.g.: make OPTS="-O0 -g3"
OPTS ?= -O3 -Wall -fmessage-length=0

# ---- Read build settings from install-rack.cnf --------------------------------
#
# configure.sh writes only -I / -L / -l paths into CCFLAGS / LDFLAGS.
# configure-venv.sh writes individual path variables (HDF5_INCLUDE, HDF5_LIB, …);
# the shell expressions below assemble equivalent flags from those as a fallback.
# -std= and -fopenmp are NOT stored in the config file — the Makefile owns them.

ifneq ($(wildcard $(CONF)),)
  CCFLAGS := $(shell bash -c '\
    . ./$(CONF); \
    : "$${CCFLAGS:=-I$${HDF5_INCLUDE:-/usr/include/hdf5/serial} \
        -I$${GEOTIFF_INCLUDE:-/usr/include/geotiff} \
        -I$${PROJ_INCLUDE:-/usr/include} \
        -I$${PNG_INCLUDE:-/usr/include} \
        -I$${TIFF_INCLUDE:-/usr/include/x86_64-linux-gnu}}"; \
    echo "$$CCFLAGS"')
  LDFLAGS := $(shell bash -c '\
    . ./$(CONF); \
    : "$${LDFLAGS:=-L$${HDF5_LIB:-/usr/lib/x86_64-linux-gnu/hdf5/serial} -lhdf5 \
        -L$${PROJ_LIB:-/usr/lib/x86_64-linux-gnu} -lproj \
        -L$${PNG_LIB:-/usr/lib/x86_64-linux-gnu} -lpng \
        -L$${TIFF_LIB:-/usr/lib/x86_64-linux-gnu} -ltiff \
        -L$${GEOTIFF_LIB:-/usr/lib/x86_64-linux-gnu} -lgeotiff}"; \
    echo "$$LDFLAGS"')
  USE_GEOTIFF := $(shell bash -c '. ./$(CONF); echo "$${USE_GEOTIFF:-YES}"')
else
  $(warning WARNING: $(CONF) not found — using system-path defaults. Run: make configure)
  CCFLAGS     := -I/usr/include/hdf5/serial -I/usr/include/geotiff -I/usr/include \
                 -I/usr/include/x86_64-linux-gnu
  LDFLAGS     := -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5 \
                 -L/usr/lib/x86_64-linux-gnu -lproj -lpng -ltiff -lgeotiff
  USE_GEOTIFF := YES
endif

# Compile flags: Makefile owns -std= and -fopenmp; CCFLAGS adds -I paths only
CXXFLAGS := -std=gnu++11 $(OPTS) -fopenmp $(CCFLAGS) -I. -DUSE_GEOTIFF_$(USE_GEOTIFF)

# ---- Source / object lists ----------------------------------------------------
SRC_DIRS := andre data drain drain/image drain/imageops drain/util drain/prog \
            hi5 main product radar

SRCS := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.cpp))
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# ---- Primary build targets ----------------------------------------------------
.PHONY: all build configure install install-python test-python clean compdb cppcheck help

all: $(TARGET)

build: all

$(TARGET): $(OBJS)
	@echo "# Linking $@"
	$(CC) $(OPTS) -fopenmp -o $@ $^ $(LDFLAGS)
	@echo "# Build successful: $@"

# Compile each .cpp to a .o and generate a companion .d dependency file
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) -MMD -MP -c -o $@ $<

# Include auto-generated dependency files; silently skip on first build
-include $(DEPS)

# ---- Utility targets ----------------------------------------------------------
configure:
	./configure.sh

install:
	./install.sh

install-python:
	./install-python.sh

test-python:
	cd ../python && python3 -m unittest discover -s rack/tests -v

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Generate compile_commands.json for IDE IntelliSense (requires the 'bear' tool)
compdb: clean
	@command -v bear >/dev/null 2>&1 || \
	    { echo "Error: 'bear' not found. Install with: sudo apt install bear"; exit 1; }
	bear -- $(MAKE)

cppcheck:
	cppcheck --language=c++ --error-exitcode=1 --quiet \
	    */*.{h,cpp} drain/*/*.{h,cpp}

help:
	@echo "Usage: make [target]"
	@echo "  all / build   build rack executable (default)"
	@echo "  configure     detect libraries, write install-rack.cnf"
	@echo "  clean         remove Build/ directory and rack executable"
	@echo "  install       install to prefix in install-rack.cnf"
	@echo "  compdb        generate compile_commands.json (needs bear)"
	@echo "  cppcheck      static analysis"
	@echo "  docker-image  build Docker image"

# ---- Version (from source) ----------------------------------------------------
VERSION := $(shell fgrep RACK_VERSION main/rack.h 2>/dev/null | \
               tr '(,)' '|.|' | tr -d \'\" | cut -d'|' -f 2)
#               tr '(,)' '|.|' | tr -d \'\" | cut -d'|' -f 2)

# ---- Docker targets (preserved from original Makefile) -----------------------
BASE_IMAGE      := ubuntu20
HOSTNAME        := $(shell hostname)
DOCKER_CONF_DIR := docker-conf-$(HOSTNAME)

.PHONY: docker-image docker-image-test docker-image-push docker-clean

$(DOCKER_CONF_DIR):
	@ mkdir -v --parents $@
	@ echo "See also the default conf: ~/.docker/config.json"

$(DOCKER_CONF_DIR)/config.json $(DOCKER_CONF_DIR)/apt-proxy.conf: $(DOCKER_CONF_DIR)
	@ echo "Creating empty '$@'."
	@ touch $@
	@ test "$${http_proxy}" && test "$${https_proxy}" && \
	    cat docker-conf/$(@F).tpl | envsubst > $@ || echo ok

docker-image: Dockerfile.$(BASE_IMAGE) \
              $(DOCKER_CONF_DIR)/config.json $(DOCKER_CONF_DIR)/apt-proxy.conf
	@ echo "Cleaning local ./Build"
	ls -d Build/ && rm -rf ./Build/*
	docker --config $(DOCKER_CONF_DIR) build . \
	    --build-arg CONF_DIR=$(DOCKER_CONF_DIR) --file $< \
	    --tag rack_$(BASE_IMAGE):$(VERSION)

docker-image-test:
	docker image history "rack_$(BASE_IMAGE):$(VERSION)" || $(MAKE) docker-image
	docker run -t "rack_$(BASE_IMAGE):$(VERSION)" rack --version

docker-image-push: docker-image
	docker tag "rack_$(BASE_IMAGE):$(VERSION)" "quay.io/fmi/rack:$(VERSION)"
	docker login quay.io
	docker push "quay.io/fmi/rack:$(VERSION)"

docker-clean:
	echo docker system prune -f
