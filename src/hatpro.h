#ifndef HATPRO
#define HATPRO

#include<iostream>
#include<fstream>
#include<cstring>
#include<iomanip>
#include<time.h>
#include<stdlib.h>
#include<math.h>
#include<map>
#include<set>

// Following RPG data file code numbers:
const int BRTcode = 666667;
const int BLBcode = 567845848;
const int OLCcode = 955874342;
const int WVLcode = 456783953;
const int ATNcode = 7757564;
const int TPCcode = 780798065;
const int TPBcode = 459769847;
const int HPCcode = 117343672;
const int LPRcode = 4567;
const int METcode = 599658944;
const int LWPcode = 934501978;
const int IWVcode = 594811068;
const int BLHcode = 1777786;
const int CBHcode = 67777499;
const int STAcode = 454532;

/* Declaring the data base variable and the rows indexes.
   The number of elements depends on the number of simulations which
   are writen at the begining of the binary data file */

/* The structure of the database is:

*/

namespace hatpro {

  // Genereic Brightness Temperature class:
  // for HATPRO files .BRT, .BLB and .OLC
  class BRT_var{
  public:
    int code;                  // File type code number
    int Ndata;                 // Total Number of recorded samples
    int Nfreq;                 // Total Number of frequencies
    int Nang;                  // Total Number of scaning angles
    int TimeRef;               // Time reference (1:UTC, 0: Local Time)
    int Retrieval;             // 0:lin-reg, 1:^2-reg, 2: NN,3: Tmr
    float *BRTMin, *BRTMax;    // Min and Max of TB database [K]
    float *Freq;               // Frequencies for TB [GHz]
    float *Ang;                // Scaning angles [deg]
    int *TimeSec;              // Number of seconds since 1.01.2001
    int *RF;                  // binary Rain Flag: 0 no rain, 1 rain
    float *AZI;                // Azimuth angle [deg]   ( 00 to 360)
    float *ELV;                // Elevation angle [deg] (-90 to 180)
    float **TB;                // Brightness Temperature [K]
    // class default constructor:
  BRT_var() : Ndata(0), Nfreq(0), Nang(0) {}
  BRT_var(int ND, int NF, int NA) : Ndata(ND), Nfreq(NF), Nang(NA) {
      Initialize_it();
    }    
    bool Read_BinFile(const char *filename);
    void Print_Data();
    void Create_BinFile(const char *foutname);
  protected:
    std::set<int> myChildren = {BRTcode-1, BRTcode, BLBcode, OLCcode, ATNcode, METcode};
    void Initialize_it(){
      BRTMin = new float[Nfreq];
      BRTMax = new float[Nfreq];
      Freq   = new float[Nfreq];
      Ang    = new float[code==BLBcode?Nang:Ndata];
      TimeSec= new int[Ndata];
      RF     = new int[Ndata];
      AZI = new float[code==BLBcode?Nang:Ndata];
      ELV = new float[code==BLBcode?Nang:Ndata];
      TB = new float*[Ndata];
      for(int i=0;i<Ndata;++i)
	TB[i] = new float[Nfreq*(Nang+1)];
    }
  };  // end of BRT class
  // --------------------


  // --------------------------------------------------
  // Retrieval of Atmospheric and Meteorological Sensor class:
  class MET_var{
  public:
   int code;
   int Ndata;
   int NaddS;
   int TimeRef;
   int Retrieval;
   char AddSensor;
   float MinP, MaxP;
   float MinT, MaxT;
   float MinH, MaxH;
   float *MinAddS, *MaxAddS;
   int *TimeSec;
   int *RF;
   float **PTH;
   float **ExSensor;
   
  MET_var() : Ndata(0), AddSensor('0') {}

