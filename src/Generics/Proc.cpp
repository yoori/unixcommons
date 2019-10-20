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



#include <stdio.h>
#include <unistd.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>

#include <String/StringManip.hpp>

#include <Generics/Proc.hpp>


namespace Generics
{
  namespace Proc
  {
    bool
    memory_status(unsigned long& vsize, unsigned long& rss)
      throw ()
    {
      char buf[1024];
      {
        sprintf(buf, "/proc/%u/stat", getpid());
        FILE* file = fopen(buf, "r");
        if (!file)
        {
          return false;
        }
        fgets(buf, sizeof(buf), file);
        fclose(file);
      }
      const char* ptr = buf;
      for (int i = 0; i < 22; i++)
      {
        ptr = strchr(ptr + 1, ' ');
        if (!ptr)
        {
          return false;
        }
      }
      long reported_rss;
      if (sscanf(ptr, "%20lu%20ld", &vsize, &reported_rss) != 2)
      {
        return false;
      }
      rss = (static_cast<unsigned long>(reported_rss) + 3) * getpagesize();
      return true;
    }

    void
    backtrace(char* buf, size_t size, size_t from, size_t to)
      throw ()
    {
      if (!size)
      {
        return;
      }
      *buf = '\0';

      static const size_t SIZE = 10;
      void* ptrs[SIZE];
      size_t st = std::min<size_t>(::backtrace(ptrs, SIZE), to);
      for (size_t i = from; i < st; i++)
      {
        Dl_info in;
        char* al = 0;
        const char* name = "??";
        uintptr_t d = 0;
        if (dladdr(ptrs[i], &in))
        {
          if (in.dli_sname)
          {
            int status;
            name = in.dli_sname;
            al = __cxxabiv1::__cxa_demangle(name, 0, 0, &status);
            if (al && !status)
            {
              name = al;
            }
            if (in.dli_saddr)
            {
              d = reinterpret_cast<uintptr_t>(ptrs[i]) -
                reinterpret_cast<uintptr_t>(in.dli_saddr);
            }
          }
          else if (in.dli_fname)
          {
            name = in.dli_fname;
            if (in.dli_fbase)
            {
              d = reinterpret_cast<uintptr_t>(ptrs[i]) -
                reinterpret_cast<uintptr_t>(in.dli_fbase);
            }
          }
        }
        String::StringManip::strlcat(buf, name, size);
        if (d)
        {
          char b[64];
          snprintf(b, sizeof(b), "+0x%X", static_cast<unsigned>(d));
          String::StringManip::strlcat(buf, b, size);
        }
        String::StringManip::strlcat(buf, ";", size);
        if (al)
        {
          free(al);
        }
      }
    }
  }
}
