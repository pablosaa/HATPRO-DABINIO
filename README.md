# HATPRO-DABINIO

## HATPRO Radiometer DAta BINary Input/Output Library

This is a Library of functions to work with the binary data files of the [RPG GmbH](http://www.radiometer-physics.de) radiometer HATPRO.
This is a research software toolbox part of the project ''Off-shore Boundary Layer Observatory ([OBLO](http://oblo.w.uib.no/))'' at the [Geophysical Institute's](https://www.gfi.uib.no), University of Bergen. 

The Library contains a set of C++ classes to read the HATPRO binary data files as well as to write data files mimicing the radiometer's binary format. The classes and subroutines can be linked as dynamic or static library directly with C/C++ or Fortran 90 programs, alternatively can also be binded to scripting languages like Matlab, GNU/Octave or Python.

This repository contains simple examples (as MEX functions) to use the library from GNU/Octave and Matlab.

## Usage
### Stand-alone
Stand alone executables can be produced by linking the static (``lib/libhatpro.a``) or dynamic (``lib/libhatpro.so``) library.
For instance, in repository directory ``examples/windplanner/`` there is a small stand-alone program which gets a ``.MET`` file as input and outputs the 10 minutes average of wind direction and speed:

    > ./getMETwinddir /home/user/data/190522.MET
    > 146.201 7.80501

### GNU/Octave & MATLAB
There are two basic binding functions for GNU/Octave & MATLAB®, one to read binary data files and the other to write data into files alike the original binary files produced by the HATPRO radiometer.

#### Files with measurend variables:
To read data files, the usage from MATLAB command window is as follow:

    > BRT = read_hatpro;
    > BRT = read_hatpro('/HATPRO/data/Y2018/M09/D13/180913.BRT');
    
where the output is a MATLAB structure containing the data. The structure fields are mainly:

    > BRT.NUM_ELEMENTS      % an (1x4) array with dimensions e.g. [N_time N_angle N_frequency N_altitudes]
    > BRT.REF               % scalar with 0=local time, 1=UTC time
    > BRT.TIME              % (N_time x 6) array containing the data time [year month day hour min sec]
    > BRT.RF                % (N_time x 1) with rain flag for the data (0=no rain, 1=rain)
    > BRT.FREQ              % (N_frequency x 1) array with the values of the microwave frequencies [GHz]
    > BRT.ELV               % (N_time x 1) array with the elevation for the corresponding measurement [deg]
    > BRT.AZI               % (N_time x 1) array with the azimuth fot the corresponding measurement [deg]
    > BRT.TB                % (N_time x N_freq x N_angle) array with the Brightness Temperatures [K]
    
the last field can be different for other data types, 

#### Files with retrieval variables:
For data files containing retrieval variables the structure fields vary according to the type of file. For instance when HATPRO's temperature profile is read (file ``*.TPC``) then the fields are:
    
    > TP = read_hatpro('/HATPRO/data/Y2018/M09/D13/180913.TPC');
    > TP.H                  % (N_altitute x 1) array with profile altitudes [m]
    > TP.T                  % (N_time x N_altitude) array with temperature profile [K]
    
Likewise for the case when a humidity file is read (``*.HPC``) then the fields ``.QV`` and ``.RH`` for specific and relative humidity respectivey are present.

    > HP = read_hatpro('/HATPRO/data/Y2018/M09/D13/180913.HPC');
    > HP.RH                 % (N_time x N_altitute) array with relative humidity profiles [%]
    > HP.QV                 % (N_time x N_altitude) array with specific humidity profiles [g/m³]


#### Writting binary data:
To write a binary file, a data structure as shown above needs to be created first. Once the data is introduced/modified in the respectives structure fields a binary file can be generated as follow:

    > write_hatpro(BRT,'/newdata/hatprofile.BRT');
    
where the first argument ``BRT`` is a structure containing the data to storage and the second argument is a string containing the full path for the file where to store the data.
__INPORTANT__: the file name's extension must comply with the data file names used by RPG, for example ``.BRT``, ``.BLB``,``.MET``, etc. Otherwise the data structure won't be in complaint with the requared data file format and the function will return an error.

### Binding with Python
The binding with Python is done via SWIG, therefore a dynamic library is created by runinng the make file as:

    > make python
    
and the ``_hatpro.so`` module will be created in the directory ``lib/python`` which can be used from python as any other module:

    > python3.5
    Python 3.5.2 (default, Nov 12 2018, 13:43:14) 
    [GCC 5.4.0 20160609] on linux
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import hatpro
    >>> help(hatpro)
    >>> brt = hatpro.BRT_var()
    >>> hatpro.BRT_var.Read_BRTfile(brt, "090905.BRT")
    >>> hatpro.BRT.var.Print_Data(brt)
    >>> hatpro.WhatAmI(brt.code)
     % BLBT code is : 567845848

### Binding with FORTRAN
Some times when HATPRO data is used as input for some high performance computing e.g. retrievals, data assimilation or model initialization, then it is useful to call the library from stablished Fortran programs which are mostly the case for HPC. The binding is done via Fortran intrinsic module ctypes. 
Work in progress...


## Compilation
To compile the library or any application using the library, from the base directory and a Linux terminal run:

    linux@home> make
for the stand-alone test program, or

    linux@home> make matlab
    linux@home> make octave
for the MATLAB or GNU/Octave MEX function, correspondently.
For a MS-Windows version of the stand-alone program, compilation is done using mingw32-g++ compiler:

    linux@home> make mingw
which will generate a ``.exe`` MS-Windows executable file in the bin/ directory.

## Compatibility
### Software
The library has been tested under Linux, for the __OpenSUSE Leap 14.3__ and __Ubuntu 16.04 Xenial__ distributions. The stand-alone program compiled with mingw-w64 has been successfully tested under MS-Windows 7.

### Hardware
It has been used with HATPRO radiometer generation 4 (G4), any contribution or testing on other generations are wellcome.

## Copyright
The library is sitributed under GPLv3 License. See LICENSE.TXT for details.

## Contact
Pablo Saavedra Garfias (pablo.saa@uib.no)

Geophysical Institute

University of Bergen

NORWAY
