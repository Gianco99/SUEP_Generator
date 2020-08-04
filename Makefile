# Makefile to build main example
# Assumes a Pythia8 installation with pythia8-config binary available in the PATH.


PY8INC := $(shell pythia8-config --cxxflags)
PY8LIB := $(shell pythia8-config --ldflags)

.PHONY: all clean

all: suep_main

suep_main: suep_main.o suep_shower.o
	$(CXX) $(PY8LIB) $^ -o $@

%.o: %.cxx
	$(CXX) -c -Wall $(PY8INC) -o $@ $^

clean:
	@rm suep_main *.o
