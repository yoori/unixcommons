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





#ifndef LOGGER_SIMPLE_LOGGER_HPP
#define LOGGER_SIMPLE_LOGGER_HPP

#include <iostream>
#include <signal.h>

#include <Logger/Logger.hpp>


namespace Logging
{
  namespace Simple
  {
    /**
     * Configuration for Simple Logger
     */
    struct Config
    {
      /**
       * Constructor
       * @param log_level Log level to be used for records filtering.
       * @param time_zone Time zone to be used for LogRecord.time assigning.
       * @param error_stream Stream to use for log function faults outputting.
       * can be 0.
       */
      explicit
      Config(unsigned long log_level = ::Logging::Logger::INFO,
        Generics::Time::TimeZone time_zone = Generics::Time::TZ_GMT,
        std::ostream* error_stream = &std::cerr) throw ();

      unsigned long log_level;
      Generics::Time::TimeZone time_zone;
      std::ostream* error_stream;
    };

    /**
     * Simple logger implementation, which creates log record from
     * information passed to log function, produces time in timezone
     * specified and passes it to log handler provided to constructor.
     */
    class Logger : public ::Logging::Logger
    {
    public:
      /**
       * Constructor
       * @param handler Log backend to be used. Reference counter will
       * be incremented.
       * @param config configuration
       */
      Logger(Handler* handler, Config&& config)
        throw (eh::Exception);

      /**
       * Gets logger trace level.
       * @return Returns current trace level.
       */
      virtual
      unsigned long
      log_level() throw ();

      /**
       * Sets logger trace level.
       * @param value Defines new log level.
       */
      virtual
      void
      log_level(unsigned long value) throw ();

      /**
       * Logs text with severity and aspect specified.
       * @param text Specifies text to be logged.
       * @param severity Specify log record severity.
       * @param aspect Specify log record aspect.
       * @param code Specify log record code.
       * @return Returns true on success.
       */
      virtual
      bool
      log(const String::SubString& text, unsigned long severity = INFO,
        const char* aspect = 0, const char* code = 0) throw ();

    protected:
      /**
       * Destructor
       */
      virtual
      ~Logger() throw ();


      mutable Sync::PosixMutex lock_;

      Handler_var handler_;
      volatile sig_atomic_t log_level_;
      Generics::Time::TimeZone time_zone_;
      std::ostream* error_stream_;
    };

    /**
     * Simple formatter for log record.
     * Converts log record into plain text, optionally prepending initial
     * line with time, severity, aspect, code, pid and tid
     */
    class Formatter :
      public ::Logging::Formatter,
      public ReferenceCounting::DefaultImpl<>
    {
    public:
      /**
       * Constructor
       * @param log_time Prepend log text with log time if true.
       * @param log_severity Prepend log text with severity if true.
       * @param log_aspect Prepend log text with aspect if true.
       * @param log_code Prepend log text with code if true.
       * @param log_thread_id Prepend log text with tid if true.
       * @param log_process_id Prepend log text with pid if true.
       */
      explicit
      Formatter(bool log_time = true, bool log_severity = true,
        bool log_aspect = true, bool log_code = true,
        bool log_thread_id = false, bool log_process_id = false)
        throw ();

      /**
       * Calculates the size for log record to format
       * @param record log record
       * @return required memory size
       */
      virtual
      size_t
      required_size(const LogRecord& record) const
        throw (Exception, eh::Exception);

      /**
       * Converts record into text string.
       * @param record Specifies log record to format.
       * @param buf external memory start
       * @param size external memory size
       * @return whether or not log record has been formatted
       */
      virtual
      bool
      format(const LogRecord& record, char* buf, size_t size) const
        throw (Exception, eh::Exception);

    protected:
      virtual
      ~Formatter() throw ();

      bool log_time_;
      bool log_severity_;
      bool log_aspect_;
      bool log_code_;
      bool log_thread_id_;
      bool log_process_id_;
    };
  }

  /**
   * Implementation of many loggers deriving from Simple::Logger
   * with custom Handler (and thus with custom Config)
   */
  template <typename Config, typename Handler>
  class DerivedLogger :
    public Simple::Logger,
    public ReferenceCounting::AtomicImpl
  {
  public:
    /**
     * Constructor
     * Creates custom handler passing config both to Simple::Logger
     * and custom handler
     * @param config configuration for logger
     */
    explicit
    DerivedLogger(Config&& config) throw (eh::Exception);

  protected:
    /**
     * Destructor
     */
    virtual
    ~DerivedLogger() throw ();
  };
}

//
// INLINES
//

namespace Logging
{
  namespace Simple
  {
    //
    // Config class
    //

    inline
    Config::Config(unsigned long log_level,
      Generics::Time::TimeZone time_zone, std::ostream* error_stream)
      throw ()
      : log_level(log_level), time_zone(time_zone), error_stream(error_stream)
    {
    }


    //
    // Logger class
    //

    inline
    Logger::Logger(Handler* handler, Config&& config)
      throw (eh::Exception)
      : handler_(ReferenceCounting::add_ref(handler)),
        log_level_(config.log_level), time_zone_(config.time_zone),
        error_stream_(config.error_stream)
    {
    }

    inline
    Logger::~Logger() throw ()
    {
    }

    inline
    unsigned long
    Logger::log_level() throw ()
    {
      return log_level_;
    }

    inline
    void
    Logger::log_level(unsigned long value) throw ()
    {
      log_level_ = static_cast<sig_atomic_t>(value);
    }


    //
    // Formatter class
    //

    inline
    Formatter::Formatter(bool log_time, bool log_severity, bool log_aspect,
      bool log_code, bool log_thread_id, bool log_process_id) throw ()
      : log_time_(log_time), log_severity_(log_severity),
        log_aspect_(log_aspect), log_code_(log_code),
        log_thread_id_(log_thread_id), log_process_id_(log_process_id)
    {
    }
  }


  //
  // DerivedLogger class
  //

  template <typename Config, typename Handler>
  DerivedLogger<Config, Handler>::DerivedLogger(Config&& config)
    throw (eh::Exception)
    : Simple::Logger(
        Handler_var(new Handler(std::move(config))), std::move(config))
  {
  }

  template <typename Config, typename Handler>
  DerivedLogger<Config, Handler>::~DerivedLogger() throw ()
  {
  }
}

#endif
