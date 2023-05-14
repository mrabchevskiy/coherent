                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2021.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Timer for intervals measured in SEC, MILLISEC, MICROSEC, NANOSEC

 2020.05.04

 2021.12.12  Time unit enumeration `TimeUnit`, related constansa and `lex` function moved to `def.h`
             Class Chronos added

 2022.01.12  struct pause added

 2022.01.13  `stop` method returns reference to Timer, so `stp` can be combined with operator `[]`.

 2022.01.19  Added compatizon operators to `Timer`

 2022.02.11  Added method `fraction( Duration& )`

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <cassert>
#include <chrono>
#include <thread>

#include "semantic.type.h"

namespace CoreAGI {

  using namespace Semantic;

  [[ deprecated ]] void sleep( unsigned t ){
    std::this_thread::sleep_for( std::chrono::milliseconds( t ) );
  }

  struct pause {
    unsigned t;
    constexpr pause( const unsigned& n ): t{ n }{};
    void operator[] ( const TimeUnit& unit ){
      switch( unit ){
        case NANOSEC  : std::this_thread::sleep_for( std::chrono::nanoseconds ( t ) ); return;
        case MICROSEC : std::this_thread::sleep_for( std::chrono::microseconds( t ) ); return;
        case MILLISEC : std::this_thread::sleep_for( std::chrono::milliseconds( t ) ); return;
        case SEC      : std::this_thread::sleep_for( std::chrono::seconds     ( t ) ); return;
        default       : return;
      }
    }
  };


  class Timer {

    using Clock = std::chrono::steady_clock;

            std::chrono::time_point< Clock > to;
    mutable std::chrono::time_point< Clock > tt;

  public:

    Timer(): to{ Clock::now() }, tt{ to }{ }

    void   start(){ to = tt = Clock::now();          }
    Timer& stop (){ tt = Clock::now(); return *this; }                                                         // [+] 2022.01.13

    double fraction( const Duration& T ) const { return usec()/T[ MICROSEC ]; }                                // [+] 2022.02.11

    [[deprecated]] double elapsed( const TimeUnit& unit = MILLISEC ) const {
      constexpr double TIME_UNIT[4] = { 1.0, 1.0e-3, 1.0e-6, 1.0e-9 };
      return TIME_UNIT[ unit ]*double( std::chrono::duration_cast< std::chrono::nanoseconds >( Clock::now() - to ).count() );
    }

//    [[deprecated]] Duration operator()( const TimeUnit& unit = MILLISEC ) const {
//      return Duration::Value( std::chrono::duration_cast< std::chrono::nanoseconds >( tt - to ).count() )[ unit ];
//    }

    double nsec() const{ tt=Clock::now(); return        std::chrono::duration_cast< std::chrono::nanoseconds >(tt-to).count(); }
    double usec() const{ tt=Clock::now(); return 1.0e-3*std::chrono::duration_cast< std::chrono::nanoseconds >(tt-to).count(); }
    double msec() const{ tt=Clock::now(); return 1.0e-6*std::chrono::duration_cast< std::chrono::nanoseconds >(tt-to).count(); }
    double  sec() const{ tt=Clock::now(); return 1.0e-9*std::chrono::duration_cast< std::chrono::nanoseconds >(tt-to).count(); }

    double operator[]( const TimeUnit& unit ) const {
//    return Duration::Value( std::chrono::duration_cast< std::chrono::nanoseconds >( tt - to ).count() )[ unit ];
      constexpr double TIME_UNIT[4] = { 1.0, 1.0e-3, 1.0e-6, 1.0e-9 };
      return TIME_UNIT[ unit ]*double( std::chrono::duration_cast< std::chrono::nanoseconds >( Clock::now() - to ).count() );
    }

    Duration operator()() const {
      return Duration::Value( std::chrono::duration_cast< std::chrono::nanoseconds >( tt - to ).count() )[ NANOSEC ];
    }

    bool operator < ( const Duration& dt ) const {
      return std::chrono::duration_cast< std::chrono::nanoseconds >( Clock::now() - to ).count() < dt.endo();
    }

    bool operator <= ( const Duration& dt ) const {
      return std::chrono::duration_cast< std::chrono::nanoseconds >( Clock::now() - to ).count() <= dt.endo();
    }

    bool operator > ( const Duration& dt ) const {
      return std::chrono::duration_cast< std::chrono::nanoseconds >( Clock::now() - to ).count() > dt.endo();
    }

    bool operator >= ( const Duration& dt ) const {
      return std::chrono::duration_cast< std::chrono::nanoseconds >( Clock::now() - to ).count() > dt.endo();
    }

  };//Timer

}//namespace CoreAGI

#endif // TIMER_H_INCLUDED
