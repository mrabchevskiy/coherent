                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2022.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Sorting array on the place and creation sorted vector from immutable vector.

 2022.01.09  Initial version

 2022.02.10  Added Timepoint operator= ( Chronos& ); = ( Timepoint& );
             a few bugs fixed

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef SEMANTIC_TYPE_H_INCLUDED
#define SEMANTIC_TYPE_H_INCLUDED

#include <cmath>

#include <concepts>
#include <string>

#include "chronos.h"


namespace CoreAGI::Semantic {

  enum TimeUnit: unsigned { NANOSEC = 0, MICROSEC = 1, MILLISEC = 2, SEC = 3 };

  const char* lex( const TimeUnit& unit ){
    switch( unit ){
      case NANOSEC : return "nanosec";
      case MICROSEC: return "microsec";
      case MILLISEC: return "millisec";
      case SEC     : return "sec";
    }
    return "?";
  }


  class Time {
  protected:

    double value; // :nanoseconds

  public:

    constexpr          Time             (                 ): value{ 0.0                            }{};
    constexpr          Time             ( const double& t ): value{ t                              }{};
    constexpr explicit Time             ( const Time&   t ): value{ t.value                        }{}
    constexpr          Time& operator = ( const Time&   t )       { value = t.value; return *this; }

    [[deprecated]] constexpr double operator ()() const { return value; }

    constexpr double endo() const { return value; }

    constexpr double operator[] ( const TimeUnit& unit ) const {
                                                                                                                              /*
      Get value converted into provided time units (interlally represented in nanoseconds):
                                                                                                                              */
      constexpr double TIME_UNIT[4] = { 1.0, 1.0e-3, 1.0e-6, 1.0e-9 };
      return value*TIME_UNIT[ int( unit ) ];
    }

    constexpr double  nsec() const { return value;        }
    constexpr double  usec() const { return value*1.0e-3; }
    constexpr double  msec() const { return value*1.0e-6; }
    constexpr double   sec() const { return value*1.0e-9; }

  };//class Time


  constexpr Time operator""_SEC     ( long double t ){ return Time( t*1.0e+9 ); }
  constexpr Time operator""_MILLISEC( long double t ){ return Time( t*1.0e+6 ); }
  constexpr Time operator""_MICROSEC( long double t ){ return Time( t*1.0e+3 ); }
  constexpr Time operator""_NANOSEC ( long double t ){ return Time( t        ); }


  class Timepoint;

  class Duration: public Time {

    friend class Timepoint;
    friend class Value;
    friend class Timer;

    constexpr Duration( const double& t ): Time{ t }{};

  public:

    constexpr          Duration             (                    ): Time{          }{};
    constexpr explicit Duration             ( const Time&      t ): Time{ t.endo() }{}
    constexpr explicit Duration             ( const Duration&  d ): Time{ d.endo() }{}
    constexpr          Duration& operator = ( const Duration&  d )      { value = d.endo(); return *this; }

    static const char* type(){ return "Duration"; }

    constexpr Duration& operator += ( const Duration& d )       { value += d.endo(); return *this;     }
    constexpr Duration& operator -= ( const Duration& d )       { value -= d.endo(); return *this;     }
    constexpr Duration  operator +  ( const Duration& d ) const { return Duration{ value + d.endo() }; }
    constexpr Duration  operator -  ( const Duration& d ) const { return Duration{ value - d.endo() }; }

    constexpr Duration  operator *  ( const double&   c ) const { return Duration{ c*value }; }
    constexpr Duration& operator *= ( const double&   c )       { value *= c; return *this;   }
    constexpr Duration  operator /  ( const double&   c ) const { return Duration{ value/c }; }
    constexpr Duration& operator /= ( const double&   c )       { value /= c; return *this;   }
    constexpr bool      operator <  ( const Duration& t ) const { return value <  t.endo();   }
    constexpr bool      operator <= ( const Duration& t ) const { return value <= t.endo();   }
    constexpr bool      operator == ( const Duration& t ) const { return value == t.endo();   }
    constexpr bool      operator >= ( const Duration& t ) const { return value >= t.endo();   }
    constexpr bool      operator  > ( const Duration& t ) const { return value  > t.endo();   }

