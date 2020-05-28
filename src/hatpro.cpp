#include "hatpro.h"

using namespace std;

/* Global variables */

// ======================================================
//    READING SUBROUTINES
// ------------------------------------------------------

// Subroutine to READ new BT binary data files:
// for file types *.BRT, *.BLB, *.OLC, *.ATN, *.WVL, *.MET [TODO: *.IRT]
bool hatpro::BRT_var::Read_BinFile(const char *foutname){

  ifstream fre;
  fre.open(foutname,ios::in|ios::binary);
  if(!fre.good()){
    cout<<"ERROR: sorry, file cannot be opened!"<<endl;
    return false;
  }

  fre.read((char *) &code, sizeof(int));
  if(myChildren.count(code)==0){
    fre.close();
    return false;
  }

  fre.read((char *) &Ndata, sizeof(int));
  
  if(code==BRTcode || code==BRTcode-1 || code==ATNcode){
    Nang = 0;
    fre.read((char *) &TimeRef, sizeof(int));
    if(code==ATNcode) fre.read((char*) &Retrieval, sizeof(int));
    fre.read((char *) &Nfreq, sizeof(int));
    Initialize_it();
    fre.read((char *) Freq, Nfreq*sizeof(float));
    fre.read((char *) BRTMin, Nfreq*sizeof(float));
    fre.read((char *) BRTMax, Nfreq*sizeof(float));
  }

  if(code==BLBcode){
    fre.read((char *) &Nfreq, sizeof(int));
    float TMPMin[Nfreq];
    float TMPMax[Nfreq];
    float TMPFreq[Nfreq];
    fre.read((char *) TMPMin, Nfreq*sizeof(float));
    fre.read((char *) TMPMax, Nfreq*sizeof(float));
    fre.read((char *) &TimeRef, sizeof(int));
    fre.read((char *) TMPFreq, Nfreq*sizeof(float));
    fre.read((char *) &Nang, sizeof(int));
    Initialize_it();
    memcpy(BRTMin, TMPMin, Nfreq*sizeof(float));
    memcpy(BRTMax, TMPMax, Nfreq*sizeof(float));
    memcpy(Freq, TMPFreq, Nfreq*sizeof(float));
    fre.read((char *) Ang, Nang*sizeof(float));
    for(int k=0; k<Nang; ++k) hatpro::Angular2ElAzi(Ang[k], ELV[k], AZI[k]);
  }

  if(code==OLCcode || code==WVLcode){  // OLC (Oxigen-Line chart)
    Nang = 0;
    fre.read((char *) BRTMin, sizeof(float));
    fre.read((char *) BRTMax, sizeof(float));
    fre.read((char *) &TimeRef, sizeof(int));
    fre.read((char *) &Nfreq, sizeof(int));
    Initialize_it();
    fre.read((char *) Freq, Nfreq*sizeof(float));
  }

  if(code==METcode){  // MET for meteorological station
    char AddSensor;
    int c;
    fre.read((char *) &AddSensor, sizeof(char));
    // AddSensor 8 bit with one bit for extra sensor (see RPG manual)
    for(c=AddSensor,Nang=0; c!=0; c>>=1)
      Nang += (c & 0x01)?1:0;
    //std::cout<<"AddSensor is: "<<AddSensor<<" NaddS="<<Nang<<std::endl;
    Nfreq = 3+Nang;  // for Pressure, Temperature and RH + extra sensor
    Nang = 0;        // Reset 
    Initialize_it();
    fre.read((char *) BRTMin, Nfreq*sizeof(float));
    fre.read((char *) BRTMax, Nfreq*sizeof(float));
    fre.read((char *) &TimeRef, sizeof(int));
  }

  for(int i=0; i<Ndata; i++){
    fre.read((char *) &TimeSec[i], sizeof(int));
    char RFtmp;
    fre.read((char *) &RFtmp, sizeof(char));
    RF[i] = (int) RFtmp;

    fre.read((char *) TB[i], Nfreq*(Nang+1)*sizeof(float));

    if(code!=BLBcode && code!=METcode){
      //float ANG;
      fre.read((char *) &Ang[i], sizeof(Ang[0]));
      hatpro::Angular2ElAzi(Ang[i], ELV[i], AZI[i]);
    }
  }
  fre.close();
  return true;
}  // ... End of Reading Generic BT binary file.
// -----------------------------------


