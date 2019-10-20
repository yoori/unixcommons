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
#include <algorithm>
#include <vector>

#include <eh/Exception.hpp>

#include <Logger/FileLogger.hpp>
#include <Logger/StreamLogger.hpp>

#include <TestCommons/CheckFileMessages.hpp>


using namespace Logging;

struct Config
{
  int count;
  std::string message;
  std::string file;
  int sleep;
  int time_span;
  int size_span;
  bool check_test;
  size_t preallocated;

  Config() throw ()
    : count(2000000000),
      message("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
              "aaaaaaaaaaaaaaaaaaaa"),
      file("test.log"),
      sleep(0),
        time_span(7),
      size_span(10000000),
      check_test(false),
      preallocated(0)
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

  Stat() throw() :
    calls(0),
    total_time(0),
    max_time(0),
    min_time(0)
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

struct Print
{
void
operator ()(const char* full_path, const struct stat&) throw (eh::Exception)
{
  std::cout << full_path << std::endl;
}
};

void
usage()
{
  std::cerr <<
    "TestFileLogger utility to test Logging::FileLogger class " << std::endl <<
    "functionality from Generics library" << std::endl <<
    "Usage: TestFileLogger [options]" << std::endl <<
    "  -c count     Count of log writes. Default " << config.count << "." << std::endl <<
    "  -m message   Log message. Default 'a' 120 times." << std::endl <<
    "  -f file      Log file name. Use 'cerr' for std::cerr. Default '" << config.file << "'" << std::endl <<
    "  -s sleep     Sleep time between writes, seconds. Default " << config.sleep << "." << std::endl <<
    "  -T sec       Time for span policy. Default " << config.time_span << "." << std::endl <<
    "  -S bytes     Size for span policy. Default " << config.size_span << "." << std::endl <<
    "  -p bytes     Preallocated buffer size. Default " << config.preallocated << "." << std::endl <<
    "  -t           Perform check test." << std::endl <<
    "  -h           Show this help." << std::endl;
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
  std::cerr << "Aborted by user" << std::endl;
  config.count = 0;
  signal(sig, SIG_DFL);
}

int
main(int argc, char* argv[])
{
  signal(SIGINT, sigterm_handler);

  while (1)
  {
    int opt = ::getopt(argc, argv, "c:m:f:s:hT:S:tp:");
    if (opt == -1)
    {
      break;
    }

    switch (opt)
    {
      case 'c':
        if (optarg)
        {
          config.count = atoi(optarg);
        }
        else
        {
          std::cerr << "Argument undefined for -c option" << std::endl;
          return 1;
        }
        break;

      case 'm':
        if (optarg)
        {
          config.message = optarg;
        }
        else
        {
          std::cerr << "Argument undefined for -m option" << std::endl;
          return 1;
        }
        break;

      case 'f':
        if (optarg)
        {
          config.file = optarg;
        }
        else
        {
          std::cerr << "Argument undefined for -f option" << std::endl;
          return 1;
        }
        break;

      case 's':
        if (optarg)
        {
          config.sleep = atoi(optarg);
        }
        else
        {
          std::cerr << "Argument undefined for -s option" << std::endl;
          return 1;
        }
        break;

      case 'T':
        if (optarg)
        {
          config.time_span = atoi(optarg);
        }
        else
        {
          std::cerr << "Argument undefined for -T option" << std::endl;
          return 1;
        }
        break;

      case 'S':
        if (optarg)
        {
          config.size_span = atoi(optarg);
        }
        else
        {
          std::cerr << "Argument undefined for -S option" << std::endl;
          return 1;
        }
        break;

      case 'p':
        if (optarg)
        {
          config.preallocated = atoi(optarg);
        }
        else
        {
          std::cerr << "Argument undefined for -p option" << std::endl;
          return 1;
        }
        break;

      case 't':
        config.check_test = true;
        break;

      case 'h':
        usage();
        return 0;

      case ':':
        return 1;

      case '?':
        return 1;

      default:
        std::cerr << "Unexpected getopt result " << opt <<
          " (" << (char)opt << ")" << std::endl;
    }

  }


  try
  {
    File::Policies::PolicyList plist;

    if (config.time_span)
    {
      File::Policies::Policy_var time_span(
        new File::Policies::TimeSpanPolicy(
          Generics::Time(config.time_span)));
      plist.push_back(time_span);
    }

    if (config.size_span)
    {
      File::Policies::Policy_var size_span(
        new File::Policies::SizeSpanPolicy(config.size_span));
      plist.push_back(size_span);
    }

    QLogger_var logger;

    if (strcmp(config.file.c_str(), "cerr") == 0)
    {
      config.check_test = false;
      logger = new OStream::Logger(OStream::Config(std::cout, Logger::DEBUG));
    }
    else
    {
      File::Config file_config(config.file.c_str(), plist, Logger::DEBUG);
      file_config.preallocated_size = config.preallocated;
      logger = new File::Logger(std::move(file_config));
    }

    int i = 0;

    Generics::ArrayChar buffer(16 * 1024 * 1024);

    TestCommons::CheckFileMessages check;

    for (; i < config.count; i++)
    {
      {
        Stream::Buffer<16 * 1024 * 1024> ostr(buffer.get());
        ostr << "message [" << i << "] " << config.message;
      }

      const char* text = buffer.get();

      if (config.check_test)
      {
        check.add_message();
      }

      Generics::Timer timer;
      timer.start();
      bool res = logger->log(String::SubString(text), Logger::NOTICE, "TestApp");
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

    print_stat();

    logger.reset();

    if (config.count && config.check_test)
    {
      check.check(config.file, config.size_span, config.time_span);
    }
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
    std::cerr << "Unknown exception." << std::endl;
    return 1;
  }

  return 0;
}
