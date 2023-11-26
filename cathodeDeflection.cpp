#include <iostream>
#include <iomanip>
#include <fstream>

#include "TROOT.h"
#include "ROOT/TThreadedObject.hxx"
#include "TTree.h"
#include "TFile.h"
#include "TSystemFile.h"
#include "TString.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TCut.h"
#include "TF1.h"
#include "TFitResult.h"
#include "TH1.h"
#include "TH2.h"
#include "TStyle.h"
#include "TList.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TVirtualFFT.h"
#include "TFile.h"
#include "TChain.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TSystemDirectory.h"
#include "TStopwatch.h"

using namespace std;

// Constants 
const float driftVel = 0.15715; // was 0.1551
//const float driftVel = 0.157565; // MC ONLY

// Custom Functions
void AddFiles(TChain *ch, const char *fileList);
void write_csv(string filename, vector<pair<string, vector<float>>> dataset);

// XRootD AddFiles                                                        
void AddFiles(TChain *ch, const char *fileList)
{
  ifstream InFile(fileList);
  vector<string> fileVec;
  string line;
  int n_files = 0;
  while(getline(InFile, line))
    {
      //cout <<line <<endl;                                               
      fileVec.push_back(line);
      n_files++;
    }

  for(unsigned int iFile=0; iFile<fileVec.size(); ++iFile)
    {
      //cout <<fileVec[iFile].c_str() <<endl;                             
      ch->AddFile(fileVec[iFile].c_str());
    }

  return;
}

void write_csv(string filename, vector<pair<string, vector<float>>> dataset)
{
  // Create an output filestream object
  ofstream myFile(filename);

  // Send column names to the stream
  for(int j = 0; j < dataset.size(); ++j)
    {
      myFile << dataset.at(j).first;
      if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
    }
  myFile << "\n";

  // Send data to the stream
  for(int i = 0; i < dataset.at(0).second.size(); ++i)
    {
      for(int j = 0; j < dataset.size(); ++j)
        {
	  myFile << dataset.at(j).second.at(i);
	  if(j != dataset.size() - 1) myFile << ","; // No comma at end of line        
        }
      myFile << "\n";
    }
  // Close the file
  myFile.close();
}