    constexpr Duration abs() const { return Duration( std::fabs( value ) ); }

                                                                                                                              /*
    Helper class for creation instance of Duration with number interpreted as value
    expressed using perticular time unit. Examples:

      auto duration = Duration::Value{ 2.78 }[ SEC ];

      Duation = Duration::Value( 3.14 )[ MICROSEC ];
                                                                                                                              */
    class Value {
      double value;
    public:
      constexpr Value( const double& v ): value{ v }{}
      constexpr Duration operator[]( const TimeUnit& unit ) const {
        constexpr double TIME_FACTOR[4] = { 1.0, 1.0e+3, 1.0e+6, 1.0e+9 };
        return Duration( value*TIME_FACTOR[ int( unit ) ] );
      }
    };//class Value

  };//class Duration


  class Timepoint: public Time {

    friend class Value;
    friend class CoreAGI::Chronos;

  protected:

    constexpr Timepoint( const double& t ): Time{ t }{}

  public:

    constexpr          Timepoint(                          ): Time{                   }{}
    constexpr explicit Timepoint( const Time&      t       ): Time{ t.endo()          }{}
              explicit Timepoint( const Chronos&   chronos ): Time{ chronos.nanosec() }{}
    constexpr explicit Timepoint( const Timepoint& t       ): Time{ t()               }{}

    constexpr Timepoint& operator = ( const Timepoint& t       ){ value = t();               return *this; }
              Timepoint& operator = ( const Chronos&   chronos ){ value = chronos.nanosec(); return *this; }

    static const char* type(){ return "Timepoint"; }

    constexpr Duration   operator-  ( const Timepoint&  t ) const { return Duration{ value - t() };       }
    constexpr Timepoint& operator-= ( const Duration&   d )       { value -= d.endo(); return *this;      }
    constexpr Timepoint& operator+= ( const Duration&   d )       { value += d.endo(); return *this;      }
    constexpr Timepoint  operator-  ( const Duration&   d ) const { return Timepoint{ value - d.endo() }; }
    constexpr Timepoint  operator+  ( const Duration&   d ) const { return Timepoint{ value + d.endo() }; }


    constexpr Timepoint  operator *  ( const double&    c ) const { return Timepoint{ c*value };   }
    constexpr Timepoint& operator *= ( const double&    c )       { value *= c; return *this;      }
    constexpr Timepoint  operator /  ( const double&    c ) const { return Timepoint{ value/c };   }
    constexpr Timepoint& operator /= ( const double&    c )       { value /= c; return *this;      }
    constexpr double     operator () (                    ) const { return value;                  }
    constexpr bool       operator <  ( const Timepoint& t ) const { return value <  t();           }
    constexpr bool       operator <= ( const Timepoint& t ) const { return value <= t();           }
    constexpr bool       operator == ( const Timepoint& t ) const { return value == t();           }
    constexpr bool       operator >= ( const Timepoint& t ) const { return value >= t();           }
    constexpr bool       operator  > ( const Timepoint& t ) const { return value  > t();           }

    class Value {
      double value;
    public:
      constexpr Value( const double& v ): value{ v }{}
      constexpr Timepoint operator[]( const TimeUnit& unit ) const {
        constexpr double TIME_FACTOR[4] = { 1.0, 1.0e+3, 1.0e+6, 1.0e+9 };
        return Timepoint( value*TIME_FACTOR[ int( unit ) ] );
      }
    };//class Value

  };//class Timepoint

                                                                                                                              /*
  Coordinate X _________________________________________________________________________________________________________________
                                                                                                                              */
  template< typename Real = double > class AbsCoordX;

