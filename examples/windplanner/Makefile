# Makefile for the getMETwinddir code 
# getMETwinddir just read the HATPRO MET binary file and calculate the
# average 10minute wind speed and direction.
# additional input argument can be used to change the 10min average.
# For quick help how to use it, just write:
# >./getMETwinddir
#
# USAGE:
# * to create getMETwinddir for linux:
# > make
# * to create getMETwinddir.exe for Windows:
# > make mingw
#
# --
# (c) 2018, Pablo Saavedra G.
# Geophysical Institute, University of Bergen
# SEE LICENCE.TXT
# -------------------------------------------------------------------------

# Definition of Compilers
MWGCC = /usr/bin/x86_64-w64-mingw32-g++

GCC = g++

# Flags
MYFLAG = -std=gnu++11
MWFLAG = $(MYFLAG) -static

# Definition of PATHS:
HOM_PATH = /home/pga082/GFI/Repos/HATPRO_Toolbox
LIB_PATH = $(HOM_PATH)/lib
BIN_PATH = $(HOM_PATH)/bin
TMP_PATH = $(HOM_PATH)/build
SRC_PATH = $(HOM_PATH)/src
EXE_PATH = $(CURDIR)

$(EXE_PATH)/getMETwinddir : $(EXE_PATH)/getMETwinddir.cpp $(LIB_PATH)/libhatpro.a
	$(GCC) $(MYFLAG) $< -o $@ -I$(SRC_PATH) -L$(LIB_PATH) -l:libhatpro.a

$(TMP_PATH)/hatpro.o: $(SRC_PATH)/hatpro.cpp $(SRC_PATH)/hatpro.h 
	$(GCC) -fPIC -shared $(MYFLAG) $^ -o $@ 


mingw:	$(EXE_PATH)/getMETwinddir.cpp $(TMP_PATH)/mingw-w64_hatpro.o
	$(MWGCC) $(TMP_PATH)/mingw-w64_hatpro.o $(EXE_PATH)/getMETwinddir.cpp $(MWFLAG) -o $(EXE_PATH)/getMETwinddir.exe -I$(SRC_PATH)

clean:
	@echo $(SRC_PATH) 
	rm *.o 


#	$(MWGCC) $(MYFLAG) -c $(SRC_PATH)/hatpro.cpp -o $(TMP_PATH)/mingw-w64_hatpro.o
