
TMPFILE := rack.tmp
#TMPFILE := $(shell mktemp --suffix _rack)
#TMPFILE := rack-$$$$.tmp
TAG=latest

help:
	@grep --only-matching  '^[a-z0-9_]\+:' Makefile | tr '\n:' ' '
#@grep '^[[:alnum:]_]\+:' Makefile | tr '\n:' ' '  
	@echo


rack6_full: Dockerfile
	docker build . --file $< --tag $@:${TAG}

bin/rack: rack6_full
	@mkdir -v --parents /tmp
# Avoid root write...
	touch /tmp/${TMPFILE}
	docker run --mount src=/tmp,dst=/tmp,type=bind -t $< cp -fv /rack/rack/Release/rack /tmp/${TMPFILE}
	echo mv /tmp/${TMPFILE} $@  
	echo mv /tmp/${TMPFILE} $@  
	mv /tmp/${TMPFILE} $@
	chmod a+rx $@


rack6_runtime: Dockerfile.runtime bin/rack
	docker build . --file $< --tag $@:${TAG}
	@echo "Success. Test with: docker run -it $@ bash " 

