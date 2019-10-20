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



#include <Generics/Decimal.hpp>
#include <Generics/SimpleDecimal.hpp>
#include <Generics/Rand.hpp>

#include "Application.hpp"
#include "PerformanceTest.hpp"


uint16_t
nine() throw ();

void
test_int() throw ()
{
  SimpleDecimal<uint64_t, 18, 9> dec1(false, nine(), 0);
  SimpleDecimal<uint64_t, 18, 9> dec2(false, 0, nine());
  Decimal<uint64_t, 18, 9> dec3(false, nine(), 0);
  Decimal<uint64_t, 18, 9> dec4(false, 0, nine());
}

void
test_cons() throw (eh::Exception)
{
  SimpleDecimal<uint64_t, 18, 9> dec1(false, nine(), 0);
  SimpleDecimal<uint64_t, 15, 9> dec2(dec1);
  SimpleDecimal<uint64_t, 15, 10> dec3(dec1);
}

uint16_t
nine() throw ()
{
  return 9;
}

template <typename Element>
void
do_create_int() throw (eh::Exception)
{
  typedef Decimal<Element, 4, 2> Self;
  Self null(false, 0 , 0);
  Self null2(true, 0 , 0);
  Self dec1(false, 9, 0);
  Self dec2(false, 13, 0);
  Self dec3(true, 4, 0);
  if (null != null)
  {
    std::cerr << "Fail: Decimal null must == Decimal null" << std::endl;
  }
  if (null2 != null2)
  {
    std::cerr << "Fail: Decimal -null must == Decimal -null" << std::endl;
  }
  if (null2 != null)
  {
    std::cerr << "Fail: Decimal -null must == Decimal null" << std::endl;
  }
  if (null != null2)
  {
    std::cerr << "Fail: Decimal null must == Decimal -null" << std::endl;
  }
  if (dec1 == null)
  {
    std::cerr << "Fail: Decimal(9) must != Decimal null" << std::endl;
  }
  if (dec2 == null)
  {
    std::cerr << "Fail: Decimal(13) must != Decimal null" << std::endl;
  }
  if (dec3 == null)
  {
    std::cerr << "Fail: Decimal(-4) must != Decimal null" << std::endl;
  }
  if (dec1 == dec2)
  {
    std::cerr << "Fail: Decimal(9) must != Decimal(13)" << std::endl;
  }
  if (dec2 == dec3)
  {
    std::cerr << "Fail: Decimal(13) must != Decimal(-4)" << std::endl;
  }
  if (dec1 == dec3)
  {
    std::cerr << "Fail: Decimal(9) must != Decimal(-4)" << std::endl;
  }
  Self dec4(false, 5, 0);
  Self sum1 = dec1 + dec3;
  if (sum1 != dec4)
  {
    std::cerr << "Fail: must 9 + -4 = 5" << std::endl;
  }
  Self sub1 = dec1 - dec2;
  if (sub1 != dec3)
  {
    std::cerr << "Fail: must 13 - 9 = -4" << std::endl;
  }
}

template <typename Element>
void
do_sum() throw (eh::Exception)
{
  {
    typedef Decimal<Element, 4, 2> Self;
    Self dec1(false, 75, 12);
    Self dec2(false, 5, 88);
    Self sum1 = dec1 + dec2;
    Self res(false, 81, 0);
    if (sum1 != res)
    {
      std::cerr << "Fail: must " << dec1.str() << " + "
                << dec2.str()
                << " = 81 but got: " << sum1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 19, 0> Self;
    Self dec1(String::SubString("9223372036854775808"));
    Self dec2(String::SubString("9223372036854775808"));
    try
    {
      Self sum1 = dec1 + dec2;
      std::cerr << "Fail: must overflow " << dec1.str() << " + "
                << dec2.str() << std::endl;
    }
    catch (...)
    {
    }
  }
}

template <typename Element>
void
do_create_str() throw (eh::Exception)
{
  typedef Decimal<Element, 12, 4> Self;
  {
    Self dec1(String::SubString("123045"));
    Self dec2(false, 123045, 0);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"123045\" == 123045" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("-123645"));
    Self dec2(true, 123645, 0);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"-123645\" == -123645" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("+323645"));
    Self dec2(false, 323645, 0);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"+323645\" == 323645 #" << dec1.str() <<
        std::endl;
    }
  }
  {
    Self dec1(String::SubString("123045.34"));
    Self dec2(false, 123045, 3400);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"123045.34\" == 123045.3400" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("-123046."));
    Self dec2(true, 123046, 0);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"-123046.\" == -123046" << std::endl;
    }
  }
  {
    Self dec1(String::SubString(".078"));
    Self dec2(false, 0, 780);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \".078\" == 0.0780" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("0.0780"));
    Self dec2(false, 0, 780);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"0.0780\" == 0.0780" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("-0.0780"));
    Self dec2(true, 0, 780);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"-0.0780\" == -0.0780" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("-0"));
    Self dec2(true, 0, 0);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"-0\" == -0" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("0"));
    Self dec2(false, 0, 0);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"0\" == 0" << std::endl;
    }
  }
  {
    Self dec1(String::SubString("-0.0"));
    Self dec2(true, 0, 0);
    if (dec1 != dec2)
    {
      std::cerr << "Fail: must \"-0\" == -0" << std::endl;
    }
  }
}

