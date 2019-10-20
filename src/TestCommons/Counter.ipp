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



#include <iostream>

namespace TestCommons
{
  inline
  Counter::Counter() throw ()
    : success_(0), failure_(0)
  {
  }

  inline
  void
  Counter::print() const throw (eh::Exception)
  {
    print(std::cout);
  }

  inline
  void
  Counter::print(std::ostream& ostr) const throw (eh::Exception)
  {
    ostr
      << "Success: " << success_
      << " Fail: " << failure_
      << " Total: " << success_ + failure_
      << std::endl;
  }

  inline
  void
  Counter::success() throw ()
  {
    __gnu_cxx::__atomic_add(&success_, 1);
  }

  inline
  void
  Counter::failure() throw ()
  {
    __gnu_cxx::__atomic_add(&failure_, 1);
  }

  inline
  int
  Counter::succeeded() const throw ()
  {
    return success_;
  }

  inline
  int
  Counter::failed() const throw ()
  {
    return failure_;
  }
}
