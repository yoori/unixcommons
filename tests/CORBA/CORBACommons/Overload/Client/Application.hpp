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



#ifndef CORBA_OVERLOAD_TEST_APPLICATION_HPP
#define CORBA_OVERLOAD_TEST_APPLICATION_HPP

#include <eh/Exception.hpp>

class Application
{
public:
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    
  void
  run(int argc, char* argv[]) throw(Exception, eh::Exception);
};

#endif /* CORBA_OVERLOAD_TEST_APPLICATION_HPP */
