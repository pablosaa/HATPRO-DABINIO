/***************************************************************
HEADER file containing funcions and subroutines for the 
GNU/Octave and Matlab wrapper for the HATPRO-dabinio library.

Part of the HATPRO_DABINIO repository.

(c) 2019 Pablo Saavedra G.
pablo.saa@uib.no
Geophysical Institute, University of Bergen

SEE LICENSE.TXT
****************************************************************/

#include "hatpro.h"
#include "mex.h"

using namespace std;

// *********************************************************************************
// ***  Struct to content variable names to create Mex Matlab structure
class MEXSTRUCT{
 public:
  const char **FieldsName;
  size_t NFields;
  MEXSTRUCT(){};
  MEXSTRUCT(int code);  // constructor
  ~MEXSTRUCT();   // destructor
 protected:

};

MEXSTRUCT::MEXSTRUCT(int code){
  vector<int> idx;
  map<int, vector<int>> indexes;
  const char *ListName[] = {"TIME","H","FRE","ELV","AZI",  // Dimension names: time, hight, freq, elv, azi {0 - 4}
			    "RF","REF","RET", // Basic names: rain-flag, timeRef, retrieval-id {5-7}
			    "TB","TAU",       // Measurement names: brightness temperature, attenuation {8-9}
			    "QV","RH","T",    // Profile names: specific, relative humidity, Temperature {10-12}
			    "P2m","T2m","RH2m", // Meteo names: Pressure, Temperature, RH {13-15}
			    "WS","WD","RR",     // Meteo names: Windspeed, direction, rain rate {16-18}
			    "LWC","LWP","IWV",  // Atmos names: Liquid water content, water path, int water vapour {19-21}
			    "STI","CBH","BLH"  // Atmos index: Stability indices, cloud base, PBL heihg {22-24}
  };
  
  indexes[BRTcode] = vector<int>{0,2,3,4,5,6,8};
  indexes[ATNcode] = vector<int>{0,2,3,4,5,6,9};
  indexes[WVLcode] = vector<int>{0,2,3,4,5,6,8};
  indexes[BLBcode] = vector<int>{0,2,3,4,5,6,8};
  indexes[HPCcode] = vector<int>{0,1,5,6,7,10,11};
  indexes[TPCcode] = vector<int>{0,1,5,6,7,12};
  indexes[TPBcode] = vector<int>{0,1,5,6,7,12};
  indexes[METcode] = vector<int>{0,5,6,13,14,15,16,17,18};
  indexes[LWPcode] = vector<int>{0,5,6,7,20};
  indexes[IWVcode] = vector<int>{0,5,6,7,3,4,21};
  indexes[STAcode] = vector<int>{0,5,6,22};
  indexes[BLHcode] = vector<int>{0,5,6,24};
  indexes[CBHcode] = vector<int>{0,5,6,23};
  
  
  idx = indexes[code];
  NFields = idx.size() + 1;
  FieldsName = (const char**) calloc(NFields, sizeof(*FieldsName));
  FieldsName[0] = "NUM_ELEMENTS";
  for(size_t i=1;i<NFields;++i) FieldsName[i] = ListName[idx.at(i-1)];
}

MEXSTRUCT::~MEXSTRUCT(void){
  delete FieldsName;
}
// --- End of NMEXSTRUCT class definition

int GetInputFile_MWR(vector<string> &InFiles){

  char *filen, *OUTDIR;
  int strLength, DirLength, status;
  mxArray *INVAR[4], *OUTVAR[2];

  //ShowGNUPL();      // displaying License, it is free!.
  mxArray *FilterCell = mxCreateString("*.BRT; *.BLB; *.TPB; *.TPC; *.MET; *.HPC; *.IWV; *.LWP; *.ATN; *.OLC; *.WVL; *.STA; *.CBH; *.BLH");
  INVAR[0] = mxCreateCellMatrix(1, 2);
  mxSetCell(INVAR[0],0,FilterCell);
  mxSetCell(INVAR[0],1,mxCreateString("HATPRO supported files"));

  INVAR[1] = mxCreateString("Select HATPRO input file...");
  INVAR[2] = mxCreateString("MultiSelect");
  INVAR[3] = mxCreateString("on");
  status = mexCallMATLAB(2,OUTVAR,4,INVAR,"uigetfile");
  if (status!=0) mexErrMsgTxt("File selection not possible!");

  // Getting the Directory where files are located:
  DirLength = mxGetN(OUTVAR[1]);
  if (DirLength<4)  mexErrMsgTxt("File selection empty or canceled!");
  OUTDIR = (char *) mxCalloc(DirLength+1,sizeof(char));
  mxGetString(OUTVAR[1], OUTDIR, DirLength);

  // Check if cell (mulltiple input files selected) or if string (single file selected)
  if (mxIsCell(OUTVAR[0])){
    mwSize Ninfile = mxGetN(OUTVAR[0]);
    for(int i=0; i<Ninfile; ++i){
      mxArray *InFile = mxGetCell(OUTVAR[0],i);
      strLength = mxGetN(InFile) +1 ;
      filen = (char *) mxCalloc(DirLength + strLength, sizeof(char));
      mxGetString(OUTVAR[1], filen, DirLength+1);
      mxGetString(InFile,filen + DirLength, strLength);
      cout<<filen<<endl;
      InFiles.push_back(filen);
    }
  }
  // passing the input and output path
  else{
    strLength = DirLength + mxGetN(OUTVAR[0]) +1 ;
    filen = (char *) mxCalloc(strLength, sizeof(char));
    mxGetString(OUTVAR[1],filen,strLength);
    // passing the file name
    strLength = mxGetN(OUTVAR[0])+1;
    mxGetString(OUTVAR[0],filen+mxGetN(OUTVAR[1]),strLength);
    InFiles.push_back(filen);
    mexPrintf("HATPRO file chosen: %s\n",filen);
  }
  
  return(status);
}
// ================ End of HATPRO Input File Dialog-Box ========================


// ********* Routine to Show HELP MESSAGE *****************************
void ShowHelp(unsigned int key){
  switch(key){
  case 1:
    cout<<"USAGE:"<<endl<<
      "Option 1: To open file browser to select a binary HATPRO file,"<<endl<<
      "> BRT = read_hatpro;"<<endl<<endl<<
      "Option 2: To read a specific given file name as string variable,"<<endl<<
      "> BRT = read_hatpro('/hatpro/data/Y2018/M09/D20/180920.BRT');"<<endl<<endl<<
      
      "Option 3: To read a set of associated binary files e.g BRT, HPC, TPC, MET"<<endl<<
      "> DAT = read_hatpro('/hatpro/data/Y2018/M09/D20/180920.BRT',{'HPC','TPC','MET'});"<<endl<<
      "WHERE:"<<endl<<
      "* BRT is a Matlab structure"<<endl<<
      "* DAT is a cell array, with every cell element corresponds to a structure"<<endl<<
      "for the corresponding data file."<<endl<<endl;

    break;
  case 2:
    cout<<"USAGE:"<<endl<<
      "   > write_hatpro(BRT,'/data/myhatpro/file_basename.BRT');"<<endl<<
      "   > status = write_hatpro(BRT,'/data/myhatpro/file_basename.BRT');"<<endl<<endl;
    break;
  }
  hatpro::ShowLicense();
  return;
}
// ========= End of Routine Show HELP MESSAGE =========================

// END OF HEADER FILE