// **********************************************
// Funtion to read MET binary data file:
bool hatpro::MET_var::Read_BinFile(const char * filename){
  ifstream fin;
  //int Ndat, METcode;
  //char AddSensor;
  fin.open(filename,ios::in|ios::binary);
  if(!fin.good()){
    cout<<"ERROR: sorry, MET file cannot be opened!"<<endl;
    return false;
  }

  fin.read((char *) &code, sizeof(int));
  if(myChildren.count(code)==0){
    fin.close();
    return false;
  }
    
  fin.read((char *) &Ndata, sizeof(int));
  fin.read((char *) &AddSensor, sizeof(char));
  //hatpro::MET_var MET(Ndat,AddSensor);
  Initialized_it();
  //MET.METcode = METcode;
  fin.read((char *) &MinP, sizeof(float));
  fin.read((char *) &MaxP, sizeof(float));
  fin.read((char *) &MinT, sizeof(float));
  fin.read((char *) &MaxT, sizeof(float));
  fin.read((char *) &MinH, sizeof(float));
  fin.read((char *) &MaxH, sizeof(float));
  for(int i=0;i<NaddS;++i){
    fin.read((char *) &MinAddS[i], sizeof(float));
    fin.read((char *) &MaxAddS[i], sizeof(float));
  }
  fin.read((char *) &TimeRef, sizeof(int));
  for(int i=0;i<Ndata;++i){
    fin.read((char *) &TimeSec[i], sizeof(int));
    char RFtmp;
    fin.read((char *) &RFtmp, sizeof(char));
    RF[i] = atoi((const char *) &RFtmp);

    fin.read((char *) PTH[i], 3*sizeof(float));
    fin.read((char *) ExSensor[i], NaddS*sizeof(float));
  }
  
  fin.close();
  return true;
}

// ---------------------------------------------------------
// Function to read Profiler variables binary data files,
// Type supported: (TPC,TPB,HPC,LPR,IWV,LWP)
bool hatpro::PRO_var::Read_BinFile(const char *filename){
  ifstream fin;

  fin.open(filename,ios::in|ios::binary);
  if(!fin.good()){
    cout<<"ERROR: sorry, file cannot be opened!"<<endl;
    return false;
  }
  
  fin.read((char *) &code, sizeof(int));
  if(myChildren.count(code)==0){
    fin.close();
    return false;
  }
  
  fin.read((char *) &Ndata, sizeof(int));
  fin.read((char *) &PROMin, sizeof(float));
  fin.read((char *) &PROMax, sizeof(float));
  fin.read((char *) &TimeRef, sizeof(int));
  if(code!=BLHcode && code!=CBHcode && code!=STAcode)
    fin.read((char *) &Retrieval, sizeof(int));

  if(code==LWPcode || code==IWVcode || 
     code==BLHcode || code==CBHcode) Nalt = 1;
  else if(code==STAcode) Nalt = 6;
  else fin.read((char *) &Nalt, sizeof(int));

  Initialized_it();

  if(code==TPCcode || code==TPBcode || 
     code==HPCcode || code==HPCcode+1 || code==STAcode)
    fin.read((char *) Alts, Nalt*sizeof(int));

  for(int i=0; i<Ndata; ++i){
    fin.read((char *) &TimeSec[i], sizeof(int));
    char RFtmp;
    fin.read((char *) &RFtmp, sizeof(char));
    RF[i] = (int) RFtmp;

    fin.read((char *) PRO[i], Nalt*sizeof(float));

    if(code==LWPcode || code==IWVcode){
      float ANG;
      fin.read((char *) &ANG, sizeof(float));
      hatpro::Angular2ElAzi(ANG, ELV[i], AZI[i]);      
    }
  }
  if(code==HPCcode+1){
    float TMPminmax[2];
    int TMPtime;
    char TMPrf;
    fin.read((char *) TMPminmax, 2*sizeof(float));
    for(int i=0; i<Ndata; ++i){
      fin.read((char *) &TMPtime, sizeof(int));
      fin.read((char *) &TMPrf, sizeof(char));
      fin.read((char *) PRO2[i], Nalt*sizeof(float));
    }
  }
  fin.close();
  return true;
}  // ... End of Read Profile variables from binary file.
// --------------------------------------------------------


