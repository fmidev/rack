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
