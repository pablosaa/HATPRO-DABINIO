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
    RF[i] = atoi((const char *) &RFtmp);

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
// Type supported: (TPC,TPB,HPC,LPR,IWV,LWP,MET?)
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
    RF[i] = atoi((const char *) &RFtmp);

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
    cout<<"% Printing data with "<<Nfreq<<" frequencies and "<<Ndata<<" data points"<<endl;
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

  int *TT;
  TT = new int[Ndata];
  hatpro::Date2TimeSec(date, Ndata, TT);
  for(int i=0; i<5; ++i) cout<<TT[i]<<endl;
  delete [] TT;
  //delete [] date;
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
    cout<<"% MWR code is : "<<code<<endl;
    break;
  case ATNcode:
    cout<<"% ATN code is : "<<code<<endl;
    break;
  case BLBcode:
    cout<<"% BLBT code is : "<<code<<endl;
    break;
  case OLCcode:
    cout<<"% OLC code is : "<<code<<endl;
    break;
  case TPCcode:
    cout<<"% TPC code is : "<<code<<endl;
    break;
  case TPBcode:
    cout<<"% TPB code is : "<<code<<endl;
    break;
  case HPCcode:
  case HPCcode+1:
    cout<<"% HPC code is : "<<code<<endl;
    break;
  case LPRcode:
    cout<<"% LPR code is : "<<code<<endl;
    break;
  case METcode:
    cout<<"% MET code is : "<<code<<endl;
    break;
  case IWVcode:
    cout<<"% IWV code is : "<<code<<endl;
    break;
  case LWPcode:
    cout<<"% LWP code is : "<<code<<endl;
    break;
  case BLHcode:
    cout<<"% BLH code is : "<<code<<endl;
    break;
  case CBHcode:
    cout<<"% CBH code is : "<<code<<endl;
    break;
  case STAcode:
    cout<<"% STA code is : "<<code<<endl;
    break;
  default:
    cout<<"Error to identify code: "<<code<<endl;
    return(-1);
  }
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


/* Function to select the subset to apply the retrieval algorithm */
// int Select_SubSet(MWR_hatpro DATA){ //(float *TB, float *PD, float Elevation){

  
//   bool TBandPD=true;
//   int DelTB[3] = {2,3,4};
//   int DelPD[3] = {1,2,3};    // delta TB and PD in K to select the subset.
//   int i,j=0, Nidx = 0;
//   int index[N_X];  //[N_Xang[0]];
//   float TB[3], PD[3], Elevation;


  // TB[0] = DATA.TB[0];
  // TB[1] = DATA.TB[1];
  // TB[2] = DATA.TB[2];
  // PD[0] = DATA.PD[0];
  // PD[1] = DATA.PD[1];
  // PD[2] = DATA.PD[2];
  // Elevation = DATA.Eleva;

  // if ((PD[0]>-1 & PD[0]<1) & (PD[1]>-1 & PD[1]<1) & (PD[2]>-1 & PD[2]<1))
  //     TBandPD = false;

  // // selecting the index for the elevation angle.
  // idx_A = -1;
  // for (i=0;i<N_A;i++) if (Elevation==Angles[i]) idx_A = i;
  // if (idx_A == -1) return(-1);   // no database available for that angle.

  // while (Nidx<20 && j<3){
  //   Nidx = 0;
  //   for (i=0;i<N_X; i++){   //ang[idx_A];i++){
  //     if (TBandPD){
  // 	if (DataBase[idx_A][i][0]>TB[0]-DelTB[j] & DataBase[idx_A][i][0]<TB[0]+DelTB[j] &
  // 	    DataBase[idx_A][i][1]>TB[1]-DelTB[j] & DataBase[idx_A][i][1]<TB[1]+DelTB[j] &
  // 	    DataBase[idx_A][i][2]>TB[2]-DelTB[j] & DataBase[idx_A][i][2]<TB[2]+DelTB[j] &
  // 	    DataBase[idx_A][i][3]>PD[0]-DelPD[j] & DataBase[idx_A][i][3]<PD[0]+DelPD[j] &
  // 	    DataBase[idx_A][i][4]>PD[1]-DelPD[j] & DataBase[idx_A][i][4]<PD[1]+DelPD[j] &
  // 	    DataBase[idx_A][i][5]>PD[2]-DelPD[j] & DataBase[idx_A][i][5]<PD[2]+DelPD[j])
  // 	  index[Nidx++] = i;
  //     }
  //     else{
  // 	if (DataBase[idx_A][i][0]>TB[0]-DelTB[j] & DataBase[idx_A][i][0]<TB[0]+DelTB[j] &
  // 	    DataBase[idx_A][i][1]>TB[1]-DelTB[j] & DataBase[idx_A][i][1]<TB[1]+DelTB[j] &
  // 	    DataBase[idx_A][i][2]>TB[2]-DelTB[j] & DataBase[idx_A][i][2]<TB[2]+DelTB[j] &
  // 	    DataBase[idx_A][i][3]>-1 & DataBase[idx_A][i][3]<0.2 &
  // 	    DataBase[idx_A][i][4]>-1 & DataBase[idx_A][i][4]<0.2 &
  // 	    DataBase[idx_A][i][5]>-1 & DataBase[idx_A][i][5]<0.2) index[Nidx++] = i;
  //     }
  //   }
  //   j++;
  // }