  template< typename Real = double > class RelCoordX {
    Real value;
  public:
    static const char* type(){ return "RelCoordX"; }

    constexpr            RelCoordX   (                    ) = default;
    constexpr explicit   RelCoordX   ( const Real&      x ): value{ x                                    }{}
    constexpr explicit   RelCoordX   ( const RelCoordX& x ): value{ x.value                              }{}

    constexpr RelCoordX& operator += ( const RelCoordX& x )       { value += x.value;                    }
    constexpr RelCoordX& operator -= ( const RelCoordX& x )       { value -= x.value;                    }
    constexpr RelCoordX  operator +  ( const RelCoordX& x )       { return RelCoordX{ value + x.value }; }
    constexpr RelCoordX  operator -  ( const RelCoordX& x )       { return RelCoordX{ value - x.value }; }

    constexpr RelCoordX& operator =  ( const RelCoordX& x )       { value = x.value;             }
    constexpr RelCoordX  operator *  ( const Real&      c ) const { return RelCoordX{ c*value }; }
    constexpr RelCoordX& operator *= ( const Real&      c )       { value *= c; return *this;    }
    constexpr RelCoordX  operator /  ( const Real&      c ) const { return RelCoordX{ value/c }; }
    constexpr RelCoordX& operator /= ( const Real&      c )       { value /= c; return *this;    }
    constexpr Real       operator () (                    ) const { return value;                }
    constexpr bool       operator <  ( const RelCoordX& x ) const { return value <  x.value; }
    constexpr bool       operator <= ( const RelCoordX& x ) const { return value <= x.value; }
    constexpr bool       operator == ( const RelCoordX& x ) const { return value == x.value; }
    constexpr bool       operator >= ( const RelCoordX& x ) const { return value >= x.value; }
    constexpr bool       operator  > ( const RelCoordX& x ) const { return value  > x.value; }
  };//class RelCoordX

  template< typename Real > class AbsCoordX {
    Real value;
  public:
    static const char* type(){ return "AbsCoordX"; }

    constexpr RelCoordX< Real> operator- ( const AbsCoordX& x ) const { return RelCoordX< Real >{ value - x.value }; }

    constexpr AbsCoordX& operator -= ( const RelCoordX< Real >& x )       { value -= x.value; return *this;      }
    constexpr AbsCoordX& operator += ( const RelCoordX< Real >& x )       { value += x.value; return *this;      }
    constexpr AbsCoordX  operator -  ( const RelCoordX< Real >& x ) const { return AbsCoordX{ value - x.value }; }
    constexpr AbsCoordX  operator +  ( const RelCoordX< Real >& x ) const { return AbsCoordX{ value + x.value }; }

    constexpr            AbsCoordX   (                    ) = default;
    constexpr explicit   AbsCoordX   ( const Real&      x ): value{ x                            }{}
    constexpr explicit   AbsCoordX   ( const AbsCoordX& x ): value{ x.value                      }{}
    constexpr AbsCoordX& operator =  ( const AbsCoordX& x )       { value = x.value;             }
    constexpr AbsCoordX  operator *  ( const Real&      c ) const { return AbsCoordX{ c*value }; }
    constexpr AbsCoordX& operator *= ( const Real&      c )       { value *= c; return *this;    }
    constexpr AbsCoordX  operator /  ( const Real&      c ) const { return AbsCoordX{ value/c }; }
    constexpr AbsCoordX& operator /= ( const Real&      c )       { value /= c; return *this;    }
    constexpr Real       operator () (                    ) const { return value;                }
    constexpr bool       operator <  ( const AbsCoordX& x ) const { return value <  x.value; }
    constexpr bool       operator <= ( const AbsCoordX& x ) const { return value <= x.value; }
    constexpr bool       operator == ( const AbsCoordX& x ) const { return value == x.value; }
    constexpr bool       operator >= ( const AbsCoordX& x ) const { return value >= x.value; }
    constexpr bool       operator  > ( const AbsCoordX& x ) const { return value  > x.value; }
  };//class AbsCoordX