int main(int argc, char** argv)
{
  // Command line arguments
  char *inputfilename;
  char *tpc;

  inputfilename = (char*) argv[1];
  tpc = (char*) argv[2];

  // Load input trees
  char* treedirname;
  if (!strcmp(tpc, "EE"))
    {
      treedirname = (char*) "caloskimE/TrackCaloSkim";
    }
  else if (!strcmp(tpc, "EW"))
    {
      treedirname = (char*) "caloskimE/TrackCaloSkim";
    }
  else if (!strcmp(tpc, "WE"))
    {
      treedirname = (char*) "caloskimW/TrackCaloSkim";
    }
  else if (!strcmp(tpc, "WW"))
    {
      treedirname = (char*) "caloskimW/TrackCaloSkim";
    }

  
  TChain* inputfiles = new TChain(treedirname);
  AddFiles(inputfiles, inputfilename);

  // Get branches
  TTreeReader readerTracks(inputfiles);
  TTreeReaderValue<int> cryo(readerTracks, "cryostat");
  TTreeReaderValue<int> selected(readerTracks, "selected");
  TTreeReaderValue<float> minTime_TPCE(readerTracks, "hit_min_time_p2_tpcE");
  TTreeReaderValue<float> maxTime_TPCE(readerTracks, "hit_max_time_p2_tpcE");
  TTreeReaderValue<float> minTime_TPCW(readerTracks, "hit_min_time_p2_tpcW");
  TTreeReaderValue<float> maxTime_TPCW(readerTracks, "hit_max_time_p2_tpcW");
  TTreeReaderArray<unsigned short> trackHitWires(readerTracks, "hits2.h.wire");
  TTreeReaderArray<float> trackHitTimes(readerTracks, "hits2.h.time");
  TTreeReaderArray<unsigned short> trackHitTPCs(readerTracks, "hits2.h.tpc");
  TTreeReaderArray<bool> trackHitIsOnTraj(readerTracks, "hits2.ontraj");
  TTreeReaderArray<float> trackHitXvals(readerTracks, "hits2.h.sp.x");
  TTreeReaderArray<float> trackHitYvals(readerTracks, "hits2.h.sp.y");
  TTreeReaderArray<float> trackHitZvals(readerTracks, "hits2.h.sp.z");

  // Output
  vector<float> cathodeXs_EE;
  vector<float> cathodeXs_EW;
  vector<float> cathodeXs_WE;
  vector<float> cathodeXs_WW;
  vector<float> cathodeYs_EE;
  vector<float> cathodeYs_EW;
  vector<float> cathodeYs_WE;
  vector<float> cathodeYs_WW;
  vector<float> cathodeZs_EE;
  vector<float> cathodeZs_EW;
  vector<float> cathodeZs_WE;
  vector<float> cathodeZs_WW;

  int numSelectedTracks = 0;
  while(readerTracks.Next())
    {
      
      // Anode-cathode crossing tracks
      if(*selected != 1)
	{
	  continue;
	}

      // TPC info
      int whichTPC = -1;
      if(*maxTime_TPCE - *minTime_TPCE > *maxTime_TPCW - *minTime_TPCW)
	{
	  whichTPC = 0;
	}
      else
	{
	  whichTPC = 1;
	}

      float minT_time = 999999;
      float minT_wire;
      float minT_Yval;
      float minT_Zval;
      float maxT_time = -999999;
      float maxT_wire;
      float maxT_Yval;
      float maxT_Zval;
      int numHits = 0;
      for(int k = 0; k < trackHitWires.GetSize(); k++)
	{
	  if((trackHitIsOnTraj[k] == true) && (((whichTPC == 0) && (trackHitTPCs[k] <2)) || ((whichTPC == 1) && (trackHitTPCs[k] >= 2))))
	    {
	      // Anode
	      if(trackHitTimes[k] < minT_time)
		{
		  minT_time = trackHitTimes[k];
		  minT_wire = trackHitWires[k] + 2536*whichTPC;
		  minT_Yval = trackHitYvals[k];
		  minT_Zval = trackHitZvals[k];
		}
	      // Cathode
	      if(trackHitTimes[k] > maxT_time)
		{
		  maxT_time = trackHitTimes[k];
		  maxT_wire = trackHitWires[k] + 2536*whichTPC;
		  maxT_Yval = trackHitYvals[k];
		  maxT_Zval = trackHitZvals[k];
		}
	      numHits++;
	    }
	}
      
      // proper TPC/cryostat matching
      if(((*cryo == 0) && (whichTPC == 0) && !strcmp(tpc,"EE")) || ((*cryo == 0) && (whichTPC ==1) && !strcmp(tpc,"EW")) || ((*cryo == 1) && (whichTPC == 0) && !strcmp(tpc,"WE")) || ((*cryo == 1) && (whichTPC == 1) && !strcmp(tpc,"WW")))
	{
	  numSelectedTracks++;
	}
      else
	{
	  continue;
	}

      float cathodeX = 0.4*driftVel*(maxT_time - minT_time);
      float cathodeY = maxT_Yval;
      float cathodeZ = maxT_Zval;

      if((*cryo == 0) && (whichTPC == 0))
	{
	  cathodeXs_EE.push_back(cathodeX);
	  cathodeYs_EE.push_back(cathodeY);
	  cathodeZs_EE.push_back(cathodeZ);
	}
      else if((*cryo == 0) && (whichTPC == 1))
	{
          cathodeXs_EW.push_back(cathodeX);
          cathodeYs_EW.push_back(cathodeY);
          cathodeZs_EW.push_back(cathodeZ);
	}
      else if((*cryo == 1) && (whichTPC == 0))
	{
	  cathodeXs_WE.push_back(cathodeX);
          cathodeYs_WE.push_back(cathodeY);
          cathodeZs_WE.push_back(cathodeZ);
	}
      else if((*cryo == 1) && (whichTPC == 1))
	{
	  cathodeXs_WW.push_back(cathodeX);
          cathodeYs_WW.push_back(cathodeY);
          cathodeZs_WW.push_back(cathodeZ);
	}

    } // track loop

  // Write to CSV
  vector<pair<string, vector<float>>> cathodeVals;
  if(!strcmp(tpc,"EE"))
    {
      cathodeVals = {{"X", cathodeXs_EE}, {"Y", cathodeYs_EE}, {"Z", cathodeZs_EE}};
    }
  else if(!strcmp(tpc,"EW"))
    {
      cathodeVals = {{"X", cathodeXs_EW}, {"Y", cathodeYs_EW}, {"Z", cathodeZs_EW}};
    }
  else if(!strcmp(tpc,"WE"))
    {
      cathodeVals = {{"X", cathodeXs_WE}, {"Y", cathodeYs_WE}, {"Z", cathodeZs_WE}};
    }
  else if(!strcmp(tpc,"WW"))
    {
      cathodeVals = {{"X", cathodeXs_WW}, {"Y", cathodeYs_WW}, {"Z", cathodeZs_WW}};
    }

  string outputFileText_cathodeVals = string("cathodeVals_") + tpc + string(".csv");
  write_csv(outputFileText_cathodeVals, cathodeVals);

  return 0;

}