//   idx = new int[Nidx];
//   for (i=0;i<Nidx;i++) idx[i]=index[i];
//   return(Nidx);
//   delete [] idx;
// }


/* Bayesian function for the inversion of TB,PD to IWV,C_LWP and R_LWP
   It is assumed that the database and the selected indexes are declared
   as global variables */
// RET_var Bayesian_Function(int Nidx, float* TBm, float* PDm){

//   bool TBandPD = true;
//   float delta[Nidx],A=0;
//   float TBret[3]={0,0,0}, PDret[3]={0,0,0};
//   float sigTBret[3]={0,0,0}, sigPDret[3]={0,0,0};
//   float W[Nidx];
//   float const SigmaTB[3]={2,1,1},SigmaPD[3]={.5,.5,.5};
//   float QIret=0, FLret=0, IWVret=0, CLWPret=0, RLWPret=0;
//   float sigFLret=0, sigIWVret=0, sigCLWPret=0, sigRLWPret=0;
//   float QImin=99e9;
//   int i,j,k;
//   RET_var ATMOS;

//   if ((PDm[0]>-1 & PDm[0]<1) & (PDm[1]>-1 & PDm[1]<1) & (PDm[2]>-1 & PDm[2]<1))
//       TBandPD = false;

//   //delta = new float[Nidx];
//   for(i=0;i<Nidx;i++){
//     // calculating the distance between points...
//     delta[i] =
//       (TBm[0]-DataBase[idx_A][idx[i]][0])*(TBm[0]-DataBase[idx_A][idx[i]][0])/
//       (SigmaTB[0]*SigmaTB[0]) +
//       (TBm[1]-DataBase[idx_A][idx[i]][1])*(TBm[1]-DataBase[idx_A][idx[i]][1])/
//       (SigmaTB[1]*SigmaTB[1]) +
//       (TBm[2]-DataBase[idx_A][idx[i]][2])*(TBm[2]-DataBase[idx_A][idx[i]][2])/
//       (SigmaTB[2]*SigmaTB[2]) +
//       (PDm[0]-DataBase[idx_A][idx[i]][3])*(PDm[0]-DataBase[idx_A][idx[i]][3])/
//       (SigmaPD[0]*SigmaPD[0]) +
//       (PDm[1]-DataBase[idx_A][idx[i]][4])*(PDm[1]-DataBase[idx_A][idx[i]][4])/
//       (SigmaPD[1]*SigmaPD[1]) +
//       (PDm[2]-DataBase[idx_A][idx[i]][5])*(PDm[2]-DataBase[idx_A][idx[i]][5])/
//       (SigmaPD[2]*SigmaPD[2]);
//     // calculating weigthing function...
//     W[i] = exp(-0.5*delta[i]);
//     A += W[i];
//     // calculating the minimum QI.
//     if (QImin>=delta[i]) QImin = delta[i];
//   }
//   if (A == 0){
//     // NOT POSSIBLE TO RETRIEVED
//     for (i=0;i<3;i++){
//       ATMOS.TBret[i] = 0.0/0.0; //-99;
//       ATMOS.PDret[i] = 0.0/0.0; //-99;
//       ATMOS.sigTB[i] = 0.0/0.0; //-99;
//       ATMOS.sigPD[i] = 0.0/0.0; //-99;
//     }
//   ATMOS.IWV = 0.0/0.0; //-99;
//   ATMOS.CLWP = 0.0/0.0; //-99;
//   ATMOS.RLWP = 0.0/0.0; //-99;
//   ATMOS.QI   = 0.0/0.0;
//   ATMOS.FL   = 0.0/0.0; //-99;
//   ATMOS.sigIWV = 0.0/0.0; //-99;
//   ATMOS.sigCLWP = 0.0/0.0; //-99;
//   ATMOS.sigRLWP = 0.0/0.0; //-99;
//   ATMOS.sigFL = 0.0/0.0; //-99;
//   return(ATMOS);
//   }
//   // CALCULATING THE RETRIEVALS:
//   for (i=0;i<Nidx;i++){
//     TBret[0] += DataBase[idx_A][idx[i]][0]*W[i]/A;
//     TBret[1] += DataBase[idx_A][idx[i]][1]*W[i]/A;
//     TBret[2] += DataBase[idx_A][idx[i]][2]*W[i]/A;
//     PDret[0] += DataBase[idx_A][idx[i]][3]*W[i]/A;
//     PDret[1] += DataBase[idx_A][idx[i]][4]*W[i]/A;
//     PDret[2] += DataBase[idx_A][idx[i]][5]*W[i]/A;
//     CLWPret +=  DataBase[idx_A][idx[i]][6]*W[i]/A;
//     RLWPret +=  DataBase[idx_A][idx[i]][7]*W[i]/A;
//     IWVret += DataBase[idx_A][idx[i]][8]*W[i]/A;
//     //QIret += delta[i]*W[i]/A;
//     FLret += Flevel[idx[i]]*W[i]/A; //DataBase[idx_A][idx[i]][0]*W[i]/A;
//   }
//   // CLWPret = pow(10,CLWPret)-1e-10;
//   // RLWPret = pow(10,RLWPret)-1e-10;