// ===============================================
//              CREATION OF PROXY FILES
// ***********************************************
// Subroutine to create Brightness Temperature binary file (alike RPG)
void hatpro::BRT_var::Create_BinFile(const char *foutname){

  ofstream fwr;
  // determining proper file name to save:
  string fname(foutname);
  size_t idx = fname.find('.');
  if(idx==string::npos){
    cout<<"Are you sure this is a valid file name?"<<endl;
    return;
  }

  if(code==BRTcode || code==BRTcode-1)
    fname.replace(idx+1,3,"BRT");
  if(code==BLBcode)
    fname.replace(idx+1,3,"BLB");
  if(code==ATNcode)
    fname.replace(idx+1,3,"ATN");
  if(code==OLCcode)
    fname.replace(idx+1,3,"OLC");

  cout<<"File to storage: "<<fname.c_str()<<endl;
  fwr.open(fname.c_str(),ios::out|ios::binary);
  if(fwr.fail()){
    return;
  }
  fwr.write((char *) &code, sizeof(int));
  fwr.write((char *) &Ndata, sizeof(int));

  if(code==BRTcode || code==BRTcode-1 || code==ATNcode){
    fwr.write((char *) &TimeRef, sizeof(int));
    if(code==ATNcode) fwr.write((char *) &Retrieval, sizeof(int));
    fwr.write((char *) &Nfreq, sizeof(int));
    fwr.write((char *) Freq, Nfreq*sizeof(float));
    fwr.write((char *) BRTMin, Nfreq*sizeof(float));
    fwr.write((char *) BRTMax, Nfreq*sizeof(float));
  }

  if(code==BLBcode){
    fwr.write((char *) &Nfreq, sizeof(int));
    fwr.write((char *) BRTMin, Nfreq*sizeof(float));
    fwr.write((char *) BRTMax, Nfreq*sizeof(float));
    fwr.write((char *) &TimeRef, sizeof(int));
    fwr.write((char *) Freq, Nfreq*sizeof(float));
    fwr.write((char *) &Nang, sizeof(int));
    fwr.write((char *) Ang, Nang*sizeof(float));
  }

  if(code==OLCcode){
    fwr.write((char *) BRTMin, Nfreq*sizeof(float));
    fwr.write((char *) BRTMax, Nfreq*sizeof(float));
    fwr.write((char *) &TimeRef, sizeof(int));
    fwr.write((char *) &Nfreq, sizeof(int));
    fwr.write((char *) Freq, Nfreq*sizeof(float));
  }

  if(code==METcode){
    // Checking for additional sensors:
    int AddSensor = 0x00;
    for(int Nadd = 0; Nadd<(Nfreq-3); ++Nadd, AddSensor |= 0x01)
      AddSensor <<= 1;

    // Storing data into file:
    fwr.write((char *) &AddSensor, sizeof(char));
    for(int k=0; k<Nfreq; ++k){
      fwr.write((char *) &BRTMin[k], sizeof(float));
      fwr.write((char *) &BRTMax[k], sizeof(float));
    }
    fwr.write((char *) &TimeRef, sizeof(int));
  }

  // General data stored as TB variable:
  for(int i=0;i<Ndata;++i){
    fwr.write((char *) &TimeSec[i], sizeof(int));
    fwr.write((char *) &RF[i], sizeof(char));
    fwr.write((char *) TB[i], Nfreq*(Nang+1)*sizeof(float));
    if(code==BRTcode){
      // WARNING: Ang[i] only for BRT data files!! but check case by case
      //ANG  = (fabs(ELV[i])+1000*AZI[i]);
      //ANG *= signbit(ELV[i])?-1:1;

      fwr.write((char *) &Ang[i], sizeof(float));
    }
  }
  fwr.close();
  return;
}  // ... End of Create BRT binary file.
// -------------------------------------

