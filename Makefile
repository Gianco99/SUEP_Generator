# Makefile to build main example

# Pythia8. Make sure pythia8-config is available in the PATH
# Please also ensure that the environment variables PYTHIA8 and PYTHIA8DATA are set properly
PY8INC := $(shell pythia8-config --cxxflags)
PY8LIB := $(shell pythia8-config --ldflags)

# HepMC
HEPMCINC := "/cvmfs/sft.cern.ch/lcg/releases/LCG_88/HepMC/2.06.09/x86_64-slc6-gcc62-opt/include/"
HEPMCLIB := "/cvmfs/sft.cern.ch/lcg/releases/LCG_88/HepMC/2.06.09/x86_64-slc6-gcc62-opt/lib/"

# Boost
BOOSTINC := "/cvmfs/sft.cern.ch/lcg/releases/LCG_88/Boost/1.62.0/x86_64-slc6-gcc62-opt/include/boost-1_62/"
BOOSTLIB := "/cvmfs/sft.cern.ch/lcg/releases/LCG_88/Boost/1.62.0/x86_64-slc6-gcc62-opt/lib/"

.PHONY: all clean

all: suep_twosteps suep_userhook

suep_twosteps: suep_twosteps.o suep_shower.o
	$(CXX) $(PY8LIB) -L ${HEPMCLIB} -lHepMC $^ -o $@

suep_userhook: suep_userhook.o suep_shower.o DecayToSUEP.o
	$(CXX) $(PY8LIB) -L ${HEPMCLIB} -lHepMC $^ -o $@

%.o: %.cxx
	$(CXX) -c -Wall $(PY8INC) -I ${HEPMCINC} -I ${BOOSTINC} -o $@ $^

clean:
	@rm suep_twosteps suep_userhook *.o
