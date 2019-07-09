# Makefile for the HATPRO Library and Toolbox project 
#
# USAGE:
# * to create the only the static/dynamic library:
# > make
# * to create the MATLAB MEX function:
# > make matlab
# * to create the OCTAVE MEX function:
# > make octave
# * to compile for Windows via MinGW32:
# > make mingw
#
# --
# Part of the 'HATPRO-DABINIO' repository
# (c) 2018, Pablo Saavedra G.
# Geophysical Institute, University of Bergen
# SEE LICENCE.TXT
# -------------------------------------------------------------------------

FOO := $(firstword $(shell which matlab))

# Definition of Compilers
MWGCC = /usr/bin/x86_64-w64-mingw32-g++

GCC = g++
GAL = ar
MEX_GCC  = $(subst matlab,mex,$(shell readlink -f $(FOO)))
OCT_GCC  = /usr/bin/mkoctfile

# Flags
MYFLAG = -std=gnu++11
MWFLAG = $(MYFLAG) -static

# Definition of PATHS:
HOM_PATH = $(CURDIR)
LIB_PATH = $(HOM_PATH)/lib
BIN_PATH = $(HOM_PATH)/bin
TMP_PATH = $(HOM_PATH)/build
SRC_PATH = $(HOM_PATH)/src

$(BIN_PATH)/test_hatprolib : $(TMP_PATH)/test_hatprolib.o $(LIB_PATH)/libhatpro.a $(LIB_PATH)/libhatpro.so
	$(GCC) $(TMP_PATH)/test_hatprolib.o -o $@ -L$(LIB_PATH) -l:libhatpro.a

$(LIB_PATH)/libhatpro.a: $(SRC_PATH)/hatpro.cpp $(SRC_PATH)/hatpro.h
	$(GCC) $(MYFLAG) -fPIC -c $(SRC_PATH)/hatpro.cpp -o $(TMP_PATH)/hatpro.o
	$(GAL) rcs $@ $(TMP_PATH)/hatpro.o

$(LIB_PATH)/libhatpro.so: $(SRC_PATH)/hatpro.cpp $(SRC_PATH)/hatpro.h 
	$(GCC) -fPIC -shared $(MYFLAG) $(SRC_PATH)/hatpro.cpp -o $@ 

$(TMP_PATH)/test_hatprolib.o: $(SRC_PATH)/test_hatprolib.cpp $(SRC_PATH)/hatpro.h
	$(GCC) -c $(MYFLAG) $< -o $@

octave: $(SRC_PATH)/read_hatpro.cpp $(LIB_PATH)/libhatpro.so $(SRC_PATH)/write_hatpro.cpp 
	$(OCT_GCC) --mex $(SRC_PATH)/read_hatpro.cpp -o $(BIN_PATH)/read_hatpro.mex "-Wl,-rpath=$(LIB_PATH)" -L$(LIB_PATH) -lhatpro
	rm $(HOM_PATH)/read_hatpro.o
	$(OCT_GCC) --mex $(SRC_PATH)/write_hatpro.cpp -o $(BIN_PATH)/write_hatpro.mex "-Wl,-rpath=$(LIB_PATH)" -L$(LIB_PATH) -lhatpro
	rm $(HOM_PATH)/write_hatpro.o

matlab: $(SRC_PATH)/read_hatpro.cpp $(LIB_PATH)/libhatpro.a $(SRC_PATH)/write_hatpro.cpp 
	$(MEX_GCC) $(SRC_PATH)/read_hatpro.cpp -outdir $(BIN_PATH) -L$(LIB_PATH) -l:libhatpro.a
	$(MEX_GCC) $(SRC_PATH)/write_hatpro.cpp -outdir $(BIN_PATH) -L$(LIB_PATH) -l:libhatpro.a

mingw:	$(SRC_PATH)/test_hatprolib.cpp $(SRC_PATH)/hatpro.cpp $(SRC_PATH)/hatpro.h
	$(MWGCC) $(MYFLAG) -c $(SRC_PATH)/hatpro.cpp -o $(TMP_PATH)/mingw-w64_hatpro.o
	$(MWGCC) $(TMP_PATH)/mingw-w64_hatpro.o $(SRC_PATH)/test_hatprolib.cpp $(MWFLAG) -o $(BIN_PATH)/test_hatprolib.exe 

clean:
	rm $(TMP_PATH)/*.o $(BIN_PATH)/test_hatprolib


# ****************** Extra Info ************************
# MATLAB:
# Option 1) in case want to compile using the dynamic library libhatpro.so:
#	$(MEX_GCC) $(SRC_PATH)/write_hatpro.cpp -outdir $(BIN_PATH) LDFLAGS='$$LDFLAGS -Wl,-rpath=$(LIB_PATH)' -L$(LIB_PATH) -lhatpro
# Option 2) in case want to cimpile using static library libhatpro.a:
#	$(MEX_GCC) $(SRC_PATH)/write_hatpro.cpp -outdir $(BIN_PATH) -L$(LIB_PATH) -l:libhatpro.a
#
# same for Octave.
