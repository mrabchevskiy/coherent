                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Template class that incapsulates set of working threads
 running in non-stop mode to execute simultaneously set
 of logical processes

 2023.05.04  Initial version

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef STAFF_H_INCLUDED
#define STAFF_H_INCLUDED

#include <concepts>
#include <thread>
#include <string>
#include <functional>

#include "logical.process.h"
#include "logger.h"

namespace CoreAGI{

  template< unsigned STAFF > class Staff {

    struct Member {

      std::string                name;
      const LogicalProcess**     P;
      std::thread                thread;
      std::atomic< bool >        terminate;
      std::atomic< bool >        terminated;
      LogicalProcess::Statistics stat;

      void run(){
                                                                                                                              /*
        Calculate number of logical processes:
                                                                                                                              */
        unsigned N{ 0 };
        while( P[N] ) N++;

        std::random_device RANDOM_DEVICE;
        std::mt19937 RANDOM( RANDOM_DEVICE() );
        std::uniform_int_distribution< int > uniform( 0, N-1 );
                                                                                                                              /*
        Open log:
                                                                                                                              */
        auto log = logger.log( name ); // :create log
        log.vital( kit( "Staff::Member started, %i branches", N ) );
                                                                                                                              /*
        Mark himself as running:
                                                                                                                              */
        terminated.store( false );
                                                                                                                              /*
        Main loop:
                                                                                                                              */
        while( not terminate.load() ) stat += P[ uniform( RANDOM ) ]->process( log );
                                                                                                                              /*
        Print statistics:
                                                                                                                              */
        stat.expose( log, ( std::string( "Thread `") + name + "` statistics:" ).c_str() );
                                                                                                                              /*
        Mark himself as terminated:
                                                                                                                              */
        terminated.store( true );
      }

      Member(): name{}, P{}, thread{}, terminate{ false }, terminated{ true }, stat{}{}

      bool live () const { return not terminated.load();               }
      void start()       { thread = std::thread( &Member::run, this ); }
      void stop ()       { terminate.store( true );                    }

     ~Member(){
        terminate.store( true );
        std::this_thread::yield();
        if( thread.joinable() ) thread.join();
      }

    };//Member

    const LogicalProcess** P;

    Member member[ STAFF ];

  public:

    Staff( const LogicalProcess** PROCESS ): P{ PROCESS }, member{}{
      constexpr const char* SEQ{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ" }; static_assert( STAFF < strlen( SEQ ) );
      char name[2]{ ' ', '\0' };
      for( unsigned i = 0; i < STAFF; i++ ){
        name  [0]      = SEQ[i];
        member[i].name = std::string( name );
        member[i].P    = P;
      }
    }//constructor

    void start(){
      for( auto& m: member ) m.start();
    }

    void stop(){
      for( auto& m: member ) m.stop();
      for(;;){
        std::this_thread::yield();
        unsigned live{ 0 };
        for( auto& m: member ) if( m.live() ) live++;
        if( live == 0 ) break;
      }
      for( auto& m: member ) if( m.thread.joinable() ) m.thread.join();
    }

   ~Staff(){ stop(); }

  };// Staff

}//namespace CoreAGI

#endif // STAFF_H_INCLUDED
