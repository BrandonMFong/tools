# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

DIRS = bin build
DIRS_CLEAN = $(addsuffix -clean,$(DIRS))
CONFIG = release
SOURCES = archiver chat check cpy define getpath listdir mytime organize passgen search spellcheck
SOURCES_BUILD = $(addsuffix -build,$(SOURCES))
SOURCES_CLEAN = $(addsuffix -clean,$(SOURCES))
SOURCES_DEPS = $(addsuffix -deps,$(SOURCES))
SOURCES_INIT = $(addsuffix -init,$(SOURCES))

help:
	@echo "Usage:"
	@echo "	make <target> <variables>"
	@echo ""
	@echo "Target(s):"
	@echo "	clean			cleans build and bin folder, as well as the sources'"
	@echo "	build 			builds release verions"
	@echo "	init 			initializes the submodules"
	@echo "	dependencies		builds all dependencies in the sources' external directory"
	@echo ""
	@echo "Example(s):"
	@echo "	Build for the first time"
	@echo "		make init dependencies build"

setup: $(DIRS)
$(DIRS):
	mkdir -p $@

clean: $(DIRS_CLEAN) $(SOURCES_CLEAN)
$(DIRS_CLEAN):
	rm -rfv $(subst -clean,,$@)
$(SOURCES_CLEAN):
	cd src/$(subst -clean,,$@) && make clean

build: setup $(SOURCES_BUILD)
$(SOURCES_BUILD):
	cd src/$(subst -build,,$@) && make build CONFIG=$(CONFIG)
	cp -afv src/$(subst -build,,$@)/bin/$(CONFIG)/* ./bin

init: $(SOURCES_INIT)
$(SOURCES_INIT):
	cd src/$(subst -init,,$@) && git submodule init && git submodule update

dependencies: $(SOURCES_DEPS)
$(SOURCES_DEPS):
	cd src/$(subst -deps,,$@) && make dependencies

