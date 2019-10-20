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



#ifndef LOGGER_DESCRIPTOR_LOGGER_HPP
#define LOGGER_DESCRIPTOR_LOGGER_HPP

#include <unistd.h>

#include <Logger/SimpleLogger.hpp>


namespace Logging
{
  namespace Descriptor
  {
    namespace Helper
    {
      /**
       * Configuration for Descriptor Handler
       */
      struct Config
      {
        /**
         * Constructor
         */
        Config(const Formatter* formatter, int fd, size_t preallocated_size)
          throw ();

        Formatter_var formatter;
        int fd;
        size_t preallocated_size;
      };

      /**
       * Logger handler allowing output to the specified file descriptor
       */
      class Handler :
        public Logging::Handler,
        public ReferenceCounting::AtomicImpl
      {
      public:
        /**
         * Constructor
         * @param config handler configuration
         */
        explicit
        Handler(Config&& config)
          throw (Exception, eh::Exception);

        /**
         * Message formatting and output function
         * @param record Log record
         */
        virtual
        void
        publish(const LogRecord& record)
          throw (Exception, eh::Exception);

      protected:
        /**
         * Destructor
         */
        virtual
        ~Handler() throw ();

        /**
         * Allows children to pass file descriptor later
         * @param fd File descriptor
         */
        void
        set_fd_(int fd) throw ();

        /**
         * Closes stored file descriptor
         */
        void
        close_fd_() throw ();

      private:
        FormatWrapper formatter_;
        int fd_;
      };
    }

    /**
     * Configuration for Descriptor Logger
     */
    struct Config :
      public Helper::Config,
      public Simple::Config
    {
      /**
       * Constructor
       * @param formatter formatter to use
       * @param fd file descriptor for output
       * @param preallocated_size preallocated memory size for formatting
       */
      explicit
      Config(const Formatter* formatter = 0, int fd = -1,
        size_t preallocated_size = 0) throw ();
    };

    /**
     * Descriptor Logger
     */
    typedef DerivedLogger<Config, Helper::Handler> Logger;
  }
}

//
// INLINES
//

namespace Logging
{
  namespace Descriptor
  {
    namespace Helper
    {
      //
      // Config class
      //

      inline
      Config::Config(const Formatter* formatter, int fd,
        size_t preallocated_size) throw ()
        : formatter(ReferenceCounting::add_ref(formatter)), fd(fd),
          preallocated_size(preallocated_size)
      {
      }


      //
      // Handler class
      //

      inline
      Handler::Handler(Config&& config) throw (Exception, eh::Exception)
        : formatter_(config.formatter, config.preallocated_size),
          fd_(config.fd)
      {
      }

      inline
      void
      Handler::close_fd_() throw ()
      {
        if (fd_ != -1)
        {
          close(fd_);
          fd_ = -1;
        }
      }

      inline
      Handler::~Handler() throw ()
      {
        close_fd_();
      }

      inline
      void
      Handler::set_fd_(int fd) throw ()
      {
        fd_ = fd;
      }
    }


    //
    // Config class
    //

    inline
    Config::Config(const Formatter* formatter, int fd,
      size_t preallocated_size) throw ()
      : Helper::Config(formatter, fd, preallocated_size)
    {
    }
  }
}

#endif
