                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
______________________________________________________________________________

 Basic definitions for CoreAGI

 2023.05.04

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef DEF_H_INCLUDED
#define DEF_H_INCLUDED

#include <cmath>   // :nan()

#include <atomic>
#include <bitset>
#include <compare>
#include <concepts>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>

#include "color.h"
#include "semantic.type.h"


namespace CoreAGI {

  using Identity  = uint32_t; // :unsigned integer that keeps entity ID
  using Key       = uint64_t; // :unsigned integer that keep combination of two entity ID, object ID & attribute ID


  template< typename T > concept Lot = requires ( T t ){
    t.begin();
    t.end();
    bool    ( t.empty() );
    unsigned( t.size () );
  };


  using Timepoint = Semantic::Timepoint; // :see semantic.type.h
  using Duration  = Semantic::Duration;  // :see semantic.type.h

  constexpr const Identity    NIHIL { 0  };  // :identity of nonexistent quasi-entity
            const std::string NIL   { "" };  // :empty string

  template< typename T > void CLEAR( T& x ){ memset( &x, 0, sizeof( x ) ); }


  template< std::equality_comparable T > bool contains( std::vector< T > V, T v ){
    for( const auto& Vi: V ) if( Vi == v ) return true;
    return false;
  };


  inline bool even( unsigned n ){ return not ( n & 0x1 ); }


  struct Imprint8x8 {
    unsigned int a : 8;
    unsigned int b : 8;
    unsigned int c : 8;
    unsigned int d : 8;
    unsigned int e : 8;
    unsigned int f : 8;
    unsigned int g : 8;
    unsigned int h : 8;
  };//class Imprint


  Key combination( const Identity& obj, const Identity& atr ){
    constexpr size_t LEN{ 8*sizeof( Identity ) };
    Identity src{ 0x1 }; // :refer obj and atr bit
    Key      out{ 0x1 }; // :refet key bit
    Key      key{   0 };
                                                                                                                              /*
    Key`s bites processed from low to high, obj befor atr:
                                                                                                                              */
    for( size_t i = 0; i < LEN; i++ ){ // Loop over obj and atr bits:
      if( src & obj ){ key |= out; } out <<= 1; // :move to next key bit
      if( src & atr ){ key |= out; } out <<= 1; // :move to next key bit
      src <<= 1;    // :move to next bit of obj and atr
    }
    return key;
  }//combination

  std::tuple< Identity, Identity > decombine( const Key& KEY ){                                                // [+] 2021.06.06
    constexpr size_t LEN{ 8*sizeof( Identity ) };
    Identity obj { 0   };
    Identity atr { 0   };
    Key      key { KEY };
    Key      src { 0x1 }; // :refer key bit
    Identity out { 0x1 }; // :refer obj and attr bit
                                                                                                                              /*
    Key`s bites processed from low to high, obj befor atr:
                                                                                                                              */
    for( size_t i = 0; i < LEN; i++ ){ // Loop over obj and atr bits:
      if( key & src ){ obj |= out; } src <<= 1;
      if( key & src ){ atr |= out; } src <<= 1;
      out <<= 1;
    }
    // assert( combination( obj, atr ) == KEY ); // DEBUG
    return std::make_tuple( obj, atr );
  }

  static_assert( sizeof( Key ) == 2*sizeof( Identity ) );

  auto kit = []( auto... parameters )->std::string{
    constexpr unsigned CAPACITY{ 2046 };
    constexpr unsigned STORAGE { CAPACITY + 2 };
    char text[ STORAGE ];
    snprintf( text, CAPACITY, parameters... );
    return std::string( text );
  };

  int choice( const char* title, const std::vector< std::string >& cases ){
    assert( title );
    using namespace CoreAGI;
    int         i;
    std::string c;
    for(;;){
      printf( "%s%s%s", YELLOW, title, RESET );
      for( int j = 0; const auto& s: cases ) printf( "\n   %s%2u%s %s", GREEN, j++, RESET, s.c_str() );
      printf( "\n Type choice %snumber%s and press ENTER: ", GREEN, RESET );
      std::cin >> c;
      if( c.empty() ) continue;
      i = std::atoi( c.data() );
      if( i < 0 or i >= int( cases.size() ) ) continue;
      break;
    }
    return i;
  }