template <typename Element>
void
do_return_str() throw (eh::Exception)
{
  {
    typedef Decimal<Element, 12, 4> Self;
    {
      Self dec1(String::SubString("123045"));
      std::string dec2 = dec1.str();
      if (dec2 != "123045.0")
      {
        std::cerr << "Fail: must \"123045\" == \"123045.0\" but got: "
                  << dec2
                  << std::endl;
      }
    }
    {
      Self dec1(String::SubString("-123045"));
      std::string dec2 = dec1.str();
      if (dec2 != "-123045.0")
      {
        std::cerr << "Fail: must \"-123045\" == \"-123045.0\" but got: "
                  << dec2
                  << std::endl;
      }
    }
    {
      Self dec1(String::SubString("-123045.12"));
      std::string dec2 = dec1.str();
      if (dec2 != "-123045.12")
      {
        std::cerr << "Fail: must \"-123045.12\" == \"-123045.12\" but got: "
                  << dec2
                  << std::endl;
      }
    }
    {
      Self dec1(String::SubString("-0.12"));
      std::string dec2 = dec1.str();
      if (dec2 != "-0.12")
      {
        std::cerr << "Fail: must \"-0.12\" == \"-0.12\" but got: "
                  << dec2
                  << std::endl;
      }
    }
  }
  {
    typedef Decimal<Element, 12, 0> Self;
    {
      Self dec1(String::SubString("123045"));
      std::string dec2 = dec1.str();
      if (dec2 != "123045")
      {
        std::cerr << "Fail: must \"123045\" == \"123045\" but got: "
                  << dec2
                  << std::endl;
      }
    }
  }
  {
    typedef Decimal<Element, 12, 0> Self;
    {
      Self dec1(true, 0, 0);
      std::string dec2 = dec1.str();
      if (dec2 == "-0")
      {
        std::cerr << "Fail: must \"0\" == \"0\" but got: "
                  << dec2
                  << std::endl;
      }
    }
  }
}

template <typename Element>
void
do_create_int2() throw (eh::Exception)
{
  {
    typedef Decimal<Element, 4, 0> Self;
    Self dec1(false, 9, 0);
  }
}

void
do_super_big() throw (eh::Exception)
{
  typedef Decimal<unsigned long, 100, 50> Self;
  Self dec1(String::SubString(
    "-12345678901234567890123456789012345678901234567890."
    "12345678901234567890123456789012345678901234567890"));
}
void
do_sum_over() throw (eh::Exception)
{
  typedef Decimal<unsigned char, 4, 2> Self;
  try
  {
    Self dec1(false, 50, 0);
    Self dec2(false, 51, 0);
    Self sum1 = dec1 + dec2;
    std::cerr << "Fail, must error for 4.2: 50.0 * 51.0 = 101.0 #"
      << sum1.str() << std::endl;
  }
  catch (const Self::Overflow&)
  {
  }
}

