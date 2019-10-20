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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <eh/Errno.hpp>

#include <Generics/ArrayAutoPtr.hpp>

#include <Logger/ProcessLogger.hpp>


namespace Logging
{
  namespace Process
  {
    namespace Helper
    {
      //
      // Handler class
      //

      Handler::Handler(Config&& config) throw (Exception, eh::Exception)
        : Descriptor::Helper::Handler(std::move(config)),
          WAIT_FOR_CHILD_(config.wait_for_child)
      {
        if (init_())
        {
          if (!config.argv)
          {
            const char* argv[4] =
              { "sh", "-c", config.command_path.c_str(), 0 };
            execve("/bin/sh", const_cast<char**>(argv), environ);
          }
          else
          {
            execve(config.command_path.c_str(), config.argv, config.envp);
          }
          _exit(-1);
        }
      }

      Handler::~Handler() throw ()
      {
        if (child_ != -1)
        {
          close_fd_();
          if (WAIT_FOR_CHILD_)
          {
            int status;
            waitpid(child_, &status, 0);
          }
        }
      }

      void
      Handler::make_sockets_(int* read_descriptor, int* write_descriptor)
        throw (Exception, eh::Exception)
      {
        int pipe_descriptors[2];
        if (pipe(pipe_descriptors) == -1)
        {
          eh::throw_errno_exception<Exception>(FNE,
            "failed to create sockets");
        }

        *read_descriptor = pipe_descriptors[0];
        *write_descriptor = pipe_descriptors[1];
      }

      bool
      Handler::make_fork_() throw (Exception, eh::Exception)
      {
        switch (child_ = fork())
        {
        case -1:
          break;
        case 0:
          return true;
        default:
          return false;
        }

        eh::throw_errno_exception<Exception>(FNE,
          "failed to fork");
        return false; //Never executed
      }

      bool
      Handler::init_() throw (Exception, eh::Exception)
      {
        int reader, writer;
        bool child;

        make_sockets_(&reader, &writer);
        try
        {
          child = make_fork_();
        }
        catch (...)
        {
          close(reader);
          close(writer);
          throw;
        }

        if (child)
        {
          close(writer);
          if (reader != STDIN_FILENO)
          {
            dup2(reader, STDIN_FILENO);
            close(reader);
          }
          return true;
        }

        signal(SIGPIPE, SIG_IGN);
        close(reader);
        set_fd_(writer);
        return false;
      }

      void
      Handler::publish(const LogRecord& record)
        throw (Exception, eh::Exception)
      {
        try
        {
          Descriptor::Helper::Handler::publish(record);
        }
        catch (...)
        {
          if (child_ == -1)
          {
            throw;
          }

          pid_t child = child_;
          child_ = -1;
          int status;
          if (waitpid(child, &status, WNOHANG) == -1)
          {
            throw;
          }
          Stream::Error ostr;
          ostr << FNS << "child terminated with status " << status;
          throw Exception(ostr);
        }
      }
    }
  }
}
