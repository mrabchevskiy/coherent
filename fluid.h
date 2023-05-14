                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
______________________________________________________________________________


 CoreAGI::Fluid is a wrapper over objects that provided two access modes:

   [1] simultaneous `read-only` access from many threads
   [2] exclusive modification-allowed (`write`) access from many threads

_______________________________________________________________________________

 2023.05.04 Initial version

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef FLUID_H_INCLUDED
#define FLUID_H_INCLUDED

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstddef>

#include <atomic>
#include <concepts>
#include <functional>

#include "def.h"
//#include "logic.h"
#include "range.h"
#include "timer.h"

namespace CoreAGI {

  class FluidCore {
  public:
                                                                                                                              /*
    Class implements state machines that provide:
      - exclusive access for data modification
      - simultaneous read-only data access with limited number of active readers

    State of dataset stored into a singe atomic variable that combines
    `core state` with number of active readers.

    Abilities are similar to provided by std::lock_guard, std::shared_mutex
    and std::unique_mutex, but logic extended for preventing deadlocks.
    ____________________________________________________________________________________________________________________________
                                                                                                                              */

    using Packed = unsigned;

    enum class Action{
      none, // :don`t change number of active readers
      incr, // :increase     number of active readers
      decr, // :decrease     number of active readers
      term  // :set to zero  number of active readers
    };

    static constexpr unsigned ACTION_SIZE{ 4 };

    static constexpr Action ACTIONS[ ACTION_SIZE ]{
                                                                                                                              /*
      Automaton actions, become part of edge labels:
                                                                                                                              */
      Action::none, // :no action
      Action::incr,
      Action::decr,
      Action::term
    };

    enum class State: unsigned{ O, I, W, r, R, f, F };

    static constexpr unsigned STATE_SIZE{ 7 };

    static constexpr State STATES[ STATE_SIZE ]{
                                                                                                                              /*
      Core states ~ nodes of state transition graph:
                                                                                                                              */
      State::O, // :undefined
      State::I, // :Idling
      State::W, // :Writing
      State::r, // :Reading   one
      State::R, // :Reading   several
      State::f, // :Finishing one
      State::F  // :Finishing several
    };
                                                                                                                              /*
    Make composite (packed) state:
                                                                                                                              */
    static Packed packup( const State& state, const unsigned& num ){
      return ( ( num << 16 ) + ( unsigned( state ) & 0xFFFF ) );
    }

    struct Unpacked{

      State    state;   // :core state
      unsigned num;     // :number of active readers

      Unpacked(                      ) = delete;
      Unpacked( const Unpacked& u    ) = default;
      Unpacked( const Packed& packed ): state{ State( packed & 0xFFFF ) }, num{ unsigned( packed >> 16 ) }{
      }

      Unpacked& operator = ( const Unpacked& U ){ state = U.state; num = U.num; return *this; }

      bool operator == ( const Unpacked& U ){ return state == U.state and num == U.num; }

    };//struct Unpacked
                                                                                                                              /*
    ____________________________________________________________________________________________________________________________

    Goals (requested service actions).
    Goals are paired ( initiate something, terminate it ) and used
    in constructor and destructor of the access guard objects:
                                                                                                                              */
    enum class Goal{
      Ri, // :initiate  read-only access
      Rt, // :terminate read-only access
      Mi, // :initiate  writable  access
      Mt  // :terminate writable  access
    };

    static constexpr unsigned GOAL_SIZE{ 4 };


    struct Edge {
                                                                                                                              /*
      Edge keeps data associated with edges of the state transition graph:
                                                                                                                              */
      State  state;   // :next state
      Action action;  // :action to be performed at transition
      bool   finish;  // :new state is final one for current goal
    };

    struct TransitionGraph {
                                                                                                                              /*
      TRANSITION GRAPH DEFINITION

      Actually there are set of graphs defined on the same set of nodes/states,
      assembled into `composite` one. Each goal/service action has associated
      subgraph; edge`s label of composite graph image refers associated action.
      Edges in the DEF_G and DEF_H listed in arbitrary order.
                                                                                                                              */
      Edge G[ GOAL_SIZE ][ STATE_SIZE ]; // :transition table

      constexpr TransitionGraph(): G{}{

        struct Def {
          Goal   goal;
          State  from;
          State  into;
          Action action;
          bool   finish;
        };//Def

        constexpr Def DEF[]{
                                                                                                                                /*
            goal       from       into       action         finish
          ____________________________________________________________
                                                                                                                                */
          { Goal::Ri,  State::I,  State::r,  Action::incr,  false },
          { Goal::Ri,  State::r,  State::R,  Action::incr,  true  },

          { Goal::Rt,  State::r,  State::I,  Action::term,  true  },
          { Goal::Rt,  State::R,  State::r,  Action::decr,  true  },
          { Goal::Rt,  State::f,  State::I,  Action::term,  true  },
          { Goal::Rt,  State::F,  State::f,  Action::decr,  true  },

          { Goal::Mi,  State::I,  State::W,  Action::none,  true  },
          { Goal::Mi,  State::r,  State::f,  Action::none,  false },
          { Goal::Mi,  State::R,  State::F,  Action::none,  false },

          { Goal::Mt,  State::W,  State::I,  Action::none,  true  },

        };//DEF

        constexpr Goal GOALS[ GOAL_SIZE ]{ Goal::Ri, Goal::Rt, Goal::Mi, Goal::Mt };

        for( const auto& goal: GOALS )
          for( const auto& from: STATES )
            G[ unsigned( goal ) ][ unsigned( from ) ] = { State::O, Action::none, true };

        for( auto D: DEF ){
          assert( D.from != D.into ); // :check for loops
          G[ unsigned( D.goal ) ][ unsigned( D.from )  ] = Edge{ D.into, D.action, D.finish };
        }
      }//constructor

