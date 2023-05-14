                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2022.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
 ______________________________________________________________________________


 2022.01.24  Initial version

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef CHRONOS_H_INCLUDED
#define CHRONOS_H_INCLUDED

#include <chrono>

namespace CoreAGI {

  class Chronos {
                                                                                                                              /*
    Time in nanosec starting from instance time creation:
                                                                                                                              */
    using Clock = std::chrono::steady_clock;
    const std::chrono::time_point< Clock > to;
  public:
    Chronos(): to{ Clock::now() }{}
    double nanosec() const { return std::chrono::duration_cast< std::chrono::nanoseconds >( Clock::now() - to ).count(); }
  };//class Chronos

}//namespace CoreAGI

#endif // CHRONOS_H_INCLUDED
