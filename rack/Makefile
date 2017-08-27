# Markus.Peura@fmi.fi


#drainroot=/opt
#hdfroot=/opt
#projroot=/opt

prefix=/opt

USE_GEOTIFF=NO


help:
	@ X="`cat Makefile Makefile-doxygen | grep '^[a-z\-\.]\+:' | cut -d' ' -f1 | tr ':' '|' `"; echo make \[ $$X \]


release: unzip
	@ echo "Using GeoTIFF: ${USE_GEOTIFF}"
	cd Release; make CCFLAGS="${CCFLAGS}" LDFLAGS="${LDFLAGS}" USE_GEOTIFF="${USE_GEOTIFF}" -k all

unzip:
	unzip -o make.zip


install: 
	mkdir -v --parents ${prefix}/bin
	cp -vi Release/rack ${prefix}/bin

make.zip: Release
	zip $@ `find Release/ -name makefile -or -name '*.d' -or -name '*.mk' `


tests:
	cd demo; ./make-tests.sh


# FMI only
update:
	svn update

# FMI only
commit: make.zip
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
	rm -fv Release/rack Release/*/*.o                 

clean-geotiff:
	rm -fv rack/Release/rack rack/Release/main/commands.o rack/Release/radar/FileGeoTIFF.o

clean-full: clean
	rm -fv make.zip `find -name makefile -or -name '*.mk' -or -name '*.d'`


PROJECT_LOGO=demo/rack-logo-110x55.png
PROJECT_NAME=' '
include Makefile-doxygen

#	rm -f $@
#	zip $@ `find install-rack/ -name *.h -or -name *.cpp`
#	zip $@ make.zip
#	zip $@ Makefile Makefile-Doxygen Doxyfile.ini INSTALL COPYING COPYING.LESSER}



