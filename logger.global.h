                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2022.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
________________________________________________________________________________________________________________________________

 Static/global `Logger` instance accessible from everywhere

 2022.04.12  Initial version

 2022.05.16  `brief.log` and `wordy.log` assigned by default

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef LOGGER_GLOBAL_H_INCLUDED
#define LOGGER_GLOBAL_H_INCLUDED

#include "logger.h"

namespace CoreAGI {

  Logger logger{
    Semantic::MILLISEC,
    {
      { logging::Note::BRIEF, "brief.log" },
      { logging::Note::WORDY, "wordy.log" }
    }
  };

}

#endif // LOGGER_GLOBAL_H_INCLUDED
