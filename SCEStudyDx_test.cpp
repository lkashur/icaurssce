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
      fileVec.push_back(line);
      n_files++;
    }

  for(unsigned int iFile=0; iFile<fileVec.size(); ++iFile)
    {                             
      ch->AddFile(fileVec[iFile].c_str());
    }

  return;
}

// Constants
float max_offset = 99999;
//float drift_vel = 0.157565; //MC
//float drift_vel =  0.15715; //data

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
  TTreeReader reader_tracks(inputfiles);
  TTreeReaderValue<int> cryo(reader_tracks, "cryostat");
  TTreeReaderValue<int> selected(reader_tracks, "selected");
  TTreeReaderValue<float> min_time_tpc_e(reader_tracks, "hit_min_time_p2_tpcE");
  TTreeReaderValue<float> max_time_tpc_e(reader_tracks, "hit_max_time_p2_tpcE");
  TTreeReaderValue<float> min_time_tpc_w(reader_tracks, "hit_min_time_p2_tpcW");
  TTreeReaderValue<float> max_time_tpc_w(reader_tracks, "hit_max_time_p2_tpcW");
  TTreeReaderArray<unsigned short> track_hit_tpcs(reader_tracks, "hits2.h.tpc");
  TTreeReaderArray<bool> track_hit_on_traj(reader_tracks, "hits2.ontraj");
  TTreeReaderArray<float> track_hit_xs(reader_tracks, "hits2.h.sp.x");
  TTreeReaderArray<float> track_hit_ys(reader_tracks, "hits2.h.sp.y");
  TTreeReaderArray<float> track_hit_zs(reader_tracks, "hits2.h.sp.z");
  TTreeReaderArray<float> track_hit_ts(reader_tracks, "hits2.h.time");

  // Output
  string output_file_text = string("offsets_") + tpc + string(".root");
  TFile outfile(output_file_text.c_str(),"RECREATE");
  outfile.cd();

  TTree *offsettree = new TTree("offsettree", "");
  
  int track_num = 0;
  float anode_y;
  float anode_z;
  float cathode_y;
  float cathode_z;
  float true_x;
  float hit_x;
  float hit_y;
  float hit_z;
  float hit_t;
  float offset;

  offsettree->Branch("track_num",&track_num);
  offsettree->Branch("anode_y",&anode_y);
  offsettree->Branch("anode_z",&anode_z);
  offsettree->Branch("cathode_y",&cathode_y);
  offsettree->Branch("cathode_z",&cathode_z);
  offsettree->Branch("true_x",&true_x);
  offsettree->Branch("hit_x",&hit_x);
  offsettree->Branch("hit_y",&hit_y);
  offsettree->Branch("hit_z",&hit_z);
  offsettree->Branch("hit_t",&hit_t);
  offsettree->Branch("offset",&offset);

  // Loop through tracks
  while(reader_tracks.Next())
    { 
      // Anode-cathode crossing tracks
      if(*selected != 1)
	{
	  continue;
	}

      // TPC info
      int which_tpc = -1;
      if(*max_time_tpc_e - *min_time_tpc_e > *max_time_tpc_w - *min_time_tpc_w)
	{
	  which_tpc = 0;
	}
      else
	{
	  which_tpc = 1;
	}

      // proper TPC/cryostat matching 
      if(((*cryo == 0) && (which_tpc == 0) && !strcmp(tpc,"EE")) || ((*cryo == 0) && (which_tpc ==1) && !strcmp(tpc,"EW")) || ((*cryo == 1) && (which_tpc == 0) && !strcmp(tpc,"WE")) || ((*cryo == 1) && (which_tpc == 1) && !strcmp(tpc,"WW")))
        {
	  track_num++;
        }
      else
        {
          continue;
	}

      // Find anode and cathode side of track
      vector<float> selected_hit_xs;
      vector<float> selected_hit_ys;
      vector<float> selected_hit_zs;
      vector<float> selected_hit_ts;
      float min_time = 99999;
      float max_time = -99999;
      float min_time_x;
      float min_time_y;
      float min_time_z;
      float max_time_x;
      float max_time_y;
      float max_time_z;

      int num_hits = 0;
      for(int k = 0; k < track_hit_ts.GetSize(); k++)
	{
	  if((track_hit_on_traj[k] == true) && (((which_tpc == 0) && (track_hit_tpcs[k] <2)) || ((which_tpc == 1) && (track_hit_tpcs[k] >= 2))))
	    {
	      selected_hit_ts.push_back(track_hit_ts[k]);
	      selected_hit_xs.push_back(track_hit_xs[k]);
	      selected_hit_ys.push_back(track_hit_ys[k]);
	      selected_hit_zs.push_back(track_hit_zs[k]);
	      
	      if(track_hit_ts[k] < min_time)
		{
		  min_time = track_hit_ts[k];
		  min_time_x = track_hit_xs[k];
		  min_time_y = track_hit_ys[k];
		  min_time_z = track_hit_zs[k];
		}

	      if(track_hit_ts[k] > max_time)
		{
		  max_time = track_hit_ts[k];
		  max_time_x = track_hit_xs[k];
		  max_time_y = track_hit_ys[k];
		  max_time_z = track_hit_zs[k];
		}    
	      num_hits++;
	    }
	}

      float anode_x = min_time_x;
      anode_y = min_time_y;
      anode_z = min_time_z;
      float anode_time = min_time;
      float cathode_x = max_time_x;
      cathode_y = max_time_y;
      cathode_z = max_time_z;
      float cathode_time = max_time;

      // Find slope of each track (needed to calculate "ideal" hit position)
      float slope = (cathode_x - anode_x)/(cathode_z - anode_z);

      // Calculate offset for each hit
      for(int i = 0; i < num_hits; i++)
	{
	  float ideal_x = slope*(selected_hit_zs[i] - anode_z) + anode_x;
	  float ideal_z = (selected_hit_xs[i] - anode_x)/slope + anode_z;
	  float dx = selected_hit_xs[i] - ideal_x;

	  if(fabs(dx) < max_offset)
	    {
	    
	      if((selected_hit_ts[i] < anode_time) || (selected_hit_ts[i] > cathode_time))
		{
		  continue;
		}
	      true_x = ideal_x;
	      hit_x = selected_hit_xs[i];
	      hit_y = selected_hit_ys[i];
	      hit_z = selected_hit_zs[i];
	      hit_t = selected_hit_ts[i];
	      offset = dx;

	      offsettree->Fill();
	    }
	}
    } // track loop

  // Save output
  outfile.cd();
  offsettree->Write();
  outfile.Close();
  return 0;

}
