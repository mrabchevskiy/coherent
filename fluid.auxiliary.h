                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
______________________________________________________________________________


 Auxiliary tools for CoreAGI::Fluid provides abilities:
 - to print info about Fluid` state machine and
 - to make state transition graphs description in `.dot` format
   (for folloving convert to `pdf` drawings using `dot` tool)

_______________________________________________________________________________

 2023.05.04 Initial version

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef FLUID_AUXILIARY_H_INCLUDED
#define FLUID_AUXILIARY_H_INCLUDED

#include <fstream>
#include <iomanip>

#include "fluid.h"

namespace CoreAGI{
                                                                                                                              /*
  Accessory routines useful for debugging and documenting `Fluid` objects:
                                                                                                                              */
  using Goal   = FluidCore::Goal;
  using Action = FluidCore::Action;
  using Edge   = FluidCore::Edge;
  using State  = FluidCore::State;

  constexpr Goal GOALS[ FluidCore::GOAL_SIZE ]{ Goal::Ri, Goal::Rt, Goal::Mi, Goal::Mt };

  constexpr char lex( const FluidCore::Goal&   goal   ){ return "RrWw"   [ int( goal   ) ]; }
  constexpr char lex( const FluidCore::Action& action ){ return "=+-0"   [ int( action ) ]; }
  constexpr char lex( const FluidCore::State&  state  ){ return "OIWrRfF"[ int( state  ) ]; }

  void exposeTransitionGraph(){
    unsigned in [ FluidCore::STATE_SIZE ]{ 0 };
    unsigned out[ FluidCore::STATE_SIZE ]{ 0 };
    printf( "\n [CoreAGI::Shared] Transition table of the state machines:\n" );
    for( const auto& goal: GOALS ) for( const auto& from: FluidCore::STATES ){
      const Edge E = FluidCore::transitionGraph.G[ unsigned( goal ) ][ unsigned( from ) ];
      if( E.state != FluidCore::State::O ){
        const char GOAL  { lex( goal     ) };
        const char FROM  { lex( from     ) };
        const char INTO  { lex( E.state  ) };
        const char ACTION{ lex( E.action ) };
        printf( "\n   %c : %c -> %c [%c] %c", GOAL, FROM, INTO, ACTION, ( E.finish ? 'T':'C' ) );
        out[ unsigned( from    ) ]++;
        in [ unsigned( E.state ) ]++;
      }
    }//for goal
    printf( "\n\n [CoreAGI::Shared] State statistics:\n" );
    for( const auto& state: FluidCore::STATES )
      printf( "\n   %c : %i inbound, %i outbound", lex( state ), in[ unsigned(state) ], out[ unsigned( state ) ] );
    printf( "\n" );
  }

  void makeGoalDotFile( char GOAL, const char* pattern ){

    struct Attributes {
      const char* edgeColor;
      const char* edgeStyle;
      const char* fontColor;
      char        goal;
    };
                                                                                                                              /*
    Define edges coloring:
                                                                                                                              */
    std::unordered_map< int, Attributes > attributes;
    for( const auto& goal: GOALS ){
      const auto i   { unsigned( goal ) };
      const char name{      lex( goal ) };
      if     ( GOAL == name          ) attributes[i] = Attributes{ "limegreen", ", style=bold", "darkgreen", name };
      else if( GOAL == toupper(name) ) attributes[i] = Attributes{ "orangered", ", style=bold", "crimson",   name };
      else                             attributes[i] = Attributes{ "gray80",    "",             "gray70",    name };
    }
                                                                                                                              /*
    Make GraphViz input file for graph led to `goal`:
                                                                                                                              */
    char path[ 256 ];
    snprintf( path, 255, pattern, GOAL );
    printf( "\n [CoreAGI::Shared] Export transition graph description as %s", path );
    std::ofstream out( path );
                                                                                                                              /*
    Type, name and image maximal size (inch):
                                                                                                                              */
    const double   FIG_SIZE         { 12.0                 }; // :picture size, inch
    const unsigned TITLE_FONT_SIZE  { 20                   }; // :pt
    const unsigned EDGE_FONT_SIZE   { 14                   }; // :pt
    const char*    HEADER           { "Finite State Graph" };
    const char*    EDGE_COLOR       { "gray40"             }; // :default
    const char*    EDGE_LABEL_COLOR { "gray20"             }; // :dafault
    const char*    NODE_COLOR       { "yellow"             };

    out << " digraph Cached {\n\n"
        << "   graph [ label=\"" << HEADER << ' ' << GOAL << "\n \", labelloc=t, fontsize=" << TITLE_FONT_SIZE
                                 << ", labeldistance=2 ]\n"
        << "   edge  [ color="   << EDGE_COLOR << ", labelfontcolor=" << EDGE_LABEL_COLOR << ", labeldistance=0.5 ]\n"
        << "   size = \"" << FIG_SIZE << ',' << FIG_SIZE << "\";\n";

    constexpr FluidCore::TransitionGraph transition;
                                                                                                                              /*
    Node data:
                                                                                                                              */
    struct Node { const char name; int col; int row; };

    Node NODES[]{
      //     name        X   Y
      { lex( State::F ), 1,  1 },
      { lex( State::R ), 2,  1 },
      { lex( State::f ), 1,  2 },
      { lex( State::r ), 2,  2 },
      { lex( State::I ), 1,  3 },
      { lex( State::W ), 2,  3 },
    };
                                                                                                                              /*
    Nodes:
                                                                                                                              */
    for( const auto& node: NODES ){
      out << "   " << std::setw( 3 )         << std::left << node.name
          << " [shape=circle pos=\""         << node.col << ',' << node.row
          << "!\", style=filled, fillcolor=" << NODE_COLOR << "]\n";
    }
                                                                                                                              /*
    Edges:
                                                                                                                              */
    for( const auto& [ goal, attr ]: attributes ){
      const auto& G_ = transition.G[ goal ];
      for( const State& from: FluidCore::STATES ){
        const Edge E = G_[ unsigned( from ) ];
        if( E.state == State::O ) continue;
        out << "   "  << std::setw( 3 ) << std::left << lex( FluidCore::STATES[ unsigned( from    ) ] )
            << " -> " << std::setw( 3 ) << std::left << lex( FluidCore::STATES[ unsigned( E.state ) ] )
            << " [ color="   << attr.edgeColor << attr.edgeStyle << ", label=\"" << attr.goal;
        if( E.action != Action::none ) out /* << '.' */ << lex( FluidCore::ACTIONS[ unsigned( E.action ) ] );
        if( not E.finish             ) out << '*';
        out << "\"" << ", fontsize=" << EDGE_FONT_SIZE << ", fontcolor=" << attr.fontColor << ", labeldistance=0.5 ]\n";
      }//for from
    }
    out << "\n }\n";
    out.close();
  }//makeGoalDotFile

  void makeDotFiles( const char* pattern = "./Shared-transition.%c.dot" ){
    printf( "\n" );
    makeGoalDotFile( 'R', pattern );
    makeGoalDotFile( 'W', pattern );
    printf( "\n" );
  }

}

#endif // FLUID_AUXILIARY_H_INCLUDED
