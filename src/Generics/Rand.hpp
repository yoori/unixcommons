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





#ifndef GENERICS_RAND_HPP
#define GENERICS_RAND_HPP

#include <cstdint>


namespace Generics
{
  /**
   * Thread safe service for random numbers generation.
   * Based on ISAAC generator with /dev/urandom seed.
   * @return random number in [0..RAND_MAX] range
   */
  uint32_t
  safe_rand() throw ();

  /**
   * Give uniform distribution in range [0..max_boundary-1].
   * Thread-safe.
   * @param max_boundary maximum random value.
   * @return uniformly distributed positive random variable in
   * [0, max_boundary - 1] range.
   */
  inline
  uint32_t
  safe_rand(uint32_t max_boundary) throw ()
  {
    return static_cast<uint32_t>(static_cast<double>(max_boundary) *
      safe_rand() / 2147483648.0);
  }

  /**
   * General method give uniform distribution in range.
   * Thread-safe.
   * @param min_boundary minimum random value
   * @param max_boundary maximum random value
   * @return uniformly distributed positive random variable in
   * [min_boundary, max_boundary] range.
   */
  inline
  uint32_t
  safe_rand(uint32_t min_boundary, uint32_t max_boundary) throw ()
  {
    return min_boundary + safe_rand(max_boundary - min_boundary + 1);
  }

  /**
   * Get uniform distribution in range
   * [0, 2^N-1], where N is bits_number ranged [0..31]
   * It calculated than general range method faster.
   * Use higher bits as better distributed.
   * Thread-safe.
   * @param bits_number define binary capacity of result random number.
   * @return uniformly distributed random positive variable in
   * [0, 2^bits_number-1] range.
   */
  inline
  uint32_t
  safe_integral_rand(uint8_t bits_number) throw ()
  {
    return safe_rand() >> (31 - bits_number);
  }

  /**
   * Obsolete
   */
  inline
  int
  four_digits_rand() throw ()
  {
    return safe_rand(1000, 9999);
  }
}

#endif
