// **********************************************************************************
//
// MEX Function to interact with Hatpro_Library from MATLAB/Octave
// The Function reads HATPRO binary files and outputs as MATLAB structure with
// field names corresponding to the type of file it belongs.
//
// USAGE:
// Option 1: To open file browser to select a binary HATPRO file,
//     > BRT = read_hatpro;
//
// Option 2: To read a specific given file name as string variable,
//     > BRT = read_hatpro('/hatpro/data/Y2018/M09/D20/180920.BRT');
//
// Option 3: To read a set of associated binary files e.g BRT, HPC, TPC, MET
//     > DAT = read_hatpro('/hatpro/data/Y2018/M09/D20/180920.BRT',{'HPC','TPC','MET'});
// WHERE:
//  * BRT is a Matlab structure
//  * DAT is a cell array, with every cell element corresponds to a structure for the
//        corresponding data file.
//
// --
// (c) 2018 Pablo Saavedra G.
// pablo.saa@uib.no
// Geophysical Institute, University of Bergen
// SEE LICENSE.TXT
// ***********************************************************************************

#include<vector>
#include "IOmex_hatpro.h"

using namespace std;

//int GetInputFile_MWR(vector<string> &);

mxArray *DataFile2MexStruct(const char *FileName){

  int metaux=0;
  int code = hatpro::GetExtFromFile(FileName);
  hatpro::WhatAmI(code);

  MEXSTRUCT SU(code);

  bool IsPRO, IsBRT;
  hatpro::PRO_var PRO;
  hatpro::BRT_var BRT;

  // Reading File and Filling Class members:
  IsPRO = PRO.Read_BinFile(FileName);
  IsBRT = BRT.Read_BinFile(FileName);

  // define dimension variables:
  uint tDims = IsPRO?PRO.Ndata:BRT.Ndata;  // time
  uint hDims = IsPRO?PRO.Nalt:0;  // height
  uint aDims = IsBRT?BRT.Nang:0;  // angle
  uint fDims = IsBRT?BRT.Nfreq:0;  // frequency

  uint NELEN[4] = {tDims, fDims, aDims, hDims};

  // ------
  // Converting RPG time format to year,month,day,hour,min,secs:
  float **date;
  date = hatpro::TimeSec2Date(tDims, IsBRT?BRT.TimeSec:PRO.TimeSec);

  for(size_t i=0;i<SU.NFields;++i) cout<<SU.FieldsName[i]<<" ";
  cout<<endl;

  // Creating the structure to put the set of variable fields:
  mxArray *stru = mxCreateStructMatrix(1, 1, SU.NFields, (const char **) SU.FieldsName);

  for(size_t i=0;i<SU.NFields;++i){
    mxClassID  myID = mxDOUBLE_CLASS;
    uint xDim=1, yDim=1, zDim=1;   // temporal dimensions
    mxArray *TMP = NULL;
    float *pointt = NULL;
    float **point2D = NULL;

    // Data Dimesions:
    if(!strcmp(SU.FieldsName[i], "NUM_ELEMENTS")){
      xDim = 1;
      yDim = 4;
      myID = mxUINT32_CLASS;
      pointt = (float *) NELEN;
    }

    // For 1-D variables:
    if(!strcmp(SU.FieldsName[i],"REF")){
      xDim=1;
      myID = mxINT32_CLASS;
      pointt = IsBRT?(float *) &BRT.TimeRef:(float *) &PRO.TimeRef;
    }

    if(!strcmp(SU.FieldsName[i],"RET")){
      xDim=1;
      myID = mxINT32_CLASS;
      pointt = (float *) &PRO.Retrieval;
    }

    if(!strcmp(SU.FieldsName[i],"H")){
      myID = mxSINGLE_CLASS;
      xDim = hDims;
      // converting from int to float;
      pointt = new float[hDims];
      for(uint h=0; h<hDims; ++h)
	pointt[h] = static_cast<decltype(myID)>(PRO.Alts[h]);
      cout<<SU.FieldsName[i]<<" H "<<hDims<<endl;
    }

    if(!strcmp(SU.FieldsName[i],"FRE")){
      myID = mxSINGLE_CLASS;
      yDim = fDims;
      pointt = (float *) BRT.Freq;
    }

    // Time depending 1D variables:
    if(!strcmp(SU.FieldsName[i],"TIME")){
      cout<<SU.FieldsName[i]<<" TIME "<<tDims<<endl;
      myID = mxDOUBLE_CLASS;
      xDim = tDims;
      yDim = 6;
      point2D = date;
    }
    if(!strcmp(SU.FieldsName[i],"RF")){
      cout<<SU.FieldsName[i]<<" RainFlag"<<endl;
      myID = mxSINGLE_CLASS;
      xDim = tDims;
      pointt = new float[tDims];
      // temporal for RFI testing:
      short *RFI;
      if(IsBRT) RFI = BRT.FlagTB_RIF_Wet();

      for(uint t=0; t<tDims; ++t)
        pointt[t] = IsBRT?(float) (0x01 & RFI[t]):(float) PRO.RF[t];
	//pointt[t] = IsBRT?(float) BRT.RF[t]:(float) PRO.RF[t];

    if(IsBRT) delete[](RFI);
    }
    if(!strcmp(SU.FieldsName[i],"ELV")){
      cout<<SU.FieldsName[i]<<" Elevation"<<endl;
      myID = mxSINGLE_CLASS;
      yDim = code==BLBcode? aDims: tDims;
      pointt = IsBRT?BRT.ELV: PRO.ELV;
    }
    if(!strcmp(SU.FieldsName[i],"AZI")){
      cout<<SU.FieldsName[i]<<" Azimuth"<<endl;
      myID = mxSINGLE_CLASS;
      yDim = code==BLBcode? aDims: tDims;
      pointt = IsBRT?BRT.AZI: PRO.AZI;
    }

    if(!strcmp(SU.FieldsName[i],"IWV") ||
       !strcmp(SU.FieldsName[i],"LWP") ||
       !strcmp(SU.FieldsName[i],"BLH") ){
      cout<<SU.FieldsName[i]<<" Integrated Water Vapour/ LWP"<<endl;
      myID = mxDOUBLE_CLASS;
      xDim = tDims;
      point2D = PRO.PRO;
    }
    if(!strcmp(SU.FieldsName[i],"STI") ){
      myID = mxDOUBLE_CLASS;
      xDim = tDims;
      yDim = 6;
      point2D = PRO.PRO;
    }
    // Time depending 2D Variables:
    if(!strcmp(SU.FieldsName[i], "TB")){
      myID = mxDOUBLE_CLASS;
      xDim = tDims;
      yDim = fDims;
      zDim = aDims+1;
      point2D = BRT.TB;
    }

    if(!strcmp(SU.FieldsName[i],"T") || !strcmp(SU.FieldsName[i],"QV")){
      myID = mxDOUBLE_CLASS;
      xDim = tDims;
      yDim = hDims;
      cout<<SU.FieldsName[i]<<" Temperature/absolute humidity"<<endl;
      point2D = PRO.PRO;
    }

    if(!strcmp(SU.FieldsName[i],"RH")){
      cout<<SU.FieldsName[i]<<" RH"<<endl;
      myID = mxDOUBLE_CLASS;
      xDim = tDims;
      yDim = hDims;
      point2D = PRO.PRO2;
    }


    // Defining Mex variable to allocate the data:
    //TMP = mxCreateNumericMatrix(xDim, yDim, myID, mxREAL);
    const mwSize xyDims[3] = {(mwSize) xDim, (mwSize) yDim, (mwSize) zDim};
    TMP = mxCreateNumericArray((mwSize) 3, xyDims, myID, mxREAL);

    // for 1D variables only:
    if(pointt!=NULL && point2D==NULL)
      memcpy(mxGetPr(TMP), pointt, xDim*yDim*sizeof(myID));

    // for 2D variables:
    else if(pointt==NULL && point2D!=NULL){
      //cout<<"2D variable "<<xDim<<"x"<<yDim<<endl;
      for(uint t=0; t<xDim; ++t){
	for(uint h=0; h<yDim; ++h)
	  for(uint z=0; z<zDim; ++z)
	    *(mxGetPr(TMP) + t + h*xDim + z*xDim*yDim) = static_cast<double>(point2D[t][h*zDim+z]);
      }
    }
    else if(code==METcode){
      // Special case for the Meteorological variables which are stored
      // at the BRT class within the TB array:
      cout<<"met variable: "<<SU.FieldsName[i]<<endl;
      TMP  = mxCreateNumericMatrix((mwSize) tDims,(mwSize) 1, mxDOUBLE_CLASS, mxREAL);

      for(uint t=0; t<tDims; ++t)
	*(mxGetPr(TMP) + t) = static_cast<double>(BRT.TB[t][metaux]);

      cout<<"before struct assigment"<<metaux<<endl;
      metaux++;
    }
    else mexErrMsgTxt("both pointer are still NULL!!! :O");

    mxSetFieldByNumber(stru,0,i,TMP);

    pointt  = NULL;
    point2D = NULL;
  } // end of NFields loop...

  delete[] date;
  return(stru);
}   // End of Template Struct
// --


