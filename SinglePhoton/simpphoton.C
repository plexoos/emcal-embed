/*
 *
 * module that shows how to process a simulated dst
 *
 */

#ifndef __CINT__

#include <iostream>

#include <TSystem.h>

#include <PHCompositeNode.h>
#include <Fun4AllServer.h>
#include <Fun4AllDstInputManager.h>
//#include <Fun4AllDstOutputManager.h>

#include <EmcGeaContainerImporter.h>
#include <EmcEvaluatorModule.h>

using namespace std;

#endif



// simply a wrapper around the gSystem->Load() system call
void myload(const char * name){
    cout << "loading " << name << "... " << flush;
    gSystem->Load(name);
    cout << "done." << endl;
}


void simpphoton(const char * indstname = "outdst.root", const char* outfilename = "result_test.root", int Runno= 432684, int nevents = 1000, int nskipevents = 0, int trig = 4) {
//void simpphoton(const char * indstname = "outdst.root", const char* outfilename = "result.root", int nevents = -1, int nskipevents = 0){
//  myload("libemc-evaluation.so");
   gSystem->Load("librecal.so");
   gSystem->Load("libcompactCNT.so");
   gSystem->Load("libdch.so");
   gSystem->Load("libpad.so");
   myload("libcteval.so");
   myload("libsimpphoton.so");
   myload("libfun4all.so");
   myload("libfun4allfuncs.so");


  Fun4AllServer * se = Fun4AllServer::instance();

  //first register the master recal
  MasterRecalibratorManager *mr = new MasterRecalibratorManager("MASTERRECALIBRATORMANAGER");
  se->registerSubsystem(mr);


  recoConsts *rconst = recoConsts::instance();
  rconst->set_IntFlag("EMCNEW_DEBUG", 0);      // set debugging verbosity level
  rconst->set_IntFlag("EMCNEW_PI0VERBOUT", 2); // 2 - write only clean pi->2g cases
  rconst->set_IntFlag("RUNNUMBER",Runno);
  rconst->set_IntFlag("TRIGGER_BIT",trig);

  Fun4AllDstInputManager * input = new Fun4AllDstInputManager("input");
  se->registerInputManager( input );
  se->fileopen("input", indstname);

  se->registerSubsystem( new EmcGeaContainerImporter() );
  se->registerSubsystem( new EmcDeadRecalReco() );
  se->registerSubsystem( new simpphoton(outfilename) );
//  se->registerSubsystem( new EmcEvaluatorModule(true, true, true) );

  //  Fun4AllDstOutputManager * output  = new Fun4AllDstOutputManager("output", outdst);
  //  output->AddNode("VtxOut");
  //  output->AddNode("emcGeaTrackContainer");
  //  output->AddNode("emcTowerContainer");
  //  output->AddNode("emcClusterContainer");
  //  se->registerOutputManager(output);

  se->run(nevents);
  se->End();

  cout << "done." << endl;
}




// in case you wanted to compile it into an executeable, here 
// is a main that calls the real thing with the default
// parameters.
int main(){ simpphoton(); return 0; }