      const Edge& operator() ( const Goal& goal, const State& from ) const {
        const unsigned& g{ unsigned( goal ) }; assert( g <  GOAL_SIZE );
        const unsigned& f{ unsigned( from ) }; assert( f < STATE_SIZE ); assert( f != unsigned( State::O ) );
        return G[g][f];
      }

    };//TransitionGraph

    static const TransitionGraph transitionGraph;

  protected:

    mutable std::atomic< Packed > packed; // :finite automaton state
    const unsigned                ARLIM;  // :active readers limit

  public:

    FluidCore( const unsigned n ): packed{ packup( State::I, 0 ) }, ARLIM{ n }{} // :initial state is `I` ~ idling

    FluidCore(       FluidCore&& ) = default;
    FluidCore( const FluidCore&  ) = delete;

    FluidCore& operator= ( const FluidCore& ) = delete;

   ~FluidCore(){ }

  protected:

    bool trans( const Packed& required, Packed desired ) const {
                                                                                                                              /*
      Attempt to transition into new state; may be successful (returns `true`) of not (return `false`).
                                                                                                                              */
      assert( required );                                                                                               // DEBUG
      assert( desired  );                                                                                               // DEBUG
      static unsigned constexpr TRANS_ATTEMPT_LIMIT{ 2 };
      for( unsigned attempt = 0; attempt < TRANS_ATTEMPT_LIMIT; attempt++ ){
        Packed expected{ required };
                                                                                                                              /*
        When `expected` state does not match actual one, `compare_exchange_strong`
        returns `false` AND `expected` repaced by the actual state:
                                                                                                                              */
        if( packed.compare_exchange_strong( /*mod*/ expected, desired ) ) return true;
                                                                                                                              /*
        If actual state distinct from required, make no sense to try again;
        otherwise try again:
                                                                                                                              */
        if( expected != required ) return false; // :transition failed because actual state changed
      }
      return false;
    }//trans

    bool run( const Goal& goal ) const {
                                                                                                                              /*
      Sequence of transitions along state machine graph defined for requested goal:
                                                                                                                              */
      for(;;){
                                                                                                                              /*
        Get current state:
                                                                                                                              */
        Packed actualState{ packed };
                                                                                                                              /*
        Split state to core state and number of readers:
                                                                                                                              */
        Unpacked unpacked{ actualState };
                                                                                                                              /*
        Get transition edge that met current state and requested operation (`goal`):
                                                                                                                              */
        const Edge& edge{ transitionGraph( goal, unpacked.state ) };
        const State into{ edge.state };
        if( into == State::O ) return false;                       // :no way from the current state
                                                                                                                              /*
        Calculate number of readers that should be a part of new state:
                                                                                                                              */
        unsigned nextNum = unpacked.num;
        switch( edge.action ){
          case Action::none:              break;
          case Action::incr: nextNum++;   break;
          case Action::decr: nextNum--;   break;
          case Action::term: nextNum = 0; break;
          default          : assert( false );
        }//switch action
        if( nextNum > ARLIM ) return false;                         // :too many readers
                                                                                                                              /*
        Try to move into new state;
                                                                                                                              */
        const unsigned desiredState = packup( into, nextNum );
        if( not trans( actualState, desiredState ) ) return false; // :trasition failed
        if( edge.finish ) return true;                             // :goal accessed
      }//forever
    }//run

  public:

    Unpacked state() const { return Unpacked{ packed.load() }; }

  };//FluidCore


  const FluidCore::TransitionGraph FluidCore::transitionGraph{};


  template< std::default_initializable Data > class Fluid: public FluidCore {

    Data data; // :shared object

  public:

    Fluid(): FluidCore( 4 ), data{} {}

    Fluid( const unsigned& n ): FluidCore( n ), data{} {}

    Fluid(       Fluid&& ) = default;
    Fluid( const Fluid&  ) = delete;
    Fluid& operator = ( const Fluid& ) = delete;

   ~Fluid(){ }

    bool alter( std::function< void( Data& ) > func ){
                                                                                                                              /*
      Obtain write permission:
                                                                                                                              */
      if( not run( Goal::Mi ) ) return false;
                                                                                                                              /*
      Call modification function:
                                                                                                                              */
      func( data );
                                                                                                                              /*
      Return write permission:
                                                                                                                              */
      if( run( Goal::Mt ) ) return true;

      constexpr Duration RETURN_ACCESS_TIMEOUT{ Duration::Value{ 10.0 }[ MILLISEC ] };
      for( Timer timer; timer < RETURN_ACCESS_TIMEOUT; ){
        if( run( Goal::Mt ) ) return true;
        std::this_thread::yield();
      }
      assert( false ); // :deadlock

    }//alter

    bool check( std::function< void( const Data& ) > func ) const {
                                                                                                                              /*
      Obtain read permission:
                                                                                                                              */
      if( not run( Goal::Mi ) ) return false;
                                                                                                                              /*
      Call access function:
                                                                                                                              */
      func( data );
                                                                                                                              /*
      Return read permission:
                                                                                                                              */
      if( run( Goal::Mt ) ) return true;

      constexpr Duration RETURN_ACCESS_TIMEOUT{ Duration::Value{ 10.0 }[ MILLISEC ] };
      for( Timer timer; timer < RETURN_ACCESS_TIMEOUT; ){
        if( run( Goal::Mt ) ) return true;
        // std::this_thread::yield();
      }
      assert( false ); // :deadlock
    }//check

  };//Fluid

}//namespace CoreAGI

#endif // FLUID_H_INCLUDED
