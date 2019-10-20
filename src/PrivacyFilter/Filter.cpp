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



#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include <openssl/sha.h>

#include <PrivacyFilter/Filter.hpp>


namespace
{
  const unsigned char EXPECTED_HASH[512 / 8] =
  {
#include "../../Data/Filter_ExpectedHash.ipp"
  };

  class Filter
  {
  public:
    Filter() throw ();

    bool
    filter() const throw ();

  private:
    bool
    check_file(const char* file) throw ();

    bool filter_;
  };


  Filter::Filter() throw ()
    : filter_(true)
  {
    check_file(getenv("loglevel_control")) ||
      check_file("loglevel.control");
  }

  inline
  bool
  Filter::filter() const throw ()
  {
    return filter_;
  }

  bool
  Filter::check_file(const char* file) throw ()
  {
    if (!file)
    {
      return false;
    }

    int fd = open(file, O_RDONLY);
    if (fd == -1)
    {
      return false;
    }

    unsigned char buf[16384];

    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    for (;;)
    {
      ssize_t size = read(fd, buf, sizeof(buf));
      if (size <= 0)
      {
        break;
      }
      SHA512_Update(&ctx, buf, size);
    }
    memset(buf, 0, sizeof(buf));
    SHA512_Final(buf, &ctx);

    close(fd);

    filter_ = memcmp(buf, EXPECTED_HASH, sizeof(EXPECTED_HASH));

    return true;
  }

  // Using global variable as singleton
  Filter global_filter;
}

namespace PrivacyFilter
{
  bool
  filter() throw ()
  {
    return global_filter.filter();
  }

  const char*
  filter(const char* original_message, const char* replace_message)
    throw ()
  {
    return global_filter.filter() ? replace_message : original_message;
  }

  const String::SubString&
  filter(const String::SubString& original_message,
    const String::SubString& replace_message)
    throw ()
  {
    return global_filter.filter() ? replace_message : original_message;
  }
}
