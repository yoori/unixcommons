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



#include <memory>
#include <string>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cassert>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <eh/Exception.hpp>

#include <String/SubString.hpp>

#include <Generics/CommonDecimal.hpp>

#include <Stream/MemoryStream.hpp>

#include "DecAsm.hpp"


using namespace Generics;

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
class RandomTestDecimal
{
  typedef DecimalType<Element, TOTAL, FRACTION> SelfDecimal;
  typedef typename SelfDecimal::Exception Exception;
  typedef typename SelfDecimal::Overflow Overflow;

  static const uint64_t MAX_INTEGER =
    DecimalHelper::Pow10<uint64_t, SelfDecimal::INTEGER_RANK>::Value;
  static const uint64_t MAX_FRACTION =
    DecimalHelper::Pow10<uint64_t, SelfDecimal::FRACTION_RANK>::Value;

  static const uint64_t MAX_VALUE =
    DecimalHelper::Pow10<
      uint64_t, std::numeric_limits<uint64_t>::digits10>::Value;

public:
  //class to check  - small representation of Decimal for test
  class DecimalState
  {
  public:
    bool sign;
    uint64_t i, r;

    DecimalState() throw ();

    DecimalState(bool sign_in, uint64_t i_in, uint64_t r_in) throw ();

    void
    add(const DecimalState& right, DecimalState& target,
      bool& overflow) const throw ();

    void
    sub(const DecimalState& right, DecimalState& target,
      bool& overflow) const throw ();

    void
    mul(const DecimalState& right, DecimalState& target,
     bool trunc, bool& overflow) const throw ();

    void
    div(const DecimalState& right, DecimalState& quotient,
      DecimalState& remainder, bool& overflow) const throw ();

    bool
    less_than(const DecimalState& right) const throw ();

    const char*
    str() const throw ();

    const char*
    debug_str() const throw ();

  private:
    char as_str_[TOTAL + 3 + (TOTAL == FRACTION)];
    char debug_str_[3 * TOTAL + 50];

    //generate values
    void
    generate_() throw ();

    //make string representation
    void
    make_str_() throw ();

    //add helper
    void
    add_(bool sign, const DecimalState& right, DecimalState& target,
      bool& overflow) const throw ();

    //sub helper
    void
    sub_(bool sign, const DecimalState& right, DecimalState& target,
      bool& overflow) const throw ();
  };

  RandomTestDecimal() throw ();

  RandomTestDecimal(const DecimalState& state_in, const SelfDecimal& n2_in)
    throw ();


  RandomTestDecimal
  maximum(bool& overflow) const throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  negate(bool& overflow) const throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  floor(bool& overflow) const throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  ceil(bool& overflow) const throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  add(const RandomTestDecimal& right, bool& overflow) const
    throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  sub(const RandomTestDecimal& right, bool& overflow) const
    throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  mul_floor(const RandomTestDecimal& right, bool& overflow) const
    throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  mul_round(const RandomTestDecimal& right, bool& overflow) const
    throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  mul_ceil(const RandomTestDecimal& right, bool& overflow) const
    throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  div_quotient(const RandomTestDecimal& right, bool& overflow) const
    throw (typename SelfDecimal::Overflow);

  RandomTestDecimal
  div_remainder(const RandomTestDecimal& right, bool& overflow) const
    throw (typename SelfDecimal::Overflow);

  bool
  equal_to(const RandomTestDecimal& right, bool& expected) const throw ();

  bool
  not_equal_to(const RandomTestDecimal& right, bool& expected) const throw ();

  bool
  less_than(const RandomTestDecimal& right, bool& expected) const throw ();

  bool
  less_than_or_equal_to(const RandomTestDecimal& right, bool& expected) const
    throw ();

  bool
  greater_than(const RandomTestDecimal& right, bool& expected) const throw ();

  bool
  greater_than_or_equal_to(const RandomTestDecimal& right, bool& expected)
    const throw ();

