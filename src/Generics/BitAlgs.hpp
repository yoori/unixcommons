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



#ifndef GENERICS_BITALGS_HPP
#define GENERICS_BITALGS_HPP

#include <cstdint>


namespace Generics
{
  namespace BitAlgs
  {
    /**
     * Calculates the lowest significant bit.
     * @param n number to process
     * @result the lowest significant bit position (0-63) or 64 for zero
     */
    inline
    unsigned
    lowest_bit_64(uint64_t n) throw ()
    {
      uint64_t r, t;
      __asm__(
        "movq $64, %1\n"
        "bsfq %2, %0\n"
        "cmoveq %1, %0\n"
        : "=&r" (r), "=&r" (t)
        : "rm" (n));
      return r;
    }

    /**
     * Calculates the lowest significant bit.
     * @param n number to process
     * @result the lowest significant bit position (0-31) or 32 for zero
     */
    inline
    unsigned
    lowest_bit_32(uint32_t n) throw ()
    {
      uint64_t r, t;
      __asm__(
        "movl $32, %%edx\n"
        "bsfl %2, %%eax\n"
        "cmoveq %%rdx, %%rax\n"
        : "=&a" (r), "=&d" (t)
        : "rm" (n));
      return r;
    }

    /**
     * Calculates the highest significant bit.
     * @param n number to process
     * @result the highest significant bit position (0-63) or 64 for zero
     */
    inline
    unsigned
    highest_bit_64(uint64_t n) throw ()
    {
      uint64_t r, t;
      __asm__(
        "movq $64, %1\n"
        "bsrq %2, %0\n"
        "cmoveq %1, %0\n"
        : "=&r" (r), "=&r" (t)
        : "rm" (n));
      return r;
    }

    /**
     * Calculates the highest significant bit.
     * @param n number to process
     * @result the highest significant bit position (0-31) or 32 for zero
     */
    inline
    unsigned
    highest_bit_32(uint32_t n) throw ()
    {
      uint64_t r, t;
      __asm__(
        "movl $32, %%edx\n"
        "bsrl %2, %%eax\n"
        "cmoveq %%rdx, %%rax\n"
        : "=&a" (r), "=&d" (t)
        : "rm" (n));
      return r;
    }

    /**
     * Leaves only the highest significant bit.
     * @param n number to process
     * @result number with the only (or none for zero) bit set.
     */
    inline
    uint64_t
    leave_highest_64(uint64_t n) throw ()
    {
      uint64_t r, t;
      __asm__(
        "xorq %0, %0\n"
        "bsrq %2, %1\n"
        "btsq %1, %0\n"
        "cmoveq %2, %0\n"
        : "=&r" (r), "=&r" (t)
        : "rm" (n));
      return r;
    }
  }
};

#endif
