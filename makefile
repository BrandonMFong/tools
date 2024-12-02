# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CONFIG = release
SOURCES = check cpy define getpath listdir mytime organize passgen search spellcheck
SOURCES_BUILD = $(addsuffix -build,$(SOURCES))
SOURCES_INIT = $(addsuffix -init,$(SOURCES))
build: $(SOURCES_BUILD)
$(SOURCES_BUILD):
	cd src/$(subst -build,,$@) && make clean build CONFIG=$(CONFIG)

init: $(SOURCES_INIT)
$(SOURCES_INIT):
	cd src/$(subst -init,,$@) && git submodule init && git submodule update && make dependencies

