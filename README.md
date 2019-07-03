# HATPRO-DABINIO

## HATPRO Radiometer DAta BINary Input/Output Library

This is a Library of functions to work with the binary data files of the [RPG GmbH](http://www.radiometer-physics.de) radiometer HATPRO.
This is a research software toolbox part of the project ''Off-shore Boundary Layer Observatory ([OBLO](http://oblo.w.uib.no/))'' at the [Geophysical Institute's](https://www.gfi.uib.no), University of Bergen. 

The Library contains a set of C++ classes to read the HATPRO binary data files as well as to write data files mimicing the radiometer's binary format. The classes and subroutines can be linked as dynamic or static library directly with C/C++ or Fortran 90 programs, alternatively can also be linked by means of wrappers for scripting languages like Matlab, GNU/Octave or Python.

This repository contains simple examples to use the library from GNU/Octave and Matlab as MEX functions.

## Usage
### Stand-alone
Stand alone executables can be produced by linking the static (``lib/libhatpro.a``) or dynamic (``lib/libhatpro.so``) library.
For instance, in repository directory ``examples/windplanner/`` there is a small stand-alone program which gets a ``.MET`` file as input and outputs the 10 minutes average of wind direction and speed:

    > ./getMETwinddir /home/user/data/190522.MET
    > 146.201 7.80501

### GNU/Octave & MATLAB
There are two main wrapper functions for Octave & MATLAB, one is to read binary data files and the other to write into files alike the original binary files produced by the HATPRO radiometer.

To read data files, the usage is as follow:

    > BRT = read_hatpro;
    > BRT = read_hatpro('/HATPRO/data/Y2018/M09/D13/180913.BRT');

To write a binary file, the usage is as follow:

    > write_hatpro(BRT,'/newdata/hatprofile.BRT');
    
where the first argument ``BRT`` is a structure containing the data to storage and the second argument is a string containing the full path file name for the file where to store the data.
INPORTANT: the file name's extension must comply with the data file names used by RPG, for example ``.BRT``, ``.BLB``,``.MET``, etc.

### Binding for FORTRAN
Work in progress...

## Compatibility
### Software
The library has been tested under Linux, for the __OpenSUSE Leap 14.3__ and __Ubuntu 16.04 Xenial__ distributions.

### Hardware
It has been used with HATPRO radiometer generation 4 (G4), any contribution or testing on other generations are wellcome.

## Copyright
The library is sitributed under GPLv3 License. See LICENSE.TXT for details.

## Contact
Pablo Saavedra Garfias (pablo.saa@uib.no)

Geophysical Institute

University of Bergen

NORWAY
