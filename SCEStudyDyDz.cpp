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
float TPC_Top_Y = 134.96; //134.96 //134.55
float TPC_Bottom_Y = -181.86;
float TPC_Upstream_Z = -894.9505;
float TPC_Downstream_Z = 894.9505;
float EE_TPC_BoundaryX0 = -358.49;
float EE_TPC_BoundaryX1 = -210.29;
float EW_TPC_BoundaryX0 = -210.14;
float EW_TPC_BoundaryX1 = -61.94;
float WE_TPC_BoundaryX0 = 61.94;
float WE_TPC_BoundaryX1 = 210.14;
float WW_TPC_BoundaryX0 = 210.29;
float WW_TPC_BoundaryX1 = 358.49;

// Custom Functions
//void AddFiles(TChain *ch, const char *dir, const char* substr);
void AddFiles(TChain *ch, const char *fileList);
void write_csv(string filename, vector<pair<string, vector<float>>> dataset);


// Function to get all input files from a given directory
/*
void AddFiles(TChain *ch, const char *dir = ".", const char* substr = "")
{
  TSystemDirectory thisdir(dir, dir);
  TList *files = thisdir.GetListOfFiles();

  if(files)
  {
    TSystemFile *file;
    TString fname;
    TIter next(files);
    while((file = (TSystemFile*)next()))
    {
      fname = file->GetName();
      if(!file->IsDirectory() && fname.Contains(substr) && fname.EndsWith(".root"))
      {
	ch->AddFile(Form("%s/%s",(char*)dir,fname.Data()));
      }
    }
  }

  return;
}
*/

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

