UNAME = $(shell uname -s)

ifndef VERBOSE
MAKEFLAGS += --no-print-directory
endif

ifeq ($(UNAME),Darwin)
ECHO=echo
else
ECHO=echo -e
endif

all: deps/czmq src

deps/czmq:
	-@$(ECHO) "\033[35mbuilding \033[1;32m./deps/czmq\033[0m"
	cd deps/czmq && ./autogen.sh && ./configure && make

src:
	-@$(ECHO) "\033[35mbuilding \033[1;32m./src\033[0m"
	mkdir -p build && cd build && cmake .. && make

clean:
	rm -rf build

.PHONY: deps/czmq src