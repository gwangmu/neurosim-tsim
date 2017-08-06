// $Id$

/*
 Copyright (c) 2007-2015, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*main.cpp
 *
 *The starting point of the network simulator
 *-Include all network header files
 *-initilize the network
 *-initialize the traffic manager and set it to run
 *
 *
 */
#include <sys/time.h>

#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>



#include <sstream>
#include "booksim.hpp"
#include "routefunc.hpp"
#include "traffic.hpp"
#include "booksim_config.hpp"
#include "trafficmanager.hpp"
#include "random_utils.hpp"
#include "network.hpp"
#include "injection.hpp"
#include "power_module.hpp"


// gwangmu
#include <functional>

///////////////////////////////////////////////////////////////////////////////
//Global declarations
//////////////////////

 /* the current traffic manager instance */
TrafficManager * trafficManager = NULL;

int GetSimTime() {
  return trafficManager->getTime();
}

class Stats;
Stats * GetStats(const std::string & name) {
  Stats* test =  trafficManager->getStats(name);
  if(test == 0){
    cout<<"warning statistics "<<name<<" not found"<<endl;
  }
  return test;
}

/* printing activity factor*/
bool gPrintActivity;

int gK;//radix
int gN;//dimension
int gC;//concentration

int gNodes;

//generate nocviewer trace
bool gTrace;

ostream * gWatchOut;

/* gwangmu: extracted from '_Prepare' */
vector<Network *> net;
int subnets;


/////////////////////////////////////////////////////////////////////////////

//bool Simulate( BookSimConfig const & config )
void _Prepare( BookSimConfig const & config )
{
  subnets = config.GetInt("subnets");
  /*To include a new network, must register the network here
   *add an else if statement with the name of the network
   */
  net.resize(subnets);
  for (int i = 0; i < subnets; ++i) {
    ostringstream name;
    name << "network_" << i;
    net[i] = Network::New( config, name.str() );
  }

  /*tcc and characterize are legacy
   *not sure how to use them 
   */

  assert(trafficManager == NULL);
  trafficManager = TrafficManager::New( config, net ) ;

  /*Start the simulation run
   */

  trafficManager->Prepare ();
}

void _Finalize () {

  //cout<<"Total run time "<<total_time<<endl;

  for (int i=0; i<subnets; ++i) {

    ///Power analysis
    /* gwangmu
    if(config.GetInt("sim_power") > 0){
      Power_Module pnet(net[i], config);
      pnet.run();
    }
    */

    delete net[i];
  }

  delete trafficManager;
  trafficManager = NULL;
}


////////////////////////////////////////////////////////////////////////////

void BookSim_Advance () {
  trafficManager->Advance ();
}

int BookSim_GeneratePacket (int source, int dest, int size) {
  return trafficManager->GeneratePacket (source, dest, size);
}

void BookSim_RegisterPacketArrivalCallback (function<void (int, int, int, int)> callback) {
  trafficManager->RegisterPacketArrivalCallback (callback);
}

void BookSim_Finalize () {
  _Finalize ();
}


void BookSim_Initialize (const char *configfile) {
  BookSimConfig config;

  if ( !ParseArgs( &config, (char *)configfile ) ) {
    cerr << "error: invalid config file" << endl;
    return;
  } 
  
  /*initialize routing, traffic, injection functions
   */
  InitializeRoutingMap( config );

  gPrintActivity = (config.GetInt("print_activity") > 0);
  gTrace = (config.GetInt("viewer_trace") > 0);
  
  string watch_out_file = config.GetStr( "watch_out" );


  if(watch_out_file == "") {
    gWatchOut = NULL;
  } else if(watch_out_file == "-") {
    gWatchOut = &cout;
  } else {
    gWatchOut = new ofstream(watch_out_file.c_str());
  }
  

  /* gwangmu: Configure simulator */
  _Prepare (config);

  return;
}
 

////////////////////////////////////////////////////////////////////////////////////

#ifdef BOOKSIM_STANDALONE
void _Test_Callback (int id, int lat, int source, int dest) {
  cout << "[gwangmu] Packet " << id << " arrival (latency: " << lat << 
          ", source: " << source << ", dest: " << dest << ")" << endl;
}
 
int main (int argc, char **argv) {
  /* gwangmu: this is supposed to be in the user's code */
  BookSim_Initialize (argv[1]);
  BookSim_RegisterPacketArrivalCallback (_Test_Callback);

  while (true) {
    BookSim_GeneratePacket (0, 2, 2);
    BookSim_Advance ();

    char in;
    cin >> in;
    if (in == 'x') break;
  }
  BookSim_Finalize ();
}
#endif
