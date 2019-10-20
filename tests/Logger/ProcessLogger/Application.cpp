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



#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include <Generics/Time.hpp>

#include <Logger/SimpleLogger.hpp>
#include <Logger/StreamLogger.hpp>
#include <Logger/ProcessLogger.hpp>

#include <TestCommons/CheckFileMessages.hpp>


using namespace Generics;
using namespace Logging;

struct Config
{
  volatile sig_atomic_t count;
  std::string message;
  int sleep;
  std::string process;
  std::string log;
  int size;
  int time;
  bool check_test;

  Config() throw ()
    : count(20000),
      message("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
              "aaaaaaaaaaaaaaaaaaaa"),
      sleep(0),
      size(0),
      time(0),
      check_test(false)
  {
  }
};

Config config;

struct Stat
{
  int calls;
  Generics::Time total_time;
  Generics::Time max_time;
  Generics::Time min_time;

  Stat() throw()
    : calls(0), total_time(0), max_time(0), min_time(0)
  {
  }

  void
  update(const Generics::Time& time) throw()
  {
    total_time += time;
    max_time = max_time < time ? time : max_time;
    min_time = (min_time > time || !calls) ? time : min_time;
    calls++;
  }

  Generics::Time
  avg_time() throw()
  {
    if (calls > 0)
    {
      return total_time / calls;
    }
    else
    {
      return Generics::Time::ZERO;
    }
  }
};

Stat test_stat;

void
usage() throw (eh::Exception)
{
  std::cerr <<
    "TestProcessLogger utility to test Logging::ProcessLogger class " << std::endl <<
    "functionality from Generics library" << std::endl <<
    "Usage: TestProcessLogger [options]" << std::endl <<
    "  -h           Show this help." << std::endl <<
    "  -c count     Count of log writes. Default " << config.count << "." << std::endl <<
    "  -m message   Log message. Default 'a' 120 times." << std::endl <<
    "  -s sleep     Sleep time between writes, seconds. Default " << config.sleep << "." << std::endl <<
    "  -p path      Process for output. Default " << config.process << "." << std::endl <<
    "  -L path      log file" << std::endl <<
    "  -S size      either size limited" << std::endl <<
    "  -T time      or rotate interval specified" << std::endl <<
    std::endl;
}

void
print_stat()
{
  std::cout << "Test result:" << std::endl <<
               "  log calls made : " << test_stat.calls << std::endl <<
               "  total time     : " << test_stat.total_time << std::endl <<
               "  average time   : " << test_stat.avg_time() << std::endl <<
               "  max time       : " << test_stat.max_time << std::endl <<
               "  min time       : " << test_stat.min_time << std::endl <<
               std::endl;
}

void
sigterm_handler(int sig)
{
  config.count = 0;
  signal(sig, SIG_DFL);
}

int
main(int argc, char* argv[])
{
  signal(SIGINT, sigterm_handler);

  for (;;)
  {
    int opt = ::getopt(argc, argv, "c:m:s:hp:L:S:T:t");
    if (opt == -1)
    {
      break;
    }

    switch (opt)
    {
    case 'c':
      config.count = atoi(optarg);
      if (config.count <= 0)
      {
        std::cerr << "Invalid count" << std::endl;
        return -1;
      }
      break;

    case 'm':
      config.message = optarg;
      break;

    case 's':
      config.sleep = atoi(optarg);
      break;

    case 'p':
      config.process = optarg;
      break;

    case 'h':
      usage();
      return 0;

    case 'L':
      config.log = optarg;
      break;

    case 'S':
      config.size = atoi(optarg);
      break;

    case 'T':
      config.time = atoi(optarg);
      break;

    case 't':
      config.check_test = true;
      break;

    default:
      std::cerr << "Unexpected getopt result " << opt <<
                   " (" << (char)opt << ")" << std::endl;
      return -1;
    }
  }

  try
  {
    QLogger_var logger;

    if (config.process == "")
    {
      logger = new OStream::Logger(OStream::Config(std::cerr, Logger::DEBUG));
    }
    else
    {
      if (config.check_test)
      {
        char size[64];
        char time[64];
        std::vector<const char*> argv;

        argv.push_back(config.process.c_str());
        argv.push_back(config.log.c_str());
        if (config.size)
        {
          snprintf(size, sizeof(size), "%i", config.size);
          argv.push_back("--size");
          argv.push_back(size);
        }
        if (config.time)
        {
          snprintf(time, sizeof(time), "%i", config.time);
          argv.push_back("--time");
          argv.push_back(time);
        }
        argv.push_back(0);

        logger = new Process::Logger(Process::Config(argv[0],
          const_cast<char**>(&argv[0]), environ));
      }
      else
      {
        logger = new Process::Logger(
          Process::Config(config.process.c_str()));
      }
    }

    TestCommons::CheckFileMessages check;

    for (int i = 0; i < config.count; i++)
    {
      Stream::Dynamic ostr(4096);
      ostr << "message [" << i << "] " << config.message;

      if (config.check_test)
        check.add_message();

      Generics::Timer timer;
      timer.start();
      bool res = logger->log(ostr.str(), Logger::NOTICE, "TestApp");
      timer.stop();

      Generics::Time time = timer.elapsed_time();
      test_stat.update(time);
      if (!config.check_test)
      {
        std::cout << time << std::endl;
      }

      if (!res)
      {
        std::cerr << "Error: log_message() failed!\n";
        break;
      }

      if (config.sleep)
      {
        sleep(config.sleep);
      }
    }
    if (!config.count)
    {
      std::cerr << "Aborted by user" << std::endl;
    }

    print_stat();

    logger.reset();

    if (config.count && config.check_test)
    {
      check.check(config.log, config.size * 1024 * 1024, config.time * 60);
    }
  }
  catch (const Logger::Exception& e)
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