  MET_var(int ND, char ADDS) : Ndata(ND), AddSensor(ADDS) {
     Initialized_it();
   }
   void Print_Data();
   bool Read_BinFile(const char *filename);
   void Create_BinFile(const char *foutname);
  protected:
   std::set<int> myChildren = {METcode};
   void Initialized_it(){
     int c; // = AddSensor;
     TimeSec = new int[Ndata];
     RF      = new int[Ndata];
     PTH     = new float*[Ndata];
     // AddSensor 8 bit with one bit for extra sensor (see RPG manual)
     for(c=AddSensor,NaddS=0; c!=0; c>>=1) NaddS += (c & 0x01)?1:0;
     std::cout<<"NaddS="<<NaddS<<std::endl;
     MinAddS = new float[NaddS];
     MaxAddS = new float[NaddS];
     ExSensor= new float*[Ndata];
     for(int i=0;i<Ndata;++i){
       PTH[i] = new float[3];
       ExSensor[i] = new float[NaddS];
     }
   }
  };   // end of Class MET_var

  // -----------------------------------------------------
  // Generic Variable Profiler (Tropo, Boundary Layer, RH) class:
  class PRO_var{
  public:
    int code;                  // File type code number
    int Ndata;                 // Total Number of recorded samples
    int Nalt;                  // Total Number of heights
    int TimeRef;               // Time reference (1:UTC, 0: Local Time)
    int Retrieval;             // 0:lin-reg, 1:^2-reg, 2: NN,3: Tmr
    float PROMin, PROMax;      // Min and Max of TB database [K]
    int *Alts;                 // Heights [m]
    int *TimeSec;              // Number of seconds since 1.01.2001
    int *RF;                  // binary Rain Flag: 0 no rain, 1 rain
    float *AZI;                // Azimuth angle [deg]   ( 00 to 360)
    float *ELV;                // Elevation angle [deg] (-90 to 180)
    float **PRO;               // Profile variable [K], [g/kg], [%]
    float **PRO2;

    PRO_var() {}
  PRO_var(int ND, int NH) : Ndata(ND), Nalt(NH) {
      Initialized_it();
    }
    void Print_Data();
    bool Read_BinFile(const char *filename);
    bool Create_BinFile(const char *foutname);

  protected:
    std::set<int> myChildren = {TPCcode,TPBcode,HPCcode,HPCcode+1,LWPcode,IWVcode,BLHcode,STAcode,CBHcode};
    void Initialized_it(){
      TimeSec = new int[Ndata];
      Alts = new int[Nalt];
      RF = new int[Ndata];
      if(code==IWVcode||code==LWPcode){
	AZI = new float[Ndata];
	ELV = new float[Ndata];
      }
      PRO = new float*[Ndata];
      if(code==HPCcode+1) PRO2 = new float*[Ndata];
      for(int i=0;i<Ndata;++i){
	PRO[i] = new float[Nalt];
	if(code==HPCcode+1) PRO2[i] = new float[Nalt];
      }
    }
  };  // end of Class PRO_var for Atmospheric Profiler

  

 
  // ==================================================
  //    Definition of AUX FUNCTIONS and SUBROUTINES:
  void Angular2ElAzi(float , float &, float &);
  void ElAzi2Angular(float *, float *, int, float *);
  int GetExtFromFile(const char *);
  int WhatAmI(int);
  void minmax_value(float **, int, int, int, float [], float []);
  // Subroutine to convert TimeSeconds from 2001.1.1 00:00:00 to Calendar Format:
  void TimeSec2Date(int, int* , int, float **);
  void Date2TimeSec(float **, int, int *);
  // --------------------------------------------------
  void ShowLicense();
  //{
  // std::cout<<"HATPRO_DABINIO library."<<std::endl;

  //std::cout<<"(c) 2019 Pablo Saavedra Garfias., Email: pablo.saa@uib.no"<<std::endl;
  //std::cout<<"Geophysical Institute, University of Bergen"<<std::endl;

  //std::cout<<"SEE LICENSE.TXT"<<std::endl;
  //};
  
} // end of namespace
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
// end of .H file