  //state str
  const char*
  str() const throw ();

  //test equality conversion to int
  template <typename IntType>
  bool
  test_to_int_equal() const
  throw (typename SelfDecimal::Overflow, typename SelfDecimal::Sign);

  //test equality conversion to int with test overflow
  template <typename IntType>
  bool
  test_to_int_with_overflow(const char* what) const throw ();

  //test equality conversion to int with test Sign
  template <typename IntType>
  bool
  test_to_int_with_sign(const char* what) const throw ();

  //test conversion to int
  bool
  test_to_int(const char* what = "") const throw ();

  //test equality of results (n1, n2)
  bool
  equal() const throw ();

  //test equality of strings of results (n1, n2)
  bool
  str_equal() const throw (eh::Exception);

  //test equality of values from sting of results (n1, n2)
  bool
  from_str_equal() const throw (eh::Exception);

  //test full equality of results (n1,n2)
  bool
  test_equal(const char* what = "") const throw ();

  //test operation method
  typedef RandomTestDecimal (RandomTestDecimal::*TestUnaryOperation)(
    bool&) const;
  typedef RandomTestDecimal (RandomTestDecimal::*TestOperation)(
    const RandomTestDecimal&, bool&) const;
  typedef bool (RandomTestDecimal::*TestLogicOperation)(
    const RandomTestDecimal&, bool&) const;

  //test
  void
  test_unary_op(TestUnaryOperation op, const char* name) throw ();

  void
  test_op(const RandomTestDecimal& right, TestOperation op, const char* name)
    throw ();

  void
  test_logic_op(const RandomTestDecimal& right, TestLogicOperation op,
    const char* name) throw ();

  //do all tests
  void
  test_ops(const RandomTestDecimal& right) throw ();

private:
  DecimalState state;
  SelfDecimal n1;
  SelfDecimal n2;
};

//
// RandomTestDecimal::DecimalState class
//

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::
  DecimalState() throw ()
{
  generate_();
  make_str_();
}


