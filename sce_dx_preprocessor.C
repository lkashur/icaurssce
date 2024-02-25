/*
 * ROOT macro to preprocess output of
 * sce_dx_analyzer.cpp before final analysis
 */
void sce_dx_preprocessor()
{
  // Output
  ofstream binlist;
  ofstream bindetails;
  binlist.open("binlist.txt");
  bindetails.open("bindetails.txt");
  string outputfile_text = "snapshot_WE.root";

  // Load data
  TChain ch("offsettree");
  const char *filelist = "/exp/icarus/app/users/lkashur/SCE2024/step0/output/all_offsets_WE.txt";
  ifstream stream(filelist);
  string line;
  while(!stream.eof())
    {
      getline(stream, line);
      cout << line.c_str() << endl;
      ch.Add(line.c_str());
    }
  
  ROOT::EnableImplicitMT();
  ROOT::RDataFrame rdf(ch);
  ROOT::RDF::RSnapshotOptions opts;

  // Make pre-proccessing cuts
  auto rdf_cuts = rdf.Filter("offset != 0") // non-zero offsets
                     .Filter("anode_y < 134.96 - 20") // y,z endpoint cuts
                     .Filter("anode_y > -181.86 + 20")
                     .Filter("anode_z > -894.9515 + 200")
                     .Filter("anode_z < 894.9515 - 200")
                     .Filter("cathode_y < 134.96 - 60")
                     .Filter("cathode_y > -181.86 + 60")    
                     .Filter("cathode_z > -894.9515 + 200")
                     .Filter("cathode_z < 894.9515 - 200");
    
  //df_cuts.Display("")->Print();
  
  // Create dummy 2d histogram for binning
  int Nybins = 4;
  int Nzbins = 4;
  int Nxbins = 15;
  TH2F *DummyHist2D = new TH2F("DummyHist2D","",Nzbins,-894.9515,894.9515,Nybins,-181.86,134.96);
  TH1F *DummyHist1D = new TH1F("DummyHist1D","",Nxbins,60,212);
  
  bindetails << "bnum, bin_low_y, bin_high_y, bin_low_z, bin_high_z" << endl;
  for(int i = 1; i <= DummyHist2D->GetNbinsX(); i++)
    {      
      float bin_low_z = DummyHist2D->GetXaxis()->GetBinLowEdge(i);
      float bin_high_z = DummyHist2D->GetXaxis()->GetBinUpEdge(i);
      
      for(int j = 1; j <= DummyHist2D->GetNbinsY(); j++)
	{
	  float bin_low_y = DummyHist2D->GetYaxis()->GetBinLowEdge(j);
	  float bin_high_y = DummyHist2D->GetYaxis()->GetBinUpEdge(j);

	  // tree name for {y,z} bin
	  string treename_yz = to_string(i) + to_string(j);
	  
	  // Save binning info for later
	  binlist << i << j << endl;
	  bindetails << i << j << "," << bin_low_y << "," << bin_high_y << "," << bin_low_z << "," << bin_high_z << endl;

	  // Bin existing rdf in bins of {y,z}
	  auto rdf_cuts_yz = rdf_cuts.Filter([&](float hit_y){ return hit_y >= bin_low_y; }, {"hit_y"} )
	                             .Filter([&](float hit_y){ return hit_y < bin_high_y; }, {"hit_y"} )
	                             .Filter([&](float hit_z){ return hit_z >= bin_low_z; }, {"hit_z"} )
	                             .Filter([&](float hit_z){ return hit_z < bin_high_z; }, {"hit_z"} );

	  // Save every {y,z} rdf to a ttree (contained in one output file)
	  if(i == 1 && j == 1)
	    {
	      opts.fMode = "RECREATE";
	    }
	  else
	    {
	      opts.fMode = "UPDATE";
	    }
	  //auto rdf_cuts_yz_snap = rdf_cuts_yz.Snapshot(treename_yz, outputfile_text.c_str(), rdf_cuts_yz.GetColumnNames(), opts);
	  
	  //rdf_cuts_yz.Display("")->Print();
	  	  
	} //end {y} bin loop
    } // end{z} loop

  binlist.close();
  bindetails.close();

}
