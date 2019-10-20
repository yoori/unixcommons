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



#ifndef TESTCOMMONS_MEMORY_HPP
#define TESTCOMMONS_MEMORY_HPP

#include <iostream>

#include <malloc.h>

#include <eh/Exception.hpp>


namespace TestCommons
{
  /**
   * Prints detailed information from mallinfo structure into the
   * specified stream.
   * @param ostr stream to use for output
   * @param info structure to print. If 0 - retrieve current information
   * automatically before printing.
   */
  void
  print_mallinfo(std::ostream& ostr, struct mallinfo* info = 0)
    throw (eh::Exception);
}

#endif
