/* 
 * This file is part of the UnixCommons distribution (https://github.com/yoori/unixcommons).
 * UnixCommons contains help classes and functions for Unix Server application writing
 *
 * Copyright (c) 2012 Yuri Kuznecov <yuri.kuznecov@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



#include <signal.h>
#include <iostream>
#include <sstream>

#include <Generics/Statistics.hpp>
#include <Logger/Syslog.hpp>

using namespace Generics;
using namespace Logging;

struct Config
{
  volatile sig_atomic_t count;
  std::string message;

  /// Init struct via constructor
  Config() throw();
};

Config config;

//////////////////////////////////////////////////////////////////////////
// Implementations 

Config::Config() throw()
  : count(20),
    message("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaa")
{
}

int
main(int /*argc*/, char* /*argv*/[])
{
  try
  {
    {
      FLogger_var logger(new Syslog::Logger(
        Syslog::Config(Logger::DEBUG, "TEST_SYS_LOGGER", LOG_PID, LOG_USER)));
    }

    QLogger_var logger(new Syslog::Logger(
      Syslog::Config(Logger::DEBUG, "TEST_SYS_LOGGER", LOG_PID, LOG_USER)));

    {
      FLogger_var logger(new Syslog::Logger(
        Syslog::Config(Logger::DEBUG, "TEST_SYS_LOGGER", LOG_PID, LOG_USER)));
    }

    Statistics::DumpRunner_var stat_runner(new Statistics::NullDumpRunner);
    Statistics::DumpPolicy_var stat_policy(new Statistics::NullDumpPolicy);
    Generics::Statistics::Collection_var statistics;
    statistics = new Statistics::Collection(stat_runner.in());
    const char STAT_NAME[] = "SyslogHandler";
    statistics->add(STAT_NAME,
      new Statistics::TimedStatSink(), stat_policy.in());
    Generics::Statistics::StatSink_var stat;
    stat = statistics->get(STAT_NAME);

    for (int i = 0; i < config.count; i++)
    {
      std::ostringstream ostr;
      ostr << "message [" << i << "] " << config.message;

      Generics::Timer timer;
      timer.start();
      bool res = logger->log(ostr.str(), Logger::NOTICE, "Aspect=TestApp",
        "code=code");
      timer.stop();

      stat->consider(Statistics::TimedSubject(timer.elapsed_time()));

      if (!res)
      {
        std::cerr << "Error: log_message() failed!\n";
        break;
      }

    }
    if (!config.count)
    {
      std::cerr << "Aborted by user" << std::endl;
    }

    statistics->dump(std::cout);

    logger.reset();
  }
  catch (const LoggerException& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "eh::Exception: " << e.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
    return 1;
  }

  return 0;
}