template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::
  DecimalState(bool sign_in, uint64_t i_in, uint64_t r_in) throw ()
  : sign(sign_in), i(i_in), r(r_in)
{
  make_str_();
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::add(
  const DecimalState& right, DecimalState& target, bool& overflow) const
  throw ()
{
  if (sign == right.sign)
  {
    return add_(sign, right, target, overflow);
  }
  if (i == right.i ? r < right.r : i < right.i)
  {
    return right.sub_(right.sign, *this, target, overflow);
  }
  return sub_(sign, right, target, overflow);
}


template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::sub(
  const DecimalState& right, DecimalState& target, bool& overflow) const
  throw ()
{
  if (sign == right.sign)
  {
    if (i == right.i ? r < right.r : i < right.i)
    {
      return right.sub_(!right.sign, *this, target, overflow);
    }
    return sub_(sign, right, target, overflow);
  }
  return add_(sign, right, target, overflow);
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::mul(
  const DecimalState& right, DecimalState& target, bool trunc,
  bool& overflow) const throw ()
{
  overflow = true;

  uint64_t mul_i, mul_r, over, tmp;
  //  mul_i = i * right.i;
  mul64<MAX_INTEGER>(i, right.i, mul_i, over);
  if (over)
  {
    return;
  }
  // mul_r = i*right.r;
  mul64<MAX_FRACTION>(i, right.r, mul_r, over);
  if (add64<MAX_INTEGER>(mul_i, over, mul_i))
  {
    return;
  }
  // mul_r += right.i*r;
  mul64<MAX_FRACTION>(r, right.i, tmp, over);
  if (add64<MAX_INTEGER>(mul_i, over, mul_i))
  {
    return;
  }
  over = add64<MAX_FRACTION>(mul_r, tmp, mul_r);
  if (add64<MAX_INTEGER>(mul_i, over, mul_i))
  {
    return;
  }
  // mul_r += (right.r*r)/MAX_FRACTION;
  mul64<MAX_FRACTION>(r, right.r, tmp, over);
  over = add64<MAX_FRACTION>(mul_r, over, mul_r);
  if (add64<MAX_INTEGER>(mul_i, over, mul_i))
  {
    return;
  }
  if (!trunc && FRACTION &&
    (tmp > MAX_FRACTION / 2 || tmp == MAX_FRACTION / 2))
  {
    over = add64<MAX_FRACTION>(mul_r, 1, mul_r);
    if (add64<MAX_INTEGER>(mul_i, over, mul_i))
    {
      return;
    }
  }
  overflow = false;
  target.sign = sign != right.sign;
  target.i = mul_i;
  target.r = mul_r;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::div(
  const DecimalState& right, DecimalState& quotient, DecimalState& remainder,
  bool& overflow) const throw ()
{
  uint64_t a = i * MAX_FRACTION + r;
  uint64_t b = right.i * MAX_FRACTION + right.r;
  if (a / MAX_INTEGER >= b)
  {
    overflow = true;
  }
  else
  {
    uint64_t c = a * MAX_FRACTION / b;
    quotient.i = c / MAX_FRACTION;
    quotient.r = c % MAX_FRACTION;
    quotient.sign = sign != right.sign;
    DecimalState mul_v;
    bool junk;
    quotient.mul(right, mul_v, true, junk);
    assert(!junk);
    sub(mul_v, remainder, junk);
    assert(!junk);
  }
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::
  less_than(const DecimalState& right) const throw ()
{
  return sign ? right.sign ? i > right.i || (i == right.i && r > right.r) :
    i || right.i || r || right.r : !right.sign &&
    (i < right.i || (i == right.i && r < right.r));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
const char*
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::
  str() const throw ()
{
  return as_str_;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
const char*
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::
  debug_str() const throw ()
{
  return debug_str_;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::
  generate_() throw ()
{
  sign = !(random() % 2);
  i = static_cast<long long>(random() / (RAND_MAX + 1.0) * MAX_INTEGER);
  r = static_cast<long long>(random() / (RAND_MAX + 1.0) * MAX_FRACTION);
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::
  make_str_() throw ()
{
  Stream::Stack<TOTAL + 3 + (TOTAL == FRACTION)> ostr;
  if (sign)
  {
    if (i || r)
    {
      ostr << '-';
    }
  }
  ostr << i;
  if (FRACTION > 0)
  {
    ostr << '.';
    if (!r)
    {
      ostr << '0';
    }
    else
    {
      ostr << std::setfill('0') << std::setw(FRACTION) << r;
    }
  }
  const String::SubString& str = ostr.str();
  memcpy(as_str_, str.data(), str.size());
  as_str_[str.size()] = '\0';
  snprintf(debug_str_, sizeof(debug_str_), "'%s' %c %llu %llu",
    as_str_, sign ? '-' : '+',
    static_cast<unsigned long long>(i), static_cast<unsigned long long>(r));
}
template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::add_(
  bool sign, const DecimalState& right, DecimalState& target,
  bool& overflow) const throw ()
{
  uint64_t add_i, add_r;
  if (add64<MAX_INTEGER>(i, right.i, add_i) ||
    add64<MAX_INTEGER>(add_i, add64<MAX_FRACTION>(r, right.r, add_r), add_i))
  {
    overflow = true;
    return;
  }
  target.sign = sign;
  target.i = add_i;
  target.r = add_r;
  //target.make_str();
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::DecimalState::sub_(
  bool sign, const DecimalState& right, DecimalState& target,
  bool& /*overflow*/) const throw ()
{
  assert(i > right.i || (i == right.i && r >= right.r));
  uint64_t sub_i = i;
  uint64_t sub_r = r;
  sub64<MAX_FRACTION>(sub_i, sub_r, right.i, right.r);
  target.sign = sign;
  target.i = sub_i;
  target.r = sub_r;
  //target.make_str();
}


//
// RandomTestDecimal class
//

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::RandomTestDecimal()
  throw ()
{
  n1 = SelfDecimal(state.sign, state.i, state.r);
  n2 = SelfDecimal(String::SubString(state.str()));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::RandomTestDecimal(
  const DecimalState& state_in, const SelfDecimal& n2_in) throw ()
  : state(state_in)
{
  n1 = SelfDecimal(state_in.sign, state_in.i, state_in.r);
  n2 = n2_in;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::maximum(
  bool& /*overflow*/) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState res(false, TOTAL > FRACTION, TOTAL == FRACTION);
  bool result = n2 <= SelfDecimal::MAXIMUM;
  return RandomTestDecimal(res,
    SelfDecimal(false, TOTAL > FRACTION ? result : 0,
      TOTAL == FRACTION ? result : 0));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::negate(
  bool& /*overflow*/) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState res(!state.sign, state.i, state.r);
  return RandomTestDecimal(res, SelfDecimal(n2).negate());
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::floor(
  bool& /*overflow*/) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState res(state);
  int level = rand() % (8 * TOTAL);
  if (level < static_cast<int>(FRACTION))
  {
    uint64_t pow = DecimalHelper::pow10<uint64_t>(FRACTION - level);
    res.r = res.r / pow * pow;
  }
  return RandomTestDecimal(res, SelfDecimal(n2).floor(level));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::ceil(
  bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState res(state);
  int level = rand() % (8 * TOTAL);
  if (level < static_cast<int>(FRACTION))
  {
    uint64_t pow = DecimalHelper::pow10<uint64_t>(FRACTION - level);
    if (res.r % pow)
    {
      if (res.r / pow == MAX_FRACTION / pow - 1)
      {
        if (res.i == MAX_INTEGER - 1)
        {
          overflow = true;
        }
        else
        {
          res.r = 0;
          res.i++;
        }
      }
      else
      {
        res.r = (res.r / pow + 1) * pow;
      }
    }
  }
  return RandomTestDecimal(res, SelfDecimal(n2).ceil(level));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::add(
  const RandomTestDecimal& right, bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState product;
  state.add(right.state, product, overflow);
  return RandomTestDecimal(product, n2 + right.n2);
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::sub(
  const RandomTestDecimal& right, bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState product;
  state.sub(right.state, product, overflow);
  return RandomTestDecimal(product, n2 - right.n2);
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::mul_floor(
  const RandomTestDecimal& right, bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState product;
  state.mul(right.state, product, DMR_FLOOR, overflow);
  return RandomTestDecimal(product,
    SelfDecimal::mul(n2, right.n2, DMR_FLOOR));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::mul_round(
  const RandomTestDecimal& right, bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState product;
  state.mul(right.state, product, DMR_ROUND, overflow);
  return RandomTestDecimal(product,
    SelfDecimal::mul(n2, right.n2, DMR_ROUND));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::mul_ceil(
  const RandomTestDecimal& right, bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState product;
  state.mul(right.state, product, DMR_CEIL, overflow);
  return RandomTestDecimal(product,
    SelfDecimal::mul(n2, right.n2, DMR_CEIL));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::div_quotient(
  const RandomTestDecimal& right, bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState quotient, remainder;
  state.div(right.state, quotient, remainder, overflow);
  SelfDecimal sd_remainder;
  return RandomTestDecimal(quotient, SelfDecimal::div(n2, right.n2,
    sd_remainder));
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::div_remainder(
  const RandomTestDecimal& right, bool& overflow) const
  throw (typename SelfDecimal::Overflow)
{
  DecimalState quotient, remainder;
  state.div(right.state, quotient, remainder, overflow);
  SelfDecimal sd_remainder;
  SelfDecimal::div(n2, right.n2, sd_remainder);
  return RandomTestDecimal(remainder, sd_remainder);
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::equal_to(
  const RandomTestDecimal& right, bool& expected) const throw ()
{
  expected = !state.less_than(right.state) && !right.state.less_than(state);
  return n2 == right.n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::not_equal_to(
  const RandomTestDecimal& right, bool& expected) const throw ()
{
  expected = state.less_than(right.state) || right.state.less_than(state);
  return n2 != right.n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::less_than(
  const RandomTestDecimal& right, bool& expected) const throw ()
{
  expected = state.less_than(right.state);
  return n2 < right.n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::
  less_than_or_equal_to(const RandomTestDecimal& right, bool& expected) const
  throw ()
{
  expected = !right.state.less_than(state);
  return n2 <= right.n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::greater_than(
  const RandomTestDecimal& right, bool& expected) const throw ()
{
  expected = right.state.less_than(state);
  return n2 > right.n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::
  greater_than_or_equal_to(const RandomTestDecimal& right, bool& expected)
  const throw ()
{
  expected = !state.less_than(right.state);
  return n2 >= right.n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
const char*
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::str() const throw ()
{
  return state.str();
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::equal() const
  throw ()
{
  return n1 == n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::str_equal() const
  throw (eh::Exception)
{
  return n1.str() == n2.str();
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::
  from_str_equal() const throw (eh::Exception)
{
  SelfDecimal n(String::SubString(n1.str()));
  return n == n2;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
template <typename IntType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::
  test_to_int_equal() const
  throw (typename SelfDecimal::Overflow, typename SelfDecimal::Sign)
{
  IntType x1, x2;
  n1.to_integer(x1);
  n2.to_integer(x2);
  return x1 == x2;
}


template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
template <typename IntType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::
  test_to_int_with_sign(const char* what) const throw ()
{
  bool expected_sign_err = state.i && !std::numeric_limits<IntType>::is_signed
    && state.sign;
  try
  {
    test_to_int_equal<IntType>();
    if (expected_sign_err)
    {
      std::cerr << "Fail expected Sign for n1.to_integer for " << what
                << " : n1 =  " << n1.str()
                << ", n2 = " << n2.str() << " "
                << std::endl;
      return false;
    }
  }
  catch (const typename SelfDecimal::Sign&)
  {
    if (!expected_sign_err)
    {
      std::cerr << "Fail unexpected Sign for n1.to_integer for " << what
                << " : n1 =  " << n1.str()
                << ", n2 = " << n2.str() << " "
                << std::endl;
      return false;
    }
  }
  return true;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
template <typename IntType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::
  test_to_int_with_overflow(const char* what) const throw ()
{
  bool expected_overflow = static_cast<unsigned long long>(state.i) >
    static_cast<unsigned long long>(std::numeric_limits<IntType>::max());
  try
  {
    if (test_to_int_equal<IntType>())
    {
      if (expected_overflow)
      {
        std::cerr << "Fail expected overflow for n1.to_integer for " << what
                  << " : n1 =  " << n1.str()
                  << ", n2 = " << n2.str() << " "
                  << std::endl;
        return false;
      }
      test_to_int_with_sign<IntType>(what);
    }
    else
    {
      std::cerr << "Fail equal n1.to_integer for " << what
                << " : n1 =  " << n1.str()
                << ", n2 = " << n2.str() << " "
                << std::endl;
      return false;
    }
  }
  catch (const typename SelfDecimal::Overflow&)
  {
    if (!expected_overflow)
    {
      std::cerr << "Fail unexpected overflow for n1.to_integer for " << what
                << " : n1 =  " << n1.str()
                << ", n2 = " << n2.str() << " "
                << std::endl;
      return false;
    }
  }
  return true;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::test_to_int(
  const char*) const throw ()
{
  test_to_int_with_overflow<signed char>("signed char");
  test_to_int_with_overflow<signed short>("signed short");
  test_to_int_with_overflow<signed int>("signed int");
  test_to_int_with_overflow<signed long>("signed long");
  test_to_int_with_overflow<signed long long>("signed long long");
  return true;
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
bool
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::test_equal(
  const char* what) const throw ()
{
  if (!equal())
  {
    std::cerr << "Fail equal for " << what
              << " : expected " << n1.str()
              << " but got " << n2.str() << " "
              << std::endl;
    return false;
  }
  else
  {
    if (!str_equal())
    {
      std::cerr << "Fail equal for " << what
                << " strings: expected " << n1.str()
                << " but got " << n2.str()
                << " with: \"" << state.debug_str() << "\" "
                << std::endl;
      return false;
    }
    else
    {
      if (!from_str_equal())
      {
        std::cerr << "Fail n2 from str of n1: must " << n1.str()
                  << " but " << n2.str()
                  << " with: \"" << state.debug_str() << "\" "
                  << std::endl;
        return false;
      }
    }
  }
  return true;
}

template <typename Element, const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::test_unary_op(
  RandomTestDecimal::TestUnaryOperation op, const char* name) throw ()
{
  bool overflow = false;
  try
  {
    RandomTestDecimal n = (this->*op)(overflow);

    if (overflow)
    {
      std::cerr << "expected exception Overflow: " << n2.str() << " " <<
        name << ' ' << TOTAL << ':' << FRACTION << " but got " << n.str() <<
        std::endl;
      return;
    }

    if (!n.test_equal(name))
    {
      std::cerr << "in " << n2.str() << " " << name << std::endl;
    }
  }
  catch (const Overflow& ex)
  {
    if (!overflow)
    {
      std::cerr << "unexpected exception Overflow: " << ex.what() <<
        " " << n2.str() << " " << name << std::endl;
    }
  }
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::test_op(
  const RandomTestDecimal& right, RandomTestDecimal::TestOperation op,
  const char* name) throw ()
{
  bool overflow = false;
  try
  {
    RandomTestDecimal n = (this->*op)(right, overflow);

    if (overflow)
    {
      std::cerr << "expected exception Overflow: " << n2.str() << " " <<
        name << " " << right.n2.str() << ' ' << TOTAL << ':' << FRACTION <<
        " but got " << n.str() << std::endl;
      return;
    }

    if (!n.test_equal(name))
    {
      std::cerr << "in " << n2.str() << " " << name << " " << right.n2.str()
        //<< " " << TOTAL << "." << FRACTION
                << std::endl;
    }
  }
  catch (const Overflow& ex)
  {
    if (!overflow)
    {
      std::cerr << "unexpected exception Overflow: " << ex.what() <<
        " " << n2.str() << " " << name << " " << right.n2.str() << std::endl;
    }
  }
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::test_logic_op(
  const RandomTestDecimal& right, RandomTestDecimal::TestLogicOperation op,
  const char* name) throw ()
{
  bool expected;
  try
  {
    bool result = (this->*op)(right, expected);
    if (result != expected)
    {
      std::cerr << "invalid result (" << (result ? "true" : "false") <<
        ") in " << n2.str() << " " << name << " " << right.n2.str() << ' ' <<
        TOTAL << ':' << FRACTION << " (expected " <<
        (expected ? "true" : "false") << ")" << std::endl;
    }
  }
  catch (const Exception& ex)
  {
    std::cerr << "unexpected exception: " << ex.what() <<
      " " << n2.str() << " " << name << " " << right.n2.str() << std::endl;
  }
}

template <typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>::test_ops(
  const RandomTestDecimal& right) throw ()
{
  test_unary_op(&RandomTestDecimal::maximum, "maximum");

  test_unary_op(&RandomTestDecimal::negate, "negate");
  test_unary_op(&RandomTestDecimal::floor, "floor");
  test_unary_op(&RandomTestDecimal::ceil, "ceil");

  test_op(right, &RandomTestDecimal::add, "+");
  test_op(right, &RandomTestDecimal::sub, "-");
  test_op(right, &RandomTestDecimal::mul_floor, "floor(*)");
  test_op(right, &RandomTestDecimal::mul_round, "round(*)");
  test_op(right, &RandomTestDecimal::mul_ceil, "ceil(*)");
  if (MAX_VALUE / MAX_FRACTION / MAX_FRACTION >= MAX_INTEGER)
  {
    test_op(right, &RandomTestDecimal::div_quotient, "/");
    test_op(right, &RandomTestDecimal::div_remainder, "%");
  }

  test_logic_op(right, &RandomTestDecimal::equal_to, "==");
  test_logic_op(right, &RandomTestDecimal::not_equal_to, "!=");
  test_logic_op(right, &RandomTestDecimal::less_than, "<");
  test_logic_op(right, &RandomTestDecimal::less_than_or_equal_to, "<=");
  test_logic_op(right, &RandomTestDecimal::greater_than, ">");
  test_logic_op(right, &RandomTestDecimal::greater_than_or_equal_to, ">=");
}

template <typename FloatType>
bool
check_convert (FloatType fvalue) throw ()
{
#if 0
  if (std::numeric_limits<FloatType>::infinity() == fvalue ||
    std::numeric_limits<FloatType>::quiet_NaN() == fvalue ||
    std::numeric_limits<FloatType>::signaling_NaN() == fvalue)
#else
  if (HUGE_VALF == fvalue || HUGE_VALL == fvalue || HUGE_VAL == fvalue)
#endif
  {
    return false;
  }
  if (0.0 == fvalue)
  {
    if (errno == ERANGE)
    {
      return false;
    }
  }
  return true;
}

inline bool
strtofloat(const char* str, float& val) throw ()
{
  val = strtof(str, 0);
  if (!check_convert(fabsf(val)))
  {
    val = std::numeric_limits<float>::signaling_NaN();
    return false;
  }
  return true;
}

inline bool
strtofloat(const char* str, double& val) throw ()
{
  val = strtod(str, 0);
  if (!check_convert(fabs(val)))
  {
    val = std::numeric_limits<double>::signaling_NaN();
    return false;
  }
  return true;
}

inline bool
strtofloat(const char* str, long double& val) throw ()
{
  val = strtold(str, 0);
  if (!check_convert(fabsl(val)))
  {
    val = std::numeric_limits<long double>::signaling_NaN();
    return false;
  }
  return true;
}

inline uint64_t
roundx(float val) throw ()
{
  return static_cast<uint64_t>(roundf(fabsf(val)));
}


inline uint64_t
roundx(double val) throw ()
{
  return static_cast<uint64_t>(round(fabs(val)));
}

inline uint64_t
roundx(long double val) throw ()
{
  return static_cast<uint64_t>(roundl(fabsl(val)));
}

template <typename FloatType, typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION,
  template <typename, const unsigned, const unsigned> class DecimalType>
void
do_from_string_test() throw ()
{
  typedef DecimalType<Element, TOTAL, FRACTION> SelfDecimal;
  static const uint64_t MAX_INTEGER =
    DecimalHelper::Pow10<uint64_t, SelfDecimal::INTEGER_RANK>::Value;
  unsigned total_size =
    random() % (8 * std::numeric_limits<FloatType>::digits10) + 1;

  bool sign = (random() % 2) == 0;
  unsigned fraction_size = random() % total_size;
  unsigned int_size = total_size - fraction_size;

  std::string num;
  num.resize(total_size + 4);
  char* p = &num[0];
  if (sign)
  {
    *p++ = '-';
  }
  if (int_size)
  {
    *p++ = '1' + (random() % 9);
    for (unsigned i = 1; i != int_size; ++i)
    {
      *p++ = '0' + (random() % 10);
    }
  }
  else
  {
    *p++ = '0';
  }
  if (fraction_size)
  {
    *p++ = '.';
    for (unsigned i = 0; i != fraction_size; ++i)
    {
      *p++ = '0' + (random() % 10);
    }
  }
  num.resize(p - &num[0]);

  FloatType fvalue = 0.0;
  bool cant_create = !strtofloat(num.c_str(), fvalue);
  bool too_big = int_size > TOTAL - FRACTION || roundx(fvalue) >= MAX_INTEGER;
  try
  {
    try
    {
      SelfDecimal decf(fvalue);
      if (cant_create)
      {
        std::cerr << "unexpected create decimal from str: " << num  <<
          " float: " << fvalue << " " << cant_create << std::endl;
      }
      if (too_big && fvalue >= MAX_INTEGER)
      {
        std::cerr << "unexpected create decimal from big str: " << num <<
          " float: " << fvalue << std::endl;
      }
    }
    catch (const typename SelfDecimal::NotNumber&)
    {
      if (!cant_create)
      {
        std::cerr << "unexpected error to create decimal from str: " <<
          num << " float: " << fvalue << std::endl;
      }
    }
  }
  catch (const typename SelfDecimal::Overflow& err)
  {
    if (!too_big)
    {
      std::cerr << "unexpected error: " << err.what() <<
        " to create decimal from str: " << num << " decimal as " <<
        TOTAL << ":" << FRACTION << ' '
        << roundx(fvalue) << ' ' << fvalue
        << ' ' << too_big << ' ' << cant_create
        << std::endl;
    }
  }
}

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element,
  const unsigned int TOTAL,
  const unsigned int FRACTION>
void
do_random(const char* name) throw ()
{
  typedef RandomTestDecimal<Element, TOTAL, FRACTION, DecimalType>
    SelfTestDecimal;

  std::cout << "Random test " << name << " " <<
    std::numeric_limits<Element>::digits10 << ":" << TOTAL << ":" <<
    FRACTION << std::endl;

  for (int i = 0; i < 100; ++i)
  {
    SelfTestDecimal n1, n2;
    n1.test_equal();
    n1.test_to_int();
    n2.test_equal();
    n2.test_to_int();
    n1.test_ops(n2);
  }

  for (int i = 0; i < 50; ++i)
  {
    do_from_string_test<float, Element, TOTAL, FRACTION, DecimalType>();
    do_from_string_test<double, Element, TOTAL, FRACTION, DecimalType>();
    do_from_string_test<long double, Element, TOTAL, FRACTION, DecimalType>();
  }
}

//
// Wrappers around do_random
//

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element, const unsigned Total, const unsigned FractionPlusOne>
struct RandomTester :
  RandomTester<DecimalType, Element, Total, FractionPlusOne - 1>
{
  RandomTester(const char* name) throw (eh::Exception);
};

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element, const unsigned Total, const unsigned FractionPlusOne>
RandomTester<DecimalType, Element, Total, FractionPlusOne>::RandomTester(
  const char* name) throw (eh::Exception)
  : RandomTester<DecimalType, Element, Total, FractionPlusOne - 1>(name)
{
  do_random<DecimalType, Element, Total, FractionPlusOne - 1>(name);
}

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element, const unsigned Total>
struct RandomTester<DecimalType, Element, Total, 0>
{
  RandomTester(const char*) throw ();
};

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element, const unsigned Total>
RandomTester<DecimalType, Element, Total, 0>::RandomTester(const char*)
  throw ()
{
}

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element, const unsigned Total>
struct Random :
  Random<DecimalType, Element, Total - 1>,
  RandomTester<DecimalType, Element, Total, Total + 1>
{
  Random(const char* name) throw (eh::Exception);
};

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element, const unsigned Total>
Random<DecimalType, Element, Total>::Random(const char* name)
  throw (eh::Exception)
  : Random<DecimalType, Element, Total - 1>(name),
    RandomTester<DecimalType, Element, Total, Total + 1>(name)
{
}

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element>
struct Random<DecimalType, Element, 0>
{
  Random(const char*) throw ();
};

template <
  template <typename, const unsigned, const unsigned> class DecimalType,
  typename Element>
Random<DecimalType, Element, 0>::Random(const char*) throw ()
{
}
