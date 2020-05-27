// This is an example main pythia script to generate to generate a dark sector shower in a strongly coupled, quasi-conformal
// hidden valley, often referred to as "soft unclustered energy patterns (SUEP)" or "softbomb" events.
// The code for the dark shower itself is in suep_shower.cc

// The algorithm relies on arXiv:1305.5226. See arXiv:1612.00850 for a description of the model. 
// Please cite both papers when using this code.

// Written by Simon Knapen on 12/22/2019

// pythia headers
#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC2.h"

// libraries needed for the softbomb part
#include <iostream>
#include <math.h>
#include <boost/math/tools/roots.hpp>
#include <boost/bind.hpp>
#include<string>
#include<stdio.h>
#include<stdlib.h>

// suep_shower header
#include "suep_shower.h"

using namespace Pythia8;

template <typename T> string tostr(const T& t) { 
   ostringstream os; 
   os<<t; 
   return os.str(); 
} 

Event AttachSuepShower(Event event, double mX, Suep_shower suep_shower){
    Vec4 higgs4mom, mesonmom;
    vector< vector <double> > suep_shower4momenta;
  
    // Generate the shower, output are 4 vectors in the rest frame of the shower
    suep_shower4momenta=suep_shower.generate_shower();
  
    // Find the Higgs in the event
    for (int i = 0; i < event.size(); ++i){ 
        if (event[i].id()==25 && event[i].isFinal()) {
          higgs4mom=event[i].p();
          
          // Loop over hidden sector mesons and append to the event
          for (unsigned j = 0; j < suep_shower4momenta.size(); ++j){
            //construct pythia 4vector
            mesonmom.p(suep_shower4momenta[j][1],suep_shower4momenta[j][2],suep_shower4momenta[j][3],suep_shower4momenta[j][0]);
            
            // boost to the lab frame
            mesonmom.bst(higgs4mom.px()/higgs4mom.e(),higgs4mom.py()/higgs4mom.e(), higgs4mom.pz()/higgs4mom.e());
            
            //append particle to the event. Hidden meson pdg code is 999999.
            event.append(999999, 91, i,0,0,0, 0, 0,mesonmom.px(), mesonmom.py(), mesonmom.pz(), mesonmom.e(), mX); 
          }
          
          // Change the status code of the Higgs to reflect that it has decayed.
          event[i].statusNeg();
          
          //set daughters of the Higgs. (Not all mesons are recognized as daughters, hopefully doesn't result in issues.)
          event[i].daughters(event.size()-2, event.size()-1); 
          break;
        }
    }   
  return event;
}



int main(int argc, char *argv[]) {
     
   // read model parameters from the command line
  if(!(argc==7)){
    std::cout << "I need the following arguments: M m T decaycard outputfilename randomseed\n";
    std::cout << "with\n";
    std::cout << "M: mass of heavy scalar\n";
    std::cout << "m: mass of dark mesons\n";
    std::cout << "T: Temperature parameter\n";
    std::cout << "decaycard: filename of the decay card\n";
    std::cout << "outputfilename: filename where events will be written\n";
    std::cout << "randomseed: an integer, specifying the random seed\n";
    return 0;
  }
     

  // model parameters and settings
  float mh, mX,T;
  string seed, filename, cardfilename;    
  mh=atof(argv[1]);
  mX=atof(argv[2]);
  T=atof(argv[3]);
  cardfilename=tostr(argv[4]);
  filename=tostr(argv[5]);
  seed=tostr(argv[6]);    
  
  // number of events    
  int Nevents=10000;    
    
  // Interface for conversion from Pythia8::Event to HepMC event.
  HepMC::Pythia8ToHepMC ToHepMC;

  // Specify file where HepMC events will be stored.
  HepMC::IO_GenEvent ascii_io(filename, std::ios::out);

  // We will run pythia twice: Once for Higgs production, and once to decay the final state mesons of softbomb.
  // We therefore need two different pythia objects, each with different settings
  Pythia pythiaProd, pythiaDecay;
  
  //Settings for the production Pythia object, before softbomb shower
  pythiaProd.readString("Beams:eCM = 13000.");
  pythiaProd.readString("HiggsSM:all = on");
  pythiaProd.readString("25:mayDecay = off"); // turn off SM decays for Higgs
  pythiaProd.readString("25:m0 = "+tostr(mh)); // set the mass of "Higgs" scalar
  pythiaProd.readString("Random:setSeed = on");
  pythiaProd.readString("Random:seed = "+seed); 
  pythiaProd.readString("Next:numberShowEvent = 0");
  // For debugging only
  //pythiaProd.readString("PartonLevel:ISR = off");
  //pythiaProd.readString("PartonLevel:FSR = off");
  //pythiaProd.readString("HadronLevel:Hadronize= off");
  pythiaProd.init();
  
  
  //Settings for the decay Pythia object, to decay the dark mesons. Asigned PDG code 999999 for the dark mesons
  pythiaDecay.readString("Random:setSeed = on");
  pythiaDecay.readString("Random:seed = "+seed);  
  pythiaDecay.readString("999999:all = GeneralResonance void 0 0 0 "+tostr(mX)+" 0.001 0.0 0.0 0.0");
  pythiaDecay.readString("Next:numberShowEvent = 0");
  
  // turn off process-level, so this pythia object only does the decays and hadronization
  pythiaDecay.readString("ProcessLevel:all = off"); 
  
  // Momentum is not exactly conserved due to small numerical errors, turn off checks to prevent pythia aborting    
  pythiaDecay.readString("Check:event = off");
  
  // get the decay channels from the decay.cmnd card    
  pythiaDecay.readFile(cardfilename);
  //pythiaDecay.readFile("decay_cards/"+cardfilename); 
  pythiaDecay.init();
       
 
  // Shortcuts
  Event& event = pythiaProd.event;
  Event& decayedEvent = pythiaDecay.event;

  // Initialize softbomb
  Suep_shower suep_shower(mX,T,mh);
  
  // Begin event loop. Generate event. Skip if error.
  for (int iEvent = 0; iEvent < Nevents; ++iEvent) {
    if (!pythiaProd.next()) continue; //generate an event with the production pythia kernel, higgs is stable.
    
    // attach the soft bomb to the event.
    event=AttachSuepShower(event,mX,suep_shower);
    
    // fill an event from the decay kernel with the particles from the "production" event 
    decayedEvent.reset();
    for (int i = 1; i < event.size(); ++i) {//skip first "system" entry to avoid double counting it
      decayedEvent.append(event[i]);
    }
    
    // Run the decay kernel, to decay all the hidden mesons
    if (!pythiaDecay.next()) {
      cout << " Event generation aborted prematurely, owing to error!\n";
      break;
    }
    
    // Print out a few events
    if (iEvent<1){  
        decayedEvent.list();
    }
    
    // Construct new empty HepMC event and fill it.
    // Units will be as chosen for HepMC build; but can be changed
    // by arguments, e.g. GenEvt( HepMC::Units::GEV, HepMC::Units::MM)
    HepMC::GenEvent* hepmcevt = new HepMC::GenEvent(HepMC::Units::GEV, HepMC::Units::MM);
    ToHepMC.fill_next_event( pythiaDecay, hepmcevt );

    // Write the HepMC event to file. Done with it.
    ascii_io << hepmcevt;
    delete hepmcevt;

  // End of event loop.
  }
  // print the cross sections etc    
  pythiaProd.stat();

  // Done.
  return 0;
}
