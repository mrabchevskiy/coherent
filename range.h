                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2021.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Timer for intervals in sec, millice, microsec, nonesec

 2021.10.04
________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef RANGE_H_INCLUDED
#define RANGE_H_INCLUDED

#include <cassert>
#include <concepts>

template< typename T > concept Sized = requires( T t ){
  t.empty();
  t.size();
};


template< std::integral Elem = unsigned > class RANGE {
  struct Iter {
    Elem i;
    constexpr Iter( Elem i ): i{ i }{}
    constexpr Elem const& operator*  (               ) const {      return i;        }
    constexpr bool        operator== ( const Iter& x ) const {      return i == x.i; }
    constexpr bool        operator!= ( const Iter& x ) const {      return i <  x.i; }
    constexpr Iter&       operator++ (               )       { i++; return *this;    }
  };
  Iter       I;
  Iter const N;
public:
  constexpr RANGE(                    const Elem&  upto ): I{ 0    }, N{ upto        }{ assert( upto        >= 0    ); }
  constexpr RANGE(                    const Elem&& upto ): I{ 0    }, N{ upto        }{ assert( upto        >= 0    ); }
  constexpr RANGE( const Elem&  from, const Elem&  upto ): I{ from }, N{ upto        }{ assert( upto        >= from ); }
  constexpr RANGE( const Elem&& from, const Elem&& upto ): I{ from }, N{ upto        }{ assert( upto        >= from ); }
  constexpr RANGE( const Sized auto& data             ): I{ 0    }, N{ data.size() }{ assert( data.size() >= 0    ); }
  constexpr Iter begin(){ return I; }
  constexpr Iter end  (){ return N; }
};//RANGE

#endif // RANGE_H_INCLUDED
