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





#ifndef LOGGER_STREAM_LOGGER_HPP
#define LOGGER_STREAM_LOGGER_HPP

#include <Logger/SimpleLogger.hpp>


namespace Logging
{
  namespace OStream
  {
    namespace Helper
    {
      /**
       * Configuration for OStream Handler
       */
      struct Config
      {
        /**
         * Constructor
         */
        Config(std::ostream& output_stream, const Formatter* formatter,
          size_t preallocated_size) throw ();

        std::ostream& output_stream;
        Formatter_var formatter;
        size_t preallocated_size;
      };

      /**
       * Writes formatted log line into the stream specified.
       */
      class Handler :
        public ::Logging::Handler,
        public ReferenceCounting::DefaultImpl<>
      {
      public:
        DECLARE_EXCEPTION(BadStream, Exception);

        /**
         * Constructor
         * @param config configuration
         */
        explicit
        Handler(Config&& config) throw (eh::Exception);

        /**
         * Writes record into stream.
         * @param record Specifies log record to publish.
         */
        virtual
        void
        publish(const LogRecord& record)
          throw (BadStream, Exception, eh::Exception);

      protected:
        std::ostream& ostr_;
        FormatWrapper formatter_;
      };
    }

    struct Config :
      public Helper::Config,
      public Simple::Config
    {
      /**
       * Constructor
       * @param output_stream Stream to write to.
       * @param log_level Log level to be used for records filtering.
       * @param formatter Formatter to be used.
       * @param preallocated_size preallocated memory size for formatting
       */
      explicit
      Config(std::ostream& output_stream,
        unsigned long log_level = ::Logging::Logger::INFO,
        const Formatter* formatter = 0, size_t preallocated_size = 0)
        throw ();
    };

    /**
     * Stream logger
     */
    typedef DerivedLogger<Config, Helper::Handler> Logger;
  }
}

//
// INLINES
//

namespace Logging
{
  namespace OStream
  {
    namespace Helper
    {
      //
      // Config class
      //

      inline
      Config::Config(std::ostream& output_stream,
        const Formatter* formatter, size_t preallocated_size) throw ()
        : output_stream(output_stream),
          formatter(ReferenceCounting::add_ref(formatter)),
          preallocated_size(preallocated_size)
      {
      }


      //
      // Handler class
      //

      inline
      Handler::Handler(Config&& config) throw (eh::Exception)
        : ostr_(config.output_stream),
          formatter_(config.formatter, config.preallocated_size)
      {
      }
    }


    //
    // Config class
    //

    inline
    Config::Config(std::ostream& output_stream, unsigned long log_level,
      const Formatter* formatter, size_t preallocated_size) throw ()
      : Helper::Config(output_stream, formatter, preallocated_size),
        Simple::Config(log_level)
    {
    }
  }
}

#endif
