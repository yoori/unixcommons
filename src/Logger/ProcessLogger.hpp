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



#ifndef LOGGER_PROCESS_LOGGER_HPP
#define LOGGER_PROCESS_LOGGER_HPP

#include <Logger/DescriptorLogger.hpp>


namespace Logging
{
  namespace Process
  {
    namespace Helper
    {
      /**
       * Configuration for Process Handler
       */
      struct Config : public Descriptor::Helper::Config
      {
        /**
         * Constructor
         */
        Config(const Formatter* formatter, const char* command,
          bool wait_for_child, size_t preallocated_size) throw ();
        /**
         * Constructor
         */
        Config(const Formatter* formatter, const char* path, char* argv[],
          char* envp[], bool wait_for_child, size_t preallocated_size)
          throw ();

        std::string command_path;
        char** argv;
        char** envp;
        bool wait_for_child;
      };

      /**
       * Logger handler allowing to start another process and send
       * log messages into it via the pipe
       * Signal handler for SIGPIPE is set to SIG_IGN in any constructor
       */
      class Handler : public Descriptor::Helper::Handler
      {
      public:
        /**
         * Constructor
         * Creates a pipe and a separate process
         * @param config configuration
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

      private:
        /**
         * Creates pipe
         * @param read_descriptor pointer to the input file descriptor
         * @param write_descriptor pointer to the output file descriptor
         */
        void
        make_sockets_(int* read_descriptor, int* write_descriptor)
          throw (Exception, eh::Exception);

        /**
         * Calls fork(2)
         * @return true, if a process is a child, false if parent
         */
        bool
        make_fork_() throw (Exception, eh::Exception);

        /**
         * Creates pipe and forks process
         * @return true, if a process is a child, false if parent
         */
        bool
        init_() throw (Exception, eh::Exception);

      private:
        const bool WAIT_FOR_CHILD_;
        pid_t child_;
      };
    }

    struct Config :
      public Helper::Config,
      public Simple::Config
    {
      /**
       * Constructor
       * To start process via execve(2) call prefixing command with "sh -c"
       * @param command Command to execute
       * @param formatter Formatter object for message formatting
       * @param wait_for_child if we should wait for child in destructor
       * @param preallocated_size if a preallocated buffer of the specified
       * size should be used for formatting
       */
      explicit
      Config(const char* command, 
        const Formatter* formatter = 0, bool wait_for_child = true,
        size_t preallocated_size = 0) throw (eh::Exception);

      /**
       * Constructor
       * To start process via execve(2) call
       * WARNING! argv and envp are stored as bare pointers
       * @param path File to execute
       * @param argv argv argument for execve call.
       * @param envp envp argument for execve call
       * @param formatter Formatter object for message formatting
       * @param wait_for_child if we should wait for child in destructor
       * @param preallocated_size if a preallocated buffer of the specified
       * size should be used for formatting
       */
      explicit
      Config(const char* path, char* argv[], char* envp[],
        const Formatter* formatter = 0, bool wait_for_child = true,
        size_t preallocated_size = 0) throw (eh::Exception);
    };

    /**
     * Process Logger
     */
    typedef DerivedLogger<Config, Helper::Handler> Logger;
  }
}

//
// INLINES
//

namespace Logging
{
  namespace Process
  {
    namespace Helper
    {
      //
      // Config class
      //

      inline
      Config::Config(const Formatter* formatter, const char* command,
        bool wait_for_child, size_t preallocated_size) throw ()
        : Descriptor::Helper::Config(formatter, -1, preallocated_size),
          command_path(command), argv(0), envp(0),
          wait_for_child(wait_for_child)
      {
      }

      inline
      Config::Config(const Formatter* formatter, const char* path,
        char* argv[], char* envp[], bool wait_for_child,
        size_t preallocated_size) throw ()
        : Descriptor::Helper::Config(formatter, -1, preallocated_size),
          command_path(path), argv(argv), envp(envp),
          wait_for_child(wait_for_child)
      {
      }
    }


    //
    // Config class
    //

    inline
    Config::Config(const char* command, 
      const Formatter* formatter, bool wait_for_child,
      size_t preallocated_size) throw (eh::Exception)
      : Helper::Config(formatter, command, wait_for_child, preallocated_size)
    {
    }

    inline
    Config::Config(const char* path, char* argv[], char* envp[],
      const Formatter* formatter, bool wait_for_child,
      size_t preallocated_size) throw (eh::Exception)
      : Helper::Config(formatter, path, argv, envp, wait_for_child,
          preallocated_size)
    {
    }
  }
}

#endif