//   // Calculating the RMS:
//   for (i=0;i<Nidx;i++){
//     sigTBret[0] += pow((DataBase[idx_A][idx[i]][0]-TBret[0]),2)*W[i]/A;
//     sigTBret[1] += pow((DataBase[idx_A][idx[i]][1]-TBret[1]),2)*W[i]/A;
//     sigTBret[2] += pow((DataBase[idx_A][idx[i]][2]-TBret[2]),2)*W[i]/A;
//     sigPDret[0] += pow((DataBase[idx_A][idx[i]][3]-PDret[0]),2)*W[i]/A;
//     sigPDret[1] += pow((DataBase[idx_A][idx[i]][4]-PDret[1]),2)*W[i]/A;
//     sigPDret[2] += pow((DataBase[idx_A][idx[i]][5]-PDret[2]),2)*W[i]/A;
//     sigCLWPret +=  pow(DataBase[idx_A][idx[i]][6]-CLWPret,2)*W[i]/A;
//     sigRLWPret +=  pow(DataBase[idx_A][idx[i]][7]-RLWPret,2)*W[i]/A;
//     sigIWVret += pow((DataBase[idx_A][idx[i]][8]-IWVret),2)*W[i]/A;
//     sigFLret += pow((Flevel[idx[i]]-FLret),2)*W[i]/A;
//   }
//   // sigCLWPret = pow(10,sigCLWPret)-1e-10;
//   // sigRLWPret = pow(10,sigRLWPret)-1e-10;

//   if (!TBandPD){RLWPret =0.; sigRLWPret = 0.;}

