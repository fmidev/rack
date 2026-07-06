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


src/js/%.h: src/js/%.js
	@ echo    '/** Automatically wrapped from "' $< '" */' > $@
	@ echo >> $@
	@ echo -n 'const char* ' >> $@
	@ echo -n $* | tr '-' '_' | tr -d '/.'  >> $@
#       @ echo    ' = R"JS(//<![CDATA[' | tr '-' '_' | tr -d '/.'  >> $@
	@ echo    ' = R"JS(/* ' $< ' */' >> $@
	cat  $<    >> $@
	@ echo ')JS";' >> $@
	git status $*
#       @ echo '//]]>)JS";' >> $@

#@ echo -n 'inline constexpr const char* ' >> $@

#https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

javascript: src/js/image_coord_tracker.h src/js/image_value_tracker.h src/js/base64ToArrayLE.h
	git status $^
	@echo Completed $@

include Makefile-doxygen