// *****************************************************
// Subroutine to create PROfile binary file
bool hatpro::PRO_var::Create_BinFile(const char *foutname){
  ofstream fwr;
  // determining proper file name to save:
  string fname(foutname);
  size_t idx = fname.find('.');
  if(idx==string::npos){
    cout<<"Are you sure this is a valid file name?"<<endl;
    return false;
  }
  
  if(code==HPCcode || code==HPCcode+1)
    fname.replace(idx+1, 3, "HPC");
  if(code==TPCcode)
    fname.replace(idx+1, 3, "TPC");
  if(code==TPBcode)
    fname.replace(idx+1, 3, "TPB");

  cout<<"File to storage: "<<fname.c_str()<<endl;
  fwr.open(fname.c_str(), ios::out|ios::binary);
  if(fwr.fail()){
    return false;
  }
  fwr.write((char *) &code, sizeof(int));
  fwr.write((char *) &Ndata, sizeof(int));
  fwr.write((char *) &PROMin, sizeof(float));
  fwr.write((char *) &PROMax, sizeof(float));
  fwr.write((char *) &TimeRef, sizeof(int));
  fwr.write((char *) &Retrieval, sizeof(int));

  if(code==LWPcode || code==IWVcode || 
     code==BLHcode || code==CBHcode) Nalt = 1;
  else if(code==STAcode) Nalt = 6;
  else fwr.write((char *) &Nalt, sizeof(int));

  if(code==TPCcode || code==TPBcode || 
     code==HPCcode || code==HPCcode+1 || code==STAcode)
    fwr.write((char *) Alts, Nalt*sizeof(int));
  

  for(int i=0; i<Ndata; ++i){
    fwr.write((char *) &TimeSec[i], sizeof(int));
    char RFtmp = static_cast<char>(RF[i]>0?1:0);
    
    fwr.write((char *) &RFtmp, sizeof(char));
    
    fwr.write((char *) PRO[i], Nalt*sizeof(float));

    if(code==LWPcode || code==IWVcode){
      float *ANG;
      hatpro::ElAzi2Angular(&ELV[i], &AZI[i], 1, ANG);
      fwr.write((char *) ANG, sizeof(float));
    }
  }

  if(code==HPCcode+1){
    float TMPminmax[2];
    hatpro::minmax_value(PRO2, Ndata, 1, Nalt, &TMPminmax[0], &TMPminmax[1]);
    fwr.write((char *) TMPminmax, 2*sizeof(float));
    for(int i=0; i<Ndata; ++i){
      fwr.write((char *) &TimeSec[i], sizeof(int));
      fwr.write((char *) &RF[i], sizeof(char));
      fwr.write((char *) PRO2[i], Nalt*sizeof(float));
    }
  }
  fwr.close();
  return true;
}
// ---------------------------------------

// *****************************************************
// Subroutine to create synthetic MET binary file
void hatpro::MET_var::Create_BinFile(const char *foutname){
  ofstream fwr;
  fwr.open(foutname,ios::out|ios::binary);
  fwr.write((char *) &code, sizeof(int));
  fwr.write((char *) &Ndata, sizeof(int));
  fwr.write((char *) &AddSensor, sizeof(char));
  fwr.write((char *) &MinP, sizeof(float));
  fwr.write((char *) &MaxP, sizeof(float));
  fwr.write((char *) &MinT, sizeof(float));
  fwr.write((char *) &MaxT, sizeof(float));
  fwr.write((char *) &MinH, sizeof(float));
  fwr.write((char *) &MaxH, sizeof(float));
  for(int i=0;i<NaddS;++i){
    fwr.write((char *) &MinAddS[i], sizeof(float));
    fwr.write((char *) &MaxAddS[i], sizeof(float));
  }
  fwr.write((char *) &TimeRef, sizeof(int));
  for(int i=0;i<Ndata;++i){
    fwr.write((char *) &TimeSec[i], sizeof(int));
    fwr.write((char *) &RF[i], sizeof(char));
    fwr.write((char *) PTH[i], 3*sizeof(float));
    fwr.write((char *) ExSensor[i], NaddS*sizeof(float));
  }  
  fwr.close();
  return;
}  // ... End of Create MET binary file.
// -------------------------------------

