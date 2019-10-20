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



#include <iostream>
#include <fstream>
#include <vector>

#include <eh/Exception.hpp>

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Generics/Listener.hpp>

#include <Logger/FileLogger.hpp>


namespace
{
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  class Config
  {
  public:
    Config() throw ();

    void
    parse(int argc, char* argv[])
      throw (Exception, eh::Exception);

    void
    parse_descriptors(const char* descriptors)
      throw (Exception, eh::Exception);

    bool file_set;
    std::string file_name;
    int size;
    int time;
    bool when_set;
    int hour;
    int minute;
    int second;
    bool local_tz;
    Logging::Logger::Severity severity;
    std::vector<int> descriptors;
    bool format_descriptors;
    std::string pid_file;
  };


  class PassThroughFormatter :
    public Logging::Formatter,
    public ReferenceCounting::AtomicImpl
  {
  public:
    virtual
    size_t
    required_size(const Logging::LogRecord& record) const
      throw (Exception, eh::Exception);

    virtual
    bool
    format(const Logging::LogRecord& record, char* buf, size_t size) const
      throw (Exception, eh::Exception);
  };

  class ListenerCallback :
    public Generics::DescriptorListenerCallback,
    public ReferenceCounting::AtomicImpl
  {
  public:
    ListenerCallback(Logging::Logger* logger,
      const Logging::Formatter* non_stdin_formatter,
      Logging::Logger::Severity used_severity) throw ();

    virtual
    void
    report_error(Severity severity, const String::SubString& description,
      const char* error_code = 0) throw ();

    virtual
    void
    on_data_ready(int fd, std::size_t fd_index, const char* str,
      std::size_t size) throw ();

  protected:
    virtual
    ~ListenerCallback() throw ();

  private:
    Logging::FLogger_var logger_;
    Logging::Formatter_var non_stdin_formatter_;
    Logging::Logger::Severity used_severity_;
  };

  Config::Config() throw ()
    : file_set(false), size(0), time(0), when_set(0),
      hour(0), minute(0), second(0), local_tz(false),
      severity(Logging::Logger::INFO),
      format_descriptors(false)
  {
  }

  void
  Config::parse_descriptors(const char* str) throw (eh::Exception, Exception)
  {
    if (!str)
    {
      return;
    }

    Stream::Parser istr(str, strlen(str));

    for (int descriptor; (istr >> descriptor);
      descriptors.push_back(descriptor));
  }

  void
  Config::parse(int argc, char* argv[])
    throw (Exception, eh::Exception)
  {
    for (argc--, argv++; argc > 0; argc--, argv++)
    {
      if (**argv == '-')
      {
        switch ((*argv)[1])
        {
        case '-':
          {
            const char* option = *argv + 2;
            argc--, argv++;

            if (!argc)
            {
              throw Exception("Argument is required for an option");
            }

            if (!strcmp(option, "size"))
            {
              size = atoi(*argv);
              break;
            }
            if (!strcmp(option, "time"))
            {
              time = atoi(*argv);
              break;
            }
            if (!strcmp(option, "severity"))
            {
              severity = static_cast<Logging::Logger::Severity>(atoi(*argv));
              break;
            }
            if (!strcmp(option, "cron"))
            {
              if (sscanf(*argv, "%10d:%10d:%10d", &hour, &minute, &second) < 2)
              {
                throw Exception("Invalid format of time start");
              }
              when_set = true;
              break;
            }
            if (!strcmp(option, "descriptors"))
            {
              parse_descriptors(*argv);
              break;
            }
            if (!strcmp(option, "daemon"))
            {
              pid_file = *argv;
              break;
            }
          }

          throw Exception("Unknown option");

        case 'l':
          local_tz = true;
          break;

        case 'f':
          format_descriptors = true;
          break;

        default:
          throw Exception("Unknown option");
        }
      }
      else
      {
        if (file_set)
        {
          throw Exception("Log file name is set more than once");
        }

        file_name = *argv;
        file_set = true;
      }
    }

    if (!file_set)
    {
      throw Exception("Log file name is not supplied");
    }

    parse_descriptors(getenv("ROTATELOG_DESCRIPTORS"));
  }


  size_t
  PassThroughFormatter::required_size(const Logging::LogRecord& record) const
    throw (Exception, eh::Exception)
  {
    return record.text.size() + 2;
  }

  bool
  PassThroughFormatter::format(const Logging::LogRecord& record, char* buf,
    size_t size) const throw (Exception, eh::Exception)
  {
    size_t length = record.text.size();
    if (size < length + 2)
    {
      return false;
    }
    memcpy(buf, record.text.data(), length);
    buf[length] = '\n';
    buf[length + 1] = '\0';
    return true;
  }

