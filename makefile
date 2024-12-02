# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

DIRS = bin build
CONFIG = release
SOURCES = check cpy define getpath listdir mytime organize passgen search spellcheck
SOURCES_BUILD = $(addsuffix -build,$(SOURCES))
SOURCES_INIT = $(addsuffix -init,$(SOURCES))

setup: $(DIRS)
$(DIRS):
	mkdir -p $@

build: setup $(SOURCES_BUILD)
$(SOURCES_BUILD):
	cd src/$(subst -build,,$@) && make clean build CONFIG=$(CONFIG)
	cp -afv src/$(subst -build,,$@)/bin/$(CONFIG)/* ./bin

init: $(SOURCES_INIT)
$(SOURCES_INIT):
	cd src/$(subst -init,,$@) && git submodule init && git submodule update && make dependencies

