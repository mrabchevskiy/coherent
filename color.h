                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2021.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________

 Color codes for Lunix terminal

 2021.04.05
________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef COLOR_H_INCLUDED
#define COLOR_H_INCLUDED

#include <cstring>
#include <cassert>

#include <string>
#include <sstream>

namespace CoreAGI {

#ifdef _WIN32

  constexpr const char* BLACK   { "" };
  constexpr const char* RED     { "" };
  constexpr const char* GREEN   { "" };
  constexpr const char* YELLOW  { "" };
  constexpr const char* BLUE    { "" };
  constexpr const char* MAGENTA { "" };
  constexpr const char* CYAN    { "" };
  constexpr const char* WHITE   { "" };

  constexpr const char* xBLACK  { ""  };
  constexpr const char* xRED    { ""  };
  constexpr const char* xGREEN  { ""  };
  constexpr const char* xYELLOW { ""  };
  constexpr const char* xBLUE   { ""  };
  constexpr const char* xMAGENTA{ ""  };
  constexpr const char* xCYAN   { ""  };
  constexpr const char* xWHITE  { ""  };

  constexpr const char* RESET   { ""  };

#else

  constexpr const char* BLACK   { "\u001b[31m" };
  constexpr const char* RED     { "\u001b[31m" };
  constexpr const char* GREEN   { "\u001b[32m" };
  constexpr const char* YELLOW  { "\u001b[33m" };
  constexpr const char* BLUE    { "\u001b[34m" };
  constexpr const char* MAGENTA { "\u001b[35m" };
  constexpr const char* CYAN    { "\u001b[36m" };
  constexpr const char* WHITE   { "\u001b[37m" };

  constexpr const char* xBLACK  { "\u001b[30;1m"  };
  constexpr const char* xRED    { "\u001b[31;1m"  };
  constexpr const char* xGREEN  { "\u001b[32;1m"  };
  constexpr const char* xYELLOW { "\u001b[33;1m"  };
  constexpr const char* xBLUE   { "\u001b[34;1m"  };
  constexpr const char* xMAGENTA{ "\u001b[35;1m"  };
  constexpr const char* xCYAN   { "\u001b[36;1m"  };
  constexpr const char* xWHITE  { "\u001b[37;1m"  };

  constexpr const char* RESET   { "\u001b[0m"  };

#endif

  constexpr const char* COLORS[17]{
     BLACK,  RED,  GREEN,  YELLOW,  BLUE,  MAGENTA,  CYAN, WHITE,
    xBLACK, xRED, xGREEN, xYELLOW, xBLUE, xMAGENTA, xCYAN, xWHITE, RESET
  };

}

#endif // COLOR_H_INCLUDED