                                                                                                                              /*
  Coordinate Y _________________________________________________________________________________________________________________
                                                                                                                              */

  template< typename Real = double > class AbsCoordY;

  template< typename Real = double > class RelCoordY {
    Real value;
  public:
    static const char* type(){ return "RelCoordY"; }

    constexpr            RelCoordY   (                    ) = default;
    constexpr explicit   RelCoordY   ( const Real&      x ): value{ x                                    }{}
    constexpr explicit   RelCoordY   ( const RelCoordY& x ): value{ x.value                              }{}

    constexpr RelCoordY& operator += ( const RelCoordY& x )       { value += x.value;                    }
    constexpr RelCoordY& operator -= ( const RelCoordY& x )       { value -= x.value;                    }
    constexpr RelCoordY  operator +  ( const RelCoordY& x )       { return RelCoordY{ value + x.value }; }
    constexpr RelCoordY  operator -  ( const RelCoordY& x )       { return RelCoordY{ value - x.value }; }

    constexpr RelCoordY& operator =  ( const RelCoordY& x )       { value = x.value;             }
    constexpr RelCoordY  operator *  ( const Real&      c ) const { return RelCoordY{ c*value }; }
    constexpr RelCoordY& operator *= ( const Real&      c )       { value *= c; return *this;    }
    constexpr RelCoordY  operator /  ( const Real&      c ) const { return RelCoordY{ value/c }; }
    constexpr RelCoordY& operator /= ( const Real&      c )       { value /= c; return *this;    }
    constexpr Real       operator () (                    ) const { return value;                }

    constexpr bool       operator <  ( const RelCoordY& x ) const { return value <  x.value; }
    constexpr bool       operator <= ( const RelCoordY& x ) const { return value <= x.value; }
    constexpr bool       operator == ( const RelCoordY& x ) const { return value == x.value; }
    constexpr bool       operator >= ( const RelCoordY& x ) const { return value >= x.value; }
    constexpr bool       operator  > ( const RelCoordY& x ) const { return value  > x.value; }
  };//class RelCoordY

  template< typename Real > class AbsCoordY {
    Real value;
  public:
    static const char* type(){ return "AbsCoordY"; }

    constexpr RelCoordY< Real> operator- ( const AbsCoordY& x ) const { return RelCoordY< Real >{ value - x.value }; }

    constexpr AbsCoordY& operator -= ( const RelCoordY< Real >& x )       { value -= x.value; return *this;      }
    constexpr AbsCoordY& operator += ( const RelCoordY< Real >& x )       { value += x.value; return *this;      }
    constexpr AbsCoordY  operator -  ( const RelCoordY< Real >& x ) const { return AbsCoordY{ value - x.value }; }
    constexpr AbsCoordY  operator +  ( const RelCoordY< Real >& x ) const { return AbsCoordY{ value + x.value }; }

    constexpr            AbsCoordY   (                    ) = default;
    constexpr explicit   AbsCoordY   ( const Real&      x ): value{ x       }{}
    constexpr explicit   AbsCoordY   ( const AbsCoordY& x ): value{ x.value }{}

    constexpr AbsCoordY& operator =  ( const AbsCoordY& x )       { value = x.value;             }
    constexpr AbsCoordY  operator *  ( const Real&      c ) const { return AbsCoordY{ c*value }; }
    constexpr AbsCoordY& operator *= ( const Real&      c )       { value *= c; return *this;    }
    constexpr AbsCoordY  operator /  ( const Real&      c ) const { return AbsCoordY{ value/c }; }
    constexpr AbsCoordY& operator /= ( const Real&      c )       { value /= c; return *this;    }
    constexpr Real       operator () (                    ) const { return value;                }

    constexpr bool       operator <  ( const AbsCoordY& x ) const { return value <  x.value; }
    constexpr bool       operator <= ( const AbsCoordY& x ) const { return value <= x.value; }
    constexpr bool       operator == ( const AbsCoordY& x ) const { return value == x.value; }
    constexpr bool       operator >= ( const AbsCoordY& x ) const { return value >= x.value; }
    constexpr bool       operator  > ( const AbsCoordY& x ) const { return value  > x.value; }
  };//class AbsCoordY