  namespace Symbol {
                                                                                                                              /*
    UTF-8 symbols ( https://www.utf8-chartable.de/unicode-utf8-table.pl?start=768 )
                                                                                                                              */
    constexpr const char* NOT      { "\xC2\xAC"     };  // :utf-8 symbol
    constexpr const char* OMEGA    { "\xCE\xA9"     };  // :utf-8 symbol
    constexpr const char* EMPTY_SET{ "\xE2\x88\x85" };  // :utf-8 symbol
    constexpr const char* EXISTS   { "\xE2\x88\x83" };  // :utf-8 symbol
    constexpr const char* EXCESS   { "\xE2\x88\xB9" };  // :utf-8 symbol
    constexpr const char* DOWNARROW{ "\xE2\x86\x93" };  // :utf-8 symbol
    constexpr const char* RING     { "\xE2\x88\x98" };  // :utf-8 symbol

    constexpr const char* SYMBOLS[7]{ NOT, OMEGA, EMPTY_SET, EXISTS, EXCESS, DOWNARROW, RING };
    constexpr const int   LENGTH [7]{ 1,   1,     2,         2,      2,      2,         2    };

  }

  unsigned actualLength( std::string s ){
                                                                                                                              /*
    Calculation screen length of the string that contains multibyte symbols:
                                                                                                                              */
    using namespace Symbol;
    std::stringstream S;
    int L{ 0 };
    for( char symbol: s ){
      S << symbol, L++;
      for(          auto color : COLORS  ) if( S.str().ends_with( color  ) ){ L -= strlen( color ); break; }
      for( int i=0; auto symbol: SYMBOLS ) if( S.str().ends_with( symbol ) ){ L -= LENGTH[ i++ ];   break; }
    }
    assert( L >= 0 );
    return L;
  };
                                                                                                                              /*
  Hasher for Identity:
                                                                                                                              */
  struct IdentityHash {
    std::size_t operator()( const Identity& i ) const { return size_t( i ); }
  };
                                                                                                                              /*
  Enumeration that defines relations between spans and patterns:
                                                                                                                              */
  enum Affinity: uint8_t {
    DISTINCT      = 0b00000000,
    CONTAINS_HEAD = 0b00000010,
    CONTAINS_TAIL = 0b00000001,
    CONTAINS      = 0b00000011,
    IDENTIC       = 0b00000100,
    CONTAINED     = 0b00000100
  };


  struct Span{

    static_assert( std::numeric_limits< double >::has_infinity );

    double o;
    double t;

    Span(                    ): o{ -std::numeric_limits<double>::infinity() }, t{ std::numeric_limits<double>::infinity() }{}
    Span( double left        ): o{ left                                     }, t{ std::numeric_limits<double>::infinity() }{};
    Span( double a, double b ): o{ std::min( a, b )                         }, t{ std::max( a, b )                        }{};

    bool contains( double x ) const { return x >= o and x <= t; }

    auto operator <=> ( double x ) const {
      if( x < o ) return std::partial_ordering::less;
      if( x > t ) return std::partial_ordering::greater;
      else        return std::partial_ordering::equivalent;
    }

    double len() const { return t - o; }

    double empty() const {
      constexpr double EPS{ 1.0e-6 };
      return len() < EPS;
    }

    double fraction( double x ) const {
      if( not contains( x ) ) return NAN;
      if( empty()           ) return NAN;
      return ( x - o )/len();
    }

    template< typename T > T interpolate( const T& Vo, const T& Vt, double x ) const {
      const double Ct = fraction( x );
      const double Co = 1.0 - Ct;
      return Vo*Co + Vt*Ct;
    }

    Affinity operator/ ( const Span& S ) const {
      if( o == S.o and t == S.t ) return IDENTIC;
      if( o <  S.o and t >  S.t ) return CONTAINS;
      if( o >  S.o and t <  S.t ) return CONTAINED;
      if( t <  S.t              ) return CONTAINS_HEAD;
      if( o >  S.o              ) return CONTAINS_TAIL;
      else                        return DISTINCT;
    }

  };//struct Span


}//namespace CoreAGI

#endif // DEF_H_INCLUDED
