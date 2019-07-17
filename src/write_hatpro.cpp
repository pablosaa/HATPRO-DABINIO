// *******************************************************************
// Code to create binari data files for HATPRO system
//
// USAGE:
//    > write_hatpro(BRT,'/data/myhatpro/file_basename');
//    > status = write_hatpro(BRT,'/data/myhatpro/file_basename');
//
// (c) 2018 Pablo Saavedra G. (pablo.saa@uib.no)
// Geophysical Institute, University of Bergen
//
// SEE LICENCE.TXT
// ********************************************************************

// TODO:
// * make subroutine to check whether all structure fields are present
// * test for MET files
// * implement for Profiles Create_BINfile subroutine in hatpro.cpp
// * make a single function by fusing with call_hatpro?

#include<vector>
#include "IOmex_hatpro.h"

using namespace std;


// Main MEX Function starts here:
void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[]){

  // Ckecking for input and output arguments
  if(nrhs!=2)
    mexErrMsgTxt("Two input arguments are needed! (See help)");

  if(!mxIsStruct(prhs[0]))
    mexErrMsgTxt("First argument needs to be a structure!");

  char *filen = NULL;
  int code = -1;
  if(mxIsChar(prhs[1])){
    size_t FileLength = mxGetN(prhs[1])+1;
    filen = (char *) mxCalloc(FileLength, sizeof(char));
    mxGetString(prhs[1], filen, FileLength);
    // Getting the HATPRO's code corresponding for file:
    code = hatpro::GetExtFromFile(filen);
    if(code<=0)
      mexErrMsgTxt("Please introduce a file with a extension supported by RPG GmbH.");
  }
  else mexErrMsgTxt("Second argument needs to be a string!");

  hatpro::WhatAmI(code);
  bool BRTflag = false;
  if(code==BRTcode || code==BRTcode-1 || code==BLBcode) BRTflag = true;
  
  MEXSTRUCT KAKES(code);

  mexPrintf("HATPRO file to create: %s with code %d, Nfileds: %d\n", filen, code,KAKES.NFields);


  // Getting the structure fields:
  int metaux = 0;
  int ND, NF, NA, NH;
  mxArray *TMP;
  
  // Retrieving the dimensions for the variables:
  TMP = mxGetField(prhs[0],(mwIndex) 0, "NUM_ELEMENTS");
  int *dims = (int*) mxGetData(TMP);
  
  ND = (int) dims[0];
  NF = (int) dims[1];
  NA = (int) dims[2];
  NH = (int) dims[3];

  hatpro::BRT_var BRT(ND, NF, NA);
  hatpro::PRO_var PRO(ND, NH);
  
  float BRTMin[NF], BRTMax[NF];

  float **myTIME = NULL; // auxiliary array to collect time array

  /* Iterating over Fields, starting at 1 because first field
     is NUM_ELEMENTS already read */
  for(uint i=1; i<KAKES.NFields; ++i){

    TMP = mxGetField(prhs[0],(mwIndex) 0, KAKES.FieldsName[i]);
  
    if(!strcmp(KAKES.FieldsName[i],"TIME")){
      myTIME = new float*[ND];
      for(int x=0; x<ND; ++x){
	myTIME[x] = new float[6];
	for(int y=0; y<6; ++y)
	  myTIME[x][y] = (float)  *(mxGetPr(TMP) + x + y*ND);
      }
    }

    if(!strcmp(KAKES.FieldsName[i],"REF"))
      BRT.TimeRef = (int) mxGetScalar(TMP);

    if(!strcmp(KAKES.FieldsName[i],"FRE"))
      BRT.Freq = (float *) mxGetData(TMP);
    
    if(!strcmp(KAKES.FieldsName[i],"ELV"))
      BRT.ELV = (float *) mxGetData(TMP);

    if(!strcmp(KAKES.FieldsName[i],"AZI"))
      BRT.AZI = (float *) mxGetData(TMP);
    
    if(!strcmp(KAKES.FieldsName[i],"RF"))
      BRT.RF = (int *) mxGetData(TMP);

    if(!strcmp(KAKES.FieldsName[i],"TB")){
      for(int x=0; x<ND; ++x)
	for(int y=0; y<NF; ++y)
	  for(int z=0; z<NA+1; ++z)
	    BRT.TB[x][y*(NA+1)+z] = (float) *(mxGetPr(TMP) + x + y*ND + z*ND*NF);

    } // end of TB if

    // ALTERNATIVE for MET files:
    if(!strcmp(KAKES.FieldsName[i],"P2m") ||
       !strcmp(KAKES.FieldsName[i],"T2m") ||
       !strcmp(KAKES.FieldsName[i],"RH2m") ||
       !strcmp(KAKES.FieldsName[i],"WS") ||
       !strcmp(KAKES.FieldsName[i],"WD") ||
       !strcmp(KAKES.FieldsName[i],"RR") ){
      for(int x=0; x<ND; ++x)
	BRT.TB[x][metaux] = (float) *(mxGetPr(TMP) + x);
      metaux++;
    }  // end of IF MET variables

    // Fields for PROFILES:
    if(!strcmp(KAKES.FieldsName[i],"RET"))
      PRO.Retrieval = (int) mxGetScalar(TMP);

    if(!strcmp(KAKES.FieldsName[i],"H"))
      PRO.Alts = (int *) mxGetData(TMP);

    if(!strcmp(KAKES.FieldsName[i],"QV") |
       !strcmp(KAKES.FieldsName[i],"T" )){
      for(int x=0; x<ND; ++x)
	for(int y=0; y<NH; ++y)
	  PRO.PRO[x][y] = (float) *(mxGetPr(TMP) + x + y*ND);
      // Finding out the min and max values
      hatpro::minmax_value(PRO.PRO, ND, 1, NH, &PRO.PROMin, &PRO.PROMax);
    }
    if(!strcmp(KAKES.FieldsName[i],"RH")){
      for(int x=0; x<ND; ++x)
	for(int y=0; y<NH; ++y)
	  PRO.PRO2[x][y] = (float) *(mxGetPr(TMP) + x + y*ND);
    }

   
  }  // end loop over number of fields (index i)

  // *** Starting assignation of class members:
  // 
  BRT.code = code;
  
  // Finding out the min and max values (frequenci dependent) values
  if(code==BLBcode || code==BLBcode-1 || code==BRTcode || code==METcode){
    hatpro::minmax_value(BRT.TB, ND, NF, NA+1, BRTMin, BRTMax);
    memcpy(BRT.BRTMin, BRTMin, NF*sizeof(BRTMin[0]));  
    memcpy(BRT.BRTMax, BRTMax, NF*sizeof(BRTMax[0]));
  }
  // Converting Calendar Date to TimeSeconds since epoc:
  hatpro::Date2TimeSec(myTIME, ND, BRT.TimeSec);
  
  // Converting Elevation and Azimuth into Angular array (BRT.Ang):
  hatpro::ElAzi2Angular(BRT.ELV, BRT.AZI, code==BLBcode?NA:ND, BRT.Ang);
  
  // coping common fields for PRO and BRT:
  if(!BRTflag){
    PRO.TimeSec = BRT.TimeSec;
    PRO.RF = BRT.RF;
    PRO.TimeRef = BRT.TimeRef;
  }

  if(code==METcode) BRT.Print_Data();
  for(int i=0; i<NF; ++i) cout<<BRT.Freq[i]<<"- "<<BRT.TimeSec[i]<<" ";
  cout<<endl;
  cout<<"MIN: ";
  for(int i=0; i<NF; ++i) cout<<BRT.BRTMin[i]<<" ";
  cout<<endl;
  cout<<"MAX: ";
  for(int i=0; i<NF; ++i) cout<<BRT.BRTMax[i]<<" ";
  cout<<endl;
  cout<<"ANG: ";
  
  for(int i=0; i<10; ++i) cout<<BRT.Ang[i]<<" ";
  cout<<endl;

  //BRT.Print_Data();
  BRT.Create_BinFile(filen);
  
  mxFree(filen);
  return;
}
// End of MEX function.


