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



#ifndef GENERICS_UNCOPYABLE_HPP
#define GENERICS_UNCOPYABLE_HPP


#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
#else
#ifndef constexpr
#define constexpr
#endif
#endif

namespace Generics
{
  /**
   * Private inheritance of this class makes impossible the usage of
   * the implicit copy constructor and the assignment operator of the
   * derived class
   */
  class Uncopyable
  {
  protected:
    constexpr
    Uncopyable() = default;
    ~Uncopyable() = default;

    Uncopyable(Uncopyable&) = delete;
    Uncopyable(const Uncopyable&) = delete;
    Uncopyable(Uncopyable&&) = delete;
    Uncopyable(const Uncopyable&&) = delete;
    void
    operator =(Uncopyable&) = delete;
    void
    operator =(const Uncopyable&) = delete;
    void
    operator =(Uncopyable&&) = delete;
    void
    operator =(const Uncopyable&&) = delete;
  };
}

#endif
