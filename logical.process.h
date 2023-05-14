                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Wrapper class that convert function into the logical process
 that can be called by any thread from `Staff` class (see 'staff.h')

 2023.05.04  Initial version

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef LOGICALPROCESS_H_INCLUDED
#define LOGICALPROCESS_H_INCLUDED

#include <atomic>
#include <functional>

#include "logger.h"

namespace CoreAGI {

  using Log = Logger::Log;


  class LogicalProcess {
  public:

    struct Statistics {

      enum RESULT: unsigned { IDLE=0, BUSY=1, DONE=2, FAIL=3 };

      std::atomic< unsigned > N[4];

      void operator+= ( const RESULT& result ){ N[ unsigned( result ) ]++; }

      void expose( const Log& log, const char* header = "Statistics:" ){
        const constexpr char* LEX[4]{ "Idle", "Busy", "Done", "Deny" };
        unsigned M[4];
        for( unsigned i = 0; i < 4; i++ ) M[i] = N[i].load();
        log.vital( header );
        double fraction = 100.0*M[IDLE]/( M[IDLE] + M[BUSY] + M[DONE] + M[FAIL] );
        log.vital( kit( "  %s      %6.2f %%  %10u", LEX[IDLE], fraction, M[IDLE] ) );
        fraction = 100.0*M[BUSY]/( M[BUSY] + M[DONE] + M[FAIL] );
        log.vital( kit( "    %s    %6.2f %%  %10u", LEX[BUSY], fraction, M[BUSY] ) );
        fraction = 100.0*M[DONE]/( M[DONE] + M[FAIL] );
        log.vital( kit( "      %s  %6.2f %%  %10u", LEX[DONE], fraction, M[DONE] ) );
        fraction = 100.0*M[FAIL]/( M[DONE] + M[FAIL] );
        log.vital( kit( "      %s  %6.2f %%  %10u", LEX[FAIL], fraction, M[FAIL] ) );
      }

    };// Statistics

  protected:

    const   char*                       ID;     // :process name (useful for logging)
    std::function< bool( const Log& ) > F;      // :process function
    mutable std::atomic< bool >         vacant; // :busy/vacant flag
    mutable std::atomic< bool >         active; // :idle/active flag
    mutable Statistics                  stat;

  public:
                                                                                                                              /*
    Constructor accepts function as an argument:
                                                                                                                              */
    LogicalProcess( const char* name, std::function< bool( const Log& ) > f ): ID{ name }, F{ f }, stat{}{
      vacant.store( true  ); // :vacant at the start
      active.store( false ); // :idle   at the start
    }

    void start() const { active.store( true  ); }
    void stop () const { active.store( false ); }

    const char* name() const { return ID;            }
    bool        live() const { return active.load(); }

    auto info( const Log& log ) const {
      stat.expose( log, ( std::string( "Process `" ) + std::string( ID ) + "` statistics:" ).c_str() );
    }

    Statistics::RESULT process( [[maybe_unused]] const Log& log ) const {
                                                                                                                              /*
      Check if process active:
                                                                                                                              */
      if( not active.load() ){ stat += Statistics::IDLE; return Statistics::IDLE; }
                                                                                                                              /*
      Try to change state from `vacant` to `occupied` process:
                                                                                                                              */
      bool expected{ true };
      if( not vacant.compare_exchange_strong( /*mod*/ expected, false ) ){
        stat += Statistics::BUSY;
        return  Statistics::BUSY;
      }
                                                                                                                              /*
      Process occupied succesfully; run next step of the logical process:
                                                                                                                              */
      Statistics::RESULT result;
      if( F( log ) ) stat += Statistics::DONE, result = Statistics::DONE;
      else           stat += Statistics::FAIL, result = Statistics::FAIL;
                                                                                                                              /*
      Make process vacant (ready to execution by any thread):
                                                                                                                              */
      expected = false;
      assert( vacant.compare_exchange_strong( /*mod*/expected, true ) );
      return result;
    }

   ~LogicalProcess(){}

  };//class LogicalProcess

}//namespace CoreAGI

#endif // LOGICALPROCESS_H_INCLUDED
