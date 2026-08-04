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


#xINCLUDE_TAG='//|#include'

#src/js/%.h: src/js/%.js
%.h: %.js
	@ echo 'Checking syntax...'
	js --check $<
	@ echo 'Ok'
	@ echo    '/** Automatically generated from "' $< '" */' > $@
	@ echo 'Wrapping to a C++ char array...'
	@ echo $(basename $@)
	@ echo $(notdir $@)
	@ echo $(notdir $(basename $@) )
	@ echo >> $@
	@ echo '#ifndef JS_INCLUDE_'$(notdir $(basename $@) ) >> $@
	@ echo '#define JS_INCLUDE_'$(notdir $(basename $@) ) >> $@
	@ echo >> $@
	@ cat $< | grep '^//|' | cut -d'|' -f2-  >> $@
	@ echo >> $@
	@ echo 'namespace javascript {' >> $@
#	@ cat $<  >> $@
	@ echo -n 'const char* ' >> $@
	@ echo -n $(notdir $(basename $@) ) >> $@
#	@ echo -n $* | tr '-' '_' | tr -d '/.'  >> $@
#       @ echo    ' = R"JS(//<![CDATA[' | tr '-' '_' | tr -d '/.'  >> $@
#	@ echo    ' = R"JS(/* ' $< ' */' >> $@
	@ echo    ' = R"JS(// Source: ' $< >> $@
	cat $< | grep -v '^//|' >> $@
	@ echo ')JS";' >> $@
	@ echo '} // javascript::' >> $@
	@ echo '#endif' >> $@
	@ echo 'Checking version control...'
	git status $*
#       @ echo '//]]>)JS";' >> $@

#@ echo -n 'inline constexpr const char* ' >> $@

#https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

javascript: src/drain/js/coords.h src/drain/js/coord_handler.h src/drain/js/textbox_flipper.h  src/js/image_coord_tracker.h src/js/image_value_tracker.h src/js/base64ToArrayLE.h
	git status $^
	@echo Completed $@

include Makefile-doxygen

