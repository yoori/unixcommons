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



// Additional.hpp
#ifndef _PERFORMANCE_TEST_ADDITIONAL_HPP_INCLUDED_
#define _PERFORMANCE_TEST_ADDITIONAL_HPP_INCLUDED_

#include <numeric>
#include <cmath>
#include <vector>

namespace Test
{
  /**
   * Conducts tests and
   * @return sample mean value
   * @param std_dev = standard deviation of sample
   * @param rep_count length of sampling for statistic.
   * @param fun measure functor, that give results.
   */
  template<class Functor>
  inline double
  acc_avg(const std::size_t rep_count, Functor fun, double & std_dev)
    throw (eh::Exception)
  {
    if (!rep_count)
    {
      std_dev = 0.;
      return 0.;
    }

    typedef std::vector<long long> data_type;
    data_type samples;
    samples.reserve(rep_count);
    // compute sample
    for (std::size_t i = 0; i < rep_count; ++i)
    {
      samples.push_back( fun() );
    }

    double acc = std::accumulate(samples.begin(), samples.end(),
                static_cast<std::size_t>(0));
    const double m_w = acc/rep_count;   // sample mean
    double variance = 0.;
    for (data_type::const_iterator it = samples.begin();
        it != samples.end(); ++it)
    {
      variance += pow (*it-m_w, 2);
    }
    variance /= rep_count - 1;  // unshifted variation
    std_dev = sqrt(variance);   // standard deviation

    return m_w;
  }

  /**
   * Index into the table below with the first byte of a UTF-8 sequence to
   * get the number of trailing bytes that are supposed to follow it.
   * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
   * left as-is for anyone who may want to do such conversion, which was
   * allowed in earlier algorithms.
   */

  const unsigned char trailingBytesUTF8[256] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //32
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //160 ill
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //192 wnd
    0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, //224
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0  //256
  };

  inline std::size_t
  get_octet_count_outdoor(char ch) throw()
  {
    return trailingBytesUTF8[static_cast<unsigned char>(ch)];
  }

  inline std::size_t
  get_octet_count_inside(char ch) throw()
  {
    const unsigned char trailingBytesForUTF8[256] = {
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //32
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //160 ill
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //192 wnd
      0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, //224
      3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0  //256
    };

    return trailingBytesForUTF8[static_cast<unsigned char>(ch)];
  }

  inline std::size_t
  get_octet_count_inside_static(char ch) throw()
  {
    static const unsigned char trailingBytesForUTF8[256] = {
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //32
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //160 ill
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //192 wnd
      0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, //224
      3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0  //256
    };

    return trailingBytesForUTF8[static_cast<unsigned char>(ch)];
  }

  inline std::size_t
  get_octet_count_if(char ch) throw()
  {
    if ((ch & 0x80) == 0)
    {
      return 1;
    }
    else if ((ch & 0xE0) == 0xC0)
    {
      return 2;
    }
    else if ((ch & 0xF0) == 0xE0)
    {
      return 3;
    }
    else if ((ch & 0xF8) == 0xF0)
    {
      return 4;
    }
    else if ((ch & 0xFC) == 0xF8)
    {
      return 5;
    }
    else if ((ch & 0xFE) == 0xFC)
    {
      return 6;
    }
    else
    {
      return 0;
    }
  }
} // namespace Test

#endif  // _PERFORMANCE_TEST_ADDITIONAL_HPP_INCLUDED_
