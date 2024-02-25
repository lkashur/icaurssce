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

void AddFiles(TChain *ch, const char *fileList);
float Median(vector<float> v, int n);

int main(int argc, char** argv)
{
  // Constants
  int NXbins = 15;
  int NYbins = 4;
  int NZbins = 2;
  float bin_low_x;
  float bin_center_x;
  float bin_high_x;
  float drift_x;
  
  // Input
  Char_t *inputfilename = (Char_t*)"";
  inputfilename = argv[1];
  TFile* inputfile = new TFile(inputfilename,"READ");
  
  Char_t *inputyzbin = (Char_t*)"";
  inputyzbin = argv[2];
  
  Char_t *tpc = (Char_t*)"";
  tpc = argv[3];
  
  // Uncomment in the case of running with NO {y,z} binning
  //TChain* inputfiles = new TChain("offsettree");
  //AddFiles(inputfiles, inputfilename);
  
  // Output
  ofstream outputfile;
  string outputfile_text = string("offsets_") + tpc + string("_yz") + inputyzbin + string(".txt");

  TTreeReader readerHits(inputyzbin, inputfile);
  TTreeReaderValue<float> hit_x(readerHits, "hit_x");
  TTreeReaderValue<float> hit_y(readerHits, "hit_y");
  TTreeReaderValue<float> hit_z(readerHits, "hit_z");
  TTreeReaderValue<float> offset(readerHits, "offset");
  
  // Define histograms
  TH1F *DummyHist1D = new TH1F("DummyHist1D","",NXbins,0.0,150.0);
  //TH2F *DummyHist2D = new TH2F("DummyHist2D","",NZbins,-894.9515,894.9515,NYbins,-181.86,134.96);

  vector<vector<float>> offset_vec(NXbins);
  
  // Loop through hits
  while(readerHits.Next())
  {

    // Keep non-zero offsets
    if(*offset == 0.0) continue;

    // Get relative drift coordinate
    drift_x = *hit_x - 61.94;
	      
    // Bin in {x}
    for(int k = 1; k <= DummyHist1D->GetNbinsX(); k++)
      {
	bin_low_x = DummyHist1D->GetXaxis()->GetBinLowEdge(k);
	bin_high_x = DummyHist1D->GetXaxis()->GetBinUpEdge(k);

	if(drift_x >= bin_low_x && drift_x < bin_high_x)
	  {
	    offset_vec[k-1].push_back(*offset);
	  }
      }
  } // hit loop
	  

// Median
outputfile.open(outputfile_text);
outputfile << "bin_center,bin_low,bin_high,offset" << endl;
for(int k = 1; k <= DummyHist1D->GetNbinsX(); k++)
  {
    bin_low_x = DummyHist1D->GetXaxis()->GetBinLowEdge(k);
    bin_center_x = DummyHist1D->GetXaxis()->GetBinCenter(k);
    bin_high_x = DummyHist1D->GetXaxis()->GetBinUpEdge(k);

    float med_offset = Median(offset_vec[k-1], offset_vec[k-1].size());
    outputfile << bin_center_x << "," << bin_low_x << "," << bin_high_x << "," << med_offset << endl;
  }

 outputfile.close();
 inputfile->Close();
 return 0;

} // end main

// Add intput files to TChain
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

// Function for calculating median 
float Median(vector<float> v, int n)
{
  // Sort the vector
  sort(v.begin(), v.end());

  // Check if the number of elements is odd
  if (n % 2 != 0)
    return (float)v[n / 2];

  // If the number of elements is even, return the average
  // of the two middle elements
  return (float)(v[(n - 1) / 2] + v[n / 2]) / 2.0;
}
