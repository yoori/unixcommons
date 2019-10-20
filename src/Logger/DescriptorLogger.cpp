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



#include <eh/Errno.hpp>

#include <Logger/DescriptorLogger.hpp>


namespace Logging
{
  namespace Descriptor
  {
    namespace Helper
    {
      //
      // Handler class
      //

      void
      Handler::publish(const LogRecord& record)
        throw (Exception, eh::Exception)
      {
        FormatWrapper::Result line(formatter_.format(record));
        if (!line.get())
        {
          Stream::Error ostr;
          ostr << FNS << "failed to format message";
          throw Exception(ostr);
        }

        for (ssize_t offset = 0, length = strlen(line.get()); length;)
        {
          ssize_t res = write(fd_, line.get() + offset, length);
          if (res == 0)
          {
            Stream::Error ostr;
            ostr << FNS << "nothing has been written";
            throw Exception(ostr);
          }
          if (res < 0)
          {
            if (errno == EINTR)
            {
              continue;
            }
            eh::throw_errno_exception<Exception>(FNE,
              "Failed to write");
          }
          length -= res;
          offset += res;
        }
      }
    }
  }
}
