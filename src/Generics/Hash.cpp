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



#define __STDC_LIMIT_MACROS
#include <cstdint>
#include <endian.h>

#include <Generics/Hash.hpp>


#if SIZE_MAX != 18446744073709551615UL
#error std::size_t type must hold exactly 2^64 values.
#endif

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error Only little-endian platforms are supported.
#endif


namespace Generics
{
  namespace HashHelper
  {
    const std::size_t Murmur64::MULTIPLIER_;
    const std::size_t Murmur64::R_;
  }
}
