                                                                                                                              /*
 Copyright Mykola Rabchevskiy 2023.
 Distributed under the Boost Software License, Version 1.0.
 (See http://www.boost.org/LICENSE_1_0.txt)
______________________________________________________________________________

 Configuration parameters for CoreAGI project.

________________________________________________________________________________________________________________________________
                                                                                                                              */
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

namespace CoreAGI {

  namespace Config {

    namespace logger {
      constexpr unsigned    LOG_RECORD_CAPACITY    {  256 }; // :max length of log record
      constexpr unsigned    LOGGER_QUEUE_CAPACITY  { 4096 };
      constexpr unsigned    CHANNEL_QUEUE_CAPACITY {  512 };
      constexpr unsigned    CHANNEL_NAME_CAPACITY  {   16 }; // :including End-Of-Line symbol
      constexpr unsigned    CHANNEL_CAPACITY       {   48 }; // :max number of channels
      constexpr unsigned    FORMAT_CAPACITY        {   64 }; // :max length of record format
      constexpr unsigned    PATH_CAPACITY          {  256 }; // :max length of log file path
      constexpr unsigned    NO_JOB_PAUSE           {   50 }; // :pause at `no request` situation, millisec     // [+] 2021.06.08

      constexpr const char* DEFAULT_FORMAT { " %8.1f [ %-10s ] %s" }; // :time, channel name, message

    }

  }//namespace Config

}//namespace CoreAGI

#endif // CONFIG_H_INCLUDED