                                                                                                                              /*
  Norm and Distance ____________________________________________________________________________________________________________
                                                                                                                              */

  template< typename Real = double > class Distance;
                                                                                                                              /*
  NormL2 can be produced only by:
    - by calculaton Euclidean norm using RelCoordX and RelCoordY
    - by constructing form number
    - by copy/assign NormL2 value
                                                                                                                              */
  template< typename Real = double > class NormL2 {
    Real value;
  public:
    static const char* type(){ return "NormL2"; }

    NormL2() = default;
    NormL2( const RelCoordX< Real >& x, const RelCoordY< Real >& y ): value{ std::hypot( x(), y() ) }{}
    NormL2( const RelCoordY< Real >& y, const RelCoordX< Real >& x ): value{ std::hypot( x(), y() ) }{}

    constexpr NormL2& operator = ( const NormL2& n ){ value = n.value; return *this; }

    constexpr Distance< Real > operator - ( const NormL2&           n ) const { return Distance< Real >{ value - n() }; }
    constexpr Distance< Real > operator + ( const NormL2&           n ) const { return Distance< Real >{ value + n() }; }
    constexpr Distance< Real > operator - ( const Distance< Real >& d ) const { return Distance< Real >{ value - d() }; }
    constexpr Distance< Real > operator + ( const Distance< Real >& d ) const { return Distance< Real >{ value + d() }; }


    constexpr Real     operator () (                 ) const { return value;            }
    constexpr bool     operator <  ( const NormL2& n ) const { return value <  n.value; }
    constexpr bool     operator <= ( const NormL2& n ) const { return value <= n.value; }
    constexpr bool     operator == ( const NormL2& n ) const { return value == n.value; }
    constexpr bool     operator >= ( const NormL2& n ) const { return value >= n.value; }
    constexpr bool     operator  > ( const NormL2& n ) const { return value  > n.value; }
  };//class NormL2

  template< typename Real > class Distance {
    Real value;
  public:
    static const char* type(){ return "Distance"; }

    constexpr           Distance(                        ) = default;
    constexpr explicit  Distance( const Real&          d ): value{ d       }{}
    constexpr explicit  Distance( const NormL2< Real>& l ): value{ l()     }{}
    constexpr explicit  Distance( const Distance& d      ): value{ d.value }{}

    constexpr Distance& operator = ( const Distance&       d ){ value = d.value; return *this; }
    constexpr Distance& operator = ( const NormL2< Real >& l ){ value = l.value; return *this; }

    constexpr Distance& operator -= ( const Distance& d )       { value -= d.value; return *this;     }
    constexpr Distance& operator += ( const Distance& d )       { value += d.value; return *this;     }
    constexpr Distance  operator -  ( const Distance& d ) const { return Distance{ value - d.value }; }
    constexpr Distance  operator +  ( const Distance& d ) const { return Distance{ value + d.value }; }

    constexpr Distance  operator *  ( const Real&     c ) const { return Distance{ c*value };    }
    constexpr Distance& operator *= ( const Real&     c )       { value *= c; return *this;      }
    constexpr Distance  operator /  ( const Real&     c ) const { return Distance{ value/c };    }
    constexpr Distance& operator /= ( const Real&     c )       { value /= c; return *this;      }
    constexpr Real      operator () (                   ) const { return value;                  }
    constexpr bool      operator <  ( const Distance& x ) const { return value <  x.value;       }
    constexpr bool      operator <= ( const Distance& x ) const { return value <= x.value;       }
    constexpr bool      operator == ( const Distance& x ) const { return value == x.value;       }
    constexpr bool      operator >= ( const Distance& x ) const { return value >= x.value;       }
    constexpr bool      operator  > ( const Distance& x ) const { return value  > x.value;       }
  };//class Distance

