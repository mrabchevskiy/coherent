                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Test application for parallelization tools (Fluid, Staff and LogicalProcess)

 2023.05.04  Initial version


________________________________________________________________________________________________________________________________
                                                                                                                              */
#include <random>

#include "logger.global.h"
#include "logical.process.h"
#include "fluid.h"
#include "staff.h"
#include "timer.h"

namespace CoreAGI {
                                                                                                                              /*
  Demo data structure having default constructor:
                                                                                                                              */
  constexpr unsigned L{ 1024 };

  struct Data {
    double R[L][L];
  };
                                                                                                                              /*
  Make array of 5 Data instances converted into R/W shared items:
                                                                                                                              */
  constexpr unsigned CAPACITY{ 5 };

  Fluid< Data > data[5];
                                                                                                                              /*
  Logical process defined as a function:
                                                                                                                              */
  int LogicalProcessAsFunction( [[maybe_unused]] const Log& log ){

    static double   avg { 0.0 };
    static unsigned step{ 0   };

    static std::random_device RANDOM_DEVICE;
    static std::mt19937 RANDOM( RANDOM_DEVICE() );
    static std::uniform_int_distribution< int > uniform( 0, 100 );
                                                                                                                    /*
    Function returns randomly 0 or 1 with a average frequency 1000 : 1:
                                                                                                                    */
    auto next = [&]()->unsigned { return uniform( RANDOM ) ? 1 : 0; };

    switch( step ){

      case 0: // Data Writing:
        if(
          data[ rand() % CAPACITY ].alter( // // Lambda function:
            []( Data& D ){ for( unsigned i = 0; i < 500; i++ ) D.R[ rand()%L ][ rand()%L ] = rand(); }
          )
        ){ // OK, assign next step:
          step = next();
          return true;
        } else { // Fail, step not changed:
          return false;
        }

      case 1: // Data Reading:

        if(
          data[ rand() % CAPACITY ].check( // Lambda function:
            [&]( const Data& D ){
              constexpr unsigned M{ 50 };
              avg = 0.0;
              for( unsigned i = 0; i < M; i++ ) avg += D.R[ rand()%L ][ rand()%L ];
              avg /= double( M );
            }
          )
        ){ // OK, assign next step:
          step = next();
          return true;
        } else {  // Fail, step not changed:
          return false;
        }

      default: assert( false );

    }// switch step

  };//LogicalProcessAsFunc

                                                                                                                              /*
  Logical process defined as a class having `operator()`:
                                                                                                                              */
  class LogicalProcessAsStructure {
                                                                                                                              /*
    Local data:
                                                                                                                              */
    unsigned state; // :0 or 1
    double   avg;

  public:
                                                                                                                              /*
    Statistical data:
                                                                                                                              */
    unsigned Nr; // :data Read  access counter
    unsigned Nw; // :data Write access counter

    LogicalProcessAsStructure(): state{ 0 }, avg{ 0.0 }, Nr{ 0 }, Nw{ 0 }{} // :constructor

    int operator()( [[maybe_unused]] const Log& log ){

      static std::random_device RANDOM_DEVICE;
      static std::mt19937 RANDOM( RANDOM_DEVICE() );
      static std::uniform_int_distribution< int > uniform( 0, 100 );

      auto next = [&]()->unsigned { return uniform( RANDOM ) ? 1 : 0; };
                                                                                                                              /*
      Execute current state:
                                                                                                                              */
      switch( state ){

        case 0:
          if(
                                                                                                                              /*
            Try to get `write` access to shared data and modify data:
                                                                                                                              */
            data[ rand() % CAPACITY ].alter(
              []( Data& D ){ for( unsigned i = 0; i < 500; i++ ) D.R[ rand()%L ][ rand()%L ] = rand(); }
            )
          )[[ unlikely ]]{ // OK, do something more and assign next state:
            Nw++;
                                                                                                                              /*
            ...
                                                                                                                              */
            state = next();
            return true;
          } else { // Can`t get `write` access to shared data, state sill uncheged, no more activity:
            return false;
          }//if

        case 1:
          if(
                                                                                                                              /*
            Try to get `read` access to shared data and calculate `avg` using shared data:
                                                                                                                              */
            data[ rand() % CAPACITY ].check(
              [&]( const Data& D ){
                constexpr unsigned M{ 50 };
                avg = 0.0;
                for( unsigned i = 0; i < M; i++ ) avg += D.R[ rand()%L ][ rand()%L ];
                avg /= double( M );
              }
            )
          )[[ unlikely ]]{ // OK, do something more and assign next state:
            Nr++;
                                                                                                                              /*
             ...
                                                                                                                              */
            state = next();
            return true;
          } else { // Can`t get `read` access to shared data, state sill uncheged, no more activity:
            return false;
          }//if

      default: assert( false );

      }//switch state

    }//operator()

  };//LogicalProcessAsStructure

}//namespace CoreAGI