template <typename Element>
void
do_mul() throw (eh::Exception)
{
  {
    typedef Decimal<Element, 4, 2> Self;
    Self dec1(false, 3, 0);
    Self dec2(false, 4, 0);
    Self dec3(false, 12, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 3.0 * 4.0 = 12.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 4, 2> Self;
    Self dec1(true, 1, 0);
    Self dec2(false, 1, 0);
    Self dec3(true, 1, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: -1.0 * 1.0 = -1.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 4, 2> Self;
    Self dec1(false, 1, 0);
    Self dec2(true, 1, 0);
    Self dec3(true, 1, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 1.0 * -1.0 = -1.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 4, 2> Self;
    Self dec1(true, 1, 0);
    Self dec2(true, 1, 0);
    Self dec3(false, 1, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: -1.0 * -1.0 = 1.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 5, 2> Self;
    Self dec1(false, 10, 0);
    Self dec2(false, 10, 0);
    Self dec3(false, 100, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 10.0 * 10.0 = 100.0 #" <<  mul1.str()
                << " " << dec3.str() << std::endl
                << " " << dec3.dump() << std::endl
                << " " << mul1.dump() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 7, 2> Self;
    Self dec1(false, 101, 0);
    Self dec2(false, 102, 0);
    Self dec3(false, 10302, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 101.0 * 102.0 = 10302.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 7, 3> Self;
    Self dec1(false, 101, 0);
    Self dec2(false, 102, 0);
    try
    {
      Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
      std::cerr << "Fail: 101.0 * 102.0 = overflow #"
        << mul1.str() << std::endl;
    }
    catch (...)
    {
    }
  }
  {
    typedef Decimal<Element, 10, 5> Self;
    Self dec1(false, 101, 0);
    Self dec2(false, 102, 0);
    Self dec3(false, 10302, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 101.0 * 102.0 = 10302.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 8, 3> Self;
    Self dec1(false, 101, 0);
    Self dec2(false, 102, 0);
    Self dec3(false, 10302, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 101.0 * 102.0 = 10302.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 8, 3> Self;
    Self dec1(false, 123, 0);
    Self dec2(false, 456, 0);
    Self dec3(false, 56088, 0);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 123.0 * 456.0 = 56088.0 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 8, 3> Self;
    Self dec1(false, 12, 12);
    Self dec2(false, 11, 11);
    Self dec3(false, 132, 264);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 12.012 * 11.011 = 132.264 #"
        << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 2, 1> Self;
    Self dec1(false, 0, 5);
    Self dec2(false, 0, 9);
    Self dec3(false, 0, 4);
    Self mul1 = Self::mul(dec1, dec2, DMR_FLOOR);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 0.5 * 0.9 = 0.4 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 2, 1> Self;
    Self dec1(false, 0, 5);
    Self dec2(false, 0, 9);
    Self dec3(false, 0, 5);
    Self mul1 = Self::mul(dec1, dec2, DMR_ROUND);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 0.5 * 0.9 = 0.5 #" << mul1.str() << std::endl;
    }
  }
  {
    typedef Decimal<Element, 2, 1> Self;
    Self dec1(false, 0, 5);
    Self dec2(false, 0, 9);
    Self dec3(false, 0, 5);
    Self mul1 = Self::mul(dec1, dec2, DMR_CEIL);
    if (mul1 != dec3)
    {
      std::cerr << "Fail: 0.5 * 0.9 = 0.5 #" << mul1.str() << std::endl;
    }
  }
}

template <typename Element>
void
test_DecimalState() throw (eh::Exception)
{
  {
    typedef typename RandomTestDecimal<Element, 4, 2, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 10, 50);
    DecimalState st2(false, 3, 60);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 2 || q.r != 91 || r.i != 0 || r.r != 3)
    {
      std::cerr << "Fail: 10.5 / 3.6 = 2.91 (0.3)" << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 4, 2, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 11, 50);
    DecimalState st2(false, 13, 60);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 0 || q.r != 84 || r.i != 0 || r.r != 8)
    {
      std::cerr << "Fail: 11.5 / 13.6 = 0.84 (0.8)" << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 1, 0, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 1, 0);
    DecimalState st2(false, 3, 0);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 0 || q.r != 0 || r.i != 1 || r.r != 0)
    {
      std::cerr << "Fail: 1 / 3 = 0.0 (1.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 1, 0, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(true, 1, 0);
    DecimalState st2(true, 3, 0);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 0 || q.r != 0 || r.i != 1 || r.r != 0)
    {
      std::cerr << "Fail: 1 / 3 = 0.0 (1.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 1, 0, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(true, 2, 0);
    DecimalState st2(true, 6, 0);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 0 || q.r != 0 || r.i != 2 || r.r != 0)
    {
      std::cerr << "Fail: 2 / 6 = 0.0 (2.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 2, 1, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 4, 8);
    DecimalState st2(false, 0, 5);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 9 || q.r != 6 || r.i != 0 || r.r != 0)
    {
      std::cerr << "Fail: 4.8 / 0.5 = 9.6 (0.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 2, 1, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 2, 8);
    DecimalState st2(false, 1, 0);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 2 || q.r != 8 || r.i != 0 || r.r != 0)
    {
      std::cerr << "Fail: 2.8 / 1.0 = 2.8 (0.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 2, 1, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 8, 0);
    DecimalState st2(false, 9, 0);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 0 || q.r != 8 || r.i != 0 || r.r != 8)
    {
      std::cerr << "Fail: 2.8 / 1.0 = 2.8 (0.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 5, 2, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 748, 0);
    DecimalState st2(false, 2, 0);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 374 || q.r != 0 || r.i != 0 || r.r != 0)
    {
      std::cerr << "Fail: 748 / 2 = 374 (0.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 5, 2, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 2, 3);
    DecimalState st2(false, 7, 0);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 0 || q.r != 29 || r.i != 0 || r.r != 0)
    {
      std::cerr << "Fail: 2.03 / 7 = 0.29 (0.0) got " << q.str() << " " <<
        r.str() << std::endl;
    }
  }
  {
    typedef typename RandomTestDecimal<Element, 5, 2, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 2, 3);
    DecimalState st2(false, 0, 7);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 29 || q.r != 0 || r.i != 0 || r.r != 0)
    {
      std::cerr << "Fail: 2.03 / 0.07 = 29 (0.0) got "
                << q.str() << " " << r.str() << std::endl;
    }
  }
#if 0
  {
    typedef typename RandomTestDecimal<Element, 36, 18, Decimal>::DecimalState
      DecimalState;
    DecimalState st1(false, 0, 1135528036251086848LL);
    DecimalState st2(false, 0, 8384628349126416384LL);
    DecimalState q, r;
    bool overflow = false;
    st1.div(st2, q, r, overflow);
    if (q.i != 0 || q.r != 0 || r.i != 0 || r.r != 1135528036251086848LL)
    {
      std::cerr << "Fail: 0.1135528036251086848 / 0.8384628349126416384 "
                << "= 0.0 (0.1135528036251086848) got "
                << q.i << '.' << q.r << ' ' << r.i << '.' << r.r << std::endl;
    }
  }
#endif
}

void
test_to_integer() throw (eh::Exception)
{
  {
    typedef Decimal<unsigned char, 5, 1> Self;
    Self val(false, 2, 3);
    if (val.integer<int>() != 2)
    {
      std::cerr << "Fail: expected integer<int>() == 2, from "
                << val.str() << " got: " << val.integer<int>() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 5, 3> Self;
    Self val(false, 21, 133);
    if (val.integer<int>() != 21)
    {
      std::cerr << "Fail: expected integer<int>() == 21, from "
                << val.str() << " got: " << val.integer<int>() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 5, 3> Self;
    Self val(true, 21, 133);
    if (val.integer<int>() > 0)
    {
      std::cerr << "Fail: expected integer<int>() < 0, from "
                << val.str() << " got: " << val.integer<int>() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 5, 3> Self;
    Self val(true, 21, 133);
    bool  exception = false;
    try
    {
      val.integer<unsigned int>();
    }
    catch (const Self::Sign&)
    {
      exception = true;
    }
    if (!exception)
    {
      std::cerr << "Fail: expected Sign to get integer<unsigned int> from "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 6, 3> Self;
    Self val(false, 256, 133);
    bool overflow = false;
    try
    {
      val.integer<char>();
    }
    catch (const Self::Overflow&)
    {
      overflow = true;
    }
    if (!overflow)
    {
      std::cerr << "Fail: expected Overflow to get integer<char> from "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 6, 2> Self;
    Self val(false, 3943, 78);
    bool overflow = false;
    try
    {
      val.integer<unsigned char>();
    }
    catch (const Self::Overflow&)
    {
      overflow = true;
    }
    if (!overflow)
    {
      std::cerr << "Fail: expected Overflow to get integer<char> from "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 7, 3> Self;
    Self val(false, 2566, 133);
    bool overflow = false;
    try
    {
      val.integer<char>();
    }
    catch (const Self::Overflow&)
    {
      overflow = true;
    }
    if (!overflow)
    {
      std::cerr << "Fail: expected Overflow to get integer<char> from "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
}

void
test_from_float() throw (eh::Exception)
{
  {
    typedef Decimal<unsigned char, 5, 1> Self;
    Self val(1.5);
    if (val.str() != "1.5")
    {
      std::cerr << "Fail: created from 1.5 got "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 5, 1> Self;
    Self val(1.49);
    if (val.str() != "1.5")
    {
      std::cerr << "Fail: created from 1.49 got "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 5, 1> Self;
    Self val(-1.49);
    if (val.str() != "-1.5")
    {
      std::cerr << "Fail: created from -1.49 got "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 5, 1> Self;
    Self val(-0.0);
    if (val.str() != "0.0")
    {
      std::cerr << "Fail: created from 0.0 got "
                << val.str() << " " << val.dump() << std::endl;
    }
  }
  {
    typedef Decimal<unsigned char, 4, 1> Self;
    bool overflow = false;
    try
    {
      Self val(-12345678901.123456789);
    }
    catch (const Self::Overflow&)
    {
      overflow = true;
    }
    if (!overflow)
    {
      std::cerr << "Fail: expected Overflow to construct 4.1"
                << " from -12345678901.123456789"
                << std::endl;
    }
  }
}

template <typename Decimal>
void
test_to_float(const char* where) throw (eh::Exception)
{
  static const char* data[] =
  {
    "0.0",
    "1.0",
    "-1.0",
    "1000000000",
    "0.1",
    "0.01",
    "10.01",
    "34567890.987654",
    0
  };
  for (int i = 0; data[i]; i++)
  {
    double res = atof(data[i]);
    double ret = Decimal(data[i]).template floating<double>();
    if (res != ret)
    {
      std::cerr << where << "::to_floating: got " << ret <<
        " instead of " << res << " for " << data[i] << std::endl;
    }
  }
}

void
test_to_float() throw (eh::Exception)
{
  test_to_float<Generics::Decimal<uint8_t, 18, 8>>("D8");
  test_to_float<Generics::Decimal<uint32_t, 18, 8>>("D32");
  test_to_float<Generics::Decimal<uint64_t, 18, 8>>("D64");
  test_to_float<Generics::SimpleDecimal<uint64_t, 18, 8>>("S64");
}


struct Data
{
  const char* dividend;
  const char* divisor;
  const char* quotient;
  const char* remainder;
};

template <typename Element, typename TestType>
void
do_div_test(const Data* data)
{
  for (size_t i = 0; data[i].dividend; i++)
  {
    const Data& d = data[i];
    bool error = false;
    try
    {
      TestType dividend(String::SubString(d.dividend));
      TestType divisor(String::SubString(d.divisor));
      TestType remainder;
      TestType quotient = TestType::div(dividend, divisor, remainder);

      if (d.quotient)
      {
        TestType expected_quotient(String::SubString(d.quotient));
        TestType expected_remainder(String::SubString(d.remainder));
        if (quotient != expected_quotient)
        {
          std::cerr << "FAIL quotient: " << quotient.str();
          error = true;
        }
        if (remainder != expected_remainder)
        {
          std::cerr << "FAIL remainder: " << remainder.str();
          error = true;
        }
      }
      else
      {
        std::cerr << "FAIL No expected exception\n";
        error = true;
      }
    }
    catch (const typename TestType::Overflow& ex)
    {
      if (d.quotient)
      {
        std::cerr << "FAIL Unexpected exception: " << ex.what();
        error = true;
      }
    }
    if (error)
    {
      std::cerr << " for test case " << d.dividend << " / " << d.divisor <<
        " = ";
      if (d.quotient)
      {
        std::cerr << d.quotient << " ( " << d.remainder << " )\n";
      }
      else
      {
        std::cerr << "overflow\n";
      }
    }
  }
}

static const Data data_2_1[] =
{
  { "1.0", "1.0", "1.0", "0.0" },
  { "-1.0", "2.0", "-0.5", "0.0" },
  { "-2.0", "1.0", "-2.0", "0.0" },
  { "1.0", "0.1", 0, 0 },
  { "0.4", "0.2", "2.0", "0.0" },
  { "1.4", "2.7", "0.5", "0.1" },
  { "1.5", "1.3", "1.1", "0.1" },
  { 0, 0, 0, 0 }
};

static const Data data_3_0[] =
{
  { "96", "233", "0", "96" },
  { "-18", "-648", "0", "-18" },
  { 0, 0, 0, 0 }
};

static const Data data_3_1[] =
{
  { "0.2", "10.0", "0.0", "0.2" },
  { "-64.3", "-64.3", "1.0", "0.0" },
  { "0.0", "-7.3", "0.0", "0.0" },
  { 0, 0, 0, 0 }
};

static const Data data_4_2[] =
{
  { "59.7", "-59.98", "-0.99", "0.32" },
  { 0, 0, 0, 0 }
};

static const Data data_4_3[] =
{
  { "9.464", "-6.381", "-1.483", "0.001" },
  { "3.446", "7.33", "0.47", "0.001" },
  { 0, 0, 0, 0 }
};

static const Data data_8_3[] =
{
  { "9999.0", "11.0", "909.0", "0.0" },
  { "-9998.0", "-99.0", "100.989", "-0.089" },
  { "-2.0", "1.0", "-2.0", "0.0" },
  { "1.0", "0.1", "10.0", "0.0" },
  { "0.4", "0.2", "2.0", "0.0" },
  { "1.4", "2.7", "0.518", "0.002" },
  { "1.4", "1.3", "1.076", "0.002" },
  { 0, 0, 0, 0 }
};

template <typename Element>
void
do_div() throw (eh::Exception)
{
  do_div_test<Element, Decimal<Element, 2, 1> >(data_2_1);
  do_div_test<Element, Decimal<Element, 3, 0> >(data_3_0);
  do_div_test<Element, Decimal<Element, 3, 1> >(data_3_1);
  do_div_test<Element, Decimal<Element, 4, 2> >(data_4_2);
  do_div_test<Element, Decimal<Element, 4, 3> >(data_4_3);
  do_div_test<Element, Decimal<Element, 8, 3> >(data_8_3);
}

template <typename Element>
void
batch_hand_test() throw (eh::Exception)
{
  do_create_int<Element>();
  do_create_int2<Element>();
  do_create_str<Element>();
  do_return_str<Element>();
  do_sum<Element>();
  do_mul<Element>();
  test_DecimalState<Element>();
  test_to_integer ();
  do_div<Element>();
}

void
do_total_test() throw (eh::Exception)
{
  srandom(time(0));
#if 0
#if 1
  //Random<Decimal, unsigned char, 2>("Decimal");
  Random<Decimal, unsigned char, 5>("Decimal");
  Random<Decimal, unsigned short, 9>("Decimal");
  //Random<Decimal, unsigned int, 9>("Decimal");
  //Random<Decimal, unsigned long, 5>("Decimal");
#ifdef __x86_64__
  //Random<Decimal, unsigned long long, 5>("Decimal");
  //Random<Decimal, unsigned long long, 25>("Decimal");
#endif
  Random<SimpleDecimal, uint8_t, 2>("SimpleDecimal");
  Random<SimpleDecimal, uint16_t, 4>("SimpleDecimal");
  //Random<SimpleDecimal, uint32_t, 8>("SimpleDecimal");
#ifdef __x86_64__
  do_random<Decimal, uint64_t, 19, 0>("Decimal");
  do_random<Decimal, uint64_t, 19, 19>("Decimal");
  do_random<Decimal, uint64_t, 19, 8>("Decimal");
  do_random<Decimal, uint64_t, 38, 19>("Decimal");
#else
  do_random<Decimal, uint32_t, 19, 0>("Decimal");
  do_random<Decimal, uint32_t, 19, 19>("Decimal");
  do_random<Decimal, uint32_t, 19, 8>("Decimal");
  do_random<Decimal, uint32_t, 38, 19>("Decimal");
#endif
#ifdef __x86_64__
  do_random<SimpleDecimal, uint64_t, 19, 0>("SimpleDecimal");
  do_random<SimpleDecimal, uint64_t, 19, 19>("SimpleDecimal");
  do_random<SimpleDecimal, uint64_t, 19, 8>("SimpleDecimal");
#else
  do_random<SimpleDecimal, uint32_t, 8, 0>("SimpleDecimal");
  do_random<SimpleDecimal, uint32_t, 8, 8>("SimpleDecimal");
  do_random<SimpleDecimal, uint32_t, 8, 4>("SimpleDecimal");
#endif
#endif
#else
  do_random<Decimal, uint32_t, 38, 19>("Decimal");
  do_random<SimpleDecimal, uint64_t, 19, 8>("SimpleDecimal");
#endif
}

struct ConstructorPower
{
  long long value;
  unsigned power;
  const char* result;
};

const ConstructorPower cp_data[] =
{
  { 0, 0, "0.0" },
  { 1234, 0, "" },
  { -1234, 1, "-123.4" },
  { 1234, 2, "12.3" },
  { 1234, 3, "1.2" },
  { -1234, 4, "-0.1" },
  { 1234, 5, "0.0" },
  { 1234, 6, "0.0" },
  { -1234, 100, "0.0" },
  { 12, 0, "12.0" },
  { 12, 1, "1.2" },
  { -12, 2, "-0.1" },
  { 12, 3, "0.0" },
  { 12, 4, "0.0" },
  { -12345, 0, "" },
  { 12345, 1, "" },
  { 12345, 2, "123.4" },
  { -12345, 3, "-12.3" },
  { 12345, 4, "1.2" },
  { 12345, 5, "0.1" },
  { -12345, 6, "0.0" },
  { 12345, 7, "0.0" },
  { 0, 0, 0 }
};

template <template <typename Element, const unsigned TOTAL,
  const unsigned FRACTION> class Decimal, typename Base>
void
test_constructor_power(const char* desc) throw (eh::Exception)
{
  typedef Decimal<Base, 4, 1> Type;
  for (unsigned i = 0; cp_data[i].result; i++)
  {
    try
    {
      Type test(cp_data[i].value, cp_data[i].power);
      if (!*cp_data[i].result)
      {
        std::cerr << "No exception for " << desc << " constructor test " <<
          i << std::endl;
      }
      else
      {
        if (test.str() != cp_data[i].result)
        {
          std::cerr << "Invalid result " << test.str() << " vs " <<
            cp_data[i].result << " for " << desc << " constructor test " <<
            i << std::endl;
        }
      }
    }
    catch (const typename Type::Exception& ex)
    {
      if (*cp_data[i].result)
      {
        std::cerr << "Unexpected exception for " << desc <<
          " constructor test " << i << ": " << ex.what() << std::endl;
      }
    }
  }
}

void
test_constructor_power() throw (eh::Exception)
{
  test_constructor_power<Generics::SimpleDecimal, uint16_t>(
    "SimpleDecimal");
  test_constructor_power<Generics::Decimal, uint16_t>(
    "Decimal_16");
  test_constructor_power<Generics::Decimal, uint8_t>(
    "Decimal_8");
}

struct DiffData
{
  const char* from;
  const char* to;
};

const DiffData diff_data[] =
{
  { "0", "0.0" },
  { "0.1", "0.1" },
  { "1", "1.0" },
  { "1.2", "1.2" },
  { "12.3", "12.3" },
  { "123.4", 0 },
  { 0, 0 },
};

template <template <typename Element, const unsigned TOTAL,
  const unsigned FRACTION> class Decimal>
void
test_diff_constructor(const char* desc) throw (eh::Exception)
{
  typedef Decimal<uint16_t, 4, 1> From;
  typedef Decimal<uint16_t, 4, 2> To;
  for (unsigned i = 0; diff_data[i].from; i++)
  {
    From from(diff_data[i].from);
    try
    {
      To to(from);
      if (!diff_data[i].to)
      {
        std::cerr << "No exception for " << desc << " diff test " <<
          i << std::endl;
      }
      else
      {
        if (to.str() != diff_data[i].to)
        {
          std::cerr << "Invalid result " << to.str() << " vs " <<
            diff_data[i].to << " for " << desc << " diff test " <<
            i << std::endl;
        }
      }
    }
    catch (const typename To::Exception& ex)
    {
      if (diff_data[i].to)
      {
        std::cerr << "Unexpected exception for " << desc <<
          " diff test " << i << ": " << ex.what() << std::endl;
      }
    }
  }
}

void
test_diff_constructor() throw (eh::Exception)
{
  test_diff_constructor<SimpleDecimal>("SimpleDecimal");
  test_diff_constructor<Decimal>("Decimal");
};

struct TestCase
{
  double number;
  const char* const STANDARD;
};

const TestCase TEST_CASES_0[] =
{
  {999999999999999, "999999999999999"},
  {1.123, "1"},
  {2.023, "2"},
  {3.123, "3"},
  {10.123, "10"},
};

const TestCase TEST_CASES_1[] =
{
  {0.123, "0.1"},
  {1.023, "1.0"},
  {10.023, "10.0"},
  {12.23, "12.2"},
  {123.44, "123.4"},
  {12, "12.0"},
  {123, "123.0"},
};

const TestCase TEST_CASES_8[] =
{
  {9999999999.12339973, "9999999999.12339973"},
  {9999999999, "9999999999.0"},
  {.99999999, "0.99999999"},

  {12345.00000001, "12345.00000001"},
  {12345.0000002, "12345.0000002"},
  {12345.000003, "12345.000003"},
  {12345.00004, "12345.00004"},
  {12345.0005, "12345.0005"},
  {54321.006, "54321.006"},
  {54321.07, "54321.07"},
  {54321.8, "54321.8"},
  {50000.0, "50000.0"},

  {1.00000001, "1.00000001"},
  {2.0000002, "2.0000002"},
  {3.000003, "3.000003"},
  {4.00004, "4.00004"},
  {5.0005, "5.0005"},
  {6.006, "6.006"},
  {7.07, "7.07"},
  {8.8, "8.8"},
  {9.0, "9.0"},

  {1.00000001, "1.00000001"},
  {2.00000021, "2.00000021"},
  {3.00000321, "3.00000321"},
  {4.00004321, "4.00004321"},
  {5.00054321, "5.00054321"},
  {6.00654321, "6.00654321"},
  {7.07654321, "7.07654321"},
  {8.87654321, "8.87654321"},

  {1.0000012, "1.0000012"},
  {2.000012, "2.000012"},
  {3.00012, "3.00012"},
  {4.0012, "4.0012"},
  {5.012, "5.012"},
  {6.12, "6.12"},

  {2.0000123, "2.0000123"},
  {3.000123, "3.000123"},
  {4.00123, "4.00123"},
  {5.0123, "5.0123"},
  {6.123, "6.123"},

  {2.0001234, "2.0001234"},
  {3.001234, "3.001234"},
  {4.01234, "4.01234"},
  {5.1234, "5.1234"},

  {2.0012345, "2.0012345"},
  {3.012345, "3.012345"},
  {4.12345, "4.12345"},

  {2.0123456, "2.0123456"},
  {3.123456, "3.123456"},

  {0.00000001, "0.00000001"},
  {0.0000002, "0.0000002"},
  {0.000003, "0.000003"},
  {0.00004, "0.00004"},
  {0.0005, "0.0005"},
  {0.006, "0.006"},
  {0.07, "0.07"},
  {0.8, "0.8"},

  {0.00000001, "0.00000001"},
  {0.00000021, "0.00000021"},
  {0.00000321, "0.00000321"},
  {0.00004321, "0.00004321"},
  {0.00054321, "0.00054321"},
  {0.00654321, "0.00654321"},
  {0.07654321, "0.07654321"},
  {0.87654321, "0.87654321"},

  {0.0000012, "0.0000012"},
  {0.000012, "0.000012"},
  {0.00012, "0.00012"},
  {0.0012, "0.0012"},
  {0.012, "0.012"},
  {0.12, "0.12"},

  {0.0000123, "0.0000123"},
  {0.000123, "0.000123"},
  {0.00123, "0.00123"},
  {0.0123, "0.0123"},
  {0.123, "0.123"},

  {0.0001234, "0.0001234"},
  {0.001234, "0.001234"},
  {0.01234, "0.01234"},
  {0.1234, "0.1234"},

  {0.0012345, "0.0012345"},
  {0.012345, "0.012345"},
  {0.12345, "0.12345"},

  {0.0123456, "0.0123456"},
  {0.123456, "0.123456"},
};

template <std::size_t FRACTION, std::size_t STD_SIZE>
void
test_str_fraction(const TestCase (&TEST_CASES)[STD_SIZE])
{
  typedef Generics::SimpleDecimal<uint64_t, 18, FRACTION> Fixed;
  for (std::size_t i = 0;
     i < sizeof(TEST_CASES) / sizeof(TEST_CASES[0]);
     ++i)
  {
    Fixed number(TEST_CASES[i].number);
    if (number.str() != TEST_CASES[i].STANDARD)
    {
      std::cerr << "Fail, incorrect output: " << number.str()
       << std::endl << "correct result is: "
       << TEST_CASES[i].STANDARD << std::endl;
    }
    if (number.negate().str() != std::string("-") + TEST_CASES[i].STANDARD)
    {
      std::cerr << "Fail, incorrect output: " << number.negate().str()
        << std::endl << "correct result is: "
        << '-' << TEST_CASES[i].STANDARD << std::endl;
    }
  }
}

void
test_str()
{
  test_str_fraction<8>(TEST_CASES_8);
  test_str_fraction<1>(TEST_CASES_1);
  test_str_fraction<0>(TEST_CASES_0);
  if (Generics::SimpleDecimal<uint64_t, 18, 8>::ZERO.str() != "0.0")
  {
    std::cerr << "Zero output fail, fraction 8" << std::endl;
  }
  if (Generics::SimpleDecimal<uint64_t, 18, 1>::ZERO.str() != "0.0")
  {
    std::cerr << "Zero output fail, fraction 1" << std::endl;
  }
  if (Generics::SimpleDecimal<uint64_t, 18, 0>::ZERO.str() != "0")
  {
    std::cerr << "Zero output fail, fraction 0" << std::endl;
  }
}

template <std::size_t FRACTION, std::size_t STD_SIZE>
void
test_input_fraction(const TestCase (&TEST_CASES)[STD_SIZE])
{
  typedef Generics::SimpleDecimal<uint64_t, 18, FRACTION> Fixed;
  for (std::size_t i = 0;
     i < sizeof(TEST_CASES) / sizeof(TEST_CASES[0]);
     ++i)
  {
    Fixed number;
    {
      Stream::Parser istr(TEST_CASES[i].STANDARD);
      istr >> number;
    }
    if (number.str() != TEST_CASES[i].STANDARD)
    {
      std::cerr << FRACTION << " Fail, incorrect input: " << number
        << std::endl << "correct result is: "
        << TEST_CASES[i].STANDARD << std::endl;
    }
    {
      std::string s("-");
      s += TEST_CASES[i].STANDARD;
      Stream::Parser istr(s);
      istr >> number;
      if (number.str() != s)
      {
        std::cerr << "Fail, incorrect input: " << number
          << std::endl << "correct result is: "
          << '-' << TEST_CASES[i].STANDARD << std::endl;
      }
    }
  }
}

void
test_input()
{
  test_input_fraction<8>(TEST_CASES_8);
  test_input_fraction<1>(TEST_CASES_1);
  test_input_fraction<0>(TEST_CASES_0);

  typedef Generics::SimpleDecimal<uint64_t, 18, 8> Fixed;
  Stream::Parser istr("00012345678901++++");
  Fixed number;
  istr >> number;
  if (istr.eof())
  {
    std::cerr << "skip fail, read extra chars" << std::endl;
  }

  static const char* INVALID_STRESSES[] =
  {
    "+00000000012345678900.12345678",
    "0000000001234567890.0012345678",
    "-000000000000000000000.1234567890123456789000001",
    "000100000000000",
    "0001000000000.000000001",
    "",
    "+",
    "-",
    ".",
    "..",
    ".+0",
    ".-0",
    "+.",
    "-.",
    "+.text",
    "-.fext",
    ".pext",
    "+.00000000",
    "-.00000000",
    ".00000000",
    "+.0",
    "-.0",
    ".0",
  };
  for (std::size_t i = 0;
    i < sizeof(INVALID_STRESSES) / sizeof(INVALID_STRESSES[0]);
    ++i)
  {
    Fixed number(123.123);
    Stream::Parser istr(INVALID_STRESSES[i]);
    istr >> number;
    if (number != Fixed(123.123) || !istr.fail())
    {
      std::cerr << "Invalid string '" << INVALID_STRESSES[i]
        << "' successfully parsed" << std::endl;
    }
  }
  static const char* VALID_STRESSES[] =
  {
    "+0000000001234567890.12345678",
    "-0000000001234567890.12345678",
    "0000000001234567890.12345678",
    "000000000000000000000.00000000",
    "+000000000000000000000.00000000",
    "-000000000000000000000.00000000",
    "+0.",
    "-0.",
    "0.",
    "+0",
    "-0",
    "+000123.",
    "-000123.",
    "000123.",
  };
  for (std::size_t i = 0;
    i < sizeof(VALID_STRESSES) / sizeof(VALID_STRESSES[0]);
    ++i)
  {
    Fixed number;
    Stream::Parser istr(VALID_STRESSES[i]);
    istr >> number;
    if (!istr || !istr.eof())
    {
      std::cerr << "Valid stresses parsing failed: "
        << VALID_STRESSES[i] << std::endl;
    }
  }
}


void
test_narrow() throw (eh::Exception)
{
  typedef Generics::Decimal<uint64_t, 36, 18> D;
  typedef Generics::SimpleDecimal<uint64_t, 18, 8> S;
  const String::SubString n("-1234567890.87654321");
  D d(n);
  S s;
  Generics::narrow_decimal(s, d);
  assert(s == S(n));
  Generics::Timer timer;
  timer.start();
  for (int i = 0; i < 10000000; i++)
  {
    S s;
    Generics::narrow_decimal(s, d);
  }
  timer.stop();
  std::cout << "Narrow " << timer.elapsed_time() << std::endl;
  timer.start();
  for (int i = 0; i < 10000000; i++)
  {
    D d(s);
  }
  timer.stop();
  std::cout << "Widen " << timer.elapsed_time() << std::endl;
}

void
test_float() throw (eh::Exception)
{
  typedef Generics::SimpleDecimal<uint64_t, 18, 8> D;

  Generics::convert_float<D>(0.0f);
  for (int i = 0; i < 100000; i++)
  {
    D s(safe_rand() > RAND_MAX / 2, safe_rand(), safe_rand() %
      DecimalHelper::Pow10<uint64_t, D::FRACTION_RANK>::Value);
    auto r = s.floating<long double>();
    D d1(r);
    D d2(Generics::convert_float<D>(r));
    D d(d1 - d2);
    if (d.is_nonpositive())
    {
      d.negate();
    }
    if (d > D::EPSILON)
    {
      std::cerr << FNS << s << " " << d1 << " " << d2 << " " << d <<
        std::endl;
    }
  }
}

int
main()
{
  int result = 0;
  std::cout << "Decimal test started" << std::endl;
  try
  {
    test_int();
    test_cons();

    batch_hand_test<unsigned char>();
    batch_hand_test<unsigned short>();
    batch_hand_test<unsigned int>();
    batch_hand_test<unsigned long>();
    batch_hand_test<unsigned long long>();
    do_super_big();
    do_sum_over();
    test_to_integer();
    test_from_float();
    test_to_float();
    test_constructor_power();
    test_diff_constructor();
    test_str();
    test_input();
    test_narrow();
    test_float();

    perfomance_test();

    //big and slow
    do_total_test();

    std::cout << "Test complete" << std::endl;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "FAIL:" << e.what() << std::endl;
    result = -1;
  }
  catch (...)
  {
    std::cerr << "unknown exception" << std::endl;
    result = -1;
  }

  return result;
}
