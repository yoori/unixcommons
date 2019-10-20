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
#include <cstdlib>

#include <dlfcn.h>


/*
  Keep one reference to libACE.so (see ADSC-2465).
*/
static
void
__attribute__((__constructor__))
init()
{
  if (!dlopen("libACE.so.6.2.1", RTLD_LAZY | RTLD_LOCAL))
  {
    std::cout << "PreloadACE: dlopen(): " << dlerror() << std::endl;
    std::exit(1);
  }
}