  void
  usage() throw (eh::Exception)
  {
    std::cerr << "Usage:" << std::endl <<
      "RotateLog <log_file> " <<
      "[--size <rotate_size>] " <<
      "[--time <rotate_time> [--cron <when>] ] "
      "[-l]" << std::endl <<
      "[--descriptors <descriptors> [-f] "
      "[--severity <log messages severity>] ]" << std::endl <<
      "[--daemon <pid file>]" << std::endl <<
      "\t<log_file>    file name prefix" << std::endl <<
      "\t<rotate_size> maximum file size (megabytes)" << std::endl <<
      "\t<rotate_time> maximum file write time (minutes)" << std::endl <<
      "\t<when>        when to start file write time " <<
      "(hh:mm:ss or hh:mm format)" << std::endl <<
      "\t-l            use local time rather then GMT time " <<
      "(for file names and 'cron' rotate policy)" << std::endl <<
      "\t<descriptors> additional space separated list of descriptor "
      "numbers to listen to" << std::endl <<
      "\t-f            format input with SimpleFormatter" << std::endl <<
      "\t<severity>    severity number for log messages, "
      "see Logging::Logger::Severity enum, default = INFO" << std::endl <<
      "\t<pid file>    file to write pid into "
      "(no descriptors are closed)" << std::endl <<
      std::endl <<
      "\tROTATELOG_DESCRIPTORS environment variable the same as "
      "<descriptors>" << std::endl;
  }

  ListenerCallback::ListenerCallback(Logging::Logger* logger,
    const Logging::Formatter* non_stdin_formatter,
    Logging::Logger::Severity used_severity)
    throw ()
    : logger_(ReferenceCounting::add_ref(logger)),
      non_stdin_formatter_(ReferenceCounting::add_ref(non_stdin_formatter)),
      used_severity_(used_severity)
  {
  }

  ListenerCallback::~ListenerCallback() throw ()
  {
  }

  void
  ListenerCallback::report_error(Severity /*severity*/,
    const  String::SubString& /*description*/,
    const char* /*error_code*/) throw ()
  {
  }

  void
  ListenerCallback::on_data_ready(int fd, std::size_t /*fd_index*/,
    const char* str, std::size_t size) throw ()
  {
    if (!size)
    {
      return;
    }

    if (str[size - 1] == '\n')
    {
      size--;
    }

    if (fd == STDIN_FILENO || !non_stdin_formatter_)
    {
      logger_->stream(Logging::Logger::INFO)().write(str, size);
    }
    else
    {
      Logging::LogRecord record = { String::SubString(str, size),
        used_severity_, String::SubString(), String::SubString(),
        Generics::Time::get_time_of_day(), Generics::Time::TZ_GMT };
      char formatted[32768];
      assert(non_stdin_formatter_->required_size(record) <=
        sizeof(formatted));
      non_stdin_formatter_->format(record, formatted, sizeof(formatted));
      logger_->log(String::SubString(formatted));
    }
  }
}


int
main(int argc, char* argv[])
try
{
  Config config;

  try
  {
    config.parse(argc, argv);
  }
  catch (...)
  {
    usage();
    return -1;
  }

  ReferenceCounting::QualPtr<ListenerCallback> callback;

  {
    Logging::File::Policies::PolicyList policies;

    if (config.size)
    {
      policies.push_back(Logging::File::Policies::Policy_var(
        new Logging::File::Policies::SizeSpanPolicy(
          config.size * 1024ull * 1024)));
    }

    if (config.time)
    {
      if (config.when_set)
      {
        Generics::ExtendedTime start(2000, 1, 1,
          config.hour, config.minute, config.second, 0);
        policies.push_back(Logging::File::Policies::Policy_var(
          new Logging::File::Policies::AlignedTimeSpanPolicy(
            start, Generics::Time::ONE_MINUTE * config.time)));
      }
      else
      {
        policies.push_back(Logging::File::Policies::Policy_var(
          new Logging::File::Policies::TimeSpanPolicy(
            Generics::Time::ONE_MINUTE * config.time)));
      }
    }

    Logging::File::Config file_config(config.file_name.c_str(), policies,
      Logging::Logger::TRACE,
      Logging::Formatter_var(new PassThroughFormatter));
    file_config.time_zone =
      config.local_tz ? Generics::Time::TZ_LOCAL : Generics::Time::TZ_GMT;
    file_config.error_stream = 0;

    Logging::FLogger_var logger(
      new Logging::File::Logger(std::move(file_config)));
    Logging::Formatter_var formatter(
      config.format_descriptors ? new Logging::Simple::Formatter : 0);
    callback = new ListenerCallback(logger, formatter, config.severity);
  }

  config.descriptors.push_back(STDIN_FILENO);
  std::sort(config.descriptors.begin(), config.descriptors.end());
  config.descriptors.resize(std::unique(config.descriptors.begin(),
    config.descriptors.end()) - config.descriptors.begin());

  if (!config.pid_file.empty())
  {
    {
      Generics::DevNull dn;

      if (dup2(dn.fd(), STDOUT_FILENO) < 0 ||
        dup2(dn.fd(), STDERR_FILENO) < 0)
      {
        eh::throw_errno_exception<Exception>(
          "Failed to redirect stdout & stderr to /dev/null");
      }
    }

    switch (pid_t pid = fork())
    {
    case -1:
      return 1;
    case 0:
      setsid();
      break;
    default:
      std::ofstream pidf(config.pid_file.c_str());
      if (!pidf)
      {
        return 1;
      }
      pidf << pid;
      return 0;
    }
  }

  Generics::DescriptorListener listener(callback, &config.descriptors[0],
    config.descriptors.size(), 16384, true);
  callback->listener(&listener);
  listener.listen();
  if (!config.pid_file.empty())
  {
    unlink(config.pid_file.c_str());
  }

  return 0;
}
catch (const eh::Exception& ex)
{
  std::cerr << "RotateLog error: " << ex.what() << std::endl;
  return -1;
}
catch (...)
{
  std::cerr << "RotateLog error" << std::endl;
  return -2;
}