// Function to write output to csv file
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
  //char *dirname;
  //char *filetext;
  //char *tpc;
  //char *plotname;

  //dirname = (char*) argv[1];
  //filetext = (char*) argv[2];
  //tpc = (char*) argv[3];
  //plotname = (char*) argv[4];

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
  //AddFiles(inputfiles, dirname, filetext);
  //AddFiles(inputfiles, "tempFileList.txt");
  AddFiles(inputfiles, inputfilename);

  TTreeReader readerTracks(inputfiles);
  TTreeReaderValue<int> cryo(readerTracks, "cryostat");
  TTreeReaderValue<int> selected(readerTracks, "selected");

  TTreeReaderValue<float> startX(readerTracks, "start.x");
  TTreeReaderValue<float> startY(readerTracks, "start.y");
  TTreeReaderValue<float> startZ(readerTracks, "start.z");
  TTreeReaderValue<float> endX(readerTracks, "end.x");
  TTreeReaderValue<float> endY(readerTracks, "end.y");
  TTreeReaderValue<float> endZ(readerTracks, "end.z");

  //TTreeReaderValue<float> startX(readerTracks, "truth.p.start.x");
  //TTreeReaderValue<float> startY(readerTracks, "truth.p.start.y");
  //TTreeReaderValue<float> startZ(readerTracks, "truth.p.start.z");
  //TTreeReaderValue<float> endX(readerTracks, "truth.p.end.x");
  //TTreeReaderValue<float> endY(readerTracks, "truth.p.end.y");
  //TTreeReaderValue<float> endZ(readerTracks, "truth.p.end.z");


  //TTreeReaderArray<unsigned short> trackHitInd1Wires(readerTracks, "hits0.h.wire");
  //TTreeReaderArray<float> trackHitInd1Xs(readerTracks, "hits0.tp.x");
  //TTreeReaderArray<float> trackHitInd1Ys(readerTracks, "hits0.tp.y");
  //TTreeReaderArray<float> trackHitInd1Zs(readerTracks, "hits0.tp.z");
  //TTreeReaderArray<float> trackHitInd2Xs(readerTracks, "hits1.tp.x");
  //TTreeReaderArray<float> trackHitInd2Ys(readerTracks, "hits1.tp.y");
  //TTreeReaderArray<float> trackHitInd2Zs(readerTracks, "hits1.tp.z");
  //TTreeReaderArray<float> trackHitColXs(readerTracks, "hits2.tp.x");
  //TTreeReaderArray<float> trackHitColYs(readerTracks, "hits2.tp.y");
  //TTreeReaderArray<float> trackHitColZs(readerTracks, "hits2.tp.z");

  // Output  
  vector<float> endPointsX_EE_Top;
  vector<float> endPointsY_EE_Top;
  vector<float> endPointsZ_EE_Top;
  vector<float> endPoints_EE_Top_dY;
  vector<float> endPointsX_EE_Bottom;
  vector<float> endPointsY_EE_Bottom;
  vector<float> endPointsZ_EE_Bottom;
  vector<float> endPoints_EE_Bottom_dY;
  vector<float> endPointsX_EE_Upstream;
  vector<float> endPointsY_EE_Upstream;
  vector<float> endPointsZ_EE_Upstream;
  vector<float> endPoints_EE_Upstream_dZ;
  vector<float> endPointsX_EE_Downstream;
  vector<float> endPointsY_EE_Downstream;
  vector<float> endPointsZ_EE_Downstream;
  vector<float> endPoints_EE_Downstream_dZ;
  
  vector<float> endPointsX_EW_Top;
  vector<float> endPointsY_EW_Top;
  vector<float> endPointsZ_EW_Top;
  vector<float> endPoints_EW_Top_dY;
  vector<float> endPointsX_EW_Bottom;
  vector<float> endPointsY_EW_Bottom;
  vector<float> endPointsZ_EW_Bottom;
  vector<float> endPoints_EW_Bottom_dY;
  vector<float> endPointsX_EW_Upstream;
  vector<float> endPointsY_EW_Upstream;
  vector<float> endPointsZ_EW_Upstream;
  vector<float> endPoints_EW_Upstream_dZ;
  vector<float> endPointsX_EW_Downstream;
  vector<float> endPointsY_EW_Downstream;
  vector<float> endPointsZ_EW_Downstream;
  vector<float> endPoints_EW_Downstream_dZ;

  vector<float> endPointsX_WE_Top;
  vector<float> endPointsY_WE_Top;
  vector<float> endPointsZ_WE_Top;
  vector<float> endPoints_WE_Top_dY;
  vector<float> endPointsX_WE_Bottom;
  vector<float> endPointsY_WE_Bottom;
  vector<float> endPointsZ_WE_Bottom;
  vector<float> endPoints_WE_Bottom_dY;
  vector<float> endPointsX_WE_Upstream;
  vector<float> endPointsY_WE_Upstream;
  vector<float> endPointsZ_WE_Upstream;
  vector<float> endPoints_WE_Upstream_dZ;  
  vector<float> endPointsX_WE_Downstream;
  vector<float> endPointsY_WE_Downstream;
  vector<float> endPointsZ_WE_Downstream;
  vector<float> endPoints_WE_Downstream_dZ;

  vector<float> endPointsX_WW_Top;
  vector<float> endPointsY_WW_Top;
  vector<float> endPointsZ_WW_Top;
  vector<float> endPoints_WW_Top_dY;
  vector<float> numHitsInd1_WW_Top;
  vector<float> numHitsInd2_WW_Top;
  vector<float> numHitsCol_WW_Top;
  vector<float> endPointsX_WW_Bottom;
  vector<float> endPointsY_WW_Bottom;
  vector<float> endPointsZ_WW_Bottom;
  vector<float> endPoints_WW_Bottom_dY;
  vector<float> endPointsX_WW_Upstream;
  vector<float> endPointsY_WW_Upstream;
  vector<float> endPointsZ_WW_Upstream;
  vector<float> endPoints_WW_Upstream_dZ;
  vector<float> endPointsX_WW_Downstream;
  vector<float> endPointsY_WW_Downstream;
  vector<float> endPointsZ_WW_Downstream;
  vector<float> endPoints_WW_Downstream_dZ;  

  // Loop through tracks
  int numSelectedTracks = 0;
  while(readerTracks.Next())
    {

      // Select throughgoing tracks
      if(*selected != 2)
	{
          continue;
        }

      // Get TPC face associated with start point
      // EE
      if((*startX > EE_TPC_BoundaryX0 + 10) & (*startX < EE_TPC_BoundaryX1 - 10) & (*startZ > TPC_Upstream_Z + 10) & (*startZ < TPC_Downstream_Z - 10))
	{
	  // Top
	  if(fabs(TPC_Top_Y - *startY) < 10) 
	    {
	      endPointsX_EE_Top.push_back(*startX);
	      endPointsY_EE_Top.push_back(*startY);
	      endPointsZ_EE_Top.push_back(*startZ);
	      endPoints_EE_Top_dY.push_back(TPC_Top_Y - *startY);
	    }
	  // Bottom
	  else if(fabs(TPC_Bottom_Y - *startY) < 10)
	    {
	      endPointsX_EE_Bottom.push_back(*startX);
              endPointsY_EE_Bottom.push_back(*startY);
              endPointsZ_EE_Bottom.push_back(*startZ);
	      endPoints_EE_Bottom_dY.push_back(TPC_Bottom_Y - *startY);
	    }
      	}
      if((*startX > EE_TPC_BoundaryX0 + 10) & (*startX < EE_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
	{
	  // Upstream
	  if(fabs(TPC_Upstream_Z - *startZ) < 10)
	    {
	      endPointsX_EE_Upstream.push_back(*startX);
	      endPointsY_EE_Upstream.push_back(*startY);
	      endPointsZ_EE_Upstream.push_back(*startZ);
	      endPoints_EE_Upstream_dZ.push_back(TPC_Upstream_Z - *startZ);
	    }

	  // Downstream
          if(fabs(TPC_Downstream_Z - *startZ) < 10)
            {
	      endPointsX_EE_Downstream.push_back(*startX);
	      endPointsY_EE_Downstream.push_back(*startY);
	      endPointsZ_EE_Downstream.push_back(*startZ);
	      endPoints_EE_Downstream_dZ.push_back(TPC_Downstream_Z - *startZ);
            }
	}
      // EW
      if((*startX > EW_TPC_BoundaryX0 + 10) & (*startX < EW_TPC_BoundaryX1 - 10) & (*startZ > TPC_Upstream_Z + 10) & (*startZ < TPC_Downstream_Z - 10))
	{
	  // Top
	  if(fabs(TPC_Top_Y - *startY) < 10)
	    {
	      endPointsX_EW_Top.push_back(*startX);
	      endPointsY_EW_Top.push_back(*startY);
	      endPointsZ_EW_Top.push_back(*startZ);
	      endPoints_EW_Top_dY.push_back(TPC_Top_Y - *startY);
	    }
	  // Bottom
	  if(fabs(TPC_Bottom_Y - *startY) < 10)
	    {
	      endPointsX_EW_Bottom.push_back(*startX);
              endPointsY_EW_Bottom.push_back(*startY);
              endPointsZ_EW_Bottom.push_back(*startZ);
	      endPoints_EW_Bottom_dY.push_back(TPC_Bottom_Y - *startY);
	    }
	}
      if((*startX > EW_TPC_BoundaryX0 + 10) & (*startX < EW_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
        {
          // Upstream
          if(fabs(TPC_Upstream_Z - *startZ) < 10)
            {
              endPointsX_EW_Upstream.push_back(*startX);
              endPointsY_EW_Upstream.push_back(*startY);
              endPointsZ_EW_Upstream.push_back(*startZ);
              endPoints_EW_Upstream_dZ.push_back(TPC_Upstream_Z - *startZ);
            }

          // Downstream
          if(fabs(TPC_Downstream_Z - *startZ) < 10)
            {
              endPointsX_EW_Downstream.push_back(*startX);
              endPointsY_EW_Downstream.push_back(*startY);
              endPointsZ_EW_Downstream.push_back(*startZ);
              endPoints_EW_Downstream_dZ.push_back(TPC_Downstream_Z - *startZ);
            }
        }
      // WE
      if((*startX > WE_TPC_BoundaryX0 + 10) & (*startX < WE_TPC_BoundaryX1 - 10) & (*startZ > TPC_Upstream_Z + 10) & (*startZ < TPC_Downstream_Z - 10))
	{
	  // Top
	  if(fabs(TPC_Top_Y - *startY) < 10)
	    {
	      endPointsX_WE_Top.push_back(*startX);
	      endPointsY_WE_Top.push_back(*startY);
	      endPointsZ_WE_Top.push_back(*startZ);
	      endPoints_WE_Top_dY.push_back(TPC_Top_Y - *startY);
	    }
	  // Bottom
	  if(fabs(TPC_Bottom_Y - *startY) < 10)
	    {
              endPointsX_WE_Bottom.push_back(*startX);
              endPointsY_WE_Bottom.push_back(*startY);
              endPointsZ_WE_Bottom.push_back(*startZ);
	      endPoints_WE_Bottom_dY.push_back(TPC_Bottom_Y - *startY);
	    }
	}
      if((*startX > WE_TPC_BoundaryX0 + 10) & (*startX < WE_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
        {
          // Upstream
          if(fabs(TPC_Upstream_Z - *startZ) < 10)
            {
              endPointsX_WE_Upstream.push_back(*startX);
              endPointsY_WE_Upstream.push_back(*startY);
              endPointsZ_WE_Upstream.push_back(*startZ);
              endPoints_WE_Upstream_dZ.push_back(TPC_Upstream_Z - *startZ);
            }

          // Downstream
          if(fabs(TPC_Downstream_Z - *startZ) < 10)
            {
              endPointsX_WE_Downstream.push_back(*startX);
              endPointsY_WE_Downstream.push_back(*startY);
              endPointsZ_WE_Downstream.push_back(*startZ);
              endPoints_WE_Downstream_dZ.push_back(TPC_Downstream_Z - *startZ);
            }
        }
      // WW
      // Change this back to zero 10 cm offsets from each side in x...
      if((*startX > WW_TPC_BoundaryX0 + 10) & (*startX < WW_TPC_BoundaryX1 - 10) & (*startZ > TPC_Upstream_Z + 10) & (*startZ < TPC_Downstream_Z - 10))
	{
	  // Top
	  if(fabs(TPC_Top_Y - *startY) < 10)
	    {
	      endPointsX_WW_Top.push_back(*startX);
	      endPointsY_WW_Top.push_back(*startY);
	      endPointsZ_WW_Top.push_back(*startZ);
	      endPoints_WW_Top_dY.push_back(TPC_Top_Y - *startY);
	      
	      /*
	      // test: can I find what planes contributed to start/end SPs?
	      float numHitsInd1 = 0;
	      float numHitsInd2 = 0;
	      float numHitsCol = 0;
	      for(int k = 0; k < trackHitInd1Xs.GetSize(); k++)
		{
		  if((trackHitInd1Xs[k] == *startX) && (trackHitInd1Ys[k] == *startY) && (trackHitInd1Zs[k] == *startZ))
		    {
		      numHitsInd1++;
		    }
		}
	      for(int k = 0; k < trackHitInd2Xs.GetSize(); k++)
		{
		  if((trackHitInd2Xs[k] == *startX) && (trackHitInd2Ys[k] == *startY) && (trackHitInd2Zs[k] == *startZ))
		    {
		      numHitsInd2++;
		    }
		}
	      for(int k = 0; k < trackHitColXs.GetSize(); k++)
		{
                  if((trackHitColXs[k] == *startX) && (trackHitColYs[k] == *startY) && (trackHitColZs[k] == *startZ))
                    {
		      numHitsCol++;
                    }
		}
		
	      
	      numHitsInd1_WW_Top.push_back(numHitsInd1);
	      numHitsInd2_WW_Top.push_back(numHitsInd2);
	      numHitsCol_WW_Top.push_back(numHitsCol);
	      */

	    }

	  // Bottom
	  if(fabs(TPC_Bottom_Y - *startY) < 10)
	    {
	      endPointsX_WW_Bottom.push_back(*startX);
	      endPointsY_WW_Bottom.push_back(*startY);
              endPointsZ_WW_Bottom.push_back(*startZ);
	      endPoints_WW_Bottom_dY.push_back(TPC_Bottom_Y - *startY);
	    }
	}
      if((*startX > WW_TPC_BoundaryX0 + 10) & (*startX < WW_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
        {
          // Upstream
          if(fabs(TPC_Upstream_Z - *startZ) < 10)
            {
              endPointsX_WW_Upstream.push_back(*startX);
              endPointsY_WW_Upstream.push_back(*startY);
              endPointsZ_WW_Upstream.push_back(*startZ);
              endPoints_WW_Upstream_dZ.push_back(TPC_Upstream_Z - *startZ);
            }

          // Downstream
          if(fabs(TPC_Downstream_Z - *startZ) < 10)
            {
              endPointsX_WW_Downstream.push_back(*startX);
              endPointsY_WW_Downstream.push_back(*startY);
              endPointsZ_WW_Downstream.push_back(*startZ);
              endPoints_WW_Downstream_dZ.push_back(TPC_Downstream_Z - *startZ);
            }
        }      

      // Get TPC face associated with end point
      // EE
      if((*endX > EE_TPC_BoundaryX0 + 10) & (*endX < EE_TPC_BoundaryX1 - 10) & (*endZ > TPC_Upstream_Z + 10) & (*endZ < TPC_Downstream_Z - 10))
        {
	  // Top
          if(fabs(TPC_Top_Y - *endY) < 10)
            {
              endPointsX_EE_Top.push_back(*endX);
	      endPointsY_EE_Top.push_back(*endY);
              endPointsZ_EE_Top.push_back(*endZ);
              endPoints_EE_Top_dY.push_back(TPC_Top_Y - *endY);
            }
	  // Bottom
          if(fabs(TPC_Bottom_Y - *endY) < 10)
            {
              endPointsX_EE_Bottom.push_back(*endX);
              endPointsY_EE_Bottom.push_back(*endY);
              endPointsZ_EE_Bottom.push_back(*endZ);
              endPoints_EE_Bottom_dY.push_back(TPC_Bottom_Y - *endY);
            }
        }
      if((*endX > EE_TPC_BoundaryX0 + 10) & (*endX < EE_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
	{
	  // Upstream
	  if(fabs(TPC_Upstream_Z - *endZ) < 10)
	    {
	      endPointsX_EE_Upstream.push_back(*endX);
	      endPointsY_EE_Upstream.push_back(*endY);
	      endPointsZ_EE_Upstream.push_back(*endZ);
	      endPoints_EE_Upstream_dZ.push_back(TPC_Upstream_Z - *endZ);
	    }
	  // Downstream
	  if(fabs(TPC_Downstream_Z - *endZ) < 10)
	    {
	      endPointsX_EE_Downstream.push_back(*endX);
	      endPointsY_EE_Downstream.push_back(*endY);
	      endPointsZ_EE_Downstream.push_back(*endZ);
	      endPoints_EE_Downstream_dZ.push_back(TPC_Downstream_Z - *endZ);
	    }
	}
      // EW
      if((*endX > EW_TPC_BoundaryX0 + 10) & (*endX < EW_TPC_BoundaryX1 - 10) & (*endZ > TPC_Upstream_Z + 10) & (*endZ < TPC_Downstream_Z - 10))
        {
	  // Top
          if(fabs(TPC_Top_Y - *endY) < 10)
            {
              endPointsX_EW_Top.push_back(*endX);
              endPointsY_EW_Top.push_back(*endY);
              endPointsZ_EW_Top.push_back(*endZ);
              endPoints_EW_Top_dY.push_back(TPC_Top_Y - *endY);
            }
	  // Bottom
          if(fabs(TPC_Bottom_Y - *endY) < 10)
            {
              endPointsX_EW_Bottom.push_back(*endX);
              endPointsY_EW_Bottom.push_back(*endY);
              endPointsZ_EW_Bottom.push_back(*endZ);
              endPoints_EW_Bottom_dY.push_back(TPC_Bottom_Y - *endY);
            }
        }
      if((*endX > EW_TPC_BoundaryX0 + 10) & (*endX < EW_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
        {
          // Upstream
          if(fabs(TPC_Upstream_Z - *endZ) < 10)
            {
              endPointsX_EW_Upstream.push_back(*endX);
              endPointsY_EW_Upstream.push_back(*endY);
              endPointsZ_EW_Upstream.push_back(*endZ);
              endPoints_EW_Upstream_dZ.push_back(TPC_Upstream_Z - *endZ);
            }
          // Downstream
          if(fabs(TPC_Downstream_Z - *endZ) < 10)
            {
              endPointsX_EW_Downstream.push_back(*endX);
              endPointsY_EW_Downstream.push_back(*endY);
              endPointsZ_EW_Downstream.push_back(*endZ);
              endPoints_EW_Downstream_dZ.push_back(TPC_Downstream_Z - *endZ);
            }
        }
      // WE
      if((*endX > WE_TPC_BoundaryX0 + 10) & (*endX < WE_TPC_BoundaryX1 - 10) & (*endZ > TPC_Upstream_Z + 10) & (*endZ < TPC_Downstream_Z - 10))
        {
	  // Top
          if(fabs(TPC_Top_Y - *endY) < 10)
            {
              endPointsX_WE_Top.push_back(*endX);
              endPointsY_WE_Top.push_back(*endY);
              endPointsZ_WE_Top.push_back(*endZ);
              endPoints_WE_Top_dY.push_back(TPC_Top_Y - *endY);
            }
	  // Bottom
          if(fabs(TPC_Bottom_Y - *endY) < 10)
            {
              endPointsX_WE_Bottom.push_back(*endX);
              endPointsY_WE_Bottom.push_back(*endY);
              endPointsZ_WE_Bottom.push_back(*endZ);
              endPoints_WE_Bottom_dY.push_back(TPC_Bottom_Y - *endY);
            }
        }
      if((*endX > WE_TPC_BoundaryX0 + 10) & (*endX < WE_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
        {
          // Upstream
          if(fabs(TPC_Upstream_Z - *endZ) < 10)
            {
              endPointsX_WE_Upstream.push_back(*endX);
              endPointsY_WE_Upstream.push_back(*endY);
              endPointsZ_WE_Upstream.push_back(*endZ);
              endPoints_WE_Upstream_dZ.push_back(TPC_Upstream_Z - *endZ);
            }
          // Downstream
          if(fabs(TPC_Downstream_Z - *endZ) < 10)
            {
              endPointsX_WE_Downstream.push_back(*endX);
              endPointsY_WE_Downstream.push_back(*endY);
              endPointsZ_WE_Downstream.push_back(*endZ);
              endPoints_WE_Downstream_dZ.push_back(TPC_Downstream_Z - *endZ);
            }
        }
      // WW
      if((*endX > WW_TPC_BoundaryX0 + 10) & (*endX < WW_TPC_BoundaryX1 - 10) & (*endZ > TPC_Upstream_Z + 10) & (*endZ < TPC_Downstream_Z - 10))
        {
	  // Top
          if(fabs(TPC_Top_Y - *endY) < 10)
            {
              endPointsX_WW_Top.push_back(*endX);
              endPointsY_WW_Top.push_back(*endY);
              endPointsZ_WW_Top.push_back(*endZ);
              endPoints_WW_Top_dY.push_back(TPC_Top_Y - *endY);

	      /*
              float numHitsInd1 = 0;
              float numHitsInd2 = 0;
              float numHitsCol = 0;
              for(int k = 0; k < trackHitInd1Xs.GetSize(); k++)
                {
                  if((trackHitInd1Xs[k] == *endX) &&(trackHitInd1Ys[k] == *endY) && (trackHitInd1Zs[k] == *endZ))
                    {
                      numHitsInd1++;
                    }
                }
              for(int k = 0; k < trackHitInd2Xs.GetSize(); k++)
                {
                  if((trackHitInd2Xs[k] == *endX) && (trackHitInd2Ys[k] == *endY) && (trackHitInd2Zs[k] == *endZ))
                    {
                      numHitsInd2++;
                    }
                }
              for(int k = 0; k < trackHitColXs.GetSize(); k++)
                {
                  if((trackHitColXs[k] == *endX) && (trackHitColYs[k] == *endY) && (trackHitColZs[k] == *endZ))
                    {
                      numHitsCol++;
                    }
                }


              numHitsInd1_WW_Top.push_back(numHitsInd1);
              numHitsInd2_WW_Top.push_back(numHitsInd2);
              numHitsCol_WW_Top.push_back(numHitsCol);
	      */

            }
	  // Bottom
          if(fabs(TPC_Bottom_Y - *endY) < 10)
            {
              endPointsX_WW_Bottom.push_back(*endX);
              endPointsY_WW_Bottom.push_back(*endY);
              endPointsZ_WW_Bottom.push_back(*endZ);
              endPoints_WW_Bottom_dY.push_back(TPC_Bottom_Y - *endY);
            }
        }
      if((*endX > WW_TPC_BoundaryX0 + 10) & (*endX < WW_TPC_BoundaryX1 - 10) & (*startY < TPC_Top_Y - 10) & (*startY > TPC_Bottom_Y + 10))
        {
          // Upstream
          if(fabs(TPC_Upstream_Z - *endZ) < 10)
            {
              endPointsX_WW_Upstream.push_back(*endX);
              endPointsY_WW_Upstream.push_back(*endY);
              endPointsZ_WW_Upstream.push_back(*endZ);
              endPoints_WW_Upstream_dZ.push_back(TPC_Upstream_Z - *endZ);
            }
          // Downstream
          if(fabs(TPC_Downstream_Z - *endZ) < 10)
            {
              endPointsX_WW_Downstream.push_back(*endX);
              endPointsY_WW_Downstream.push_back(*endY);
              endPointsZ_WW_Downstream.push_back(*endZ);
              endPoints_WW_Downstream_dZ.push_back(TPC_Downstream_Z - *endZ);
            }
        }
    
    } // track loop

  
  // Write to CSV
  vector<pair<string, vector<float>>> valsTop;
  vector<pair<string, vector<float>>> valsBottom;
  vector<pair<string, vector<float>>> valsUpstream;
  vector<pair<string, vector<float>>> valsDownstream;

  if(!strcmp(tpc,"EE"))
    {
      valsTop = {{"X", endPointsX_EE_Top}, {"Y", endPointsY_EE_Top}, {"Z", endPointsZ_EE_Top}, {"dY", endPoints_EE_Top_dY}};
      valsBottom = {{"X", endPointsX_EE_Bottom}, {"Y", endPointsY_EE_Bottom}, {"Z", endPointsZ_EE_Bottom}, {"dY", endPoints_EE_Bottom_dY}};
      valsUpstream = {{"X", endPointsX_EE_Upstream}, {"Y", endPointsY_EE_Upstream}, {"Z", endPointsZ_EE_Upstream}, {"dZ", endPoints_EE_Upstream_dZ}};
      valsDownstream = {{"X", endPointsX_EE_Downstream}, {"Y", endPointsY_EE_Downstream}, {"Z", endPointsZ_EE_Downstream}, {"dZ", endPoints_EE_Downstream_dZ}};
    }
  else if(!strcmp(tpc,"EW"))
    {
      valsTop = {{"X", endPointsX_EW_Top}, {"Y", endPointsY_EW_Top}, {"Z", endPointsZ_EW_Top}, {"dY", endPoints_EW_Top_dY}};
      valsBottom = {{"X", endPointsX_EW_Bottom}, {"Y", endPointsY_EW_Bottom}, {"Z", endPointsZ_EW_Bottom}, {"dY", endPoints_EW_Bottom_dY}};
      valsUpstream = {{"X", endPointsX_EW_Upstream}, {"Y", endPointsY_EW_Upstream}, {"Z", endPointsZ_EW_Upstream}, {"dZ", endPoints_EW_Upstream_dZ}};
      valsDownstream = {{"X", endPointsX_EW_Downstream}, {"Y", endPointsY_EW_Downstream}, {"Z", endPointsZ_EW_Downstream}, {"dZ", endPoints_EW_Downstream_dZ}};
    }
  else if(!strcmp(tpc,"WE"))
    {
      valsTop = {{"X", endPointsX_WE_Top}, {"Y", endPointsY_WE_Top}, {"Z", endPointsZ_WE_Top}, {"dY", endPoints_WE_Top_dY}};
      valsBottom = {{"X", endPointsX_WE_Bottom}, {"Y", endPointsY_WE_Bottom}, {"Z", endPointsZ_WE_Bottom}, {"dY", endPoints_WE_Bottom_dY}};
      valsUpstream = {{"X", endPointsX_WE_Upstream}, {"Y", endPointsY_WE_Upstream}, {"Z", endPointsZ_WE_Upstream}, {"dZ", endPoints_WE_Upstream_dZ}};
      valsDownstream = {{"X", endPointsX_WE_Downstream}, {"Y", endPointsY_WE_Downstream}, {"Z", endPointsZ_WE_Downstream}, {"dZ", endPoints_WE_Downstream_dZ}};
    }
  else if(!strcmp(tpc,"WW"))
    {
      valsTop = {{"X", endPointsX_WW_Top}, {"Y", endPointsY_WW_Top}, {"Z", endPointsZ_WW_Top}, {"dY", endPoints_WW_Top_dY}};
      valsBottom = {{"X", endPointsX_WW_Bottom}, {"Y", endPointsY_WW_Bottom}, {"Z", endPointsZ_WW_Bottom}, {"dY", endPoints_WW_Bottom_dY}};
      valsUpstream = {{"X", endPointsX_WW_Upstream}, {"Y", endPointsY_WW_Upstream}, {"Z", endPointsZ_WW_Upstream}, {"dZ", endPoints_WW_Upstream_dZ}};
      valsDownstream = {{"X", endPointsX_WW_Downstream}, {"Y", endPointsY_WW_Downstream}, {"Z", endPointsZ_WW_Downstream}, {"dZ", endPoints_WW_Downstream_dZ}};
    }
  
  string outputFileText_top = string("offsetsDyDzTop_") + tpc + string(".csv");
  string outputFileText_bottom = string("offsetsDyDzBottom_") + tpc + string(".csv");
  string outputFileText_upstream = string("offsetsDyDzUpstream_") + tpc + string(".csv");
  string outputFileText_downstream = string("offsetsDyDzDownstream_") + tpc + string(".csv");
  write_csv(outputFileText_top, valsTop);
  write_csv(outputFileText_bottom, valsBottom);
  write_csv(outputFileText_upstream, valsUpstream);
  write_csv(outputFileText_downstream, valsDownstream);
  
  return 0;
}

