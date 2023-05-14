                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Auxilliary application for creating graph images describing
 state machine implemented in the `Fluid` class

 2023.05.04  Initial version


________________________________________________________________________________________________________________________________
                                                                                                                              */

#include "fluid.h"
#include "fluid.auxiliary.h"

int main(){

  using namespace CoreAGI;

  exposeTransitionGraph();

  makeDotFiles( "./Fluid-transition.%c.dot" );

  return 0;
}
