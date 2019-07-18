// Program to test the HATPRO library
// Part of the project HATPRO_Toolbox
// 
#include "hatpro.h"

using namespace std;

template<typename T>
int Func(const char *infile, T &DATA){
  if(DATA.Read_BinFile(infile))
    DATA.Print_Data();
  else
    return(-1);
  return(DATA.Ndata);
};

int main(int argc, char *argv[]){

  int Nlines;  
  const char *fileBRT;
  const char filename[] = "/home/pablo/UiB/data/HATPRO/Y2015/M05/D23/150523.MET";
  const char fname[] = "/home/pablo/UiB/data/HATPRO/Y2015/M05/D23/150523.HPC";
  const char newname[] = "/tmp/raquelinha.MET";
  hatpro::PRO_var data;
  hatpro::BRT_var tb;
  //hatpro::MET_var met;
  int status, code;
  int output=0;  
  // for standard BRT data:


  switch(argc){
  case 3:
	Nlines = atoi(argv[2]);
  case 2:
	fileBRT  = argv[1];
	break;
  default:
	cout<<"ERROR: max two input arguments supported!"<<endl;
  }

  cout<<"trying... "<<fileBRT<<endl;
  
  status = hatpro::GetExtFromFile(fileBRT);

  switch(status){
  case BRTcode:
  case BLBcode:
  case OLCcode:
  case WVLcode:
  case METcode:
    {
      //output = Func<hatpro::BRT_var>(fileBRT, tb);
      output = Func<decltype(tb)>(fileBRT, tb);

      float BRTmin[tb.Nfreq], BRTmax[tb.Nfreq];
      hatpro::minmax_value(tb.TB, tb.Ndata, tb.Nfreq, tb.Nang+1, BRTmin, BRTmax);
      cout<<"MINIMUM::::"<<endl;
      for(int kk=0; kk<tb.Nfreq; ++kk) cout<<BRTmin[kk]<<" ";
      cout<<endl;
      cout<<"MIXIMUM____"<<endl;
      for(int kk=0; kk<tb.Nfreq; ++kk) cout<<BRTmax[kk]<<" ";
      cout<<endl;
      break;
    }
  case TPCcode: 
  case TPBcode:
  case HPCcode:
  case IWVcode:
  case LWPcode:
    output = Func<hatpro::PRO_var>(fileBRT, data);
    cout<<data.Ndata;
    break;
    //case METcode:
    //output = Func<hatpro::MET_var>(fileBRT, met);
    //break;
  default:
    cout<<"ERROR: unknown input file type!"<<endl;
  }

  if(output>0)
    cout<<" := "<<output<<" Input file loaded! "<<endl;
  else
    cout<<":/"<<endl;


  return 0;
  // ??? hatpro::TPB_var TP = hatpro::Read_TPB_BinFile("../data/D23/150523.TPB");

  // ?? for(int i=0;i<TP.AltAnz;++i) cout<<TP.Alts[i]<<" ";
  // ? cout<<endl;

  //hatpro::BLB_var BL = hatpro::Read_BLB_BinFile(fname);
  
  //hatpro::Print_BLB_Data(BL);

  cout<<" -- o --"<<endl;
  //TimeSec2Date(HP.RHTimeSec, 5);
  
  //met.Read_BinFile(filename);

  for(int i=0;i<10;i++){
    //cout<<met.TimeSec[i];
    //for(int j=0;j<3;++j) cout<<" "<<met.PTH[i][j];
    //for(int j=0;j<3;++j) cout<<" "<<met.ExSensor[i][j];
    cout<<endl;
  }
  //met.Create_BinFile(newname);

  //hatpro::Create_BLB_BinFile(newname, BL);
  //cout<<"BLB created!"<<endl;
  //hatpro::Create_BRT_BinFile(newname, &data);

  //hatpro::Print_Hatpro_Data(&data);

  //data.Delete_it();
 
}
// end of code
