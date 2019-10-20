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



// Generics/Trace.hpp
#ifndef GENERICS_TRACE_HPP
#define GENERICS_TRACE_HPP

#ifdef BUILD_WITH_DEBUG_MESSAGES
#include <pthread.h>
#include <iostream>
#include <sstream>

#include <Generics/Time.hpp>


namespace
{
  template <typename Function, typename Param>
  inline
  void
  trace_message(Function fun, const Param& param) throw (eh::Exception)
  {
    Generics::Time tm = Generics::Time::get_time_of_day();
    std::ostringstream ostr;
    ostr << " [" << tm.get_local_time() << ",tid=" << pthread_self() << "]: "
         << fun << " " << param << std::endl;
    std::cout << ostr.str() << std::flush;
  }
}
#else
#define trace_message(x, y)
#endif

#endif
