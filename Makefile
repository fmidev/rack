# Root convenience wrapper — delegates all targets to src/Makefile.
# Run from the repository root or from within src/ — both work.
#
# Examples:
#   make              build rack
#   make configure    generate src/install-rack.cnf
#   make clean        remove build artefacts
#   make install      install rack

.PHONY: all build configure install install-python test-python clean compdb cppcheck help \
        docker-image docker-image-test docker-image-push docker-clean

all build configure install install-python test-python clean compdb cppcheck help \
docker-image docker-image-test docker-image-push docker-clean:
	$(MAKE) -C src $@

release:
	cd python && rack --python rack/core.py


javascript: src/drain/js/coords.h src/drain/js/coord_handler.h src/drain/js/textbox_flipper.h src/drain/js/data_value_tracker.h src/js/radar_data_encoding.h  src/js/image_coord_tracker.h  src/js/image_value_tracker.h src/js/base64ToArrayLE.h
	git status --short $^ $(patsubst %.h,%.cpp,$^)
	@echo Completed $@

%.h: %.js
	src/js2cpp.sh $<
	@ echo 'Checking version control...'
	git status $< $@ $(basename $@).cpp 

# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html



include Makefile-doxygen

