void split_realdata(const char *realfile = "realdst.root", const int nevent = 0, const int nskipevent = 0, const int nmax = 5500)
{
    // gSystem->Setenv("ODBCINI","/opt/phenix/core/etc/odbc.ini.test");
    gSystem->Load("liblvl2.so");
    gSystem->Load("libcompactCNT.so");
    gSystem->Load("libsimreco");
    gSystem->Load("libfun4allfuncs.so");
    gSystem->Load("librecal.so");
    gSystem->Load("libTOAD");
    gSystem->Load("libRealEventSelector.so");

    gSystem->ListLibraries();

    Fun4AllServer *se = Fun4AllServer::instance();
    se->Verbosity(0);
    
    //first register the master recal
    MasterRecalibratorManager *mr = new MasterRecalibratorManager("MASTERRECALIBRATORMANAGER");
    se->registerSubsystem(mr);
    const char* module_name[5] = {"RealEventSelector020","RealEventSelector2040","RealEventSelector4060","RealEventSelector6093", "RealEventSelector0088"};
    for (int icent = 4; icent < 5; icent++)
    {
      se->registerSubsystem( new RealEventSelector(module_name[icent],icent,nmax) );
    }
 
    //  Input Managers...
    Fun4AllDstInputManager *in1 = new Fun4AllDstInputManager("DSTin1","DST");
    in1->Verbosity(0);
    
    se->registerInputManager(in1);
    se->fileopen("DSTin1",realfile);

    // Output Managers...
    Fun4AllDstOutputManager* output[4] = {0};
    for (int icent = 4; icent < 5; icent++)
    {
      output[icent] = new Fun4AllDstOutputManager(Form("output%d",icent), Form("realdst%d.root",icent));
      output[icent]->AddEventSelector(module_name[icent]);
      output[icent]->AddNode("RunHeader");
      output[icent]->AddNode("EventHeader");
      output[icent]->AddNode("TrigLvl1");
      output[icent]->AddNode("PreviousEvent");
      output[icent]->AddNode("ErtOut");
      output[icent]->AddNode("TFvtxCompactTrk");
      output[icent]->AddNode("Sync");
      output[icent]->AddNode("VtxOut");
      output[icent]->AddNode("emcHitContainer");
      output[icent]->AddNode("PHGlobal");
      output[icent]->AddNode("PHGlobal_CENTRAL");
      output[icent]->AddNode("TrackProjection_VarArray");
      output[icent]->AddNode("TrackLineProjection_VarArray");
      output[icent]->AddNode("TrackPathLength_VarArray");
      output[icent]->AddNode("CglTrackHits_VarArray");
      output[icent]->AddNode("CglTrackBackHits_VarArray");
      output[icent]->AddNode("DchHit_VarArray");
      output[icent]->AddNode("Pc1Hit_VarArray");
      output[icent]->AddNode("Pc2Hit_VarArray");
      output[icent]->AddNode("Pc3Hit_VarArray");
      output[icent]->AddNode("TofeHit_VarArray");
      output[icent]->AddNode("TofwHit_VarArray");
      output[icent]->AddNode("CrkHit_VarArray");
      output[icent]->AddNode("AccHit_VarArray");
      output[icent]->AddNode("EmcHit_VarArray");
      output[icent]->AddNode("CntRpSumXYObject");
      se->registerOutputManager(output[icent]);
    }

    //  OK...Do the analysis!!!!
    se->skip(nskipevent);
    se->run(nevent);
    se->End();
    delete se;
}
