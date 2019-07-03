// Program to read the latest MET files and get the wind speed and direction
// Part of the project HATPRO_Toolbox
// 
#include "hatpro.h"

#define PI acos(-1.)

using namespace std;

float deg2rad(float d){
  return d*PI/180.;
}
float rad2deg(float r){
  return r*180./PI;
}

template<typename T>
int Func(const char *infile, T &DATA){
  if(DATA.Read_BinFile(infile))
    DATA.Print_Data();
  else
    return(-1);
  return(DATA.Ndata);
};

int main(int argc, char *argv[]){

  const char *fileBRT;

  hatpro::BRT_var MET;

  int status, code;
  int minave = 10, output=0;  
  
  // checking for input arguments:
  switch(argc){
  case 3:
    minave = atoi(argv[2]);
  case 2:
    fileBRT = argv[1];
    break;
  case 1:
  default:
    cout<<"USAGE:"<<endl;
    cout<<">./getMETwinddir HATPRO_FILE.MET [MINUTES]"<<endl;
    cout<<"WHERE:"<<endl;
    cout<<"* HATPRO_FILE.MET is the binary file for Hatpro Meteorological data,"<<endl;
    cout<<"* [MINUTES] Optional number of minutes to average over, (by default is 10 minutes)."<<endl;
    cout<<"EXAMPLE 1: >./getMETwinddir /path_to_data/190521.MET"<<endl;
    cout<<"EXAMPLE 2: >./getMETwinddir /path_to_data/190521.MET 5"<<endl;
    return 1;
  }

  status = hatpro::GetExtFromFile(fileBRT);
  if(status!=METcode){
    cout<<"ERROR: wrong file type!"<<endl;
    return -1;
  }

  // Number of minutes to consider for the averaging:
  int Nave = minave*60;

  if(!MET.Read_BinFile(fileBRT)){
    cout<<"ERROR: reading file "<<fileBRT<<endl;
    return -1;
  }

  if(Nave>MET.Ndata) Nave = MET.Ndata;
  float **date = hatpro::TimeSec2Date(MET.TimeSec, MET.Ndata);
  float WindVel, WindDir;
  float UU = 0, VV = 0;
  for(int i=Nave; i>0; --i){
    float WS = MET.TB[MET.Ndata-i][3];
    float WD = deg2rad(MET.TB[MET.Ndata-i][4]);
    //cout<<date[MET.Ndata-i][3]<<":"<<date[MET.Ndata-i][4]<<":"<<date[MET.Ndata-i][5];
    //cout<<" "<<MET.TB[MET.Ndata-i][4]<<" "<<WS<<endl;
    if(MET.TB[MET.Ndata-i][4]<0 || MET.TB[MET.Ndata-i][4]>360) continue;
    UU += WS*sin(WD);
    VV += WS*cos(WD);
  }
  // Wind vector average:
  UU /= Nave;
  VV /= Nave;

  // Wind direction and speed:
  WindVel = sqrt(VV*VV + UU*UU);
  WindDir = fmod(360 + rad2deg(atan2(UU,VV)), 360);
	  
  // Returning or storing the average Wind Direction and Speed:
  cout<<WindDir<<" "<<WindVel<<endl;

  //ofstream fp;
  //fp.open("winddir", ios::out);
  //fp<<WindDir<<endl;
  //fp.close();
  
  return 0;
}
// end of code
