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
#include <limits>

#include <eh/Exception.hpp>

#include <Generics/BitAlgs.hpp>
#include <Generics/Rand.hpp>


template <typename Type>
Type
generate_lowest(unsigned bit) throw ()
{
  const unsigned BITS = std::numeric_limits<Type>::digits;
  Type number = static_cast<Type>(1) << bit;
  for (unsigned bits = Generics::safe_rand(4); bits--;)
  {
    number |= static_cast<Type>(1) << Generics::safe_rand(bit, BITS - 1);
  }
  return number;
}

template <typename Type>
Type
generate_highest(unsigned bit) throw ()
{
  Type number = static_cast<Type>(1) << bit;
  for (unsigned bits = Generics::safe_rand(4); bits--;)
  {
    number |= static_cast<Type>(1) << Generics::safe_rand(bit);
  }
  return number;
}

template <typename Type, typename Lowest, typename Highest>
void
test_type(const char* type, Lowest lowest, Highest highest)
  throw (eh::Exception)
{
  const unsigned BITS = std::numeric_limits<Type>::digits;
  unsigned res;

  res = lowest(0);
  if (res != BITS)
  {
    std::cerr << "lowest " << type << " for 0 failed " << res << std::endl;
  }
  res = highest(0);
  if (res != BITS)
  {
    std::cerr << "highest " << type << " for 0 failed " << res << std::endl;
  }
  Type al = Generics::BitAlgs::leave_highest_64(0);
  if (al != 0)
  {
    std::cerr << "leave_highest for 0 failed " << al << std::endl;
  }
  for (int i = 0; i < 1000; i++)
  {
    unsigned bit = Generics::safe_rand(BITS);
    Type value = generate_lowest<Type>(bit);
    res = lowest(value);
    if (res != bit)
    {
      std::cerr << "lowest " << type << " for " << value << " (" <<
        bit << ") failed " << res << std::endl;
    }
    value = generate_highest<Type>(bit);
    res = highest(value);
    if (res != bit)
    {
      std::cerr << "highest " << type << " for " << value << " (" <<
        bit << ") failed " << res << std::endl;
    }
    al = Generics::BitAlgs::leave_highest_64(value);
    if (al != (static_cast<Type>(1) << bit))
    {
      std::cerr << "leave_highest for " << value << " (" <<
        bit << ") failed " << al << std::endl;
    }
  }
}

void
test() throw (eh::Exception)
{
  test_type<uint64_t>("uint64_t", Generics::BitAlgs::lowest_bit_64,
    Generics::BitAlgs::highest_bit_64);
  test_type<uint32_t>("uint32_t", Generics::BitAlgs::lowest_bit_32,
    Generics::BitAlgs::highest_bit_32);
}

int
main()
{
  test();
  return 0;
}
