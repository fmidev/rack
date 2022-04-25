
# Default variables that can be bypassed, eg. make TMPFILE=rack-$$ TAG=test rack6_runtime
TMPFILE := rack.tmp
TAG=latest

# The build steps can be listed with a plain ``make`` or ``make help`` command.
help:
	@grep --only-matching  '^[a-z0-9_]\+:' Makefile | tr '\n:' ' '
	@echo

# Build the initial, full image containing all the dependencies:
rack6_full: Dockerfile
	docker build . --file $< --tag $@:${TAG}

# Export the executable from the container to the working dir:
bin/rack: rack6_full
	@mkdir -v --parents /tmp
	@touch /tmp/${TMPFILE}
	docker run --mount src=/tmp,dst=/tmp,type=bind -t $< cp -fv /rack/rack/Release/rack /tmp/${TMPFILE}
	mv /tmp/${TMPFILE} $@
	chmod a+rx $@

# Build the final (runtime) image
rack6_runtime: Dockerfile.runtime bin/rack
	docker build . --file $< --tag $@:${TAG}
	@echo "Success. Test with: docker run -it $@ bash " 

# Testing a runtime container
test:
	docker run rack6_runtime rack --help
# docker run -it rack6_runtime /bin/bash
