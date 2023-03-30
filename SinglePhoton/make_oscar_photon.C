class TMyRandom {

    public:
        TMyRandom(int seed=987654321){
            fourVec = new TLorentzVector();
            unif    = new TRandom3(seed);
            fgaus = new TF1("fgaus","gaus",-30e13, 30e13);
            pi = 3.14159;
        }

        double Rndm(){return unif->Rndm();}

        double Gaus(double mean, double sigma){
             fgaus->SetParameters(1, mean, sigma);
             return fgaus->GetRandom();
        }

        double GetPowLaw(double n, double lpt, double upt){
            return pow(pow(lpt,1-n)+unif->Rndm()*(pow(upt,1-n) - pow(lpt,1-n)), 1/(1-n) );
        }

        TLorentzVector *GetFMomGaussYPowPT(double ySig, double rapwin, double n, double lpt, double upt, double m0){
            do y  = unif->Gaus(0, ySig); while(fabs(y)>rapwin);
            n>0 ? pt = GetPowLaw(n, lpt, upt): pt=lpt+(upt-lpt)*Rndm();
            phi= unif->Rndm()*2*pi-pi;
            mt = sqrt(m0*m0+pt*pt);
            pl = mt*sinh(y);
            fourVec->SetPxPyPzE(pt*cos(phi), pt*sin(phi), pl, sqrt(mt*mt+pl*pl) );
            return fourVec;
        }
        
    private:
        TLorentzVector *fourVec;
        TRandom3 *unif;
        TF1* fgaus;
        TF1 *f1;;
        double phi,y,pt,pl,mt;
        double phiPh,yPh,ptPh,plPh;
        double pi;
        double gphi,z,gtheta,px,py,pz;

};

void make_oscar_photon(TString fout = "oscar_single_photon.input.00001",TString VertexSample = "/gpfs/mnt/gpfs02/phenix/plhf/plhf1/nivram/Simulation/common/VertexFromData/trimData/VertexSample454774-07.txt" ) {

// the only key part above is the
// OSC1999A which specifies the format
// the rest of the file is listed by
// events
// "0 2" (e.g. for two particles)
// then a list for each particle
// idpart id ist px,py,pz,E,
// mass, x,y,z,t
// and then "0 0"

  ifstream fp;
  fp.open(VertexSample);
  double v;
  char s[100];
  double v2;
  double vtx[20000];
  for (int a=0; a<20000; a++)
    vtx[a]=0;
  int count=0;
  while(fp>>s>>v>>v2)
    {
      vtx[count]=v;
      // cout<<"\n"<<vtx[count];
      count++;
    }



// ------- INPUT CARD --------
double Tpi = 3.14159; 
//const int MAXEVT = 200;
const int MAXEVT = 50e3;
const int NPARTICLES = 1;
Double_t PTMAX = 30.0;
double scale = 1e13; //cm to fm conversion
// ---------------------------

TH1D *hTest = new TH1D("hTest","",100, -50e13, 50e13);
double rapwidth=10.,rapwin=.5;
double n=-7;
Double_t IDpi0[3]     = { 111, 0, 0.1349766};
Double_t IDpiplus[3]  = { 211, 0, 0.13957018};
Double_t IDpiminus[3] = {-211, 0, 0.13957018};
Double_t IDkaonplus[3]  = { 321, 0, 0.493677};
Double_t IDkaonminus[3] = {-321, 0, 0.493677};
Double_t IDprotonplus[3]  = { 2212, 0, 0.938272};
Double_t IDprotonminus[3] = {-2212, 0, 0.938272};
Double_t IDgamma[3] = {22,0,0};

myrand = new TMyRandom(0);

TH1D* hVpt = new TH1D("hVpt", "Pt distribution", 100, 0, 30);
TH1D* hVtx = new TH1D("hVtx", "vtx distribution", 100, -30, 30);
TH1D* hRap = new TH1D("hRap", "rapidity distribution", 100, -0.7, 0.7);

// Write out the file
ofstream fileout( fout );
fileout << "# OSC1999A" << endl;
fileout << "# final_id_p_x" << endl;
fileout << "# SimName 1.0" << endl;
fileout << "#" << endl;
fileout << "# Some comments..." << endl;

TF1* fGaus = new TF1("fGaus","gaus",-10,10);
fGaus->SetParameters(0.99838, 0.504974, 11.4193);

for(int nevt=0; nevt<MAXEVT; nevt++) {
  for(int np=0; np<NPARTICLES; np++) {
    //double vertex = scale*fGaus->GetRandom();
	  //cout << vertex/scale << endl;
    vertex = scale * vtx[nevt];
  
    hVtx->Fill(vertex/scale);
      //-------------------  gamma --------------------
      TLorentzVector *PiZero = new TLorentzVector();
      PiZero = myrand->GetFMomGaussYPowPT(rapwidth, rapwin, n, 0., PTMAX, IDgamma[2]);
	  double PT = PiZero->Pt();
	  hVpt->Fill(PT);
	  hRap->Fill(PiZero->Rapidity());
      //  Number of particles in event
      fileout << "0 " << NPARTICLES <<endl;
      // Particle index
      Int_t index = 0;
      // note that these positions are in femtometers !
      Double_t xpos=0.0; Double_t ypos=0.0; Double_t zpos=vertex; Double_t time=0.0;
      // idpart id ist px,py,pz,E, mass, x,y,z,t

      //-------------------  gamma --------------------
      fileout << index << " " << IDgamma[0] << " " << IDgamma[1] << " " << PiZero->Px() << " " << PiZero->Py() << " " << PiZero->Pz() << " " << PiZero->E() << " " <<
	  IDgamma[2] << " " << xpos << " " << ypos << " " << zpos << " " << time << endl;
	
  }//End of particle
  fileout << "0 0" << endl; // not sure what this is for....
}//End of events

fileout.close();

//hVtx->Draw();

}
