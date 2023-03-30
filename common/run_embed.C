
// simply a wrapper around the gSystem->Load() system call
void myload(const char * name){
    cout << "loading " << name << "... " << flush;
    gSystem->Load(name);
    cout << "done." << endl;
}

void run_embed(const char * realfile = "CNT_MB_run16dAu_200GeV_CA_noVTX_pro107-0000455639-9007.root", const char* simfile = "dst/dst_out_1.root  ", const char * outfile = "outdst.root", int nevents = -1, int nskipevents = 0)

//void run_embed2(const char * realfile = "CNT_MB_run15pAu_200GeV_CA_pro104-0000432684-9000.root", const char* simfile = "./dst/dst_out_pi0-0-30GeV-00000.root", const char * outfile = "outdst.root", int nevents = 0, int nskipevents = 0)
{
	cout << "Embedding data=" << realfile << " & simu=" << simfile << endl;

	//const double myPbScScale= 0.985;
	//const double myPbGlScale= 1.028;

	//	const double myPbScScale= 0.985/0.988;//1.2% up
	//const double myPbGlScale= 1.028/0.988;//1.2% up


	const double myPbScScale= 0.985*0.988;//1.2% down
	const double myPbGlScale= 1.028*0.988;//1.2% down

	const double myPbScSmear= 0.05;
	const double myPbGlSmear= 0.12;

	/*
	const double myPbScScale= 0.9831/(0.9787*0.9983);
	const double myPbGlScale= 1.0362/(0.9463*0.9636);
	const double myPbScSmear= 0.0500/(1.3741*1.3803);
	const double myPbGlSmear= 0.1000/(1.5080*1.4204);


	*/

  myload("libfun4all.so");
  myload("libfun4allfuncs.so");
  myload("libFROG.so"); FROG fr;
  myload("libsimreco.so");  
  gSystem->Load("libCNT.so");
  gSystem->Load("librecal.so");
  gSystem->Load("libcompactCNT.so");
  myload("libtrigger.so");
  //myload("libemc.so");
  //gSystem->Load("libsimDSTCheck.so");
  // gSystem->Load("/direct/phenix+u/nivram/myinstall/lib/libEmbedAnalyzer.so");



  myload("libemcEmbed4all.so");
  //myload("libemc-evaluation.so");

  gSystem->ListLibraries();

  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(0);
  
  //1 recoConsts *rc = recoConsts::instance();
  //1rc->set_IntFlag("EMCGEOFLAG", 1);


  // input file with real data
  const char* realname = "REAL";
  Fun4AllDstInputManager * input1 = new Fun4AllNoSyncDstInputManager("real", "DST", realname);
  se->registerInputManager( input1 );
  realfile = fr.location( realfile );
  if( se->fileopen("real", realfile) != 0 ){ cerr << "failed to open '" << realfile << "'" << endl; exit(-1); }

  // input file with simulated data
  const char* simname = "SIM";
  Fun4AllNoSyncDstInputManager * input2 = new Fun4AllNoSyncDstInputManager("sim", "DST", simname);
  input2->NoRunTTree();
  se->registerInputManager( input2 );
  simfile = fr.location( simfile );
//  cout << "I am okay" << endl;
  if( se->fileopen("sim", simfile) != 0 ){ cerr << "failed to open '" << simfile << "'" << endl; exit(-1); }


  

  // for importing emcTowerContainerDST
  SubsysRecoStack * chroot = new SubsysRecoStack("emcTowerContainerDSTImp", Fun4AllServer::instance()->topNode(realname));
  chroot->x_push_back( new EmcTowerContainerResurrector() );
  //chroot->x_push_back( new MasterRecalibrator );// quito para chequear si este es el problema 3-26-19 pero NECESARIO!!  
  
  se->registerSubsystem( chroot );


 

#if 1
  //
  // if you want to fine-tune the emcal embedding process, use this 
  // expanded version.  if you are happy with the defaults (or you can
  // achive what you want using the EmcEmbedDriver2::set*() methods)
  // then you can safely delete this part and use the simple form
  // in the #else branch.
  //

  PHCompositeNode * real = Fun4AllServer::instance()->topNode( realname );
  PHCompositeNode * sim = Fun4AllServer::instance()->topNode( simname ); 
  
  //Selecting vertex of sim to be within 1 cm of vertex from real
  //SubsysReco* zsel = new EmcEmbedEventSelector(simfile,20,1.0,realname,simname,50,1.0);
  //se->registerSubsystem( zsel );
  

  //cout << "cent=============="  << getcent(real) << endl;
//  cout << "cent=============="  << real->getName() << endl;

  // copy nonemc data: our modules will need some of them as input.
  // also so that they will appear in the output dst.
  se->registerSubsystem( new CopyNonEMCNodes(sim, "CopyNonEMCNodesSimPre") ); // copy from simulation..
  se->registerSubsystem( new CopyNonEMCNodes(real, "CopyNonEMCNodesRealPre") ); // ..and overwrite with real
  //se->registerSubsystem( new EmbedAnalyzer() );

  
  // import real data
  SubsysRecoStack * realimp = new EmcRealContainerImporter( real );
  realimp->x_push_back( new EmcUnclusterizer() );
  realimp->x_push_back( new EmcApplyQA( EmcApplyQA::TOWER ) ); // importer does a poor job
  se->registerSubsystem( realimp );

  // import simulated data
  SubsysRecoStack * simimp = new EmcGeaContainerImporter( sim );
  simimp->x_push_back( new EmcUnclusterizer() );
  //simimp->x_push_back( new EmcApplyQA( EmcApplyQA::TOWER ) );
  EmcTowerScalerSmearer * emcsm = new EmcTowerScalerSmearer(1., 0.02);
  emcsm->SetScale(myPbScScale,myPbGlScale);
  emcsm->SetSmear(myPbScSmear,myPbGlSmear);
 
  //this is the vertex selector - it is disabled for now 
  EmcEmbedVertexSelector* zsel = new EmcEmbedVertexSelector(-0.5,0.5,"REAL","SIM");
  simimp->x_push_back( zsel );

  simimp->x_push_back( emcsm );
  se->registerSubsystem( simimp );

  // merge data
  EmcDataMerger * merger = new EmcDataMerger();
  merger->AddSourceNode( realname );
  merger->AddSourceNode( simname );
  se->registerSubsystem( merger );
  se->registerSubsystem( new EmcApplyQA( EmcApplyQA::TOWER ) );
    
  // clusterize data
  se->registerSubsystem( new EmcEmbedReclusterizer("TOP", "TOP", "TOP", "") );

#else
  se->registerSubsystem( new EmcEmbedDriver2(realname, simname) );

#endif

  //se->registerSubsystem( new EmcEvaluatorModule(false, false, false) );
 
  // output file
  Fun4AllDstOutputManager * output  = new Fun4AllDstOutputManager("output", outfile);
 
  output->RemoveNode("*");

  output->AddNode("EventHeader");
  output->AddNode("McSingle");
  output->AddNode("TrigLvl1");
  output->AddNode("Sync");
  output->AddNode("PHGlobal");
  output->AddNode("VtxOut");
  output->AddNode("PHCentralTrack");
  output->AddNode("emcGeaTrackContainer");
  output->AddNode("emcTowerContainer");
  output->AddNode("emcClusterContainer");

  se->registerOutputManager(output);




  // run analysis
  gBenchmark->Start("embed"); 
  cout << "test" << endl;
  se->run(nevents);
  gBenchmark->Show("embed");
  se->End();





  cout << "done." << endl;
}
