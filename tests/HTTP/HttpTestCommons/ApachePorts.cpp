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



// ApachePorts.cpp

#include <stdlib.h>

#include "ApachePorts.hpp"

//
// class ApachePorts
//

int ApachePorts::base_port_ = 0;

inline
int
ApachePorts::get_base_port_() throw (InvalidPortRequested)
{
  if (base_port_ == 0)
  {
    char *p = getenv("USER_BASE_PORT");
    int port = 10000;
    if (p)
    {
      port = atoi(p);
    }
    if (port > 65535 || port <= 0)
    {
      Stream::Error ostr;
      ostr << "Incorrect base clients port value: USER_BASE_PORT="
        << (p? p : "null") << std::endl;
      throw InvalidPortRequested(ostr);
    }
    base_port_ = port;
  }
  return base_port_;
}

int
ApachePorts::get_port(std::size_t shift)
  throw (InvalidPortRequested)
{
  int port = ApachePorts::get_base_port_() + shift;
  if (port > 65535 || port <= 0)
  {
    Stream::Error ostr;
    ostr << "Incorrect clients port requested: USER_BASE_PORT="
      << base_port_ << ", shift=" << shift << std::endl;
    throw InvalidPortRequested(ostr);
  }
  return port;
}

std::string
ApachePorts::get_port_string(std::size_t shift)
  throw (InvalidPortRequested)
{
  char buf[6];
  std::snprintf(buf, 6, "%d", get_port(shift));
  return buf;
}