// ***********************************************************
// ***********************************************************
//           PRINTING SUBROUTINES
//
// **** Subroutine to print BRT Hatpro data:
void hatpro::BRT_var::Print_Data(){
  hatpro::WhatAmI(code);
  float **date;
  date = new float*[Ndata];
  for(int i=0; i<Ndata; ++i) date[i] = new float[6];

  TimeSec2Date(Ndata, TimeSec, Ndata, date);

  if(code!=METcode)
    cout<<"% Printing data with "<<Nfreq<<" frequencies, "<<Nang<<" angles and "<<Ndata<<" data points"<<endl;
  else
    cout<<"% Printing data with "<<Nfreq<<" variables "<<endl;

  for (int i=0;i<5;++i){
    cout<<date[i][0]<<"-"<<date[i][1]<<"-"<<date[i][2]<<" ";
    cout<<date[i][3]<<":"<<date[i][4]<<":"<<date[i][5]<<"->";
    if(code!=METcode)
      cout<<setw(7)<<TimeSec[i]<<": ("<<AZI[i]<<","<<ELV[i]<<") "<<endl;
    else
      cout<<setw(7)<<TimeSec[i]<<": "<<endl;
    
    cout<<setprecision(2)<<setfill(' ')<<fixed;
    for(int j=0;j<Nfreq;++j){
      if(code!=METcode) cout<<"f[GHz] "<<Freq[j]<<": ";
      for(int k=0; k<(Nang+1); ++k)
	cout<<setw(7)<<TB[i][k+j*(Nang+1)]<<' ';
      if(code!=METcode) cout<<endl;
    }
    cout<<endl;
  }

  delete [] date;
  // float **Datum;
  // Datum = TimeSec2Date(TT, Ndata);
  // for(int i=0;i<10;++i) cout<<Datum[i][0]<<" "<<Datum[i][1]<<" "<<Datum[i][2]<<" "<<Datum[i][3]<<" "<<Datum[i][4]<<" "<<Datum[i][5]<<" "<<endl;
  
  return;
}  // ... End of Printing BRT data.
// ----

// *** Subroutine to print Profile Hatpro data:
void hatpro::PRO_var::Print_Data(){
  if(hatpro::WhatAmI(code)) return;
  float **date;
  date = new float*[Ndata];
  for(int i=0; i<Ndata; ++i) date[i] = new float[6];

  TimeSec2Date(Ndata, TimeSec, Ndata, date);
  cout<<"% Printing data with "<<Nalt<<" altitudes and "<<Ndata<<" data points"<<endl;
  cout<<setprecision(2)<<setfill(' ')<<fixed;
  if(code==TPCcode||code==TPBcode||code==HPCcode||code==HPCcode+1){
    cout<<"H[m]:";
    for(int j=0;j<Nalt;++j) cout<<" "<<Alts[j];
    cout<<endl;
  }
  for (int i=0;i<5;++i){
    for(int t=0;t<6;++t) cout<<setw(4)<<date[i][t];
    cout<<": "<<RF[i]<<": ";
    //cout<<setw(7)<<TimeSec[i]<<" :";
    for(int j=0;j<Nalt;++j)
      cout<<setw(7)<<PRO[i][j]<<' ';
    cout<<endl;
  }

  delete [] date;
  return;
}  // ... End of Printing Profile data.
// -----

// *** Subroutine to print Meteorological Hatpro data:
void hatpro::MET_var::Print_Data(){
  if(hatpro::WhatAmI(code)) return;
  float **date;
  TimeSec2Date(Ndata, TimeSec, Ndata, date);
  cout<<"% Printing data with "<<Ndata<<" data points"<<endl;
  cout<<"Time_stamp  | "<<" Pressure |  Wind_sp | "<<" Temp | Wind_dir | "<<" RH  | RR"<<endl;
  cout<<setprecision(2)<<setfill(' ')<<fixed;
  for (int i=0;i<5;++i){
    for(int t=0;t<6;++t) cout<<setw(4)<<date[i][t];
    cout<<": ";
    //cout<<setw(7)<<TimeSec[i]<<" :";
    for(int j=0;j<3;++j)
      cout<<setw(7)<<PTH[i][j]<<' '<<ExSensor[i][j];
    cout<<endl;
  }

  return;
}  // ... End of Printing Meteorological data.
// -------