                                                                                                                              /*
  Velocity _____________________________________________________________________________________________________________________
                                                                                                                              */

  template< typename Real = double > class DxDt {
    Real value;
  public:
    static const char* type(){ return "DxDt"; }

    constexpr          DxDt(                                                 ) = default;
    constexpr explicit DxDt( const Real& dxdt                                ): value{ dxdt      }{}
    constexpr explicit DxDt( const RelCoordX< Real >& dx, const Duration& dt ): value{ dx()/dt() }{}

    constexpr DxDt& operator = ( const DxDt& dxdt ){ value = dxdt.value; }

    constexpr DxDt& operator += ( const DxDt& dxdt ){ value += dxdt.value;               }
    constexpr DxDt& operator -= ( const DxDt& dxdt ){ value -= dxdt.value;               }
    constexpr DxDt  operator +  ( const DxDt& dxdt ){ return DxDt{ value + dxdt.value }; }
    constexpr DxDt  operator -  ( const DxDt& dxdt ){ return DxDt{ value - dxdt.value }; }

    constexpr DxDt  operator *  ( const Real& c    ) const { return DxDt{ c*value };     }
    constexpr DxDt& operator *= ( const Real& c    )       { value *= c; return *this;   }
    constexpr DxDt  operator /  ( const Real& c    ) const { return DxDt{ value/c };     }
    constexpr DxDt& operator /= ( const Real& c    )       { value /= c; return *this;   }
    constexpr Real  operator () (                  ) const { return value;               }
    constexpr bool  operator <  ( const DxDt& dxdt ) const { return value <  dxdt.value; }
    constexpr bool  operator <= ( const DxDt& dxdt ) const { return value <= dxdt.value; }
    constexpr bool  operator == ( const DxDt& dxdt ) const { return value == dxdt.value; }
    constexpr bool  operator >= ( const DxDt& dxdt ) const { return value >= dxdt.value; }
    constexpr bool  operator  > ( const DxDt& dxdt ) const { return value  > dxdt.value; }
  };//class DxDt


  template< typename Real = double > class DyDt {
    Real value;
  public:
    static const char* type(){ return "DyDt"; }

    constexpr          DyDt(                                                 ) = default;
    constexpr explicit DyDt( const Real& dydt                                ): value{ dydt      }{}
    constexpr explicit DyDt( const RelCoordY< Real >& dy, const Duration& dt ): value{ dy()/dt() }{}

    constexpr DyDt& operator = ( const DyDt& dydt ){ value = dydt.value; }

    constexpr DyDt& operator += ( const DyDt& dydt ){ value += dydt.value;               }
    constexpr DyDt& operator -= ( const DyDt& dydt ){ value -= dydt.value;               }
    constexpr DyDt  operator +  ( const DyDt& dydt ){ return DyDt{ value + dydt.value }; }
    constexpr DyDt  operator -  ( const DyDt& dydt ){ return DyDt{ value - dydt.value }; }

    constexpr DyDt  operator *  ( const Real& c    ) const { return DyDt{ c*value };     }
    constexpr DyDt& operator *= ( const Real& c    )       { value *= c; return *this;   }
    constexpr DyDt  operator /  ( const Real& c    ) const { return DyDt{ value/c };     }
    constexpr DyDt& operator /= ( const Real& c    )       { value /= c; return *this;   }
    constexpr Real  operator () (                  ) const { return value;               }
    constexpr bool  operator <  ( const DyDt& dydt ) const { return value <  dydt.value; }
    constexpr bool  operator <= ( const DyDt& dydt ) const { return value <= dydt.value; }
    constexpr bool  operator == ( const DyDt& dydt ) const { return value == dydt.value; }
    constexpr bool  operator >= ( const DyDt& dydt ) const { return value >= dydt.value; }
    constexpr bool  operator  > ( const DyDt& dydt ) const { return value  > dydt.value; }
  };//class DyDt


  template< typename Real = double > class Velocity;

