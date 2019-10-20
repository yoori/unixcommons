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



#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Logger/Syslog.hpp>


namespace Logging
{
  namespace Syslog
  {
    namespace Helper
    {
      //
      // Connection class
      //

      static Sync::PosixMutex mutex;
      Connection* Connection::connection_;

      Connection::Connection(Config&& config) throw (eh::Exception)
        : config_(std::move(config))
      {
        config_.formatter.reset();
        openlog(config.openlog_identity.empty() ? 0 :
          config.openlog_identity.c_str(),
          config.openlog_option, config.openlog_facility);
      }

      Connection::~Connection() throw ()
      {
        closelog();
      }

      void
      Connection::delete_this_() const throw ()
      {
        Sync::PosixGuard guard(mutex);
        add_ref();
        if (remove_ref_no_delete_())
        {
          connection_ = 0;
          AtomicImpl::delete_this_();
        }
      }

      Connection*
      Connection::connection(Config&& config)
        throw (eh::Exception, Handler::Exception)
      {
        Sync::PosixGuard guard(mutex);

        if (connection_)
        {
          if (config.openlog_option != connection_->config_.openlog_option ||
            config.openlog_facility !=
              connection_->config_.openlog_facility ||
            config.openlog_identity != connection_->config_.openlog_identity)
          {
            Stream::Error ostr;
            ostr << FNS << "different connection configuration";
            throw Handler::Exception(ostr);
          }

          connection_->add_ref();
        }
        else
        {
          connection_ = new Connection(std::move(config));
        }
        return connection_;
      }


      //
      // Handler class
      //

      inline
      void
      Handler::publish(const LogRecord& record)
        throw (Exception, eh::Exception)
      {
        FormatWrapper::Result line(formatter_.format(record));
        if (char* str = const_cast<char*>(line.get()))
        {
          for (char* ptr = str; *ptr; ++ptr)
          {
            if (*ptr == '\n' || *ptr == '\r')
            {
              *ptr = ' ';
            }
            if (ptr - str == 7 * 1024)
            {
              ptr[-4] = ' ';
              ptr[-3] = '.';
              ptr[-2] = '.';
              ptr[-1] = '.';
              *ptr = '\0';
              break;
            }
          }
  
          static const int CONVERT_TO_SYSLOG_SEVERITY[] =
          {
            LOG_ALERT, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING,
            LOG_NOTICE, LOG_INFO
          };

          int priority =
            (record.severity < sizeof(CONVERT_TO_SYSLOG_SEVERITY) 
              / sizeof(CONVERT_TO_SYSLOG_SEVERITY[0]) ?
                CONVERT_TO_SYSLOG_SEVERITY[record.severity] : LOG_DEBUG);

          syslog(priority, "%s", str);
        }
      }
    }
  }
}