// **************************************************
//             OTHER SUBROUTINES OR FUNCTIONS
// --------------------------------------------------- 
void hatpro::Angular2ElAzi(float ANG, float &ELV, float &AZI){
    double whole, fractional;
    float elv0, azi0;
    bool flag;
    
    // Converting encoded ANG to AZIMUTH and ELEVATION values:
    flag = (ANG-1E6)<0?false:true;
    fractional = modf((double) ANG/100,&whole);
    elv0 =  (float) fractional*100;
    elv0 += flag?100:0;
    elv0 *= signbit(ANG)?-1:1;
    azi0 = (float) flag?(whole-1E4):whole;
    azi0 /= 10;
    // assigning values for Elevation and Azimuth:
    ELV = elv0;
    AZI = azi0;
    return;
}

// subroutine to convert Elevation and Azimuth arrays into RPG's ANGULAR array
void hatpro::ElAzi2Angular(float *ELV, float *AZI, int ND, float *ANG){
  for(int i=0; i<ND; ++i){
    if(ELV[i]>=100) AZI[i] += 1000.000;
    ANG[i] = fabs(ELV[i]) + 1000*AZI[i];
    ANG[i] *= signbit(ELV[i])?-1:1;
  }
  return;
}
// --

// **** Show a short message what HATPRO file is:
int hatpro::WhatAmI(int code){
  switch(code){
  case BRTcode:
  case BRTcode-1:
    cout<<"% BRT file, Brightness Temperature";
    break;
  case ATNcode:
    cout<<"% ATN file, Atmospheric Attenuation";
    break;
  case BLBcode-1:
  case BLBcode:
    cout<<"% BLB file, Boundary Layer Brightness Temperature";
    break;
  case WVLcode:
    cout<<"% WVL file, Water Vapour Line";
    break;
  case OLCcode:
    cout<<"% OLC file, Oxygen Line";
    break;
  case TPCcode:
    cout<<"% TPC file, Tropospheric Temperature Profile";
    break;
  case TPBcode:
    cout<<"% TPB file, Boundary Layer Temperature Profile";
    break;
  case HPCcode:
  case HPCcode+1:
    cout<<"% HPC file, Humidity Profile (w/o RH profile)";
    break;
  case LPRcode:
    cout<<"% LPR file, Liquid Water Profile";
    break;
  case METcode-1:
  case METcode:
    cout<<"% MET file, Meteorological Sensors";
    break;
  case IWVcode:
    cout<<"% IWV file, Integrated Water Vapour";
    break;
  case LWPcode:
    cout<<"% LWP file, Liquid Water Path";
    break;
  case BLHcode:
    cout<<"% BLH file, Boundary Layer Height";
    break;
  case CBHcode:
    cout<<"% CBH file, Cloud Base Height";
    break;
  case STAcode:
    cout<<"% STA file, Stability indices";
    break;
  default:
    cout<<"Error to identify code: "<<code<<endl;
    return(-1);
  }
  cout<<" with code: "<<code<<endl;
  return(0);
}
// -----

// *** Get File code from extention of the input file:
int hatpro::GetExtFromFile(const char *infile){
  // defining an operator to test true/false when compared with map 
  struct cmp_str
  {
    bool operator()(char const *a, char const *b) const
    {
      return std::strcmp(a, b) < 0;
    }
  };

  // mapping file extension to Radiometer data code:
  std::map<const char *, int, cmp_str> ext2code;
  ext2code["BRT"] = BRTcode;
  ext2code["BLB"] = BLBcode;
  ext2code["OLC"] = OLCcode;
  ext2code["WVL"] = WVLcode;
  ext2code["ATN"] = OLCcode;
  ext2code["TPC"] = TPCcode;
  ext2code["TPB"] = TPBcode;
  ext2code["HPC"] = HPCcode;
  ext2code["MET"] = METcode;
  ext2code["IWV"] = IWVcode;
  ext2code["LWP"] = LWPcode;
  ext2code["BLH"] = BLHcode;
  ext2code["CBH"] = CBHcode;
  ext2code["STA"] = STAcode; 

  // Finding the input file's extension type:
  string garbage(infile);
  size_t idx = garbage.find('.');
  if(idx==string::npos){
    cout<<"ERROR: be sure input file has the form [namebase.EXT]"<<endl;
    return(-1);
  }
  const char *EXT = garbage.substr(idx+1,3).c_str();
  if (ext2code.find(EXT) == ext2code.end()) {
    cout<<"ERROR: file extension ."<<EXT<<" not recognized!"<<endl;
    return(-1);
  }
  //cout<<"Extension is : "<<EXT<<" with code "<<ext2code[EXT]<<endl;
  return(ext2code[EXT]);
}
// -----