int main(){

  using namespace CoreAGI;
                                                                                                                              /*
  Open log:
                                                                                                                              */
  auto log = logger.log( "main" );
  log( "Started" ); log.flush();
                                                                                                                              /*
  Instaniation of the 9 `raw` logical processes defined as class:
                                                                                                                              */
  LogicalProcessAsStructure S[9];
                                                                                                                              /*
  Creation of the 10 logical processes accessible from working threads:
                                                                                                                              */
  LogicalProcess Pf( "F", LogicalProcessAsFunction                            );
  LogicalProcess Pi( "I", [&]( const Log& log )->bool{ return S[0]( log ); }  );
  LogicalProcess Pj( "J", [&]( const Log& log )->bool{ return S[1]( log ); }  );
  LogicalProcess Pk( "K", [&]( const Log& log )->bool{ return S[2]( log ); }  );
  LogicalProcess Pu( "U", [&]( const Log& log )->bool{ return S[3]( log ); }  );
  LogicalProcess Pv( "V", [&]( const Log& log )->bool{ return S[4]( log ); }  );
  LogicalProcess Pw( "W", [&]( const Log& log )->bool{ return S[5]( log ); }  );
  LogicalProcess Px( "X", [&]( const Log& log )->bool{ return S[6]( log ); }  );
  LogicalProcess Py( "Y", [&]( const Log& log )->bool{ return S[7]( log ); }  );
  LogicalProcess Pz( "Z", [&]( const Log& log )->bool{ return S[8]( log ); }  );
                                                                                                                              /*
  Make array of pointers to logical processes ended by null pointer:
                                                                                                                              */
  const LogicalProcess* P[]{ &Pf, &Pi, &Pj, &Pk, &Pu, &Pv, &Pw, &Px, &Py, &Pz, nullptr };

  unsigned n{ 0 };
  while( P[n] ) n++;
  log.vital( kit( "%i logical processes", n ) );
                                                                                                                              /*
  Creation of the `staff` ~ set of 2 working threads:
                                                                                                                              */
  constexpr unsigned STAFF{ 2 };
  Staff< STAFF > staff( P );
  staff.start();
                                                                                                                              /*
  Allow processed to be executed:
                                                                                                                              */
  for( unsigned i = 0; i < n; i++ ){
    P[i]->start();
    log.vital( kit( "Logical process %s started", P[i]->name() ) );
  }
                                                                                                                              /*
  Run processes for 250 millisecons:
                                                                                                                              */
  CoreAGI::pause( 250 )[ MILLISEC ];
                                                                                                                              /*
  Stop processes and print statistics:
                                                                                                                              */
  for( unsigned i = 0; i < n; i++ ){
    P[i]->stop();
    P[i]->info( log );
  }

  log.vital( "R/W statistics:" );
  for( auto& Si: S ) log.vital( kit( "%5u R  %5u W", Si.Nr, Si.Nw ) );
                                                                                                                              /*
  Stop working threads and finish:
                                                                                                                              */
  staff.stop();

  CoreAGI::pause( 100 )[ MILLISEC ];

  return 0;
}
