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
const float offsetCutY_anode = 20.0;
const float offsetCutZ_anode = 300.0;
const float offsetCutY_cathode = 80.0;
const float offsetCutZ_cathode = 300.0;
const float WF_top = 134.96;
const float WF_bottom = -181.86;
const float WF_upstream = -894.951;
const float WF_downstream = 894.951;

const int anodeWireOffset = 4; // 4                                                                
const int cathodeWireOffset = 0; // ?                                                              
const float maxSpatialOffsetMag = 5.0; // 2.5                                                      
const float maxSpatialOffsetMag2 = 1.5; // 1.5 
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
  vector<float> trackNums;

  vector<float> xPositions_EE;
  vector<float> xPositions_EW;
  vector<float> xPositions_WE;
  vector<float> xPositions_WW;
  
  vector<float> xHitPositions_EE;
  vector<float> xHitPositions_EW;
  vector<float> xHitPositions_WE;
  vector<float> xHitPositions_WW;
  vector<float> yHitPositions_EE;
  vector<float> yHitPositions_EW;
  vector<float> yHitPositions_WE;
  vector<float> yHitPositions_WW;
  vector<float> zHitPositions_EE;
  vector<float> zHitPositions_EW;
  vector<float> zHitPositions_WE;
  vector<float> zHitPositions_WW;
  vector<float> offsets_EE;
  vector<float> offsets_EW;
  vector<float> offsets_WE;
  vector<float> offsets_WW;


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
      vector<float> selectedHitWires;
      vector<float> selectedHitTimes;
      vector<float> selectedHitXvals;
      vector<float> selectedHitYvals;
      vector<float> selectedHitZvals;

      for(int k = 0; k < trackHitWires.GetSize(); k++)
	{
	  if((trackHitIsOnTraj[k] == true) && (((whichTPC == 0) && (trackHitTPCs[k] <2)) || ((whichTPC == 1) && (trackHitTPCs[k] >= 2))))
	    {
	      selectedHitTimes.push_back(trackHitTimes[k]);
	      selectedHitWires.push_back(trackHitWires[k] + 2536*whichTPC);
	      selectedHitXvals.push_back(trackHitXvals[k]);
	      selectedHitYvals.push_back(trackHitYvals[k]);
	      selectedHitZvals.push_back(trackHitZvals[k]);
	      
	      if(trackHitTimes[k] < minT_time)
		{
		  minT_time = trackHitTimes[k];
		  minT_wire = trackHitWires[k] + 2536*whichTPC;
		  minT_Yval = trackHitYvals[k];
		  minT_Zval = trackHitZvals[k];
		}
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
      
      // y/z cuts
      if((minT_Yval > WF_top-offsetCutY_anode) || (minT_Yval < WF_bottom+offsetCutY_anode) || (minT_Zval > WF_downstream-offsetCutZ_anode) || (minT_Zval < WF_upstream+offsetCutZ_anode))
	{
	  continue;
	}

      if((maxT_Yval > WF_top-offsetCutY_cathode) || (maxT_Yval < WF_bottom+offsetCutY_cathode) || (maxT_Zval > WF_downstream-offsetCutZ_cathode) || (maxT_Zval < WF_upstream+offsetCutZ_cathode))
	{
	  continue;
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


      float trackNum = static_cast<float>(numSelectedTracks);

      // Find slope of each track
      float slope = (maxT_time - minT_time)/(maxT_wire - minT_wire);

      float anode_wire;
      float anode_time;
      float cathode_wire;
      float cathode_time;
      if(slope > 0)
	{
	  anode_wire = 99999;
	  cathode_wire = -99999;
	  for(int k = 0; k < numHits; k++)
	    {
	      if((selectedHitWires[k] >= minT_wire + anodeWireOffset) && (selectedHitWires[k] < anode_wire))
		{
		  anode_wire = selectedHitWires[k];
		  anode_time = selectedHitTimes[k];
		}
	      if((selectedHitWires[k] <= maxT_wire - cathodeWireOffset) && (selectedHitWires[k] > cathode_wire))
		{
		  cathode_wire = selectedHitWires[k];
		  cathode_time = selectedHitTimes[k];
		}
	    }
	  slope = (cathode_time - anode_time)/(cathode_wire - anode_wire);
	}
      else
	{
	  anode_wire = -99999;
	  cathode_wire = 99999;
	  for(int k = 0; k < numHits; k++)
	    {
	      if((selectedHitWires[k] <= minT_wire - anodeWireOffset) && (selectedHitWires[k] > anode_wire))
		{
		  anode_wire = selectedHitWires[k];
		  anode_time = selectedHitTimes[k];
		}
	      if((selectedHitWires[k] >= maxT_wire + cathodeWireOffset) && (selectedHitWires[k] < cathode_wire))
		{
		  cathode_wire = selectedHitWires[k];
		  cathode_time = selectedHitTimes[k];
		}
	    }
	  slope = (cathode_time - anode_time)/(cathode_wire - anode_wire);
	}

      // Calculate offset for each hit
      for(int i = 0; i < numHits; i++)
	{
	  if((selectedHitTimes[i] < anode_time) || (selectedHitTimes[i] > cathode_time))
	    {
	      continue;
	    }

	  float interpTime = slope*(selectedHitWires[i] - anode_wire) + anode_time;
	  float offset = 0.4*driftVel*(selectedHitTimes[i]-interpTime);
	  
	  if(fabs(offset) < maxSpatialOffsetMag)
	    {
	      
	      //int index = round(((((float) numBins)/150.0)*0.4*driftVel*(interpTime - minT_time))-0.5);
	      
	      if((*cryo == 0) && (whichTPC == 0))
		{
		  trackNums.push_back(trackNum);
                  xPositions_EE.push_back(0.4*driftVel*(interpTime-minT_time));
		  //xHitPositions_EE.push_back(selectedHitXvals[i]);
		  xHitPositions_EE.push_back(0.4*driftVel*(selectedHitTimes[i]-minT_time));
                  yHitPositions_EE.push_back(selectedHitYvals[i]);
                  zHitPositions_EE.push_back(selectedHitZvals[i]);
                  offsets_EE.push_back(offset);
		}
	      else if((*cryo == 0) && (whichTPC == 1))
		{
		  trackNums.push_back(trackNum);
                  xPositions_EW.push_back(0.4*driftVel*(interpTime-minT_time));
		  xHitPositions_EW.push_back(0.4*driftVel*(selectedHitTimes[i]-minT_time));
                  //xHitPositions_EW.push_back(selectedHitXvals[i]);
		  yHitPositions_EW.push_back(selectedHitYvals[i]);
                  zHitPositions_EW.push_back(selectedHitZvals[i]);
                  offsets_EW.push_back(offset);
		}
	      else if((*cryo == 1) && (whichTPC == 0))
		{
		  trackNums.push_back(trackNum);
                  xPositions_WE.push_back(0.4*driftVel*(interpTime-minT_time));
		  xHitPositions_WE.push_back(0.4*driftVel*(selectedHitTimes[i]-minT_time));
		  //xHitPositions_WE.push_back(selectedHitXvals[i]);
                  yHitPositions_WE.push_back(selectedHitYvals[i]);
                  zHitPositions_WE.push_back(selectedHitZvals[i]);
                  offsets_WE.push_back(offset);
		}
	      else if((*cryo == 1) && (whichTPC == 1))
		{
		  trackNums.push_back(trackNum);
                  xPositions_WW.push_back(0.4*driftVel*(interpTime-minT_time));
		  //xHitPositions_WW.push_back(selectedHitXvals[i]);
                  xHitPositions_WW.push_back(0.4*driftVel*(selectedHitTimes[i]-minT_time));
		  yHitPositions_WW.push_back(selectedHitYvals[i]);
                  zHitPositions_WW.push_back(selectedHitZvals[i]);
                  offsets_WW.push_back(offset);
		}
	    } 
	}
    } // track loop

  // Write to CSV
  vector<pair<string, vector<float>>> offsetVals;
  if(!strcmp(tpc,"EE"))
    {
      offsetVals = {{"trackNum", trackNums}, {"xPosition", xPositions_EE}, {"xHitPosition", xHitPositions_EE}, {"yHitPosition", yHitPositions_EE}, {"zHitPosition", zHitPositions_EE}, {"offset", offsets_EE}};
    }
  else if(!strcmp(tpc,"EW"))
    {
      offsetVals = {{"trackNum", trackNums}, {"xPosition", xPositions_EW}, {"xHitPosition", xHitPositions_EW}, {"yHitPosition", yHitPositions_EW}, {"zHitPosition", zHitPositions_EW}, {"offset", offsets_EW}};
    }
  else if(!strcmp(tpc,"WE"))
    {
      offsetVals = {{"trackNum", trackNums}, {"xPosition", xPositions_WE}, {"xHitPosition", xHitPositions_WE}, {"yHitPosition", yHitPositions_WE}, {"zHitPosition", zHitPositions_WE}, {"offset", offsets_WE}};
    }
  else if(!strcmp(tpc,"WW"))
    {
      offsetVals = {{"trackNum", trackNums}, {"xPosition", xPositions_WW}, {"xHitPosition", xHitPositions_WW}, {"yHitPosition", yHitPositions_WW}, {"zHitPosition", zHitPositions_WW}, {"offset", offsets_WW}};
    }

  string outputFileText_offsets = string("offsets_") + tpc + string(".csv");
  write_csv(outputFileText_offsets, offsetVals);

  return 0;

}
