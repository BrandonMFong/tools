# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

DIRS = bin build
DIRS_CLEAN = $(addsuffix -clean,$(DIRS))
CONFIG = release
SOURCES = check cpy define getpath listdir mytime organize passgen search spellcheck
SOURCES_BUILD = $(addsuffix -build,$(SOURCES))
SOURCES_CLEAN = $(addsuffix -clean,$(SOURCES))
SOURCES_DEPS = $(addsuffix -deps,$(SOURCES))
SOURCES_INIT = $(addsuffix -init,$(SOURCES))

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