// General function to convert RPG TimeSec to Calendar Date
void hatpro::TimeSec2Date(int M, int *TimeSec, int N, float **date){
  time_t basetime, acttime;
  struct tm * time0, *timeF;
  //float **date; 
  //date = new float*[N];
  time(&basetime);
  time0 = gmtime(&basetime);
  time0->tm_year = 101;
  time0->tm_mon = 0;
  time0->tm_mday = 1;
  time0->tm_hour = 0;
  time0->tm_min  = 0;
  time0->tm_sec  = 0;
  basetime = mktime(time0)-timezone;

  for(int i=0;i<N;i++){
    //date[i] = new float[6];
    acttime = basetime+TimeSec[i];
    timeF = gmtime(&acttime);
    date[i][0] = (float) (1900+timeF->tm_year);
    date[i][1] = (float) (1+timeF->tm_mon);
    date[i][2] = (float) timeF->tm_mday;
    date[i][3] = (float) timeF->tm_hour;
    date[i][4] = (float) timeF->tm_min;
    date[i][5] = (float) timeF->tm_sec;
    //cout<<TimeSec[i]<<": "<<1900+timeF->tm_year<<"."<<1+timeF->tm_mon<<"."<<timeF->tm_mday<<" "<<timeF->tm_hour<<":"<<timeF->tm_min<<":"<<timeF->tm_sec<<"--->"<<asctime(timeF)<<endl;
  }
  
  return;
}
// ---
// *****************************************************
// Funtion to convert Calendar Date into RPG TimeSec:
void hatpro::Date2TimeSec(float **date, int ND, int *TimeSec){
  //int *TimeSec;
  struct tm * timeIN;
  time_t base_time, rpg_time = time(nullptr);
  timeIN = gmtime(&rpg_time);
  //TimeSec = new int[ND];

  time(&base_time);
  timeIN = gmtime(&base_time);
  timeIN->tm_year = 101;
  timeIN->tm_mon = 0;
  timeIN->tm_mday = 1;
  timeIN->tm_hour = 0;
  timeIN->tm_min  = 0;
  timeIN->tm_sec  = 0;
  base_time = mktime(timeIN) - timezone;  // base time relative to 2001.1.1

  for(int i=0; i<ND; ++i){
    timeIN->tm_year = (int) (date[i][0]-1900);
    timeIN->tm_mon  = (int) (date[i][1]-1);
    timeIN->tm_mday = (int) date[i][2];
    timeIN->tm_hour = (int) date[i][3];
    timeIN->tm_min  = (int) date[i][4];
    timeIN->tm_sec  = (int) date[i][5];
    timeIN->tm_isdst= 0;   // flag to false for daylight saving time!
    rpg_time = mktime(timeIN) - timezone - base_time;
    TimeSec[i] = (int) rpg_time;
  }
  
  return;
}
// -----------------------------------------------------


// Function to obtain the minimum and maximum value of a 2D or 3D variable, alond second-dim:
void hatpro::minmax_value(float **DATA, int ND, int NF, int NA, float min[], float max[]){

  for(int j=0; j<NF; ++j){
    min[j] = 99999;
    max[j] = 0;
    for(int i=0; i<ND; ++i)
      for(int k=0; k<NA; ++k){
	float dummy = DATA[i][j*NA + k];
	if(min[j]>dummy) min[j] = dummy;
	if(max[j]<dummy) max[j] = dummy;
      }
  }
  return;
}
// ==========================================================================================


void hatpro::ShowLicense(){
  std::cout<<"HATPRO_DABINIO library."<<std::endl;
  
  std::cout<<"(c) 2019 Pablo Saavedra Garfias., Email: pablo.saa@uib.no"<<std::endl;
  std::cout<<"Geophysical Institute, University of Bergen"<<std::endl;

  std::cout<<"SEE LICENSE.TXT"<<std::endl;
}