// ***********************************************************************
// Main MEX Function:
// Called from Matlab/Octave to read HATPRO binary files.
// The output is a single structure or cell of structures containing
// the data according to the input file type.
//
void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[]){


  char *filen = NULL;
  vector<string> InFiles;
  bool MULTIFILES = false;
  unsigned int NTotalFiles = 0;
  // Checking consistency of the input and output arguments:
  if(nlhs==0){
    ShowHelp(1);
    mexErrMsgTxt("One output parameter needed! (See above help)");
  }
  if(nrhs>2){
    ShowHelp(1);
    mexErrMsgTxt("Max Two input arguments needed! (See help)");
  }
  if(nrhs==2 && (!mxIsChar(prhs[0]) || !mxIsCell(prhs[1]))){
    ShowHelp(1);
    mexErrMsgTxt("ERROR: input argument should be Char and cell e.g. 'input_file.BRT',{'TPB','MET','BLB'}");
  }

  switch(nrhs){
  case 0:
    // No input argument is given:
    if(GetInputFile_MWR(InFiles)!=0)
      mexErrMsgTxt("Wrong input radiometer file!");
    NTotalFiles = InFiles.size();
    if(NTotalFiles == 1)
      filen =  &(InFiles.at(0)[0]);
    else
      MULTIFILES = true;
    break;
  case 1:
    // one input argument: filename
    if(mxIsChar(prhs[0])){
      NTotalFiles = 1;
      size_t FileLength = mxGetN(prhs[0])+1;
      filen = (char *) mxCalloc(FileLength, sizeof(char));
      mxGetString(prhs[0], filen, FileLength);
      InFiles.push_back(filen);
      mexPrintf("HATPRO file to open: %s\n",filen);
    }
    else if(mxIsCell(prhs[0])){
      NTotalFiles = mxGetN(prhs[0]);
      for(unsigned int i=0; i<NTotalFiles; ++i){
	mxArray *aFile = mxGetCell(prhs[0],i);
	if(!mxIsChar(aFile)) mexErrMsgTxt("Sorry input file is not a string!");
	size_t FileLength = mxGetN(aFile)+1;
	mxGetString(aFile,filen,FileLength);
	InFiles.push_back(filen);
      }
    }
    else mexErrMsgTxt("First input needs to be a string FILENAME.");
    break;
  case 2:
    {
    // two input arguments: base_filename, {'ext1','ext2','ext3'}
    // For base filename:
    size_t FileLength = mxGetN(prhs[0])+1;
    mxGetString(prhs[0],filen,FileLength);
    InFiles.push_back(filen);
    cout<<"Input file: "<<InFiles.at(0)<<endl;
    // For secundary files:
    NTotalFiles += mxGetN(prhs[1]);
    // Extracting the file extension from base file name:
    size_t ppos = InFiles.at(0).find_last_of('.');

    for(unsigned int i=0;i<NTotalFiles-1; ++i){
      mxArray *aFile = mxGetCell(prhs[1],i);
      if(!mxIsChar(aFile)) mexErrMsgTxt("Element cell is not a string");
      size_t FileLength = mxGetN(prhs[1])+1;
      mxGetString(aFile,filen+ppos,FileLength);
      InFiles.push_back(filen);
      cout<<"Input file: "<<InFiles.at(i+1)<<endl;
    }
    break;
    }
  default:
    mexErrMsgTxt("Ups! something went wrong with input variables!");
  }   // end switch

  cout<<"starting to read "<<NTotalFiles<<" input files..."<<endl;
  if(MULTIFILES)  plhs[0] = mxCreateCellMatrix(NTotalFiles,1);

  mxArray *stru2;
  for(uint i=0;i<NTotalFiles;++i){
    stru2 = DataFile2MexStruct(InFiles.at(i).c_str());

    if(NTotalFiles>1) mxSetCell(plhs[0],i,stru2);
    else plhs[0] = stru2;
  }
  return;
}
// --- End of main MEX Function.
// ======================================================================================

// ************************************************************************************
// END OF MEX FUNCTION FILE

