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





#ifndef LOGGER_SYS_LOGGER_HPP
#define LOGGER_SYS_LOGGER_HPP

#include <syslog.h>

#include <Logger/SimpleLogger.hpp>


namespace Logging
{
  namespace Syslog
  {
    namespace Helper
    {
      /**
       * Configuration for Syslog Handler
       */
      struct Config
      {
        /**
         * Constructor
         */
        Config(const Formatter* formatter, const char* openlog_identity,
          int openlog_option, int openlog_facility, size_t preallocated_size)
          throw (eh::Exception);

        Formatter_var formatter;
        std::string openlog_identity;
        int openlog_option;
        int openlog_facility;
        size_t preallocated_size;
      };

      /**
       * Connection to syslog
       */
      class Connection : public ReferenceCounting::AtomicImpl
      {
      public:
        /**
         * Constructor
         * @param config configuration
         */
        explicit
        Connection(Config&& config) throw (eh::Exception);

        /**
         * Creates a new instance of connection or checks configuration
         * against existing
         */
        static
        Connection*
        connection(Config&& config)
          throw (eh::Exception, Handler::Exception);

      protected:
        /**
         * Destructor
         */
        virtual
        ~Connection() throw ();

        virtual
        void
        delete_this_() const throw ();

      private:
        static Connection* connection_;

        Config config_;
      };
      typedef ReferenceCounting::FixedPtr<Connection> Connection_var;

      /**
       * Directs log requests in system log.
       */
      class Handler :
        public Logging::Handler,
        public ReferenceCounting::DefaultImpl<>
      {
      public:
        /**
         * Constructor
         * @param config configuration
         */
        explicit
        Handler(Config&& config) throw (eh::Exception);

        /**
         * Writes record into the system log
         * @param record Specifies log record to publish.
         */
        virtual
        void
        publish(const LogRecord& record) throw (Exception, eh::Exception);

      protected:
        /**
         * Destructor
         */
        virtual
        ~Handler() throw ();

      private:
        FormatWrapper formatter_;
        Connection_var connection_;
      };
    }

    /**
     * Configuration for Syslog Logger
     */
    struct Config :
      public Helper::Config,
      public Simple::Config
    {
      /**
       * Constructor
       * @param log_level log level for the logger
       * @param openlog_identity Parameter to do openlog system call 
       * The string pointed to by openlog_identity is prepended to every
       * message, and is typically set to the program name
       * @param openlog_option Parameter to do openlog system call
       * @param openlog_facility Parameter to do openlog system call
       * @param formatter Pointer to object that can format log messages
       * @param preallocated_size if a preallocated buffer of the specified
       * size should be used for formatting
       */
      explicit
      Config(unsigned long log_level = ::Logging::Logger::INFO,
        const char* openlog_identity = "",
        int openlog_option = LOG_PID | LOG_CONS,
        int openlog_facility = LOG_USER,
        const Formatter* formatter = 0, size_t preallocated_size = 0)
          throw (eh::Exception);
    };

    /**
     * Syslog Logger
     */
    typedef DerivedLogger<Config, Helper::Handler> Logger;
  }
}

//
// INLINES
//

namespace Logging
{
  namespace Syslog
  {
    namespace Helper
    {
      //
      // Config class
      //

      inline
      Config::Config(const Formatter* formatter,
        const char* openlog_identity, int openlog_option,
        int openlog_facility, size_t preallocated_size)
        throw (eh::Exception)
        : formatter(ReferenceCounting::add_ref(formatter)),
          openlog_identity(openlog_identity),
          openlog_option(openlog_option),
          openlog_facility(openlog_facility),
          preallocated_size(preallocated_size)
      {
      }


      //
      // Handler class
      //

      inline
      Handler::Handler(Config&& config) throw (eh::Exception)
        : formatter_(config.formatter ? std::move(config.formatter) :
            Formatter_var(
              new Simple::Formatter(false, false, true, true)),
            config.preallocated_size),
          connection_(Connection::connection(std::move(config)))
      {
      }

      inline
      Handler::~Handler() throw ()
      {
      }
    }


    //
    // Config class
    //

    inline
    Config::Config(unsigned long log_level, const char* openlog_identity,
      int openlog_option, int openlog_facility, const Formatter* formatter,
      size_t preallocated_size) throw (eh::Exception)
      : Helper::Config(formatter, openlog_identity, openlog_option,
        openlog_facility, preallocated_size),
        Simple::Config(log_level)
    {
    }
  }
}

#endif