//   for (i=0;i<3;i++){
//     ATMOS.TBret[i] = TBret[i];
//     ATMOS.PDret[i] = PDret[i];
//     ATMOS.sigTB[i] = sqrt(sigTBret[i]);
//     ATMOS.sigPD[i] = sqrt(sigPDret[i]);
//   }
//   ATMOS.IWV = IWVret;
//   ATMOS.CLWP = CLWPret; 
//   ATMOS.RLWP = RLWPret;
//   ATMOS.QI   = QImin; //QIret;
//   ATMOS.FL   = FLret;
//   ATMOS.sigIWV = sqrt(sigIWVret);
//   ATMOS.sigCLWP = sqrt(sigCLWPret);
//   ATMOS.sigRLWP = sqrt(sigRLWPret);
//   ATMOS.sigFL = sqrt(sigFLret);
//   return ATMOS;
// }


// /* Function to save the retrieval result in the specified file */
// void SaveRetrieval(char * filename, MWR_hatpro DATA, RET_var RET){

//   ofstream out;
//   out.open(filename,ios::out|ios::app);

//   out<<setprecision(2)<<setfill(' ')<<fixed;
//   out<<setw(10)<<DATA.TimeSec
//      <<setw(8)<<DATA.TB[0]<<setw(8)<<DATA.TB[1]<<setw(8)<<DATA.TB[2]
//     //     <<setw(8)<<DATA.PD[0]<<setw(8)<<DATA.PD[1]<<setw(8)<<DATA.PD[2]
//      <<setw(8)<<RET.TBret[0]<<setw(8)<<RET.sigTB[0]
//      <<setw(8)<<RET.TBret[1]<<setw(8)<<RET.sigTB[1]
//      <<setw(8)<<RET.TBret[2]<<setw(8)<<RET.sigTB[2]
//      <<setw(8)<<RET.PDret[0]<<setw(8)<<RET.sigPD[0]
//      <<setw(8)<<RET.PDret[1]<<setw(8)<<RET.sigPD[1]
//      <<setw(8)<<RET.PDret[2]<<setw(8)<<RET.sigPD[2]
//      <<setw(8)<<RET.QI<<setw(8)<<RET.FL<<setw(8)<<RET.sigFL
//      <<setw(8)<<RET.IWV<<setw(8)<<RET.sigIWV
//      <<setw(8)<<RET.CLWP<<setw(8)<<RET.sigCLWP
//      <<setw(8)<<RET.RLWP<<setw(8)<<RET.sigRLWP;
//     //<<setw(8)<<DATA.Eleva<<setw(8)<<DATA.Azimuth<<endl;

//   out.close();
// }
// end of program.


// int Load_DataBase(char *DatabaseFile){
//   // ******* New version with constant DATABASE matrix  ******//
//   /* DataBase structure: (N_A,N_X,N_Y)
//      N_A number of elevation angles (normally 24).
//      N_X number of simulation per angle.
//      N_Y number of variables in data-base, normally 9 as follow:
//      1,2 and 3: TB at 10, 21 and 36 GHz,
//      4,5 and 6: PD at 10, 21 and 36 GHz,
//      7: cloud LWP [kg/m^2],
//      8: rain LWP [kg/m^2],
//      9: IWV [kg/m^2].
//   */
//   int i,j,k, N_Y;
//   float garbage[15];
//   ifstream in;
//   in.open(DatabaseFile,ios::in|ios::binary);
//   in.read((char *) &N_A, sizeof N_A);
//   in.read((char *) &N_Y, sizeof N_Y);
//   in.read((char *) &N_X, sizeof N_X);
//   Angles = new float[N_A];
//   Flevel = new float[N_X];
//   in.read((char *) Angles, N_A*sizeof(float));
//   in.read((char *) Flevel, N_X*sizeof(float));
//   cout<<N_A<<' '<<N_Y<<' '<<N_X<<' '<<DatabaseFile<<endl;
//   DataBase = new float**[N_A];

//   for(k=0;k<N_A;k++){
//     //N_X = N_Xang[k];
//     DataBase[k] = new float*[N_X];
//     for (i=0;i<N_X;i++){
//       in.read((char *) &garbage, N_Y*sizeof(float)); //garbage);
//       DataBase[k][i] = new float[N_Y];
//       for (j=0;j<N_Y;j++) DataBase[k][i][j] = garbage[j];
//     }
//   }
//   in.close();

//   return(0);
// }   // end of loading the database.

