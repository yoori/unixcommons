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



#ifndef CHECKCOMMONS_COUNTER
#define CHECKCOMMONS_COUNTER

#include <sstream>
#include <bits/atomic_word.h>

#include <eh/Exception.hpp>


namespace TestCommons
{
  class Counter
  {
  public:
    Counter() throw ();

    void
    print() const throw (eh::Exception);

    void
    print(std::ostream& ostr) const throw (eh::Exception);

    void
    success() throw ();

    void
    failure() throw ();

    int
    succeeded() const throw ();

    int
    failed() const throw ();

  private:
    volatile _Atomic_word success_, failure_;
  };
}

#include "Counter.ipp"

#endif