  template< typename Real = double > class NormV2 {
    Real value;
  public:
    static const char* type(){ return "NormV2"; }

    NormV2() = default;
    NormV2( const DxDt< Real >& x, const DyDt< Real >& y ): value{ std::hypot( x(), y() ) }{}
    NormV2( const DyDt< Real >& y, const DxDt< Real >& x ): value{ std::hypot( x(), y() ) }{}

    constexpr NormV2& operator = ( const NormV2& n ){ value = n.value; return *this; }

    constexpr Velocity< Real > operator - ( const NormV2&           n ) const { return Velocity< Real >{ value - n() }; }
    constexpr Velocity< Real > operator + ( const NormV2&           n ) const { return Velocity< Real >{ value + n() }; }
    constexpr Velocity< Real > operator - ( const Velocity< Real >& d ) const { return Velocity< Real >{ value - d() }; }
    constexpr Velocity< Real > operator + ( const Velocity< Real >& d ) const { return Velocity< Real >{ value + d() }; }

    constexpr Real operator () (                 ) const { return value;            }
    constexpr bool operator <  ( const NormV2& n ) const { return value <  n.value; }
    constexpr bool operator <= ( const NormV2& n ) const { return value <= n.value; }
    constexpr bool operator == ( const NormV2& n ) const { return value == n.value; }
    constexpr bool operator >= ( const NormV2& n ) const { return value >= n.value; }
    constexpr bool operator  > ( const NormV2& n ) const { return value  > n.value; }
  };//class NormV2


  template< typename Real > class Velocity {
    Real value;
  public:
    static const char* type(){ return "Velocity"; }

    constexpr           Velocity(                        ) = default;
    constexpr explicit  Velocity( const Real&          v ): value{ v       }{}
    constexpr explicit  Velocity( const NormV2< Real>& v ): value{ v()     }{}
    constexpr explicit  Velocity( const Velocity& v      ): value{ v.value }{}

    constexpr Velocity& operator = ( const Velocity&       v ){ value = v.value; return *this; }
    constexpr Velocity& operator = ( const NormV2< Real >& v ){ value = v.value; return *this; }

    constexpr Velocity& operator -= ( const Velocity& v )       { value -= v.value; return *this;     }
    constexpr Velocity& operator += ( const Velocity& v )       { value += v.value; return *this;     }
    constexpr Velocity  operator -  ( const Velocity& v ) const { return Velocity{ value - v.value }; }
    constexpr Velocity  operator +  ( const Velocity& v ) const { return Velocity{ value + v.value }; }

    constexpr Velocity  operator *  ( const Real&     c ) const { return Velocity{ c*value }; }
    constexpr Velocity& operator *= ( const Real&     c )       { value *= c; return *this;   }
    constexpr Velocity  operator /  ( const Real&     c ) const { return Velocity{ value/c }; }
    constexpr Velocity& operator /= ( const Real&     c )       { value /= c; return *this;   }
    constexpr Real      operator () (                   ) const { return value;               }
    constexpr bool      operator <  ( const Velocity& v ) const { return value <  v.value;    }
    constexpr bool      operator <= ( const Velocity& v ) const { return value <= v.value;    }
    constexpr bool      operator == ( const Velocity& v ) const { return value == v.value;    }
    constexpr bool      operator >= ( const Velocity& v ) const { return value >= v.value;    }
    constexpr bool      operator  > ( const Velocity& v ) const { return value  > v.value;    }
  };//class Velocity

                                                                                                                              /*
  Angle ________________________________________________________________________________________________________________________
                                                                                                                              */

  template< typename Real > Real angle( const RelCoordY< Real >& y, const RelCoordX< Real >& x ){
    return std::atan2( y(), x() );
  }

  template< typename Real > Real angle( const DyDt< Real >& dydt, const DxDt< Real >& dxdt ){
    return std::atan2( dydt(), dxdt() );
  }

}//namespace CoreAGI::Semantic

#endif // SEMANTIC_TYPE_H_INCLUDED
