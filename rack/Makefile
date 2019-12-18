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


tests:
	cd demo; ./make-tests.sh


# FMI only
update:
	svn update

# FMI only
commit: #make.tgz
	svn commit -m "${USER}@${HOSTNAME}:${PWD}" 
#	rm -vf make.zip



# DOCUMENTATION
doc-all: help.txt help-drain.txt help-composite.txt help-andre.txt examples.txt doc
	touch help.txt

examples.txt:
	grep '^[ ]\+FILES\?=.*'  main/*.dox > $@
	grep '^[ ]\+rack [^<]*$$' main/*.dox >> $@

help.txt:
	rack --help > $@

help-composite.txt:
	rack --help cart > $@

help-drain.txt:
	rack --help drain > $@

help-andre.txt:
	rack --man andre > $@
	rack --man ropo >> $@

#install-rack.zip: make.zip

clean:
	rm -fv Release/rack Release/*/*.o Release/drain/*/*.o 

clean-geotiff:
	rm -fv rack/Release/rack rack/Release/main/commands.o rack/Release/radar/FileGeoTIFF.o

clean-full: clean
	rm -fv  `find -name makefile -or -name '*.mk' -or -name '*.d'` # make.tgz


PROJECT_LOGO=demo/rack-logo-110x55.png
PROJECT_NAME=' '
include Makefile-doxygen

#	rm -f $@
#	zip $@ `find install-rack/ -name *.h -or -name *.cpp`
#	zip $@ make.zip
#	zip $@ Makefile Makefile-Doxygen Doxyfile.ini INSTALL COPYING COPYING.LESSER}



