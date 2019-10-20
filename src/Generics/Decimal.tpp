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





//#define DEBUG_DECIMAL

#include <cstddef>


namespace Generics
{
  //
  // Decimal::MulTmpArray class
  //

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::
    TMP_FRACTION_RANK;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::
    TMP_TOTAL_RANK;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::
    TMP_SIZE;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Element Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::
    TMP_INTEGER_MAX_OVER;

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  std::string
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::dump() const
    throw (eh::Exception)
  {
    Stream::Stack<SIZE * (DIGITS_PER_ELEMENT + 1) + 33> ostr;
    ostr << SIZE << ':' << TOTAL_RANK << '.' << FRACTION_RANK;
    for (unsigned i = 0; i != SIZE; i++)
    {
      ostr << ' ' << std::setfill('0') << std::setw(DIGITS_PER_ELEMENT) <<
        tmp_array_[i];
    }
    return std::string(ostr->available_data(), ostr->available_size());
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::MulTmpArray()
    throw ()
  {
    std::fill(tmp_array_, tmp_array_ + TMP_SIZE, 0);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::add(
    Element value, unsigned index) throw ()
  {
    if (index >= TMP_SIZE)
    {
      return true;
    }

    Element overflow;
    unsigned i = index;
    if (MAX_SUM > 2 || static_cast<Element>(-1) - value >= tmp_array_[i])
    {
      tmp_array_[i] += value;
      if (tmp_array_[i] >= BASE)
      {
        overflow = tmp_array_[i] / BASE;
        tmp_array_[i] %= BASE;
      }
      else
      {
        overflow = 0;
      }
    }
    else
    {
      assert(value > BASE);
      overflow = value / BASE;
      value %= BASE;
      tmp_array_[i] += value;
      if (tmp_array_[i] >= BASE)
      {
        overflow++;
        tmp_array_[i] -= BASE;
      }
    }

    for (i++; i < TMP_SIZE; i++)
    {
      tmp_array_[i] += overflow;
      if (tmp_array_[i] < Decimal::BASE)
      {
        if (i != TMP_SIZE - 1)
        {
          return false;
        }
        overflow = 0;
        break;
      }
      tmp_array_[i] -= BASE;
      overflow = 1;
    }

    return overflow || tmp_array_[TMP_SIZE - 1] >= TMP_INTEGER_MAX_OVER;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::round() throw ()
  {
    if (FRACTION_REMAINDER == 1 ?
      tmp_array_[FRACTION_END - 1] >= BASE / 2 :
      tmp_array_[FRACTION_END] % FRACTION_REMAINDER >=
        FRACTION_REMAINDER / 2)
    {
      if (add(FRACTION_REMAINDER, FRACTION_END))
      {
        return true;
      }
    }
    return false;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::ceil() throw ()
  {
    if (FRACTION_REMAINDER == 1 ? tmp_array_[FRACTION_END - 1] :
      tmp_array_[FRACTION_END] % FRACTION_REMAINDER)
    {
      if (add(FRACTION_REMAINDER, FRACTION_END))
      {
        return true;
      }
    }
    return false;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MulTmpArray::export_to(
    Decimal& result) const throw ()
  {
    if (FRACTION_REMAINDER == 1)
    {
      std::copy(this->tmp_array_ + FRACTION_END,
        this->tmp_array_ + FRACTION_END + SIZE, result.array_);
    }
    else
    {
      unsigned index = FRACTION_END;
      Element last = this->tmp_array_[index++] / FRACTION_REMAINDER;
      for (unsigned i = 0; i != SIZE; i++, index++)
      {
        if (index == TMP_SIZE)
        {
          result.array_[i] = last;
          break;
        }
        result.array_[i] = last +
          this->tmp_array_[index] % FRACTION_REMAINDER * FRACTION_OVER;
        last = this->tmp_array_[index] / FRACTION_REMAINDER;
      }
    }
  }


  //
  // Decimal::DivTmpArrayBase class
  //

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <const unsigned TMP_DIV_SIZE>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DivTmpArrayBase<TMP_DIV_SIZE>::DivTmpArrayBase() throw ()
    : size_(TMP_DIV_SIZE), initial_size_(0)
  {
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <const unsigned TMP_DIV_SIZE>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DivTmpArrayBase<TMP_DIV_SIZE>::shrink() throw ()
  {
    for (; size_ > 1 && !tmp_array_[size_ - 1]; size_--)
    {
    }
    if (!initial_size_)
    {
      initial_size_ = size_;
    }
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <const unsigned TMP_DIV_SIZE>
  unsigned
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DivTmpArrayBase<TMP_DIV_SIZE>::size() throw ()
  {
    return size_;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <const unsigned TMP_DIV_SIZE>
  unsigned
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DivTmpArrayBase<TMP_DIV_SIZE>::initial_size() throw ()
  {
    return initial_size_;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <const unsigned TMP_DIV_SIZE>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DivTmpArrayBase<TMP_DIV_SIZE>::mul(Element multiplicator) throw ()
  {
#ifdef DEBUG_DECIMAL
    std::cerr << "BM: " << dump() << "\n";
#endif
    Element overflow = 0;
    for (unsigned i = 0; i < size_; i++)
    {
      Element result, over;
      mul_elements_(tmp_array_[i], multiplicator, result, over);
      result += overflow;
      if (result >= BASE)
      {
        result -= BASE;
        over++;
      }
      tmp_array_[i] = result;
      overflow = over;
    }
    if (overflow)
    {
      assert(size_ < TMP_DIV_SIZE);
      tmp_array_[size_++] = overflow;
    }
#ifdef DEBUG_DECIMAL
    std::cerr << "AM: " << dump() << "\n";
#endif
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <const unsigned TMP_DIV_SIZE>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DivTmpArrayBase<TMP_DIV_SIZE>::div(Element divisor, Element& remainder)
    throw ()
  {
    Element r = 0;
    for (int i = size_ - 1; i >= 0; i--)
    {
      div_elements_(r, tmp_array_[i], divisor, tmp_array_[i], r);
    }
    remainder = r;
    shrink();
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <const unsigned TMP_DIV_SIZE>
  std::string
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DivTmpArrayBase<TMP_DIV_SIZE>::dump() const throw (eh::Exception)
  {
    char out[TMP_DIV_SIZE*(DIGITS_PER_ELEMENT+1) + 128];
    {
      Stream::Buffer<sizeof(out)> strm(out);
      strm << TMP_DIV_SIZE << ':' << TOTAL_RANK << '.' << FRACTION_RANK <<
        " " << size_ << " " << initial_size_;
      for (int i = TMP_DIV_SIZE - 1; i >= 0; i--)
      {
        strm << ' ' << std::setfill('0') << std::setw(DIGITS_PER_ELEMENT) <<
          static_cast<unsigned long long>(tmp_array_[i]);
      }
    }
    return out;
  }


  //
  // Decimal::DivTmpDividend class
  //

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDividend::
    DivTmpDividend(const Decimal& dividend) throw ()
  {
    if (FRACTION_REMAINDER == 1)
    {
      std::fill(this->tmp_array_, this->tmp_array_ + FRACTION_END, 0);
      std::copy(dividend.array_, dividend.array_ + SIZE,
        this->tmp_array_ + FRACTION_END);
      std::fill(this->tmp_array_ + FRACTION_END + SIZE,
        this->tmp_array_ + DIV_TMP_SIZE, 0);
    }
    else
    {
      std::fill(this->tmp_array_, this->tmp_array_ + FRACTION_END, 0);
      Element over = 0;
      for (unsigned i = 0; i < SIZE; i++)
      {
        this->tmp_array_[FRACTION_END + i] =
          dividend.array_[i] % FRACTION_OVER * FRACTION_REMAINDER + over;
        over = dividend.array_[i] / FRACTION_OVER;
      }
      this->tmp_array_[FRACTION_END + SIZE] = over;
      std::fill(this->tmp_array_ + FRACTION_END + SIZE + 1,
        this->tmp_array_ + DIV_TMP_SIZE, 0);
    }
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Element
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDividend::
    guess_next_quotient(unsigned index, Element max_div,
      Element pre_max_div) throw ()
  {
#ifdef DEBUG_DECIMAL
    std::cerr << "guess_next_quotient " << index << "\n";
#endif
    assert(index >= 1);
    Element guess, guess_high;
    Element r;
    assert(this->tmp_array_[index] <= max_div ||
      max_div / 2 < this->tmp_array_[index]);
    if (this->tmp_array_[index] >= max_div)
    {
      div_elements_(this->tmp_array_[index] - max_div,
        this->tmp_array_[index - 1], max_div, guess, r);
      guess_high = 1;
    }
    else
    {
      div_elements_(this->tmp_array_[index], this->tmp_array_[index - 1],
        max_div, guess, r);
      guess_high = 0;
    }
    while (r < BASE)
    {
      assert(index >= 2);
      Element minor, major;
      mul_elements_(pre_max_div, guess, minor, major);
      major += guess_high * pre_max_div;
      if ((major < r ||
        (major == r && minor <= this->tmp_array_[index - 2])) &&
        (guess_high != 1 || guess))
      {
        break;
      }
      if (!guess)
      {
        guess_high--;
        guess = BASE - 1;
      }
      else
      {
        guess--;
      }
      r += max_div;
    }
    return guess;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDividend::
    apply_next_quotient(unsigned index, Element guess,
    const DivTmpDivisor& divisor) throw ()
  {
#ifdef DEBUG_DECIMAL
    std::cerr << "apply_next_quotient " << index << " " <<
      static_cast<unsigned long long>(guess) << "\n" << dump() << "\n" <<
      divisor.dump() << "\n";
#endif
    assert(index + divisor.initial_size_ <= DIV_TMP_SIZE);

    Element* target = this->tmp_array_ + index;
    unsigned size = divisor.initial_size_;

    Element carry = 0;
    Element borrow = 0;

    for (unsigned i = 0; i < size; i++)
    {
      Element temp;
      {
        Element temp2;
        mul_elements_(divisor.tmp_array_[i], guess, temp, temp2);
        temp += carry;
        if (temp >= BASE)
        {
          temp -= BASE;
          temp2++;
        }
        carry = temp2;
      }
      temp += borrow;
#ifdef DEBUG_DECIMAL
      std::cerr << "T[i] T"
        " " << static_cast<unsigned long long>(target[i]) <<
        " " << static_cast<unsigned long long>(temp) <<
        "\n";
#endif
      if (target[i] < temp)
      {
        target[i] += BASE - temp;
        borrow = 1;
      }
      else
      {
        target[i] -= temp;
        borrow = 0;
      }
    }

    {
      Element temp = carry + borrow;
#ifdef DEBUG_DECIMAL
      std::cerr << "T[size] T"
        " " << static_cast<unsigned long long>(target[size]) <<
        " " << static_cast<unsigned long long>(temp) <<
        "\n";
#endif
      if (target[size] < temp)
      {
        target[size] += BASE - temp;
        borrow = 1;
      }
      else
      {
        target[size] -= temp;
        borrow = 0;
      }
    }

    return borrow != 0;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDividend::
    fix_next_quotient(unsigned index, const DivTmpDivisor& divisor) throw ()
  {
    Element* target = this->tmp_array_ + index;
    unsigned size = divisor.initial_size_;

    Element carry = 0;
    for (unsigned i = 0; i < size; i++)
    {
      Element temp = target[i] + divisor.tmp_array_[i] + carry;
      if (temp >= BASE)
      {
        target[i] = temp - BASE;
        carry = 1;
      }
      else
      {
        target[i] = temp;
        carry = 0;
      }
    }
    target[size] -= BASE - carry;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDividend::
    export_to(Decimal& result) throw ()
  {
    if (this->tmp_array_[SIZE - 1] >= INTEGER_MAX_OVER)
    {
      return true;
    }
    for (unsigned i = SIZE; i != DIV_TMP_SIZE; i++)
    {
      if (this->tmp_array_[i])
      {
        return true;
      }
    }
    std::copy(this->tmp_array_, this->tmp_array_ + SIZE, result.array_);
    return false;
  }


  //
  // Decimal::DivTmpDivisor class
  //

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDivisor::DivTmpDivisor(
    const Decimal& divider) throw ()
  {
    std::copy(divider.array_, divider.array_ + SIZE, this->tmp_array_);
    this->tmp_array_[SIZE] = 0;
  }

  template <typename Element,
    const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Element
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDivisor::max_element()
    throw ()
  {
    return this->tmp_array_[this->size_ - 1];
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Element
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DivTmpDivisor::
    pre_max_element() throw ()
  {
    return this->tmp_array_[this->size_ - 2];
  }


  //
  // Decimal class
  //

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DIGITS_PER_ELEMENT;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Element Decimal<Element, TOTAL_RANK, FRACTION_RANK>::BASE;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MAX_SUM;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::SIZE;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Element Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    INTEGER_MAX_OVER;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::FRACTION_END;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Element Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    FRACTION_REMAINDER;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Element Decimal<Element, TOTAL_RANK, FRACTION_RANK>::FRACTION_OVER;

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::PACK_SIZE;

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Element Decimal<Element, TOTAL_RANK, FRACTION_RANK>::INVALID_FLAG_;

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DIV_TMP_FRACTION_RANK;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::
    DIV_TMP_TOTAL_RANK;
  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const unsigned Decimal<Element, TOTAL_RANK, FRACTION_RANK>::DIV_TMP_SIZE;

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename Integer, typename Fraction>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::construct_(bool negative,
    Integer integer, Fraction fraction) throw (Overflow)
  {
    DecimalIntegerCheck<Integer>();
    DecimalIntegerCheck<Fraction>();

    Integer old_integer = integer;
    Fraction old_fraction = fraction;

    for (unsigned i = 0; i != FRACTION_END; i++)
    {
      array_[i] = static_cast<Element>(fraction % BASE);
      fraction = static_cast<Fraction>(fraction / BASE);
    }
    if (DecimalHelper::exceeds(fraction, FRACTION_REMAINDER))
    {
      Stream::Error ostr;
      ostr << FNS << "fraction " << old_fraction <<
        " exceeds maximum allowed of " << FRACTION_RANK << " digits";
      throw Overflow(ostr);
    }
    array_[FRACTION_END] = static_cast<Element>(fraction +
      static_cast<Element>(integer % FRACTION_OVER) * FRACTION_REMAINDER);
    if (INTEGER_RANK)
    {
      integer = static_cast<Integer>(integer / FRACTION_OVER);
      unsigned i = FRACTION_END + 1;
      for (; i + 1 < SIZE; i++)
      {
        array_[i] = static_cast<Element>(integer % BASE);
        integer = static_cast<Integer>(integer / BASE);
      }
      if (i == SIZE - 1)
      {
        array_[i] = static_cast<Element>(integer % INTEGER_MAX_OVER);
        integer = static_cast<Integer>(integer / INTEGER_MAX_OVER);
      }
    }
    if (integer)
    {
      Stream::Error ostr;
      ostr << FNS << "integer " << old_integer <<
        " exceeds maximum allowed of " << INTEGER_RANK << " digits";
      throw Overflow(ostr);
    }
    negative_ = negative;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename Integer>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::construct_(Integer integer,
    unsigned power) throw (Overflow)
  {
    DecimalIntegerCheck<Integer>();

    Integer old_integer = integer;
    unsigned old_power = power;

    if (power >= std::numeric_limits<Integer>::digits10 + FRACTION_RANK ||
      !integer)
    {
      negative_ = false;
      std::fill(array_, array_ + SIZE, 0);
      return;
    }

    DecimalHelper::split(integer, negative_);

    if (power > FRACTION_RANK)
    {
      integer /= DecimalHelper::pow10<Integer>(power - FRACTION_RANK);
      power = FRACTION_RANK;
    }

    unsigned diff = FRACTION_RANK - power;
    unsigned i = diff / DIGITS_PER_ELEMENT;
    std::fill(array_, array_ + i, 0);
    diff %= DIGITS_PER_ELEMENT;
    if (diff)
    {
      Element mask =
        DecimalHelper::pow10<Element>(DIGITS_PER_ELEMENT - diff);
      array_[i++] = static_cast<Element>(integer % mask) * (BASE / mask);
      integer /= mask;
    }
    for (; integer && i + 1 < SIZE; i++)
    {
      array_[i] = static_cast<Element>(integer % BASE);
      integer = static_cast<Integer>(integer / BASE);
    }
    if (i == SIZE - 1)
    {
      array_[i] = static_cast<Element>(integer % INTEGER_MAX_OVER);
      integer = static_cast<Integer>(integer / INTEGER_MAX_OVER);
    }
    else
    {
      std::fill(array_ + i, array_ + SIZE, 0);
    }
    if (integer)
    {
      Stream::Error ostr;
      ostr << FNS << "Initializer " << old_integer << " with power " <<
        old_power << " exceeds maximum allowed of " << INTEGER_RANK <<
        " integer part digits";
      throw Overflow(ostr);
    }
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::construct_(
    const String::SubString& str) throw (Overflow, NotNumber)
  {
    const char* begin = str.begin();
    const char* end = str.end();
    if (begin == end)
    {
      Stream::Error ostr;
      ostr << FNS << "empty string passed";
      throw NotNumber(ostr);
    }
    negative_ = false;
    switch (*begin)
    {
    case '-':
      negative_ = true;
    case '+':
      begin++;
    }
    if (begin == end)
    {
      Stream::Error ostr;
      ostr << FNS << "empty number passed";
      throw NotNumber(ostr);
    }
    while (begin != end && '0' == *begin)
    {
      begin++;
    }
    const String::SubString::SizeType point_pos = str.find('.');
    const char* integer_end = end;
    const char* fraction_begin = end;
    if (point_pos != String::SubString::NPOS)
    {
      integer_end = str.begin() + point_pos;
      fraction_begin = integer_end + 1;
      while (end != begin && '0' == end[-1])
      {
        end--;
      }
      if (end[-1] == '.')
      {
        integer_end = fraction_begin = --end;
      }
    }
    if (begin == end)
    {
      std::fill(array_, array_ + SIZE, 0);
      return;
    }
    if (static_cast<ptrdiff_t>(FRACTION_RANK) < (end - fraction_begin))
    {
      Stream::Error ostr;
      ostr << FNS << "number of digits in fraction of '" << str <<
        "' is bigger than " << FRACTION_RANK;
      throw Overflow(ostr);
    }
    if (static_cast<ptrdiff_t>(INTEGER_RANK) < (integer_end - begin))
    {
      Stream::Error ostr;
      ostr << FNS << "number of digits in integer of '" << str <<
        "' is bigger than " << INTEGER_RANK;
      throw Overflow(ostr);
    }

    unsigned char num[TOTAL_RANK];

    unsigned char* fraction = num + INTEGER_RANK;
    for (const char* i = fraction_begin; i != end; i++, fraction++)
    {
      if (!isdigit(*i))
      {
        Stream::Error ostr;
        ostr << FNS << "string '" << str <<
          "' contains non-digit character";
        throw NotNumber(ostr);
      }
      *fraction = *i - '0';
    }
    std::fill(fraction, num + TOTAL_RANK, 0);
    unsigned char* integer = num + (INTEGER_RANK - (integer_end - begin));
    std::fill(num, integer, 0);
    for (const char* i = begin; i != integer_end; i++, integer++)
    {
      if (!isdigit(*i))
      {
        Stream::Error ostr;
        ostr << FNS << "string '" << str <<
          "' contains non-digit character";
        throw NotNumber(ostr);
      }
      *integer = *i - '0';
    }

    {
      unsigned n = TOTAL_RANK;
      for (unsigned i = 0; i < SIZE; i++)
      {
        unsigned digits = n >= DIGITS_PER_ELEMENT ? DIGITS_PER_ELEMENT : n;
        n -= digits;
        array_[i] =
          DecimalHelper::assemble_decimal<Element>(digits, num + n);
      }
    }
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::Decimal() throw ()
    : negative_(false)
  {
    array_[0] = INVALID_FLAG_;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename Integer, typename Fraction>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::Decimal(bool negative,
    Integer integer, Fraction fraction) throw (Overflow)
  {
    construct_(negative, integer, fraction);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename Integer>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::Decimal(Integer integer,
    unsigned power) throw (Overflow)
  {
    construct_(integer, power);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
    template <typename DiffBase, const unsigned DIFF_TOTAL,
      const unsigned DIFF_FRACTION>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::Decimal(
    const SimpleDecimal<DiffBase, DIFF_TOTAL, DIFF_FRACTION>& diff)
    throw (Overflow)
  {
    static_assert(DIFF_FRACTION <= Decimal::FRACTION_RANK,
      "different SimpleDecimal is more precise");

    construct_(diff.data_, DIFF_FRACTION);
    negative_ = diff.negative_;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::Decimal(
    const String::SubString& str) throw (Overflow, NotNumber)
  {
    construct_(str);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename General>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::Decimal(General num)
    throw (Overflow, NotNumber)
  {
    Stream::Stack<SIZE * DIGITS_PER_ELEMENT + 4> ostr;
    ostr << std::setprecision(FRACTION_RANK) << std::fixed << num;
    construct_(ostr.str());
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename DiffElement, const unsigned DIFF_TOTAL,
    const unsigned DIFF_FRACTION>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::Decimal(
    const Decimal<DiffElement, DIFF_TOTAL, DIFF_FRACTION>& diff)
    throw (Overflow)
  {
    static_assert(DIFF_FRACTION <= Decimal::FRACTION_RANK,
      "different Decimal is more precise");
    construct_(diff.str());
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::maximum_() throw ()
  {
    Decimal decimal;
    decimal.negative_ = false;
    std::fill(decimal.array_, decimal.array_ + SIZE - 1, BASE - 1);
    decimal.array_[SIZE - 1] = INTEGER_MAX_OVER - 1;
    return decimal;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename ToInteger>
  ToInteger
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::integer() const
    throw (Overflow, Sign)
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    if (!INTEGER_RANK)
    {
      return 0;
    }

    static const unsigned long long INTEG_MAX =
      std::numeric_limits<ToInteger>::max();
    ToInteger ret = 0;
    for (unsigned i = SIZE - 1; i != FRACTION_END; i--)
    {
      if (INTEG_MAX >= array_[i] &&
        static_cast<unsigned long long>(ret) <=
          (INTEG_MAX - array_[i]) / BASE)
      {
        ret = static_cast<ToInteger>(ret * BASE + array_[i]);
      }
      else
      {
        Stream::Error ostr;
        ostr << FNS << "return type is too narrow to contain "
          "the integer value of " << str();
        throw Overflow(ostr);
      }
    }
    if (INTEG_MAX < array_[FRACTION_END] / FRACTION_REMAINDER ||
      static_cast<unsigned long long>(ret) >
        (INTEG_MAX - array_[FRACTION_END] / FRACTION_REMAINDER) /
          FRACTION_OVER)
    {
      Stream::Error ostr;
      ostr << FNS << "return type is too narrow to contain "
        "the integer value of " << str();
      throw Overflow(ostr);
    }
    ret = ret * static_cast<ToInteger>(FRACTION_OVER) +
      static_cast<ToInteger>(array_[FRACTION_END] / FRACTION_REMAINDER);
    if (negative_ && ret && !std::numeric_limits<ToInteger>::is_signed)
    {
      Stream::Error ostr;
      ostr << FNS << "return type is unsigned "
        "but the value to return is negative";
      throw Sign(ostr);
    }
    ret = negative_ ? -ret : ret;
    return ret;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename ToInteger>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::to_integer(
    ToInteger& val) const throw (Overflow, Sign)
  {
    val = integer<ToInteger>();
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename ToFloating>
  ToFloating
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::floating() const throw ()
  {
    static_assert(!std::numeric_limits<ToFloating>::is_integer,
      "Floating type is integer");
    static_assert(std::numeric_limits<ToFloating>::is_signed,
      "Floating type is not signed");

    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    ToFloating ret(0);
    const ToFloating FBASE(BASE);
    for (unsigned i = SIZE; i--;)
    {
      ret = ret * FBASE + static_cast<ToFloating>(array_[i]);
    }
    for (unsigned i = 0; i != FRACTION_END; i++)
    {
      ret /= FBASE;
    }
    ret /= static_cast<ToFloating>(FRACTION_REMAINDER);
    return negative_ ? -ret : ret;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  template <typename ToFloating>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::to_floating(
    ToFloating& val) const throw ()
  {
    val = floating<ToFloating>();
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  std::string
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::str() const
    throw (eh::Exception)
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    char ret[TOTAL_RANK + 3 + !INTEGER_RANK];

    unsigned char num[TOTAL_RANK];
    unsigned char* num_cur = num + TOTAL_RANK;
    bool not_null = false;
    for (unsigned i = 0; i != SIZE; i++)
    {
      unsigned digits = i == SIZE - 1 ? num_cur - num : DIGITS_PER_ELEMENT;
      num_cur -= digits;
      DecimalHelper::disassemble_decimal(digits, array_[i], num_cur);
      if (array_[i])
      {
        not_null = true;
      }
    }

    char* res = ret;

    if (negative_ && not_null)
    {
      *res++ = '-';
    }

    {
      unsigned i = 0;
      for (; i != INTEGER_RANK && !num[i]; i++)
      {
      }
      if (i  == INTEGER_RANK)
      {
        *res++ = '0';
      }
      else
      {
        for (; i != INTEGER_RANK; i++)
        {
          *res++ = '0' + num[i];
        }
      }
    }

    char* last = res - 1;
    if (FRACTION_RANK)
    {
      *res++ = '.';
      last = res;
      for (unsigned i = 0; i != FRACTION_RANK; i++)
      {
        if (num[INTEGER_RANK + i])
        {
          last = res;
        }
        *res++ = '0' + num[INTEGER_RANK + i];
      }
      last[1] = '\0';
    }

    return std::string(ret, last - ret + 1);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  std::string
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::dump() const
    throw (eh::Exception)
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    Stream::Stack<SIZE * (DIGITS_PER_ELEMENT + 1) + 128> ostr;
    ostr << SIZE << ':' << TOTAL_RANK << '.' << FRACTION_RANK <<
      "(" << static_cast<unsigned long long>(INTEGER_MAX_OVER) << "," <<
      FRACTION_END << "," <<
      static_cast<unsigned long long>(FRACTION_REMAINDER) << "," <<
      static_cast<unsigned long long>(FRACTION_OVER) << ")";
    for (int i = SIZE - 1; i >= 0; i--)
    {
      ostr << ' ' << std::setfill('0') << std::setw(DIGITS_PER_ELEMENT) <<
        static_cast<unsigned long long>(array_[i]);
    }
    return ostr.str().str();
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::pack(void* buffer) const
    throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    memcpy(buffer, array_, SIZE * sizeof(Element));
    static_cast<unsigned char*>(buffer)[SIZE * sizeof(Element)] =
      negative_ ? 1 : 0;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::unpack(const void* buffer)
    throw ()
  {
    memcpy(array_, buffer, SIZE * sizeof(Element));
    negative_ = static_cast<const unsigned char*>(buffer)[
      SIZE * sizeof(Element)] != 0;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>&
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::negate() throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    negative_ = !negative_;
    return *this;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>&
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::floor(unsigned fraction)
    throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    if (fraction > FRACTION_RANK || fraction == FRACTION_RANK)
    {
      return *this;
    }

    fraction = FRACTION_RANK - fraction;
    unsigned index = fraction / DIGITS_PER_ELEMENT;
    std::fill(array_, array_ + index, 0);

    if (index < SIZE)
    {
      Element pow = DecimalHelper::pow10<Element>(
        fraction % DIGITS_PER_ELEMENT);
      if (array_[index] % pow)
      {
        array_[index] = array_[index] / pow * pow;
      }
    }

    return *this;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>&
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::ceil(unsigned fraction)
    throw (eh::Exception, Overflow)
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    if (fraction > FRACTION_RANK || fraction == FRACTION_RANK)
    {
      return *this;
    }

    fraction = FRACTION_RANK - fraction;
    unsigned index = fraction / DIGITS_PER_ELEMENT;
    bool overflow = false;
    for (unsigned i = 0; i != index; i++)
    {
      if (array_[i])
      {
        overflow = true;
        array_[i] = 0;
      }
    }

    if (index < SIZE)
    {
      Element pow = DecimalHelper::pow10<Element>(
        fraction % DIGITS_PER_ELEMENT);
      if (overflow || array_[index] % pow)
      {
        Element res = (array_[index] / pow + 1) * pow;
        if (res < BASE)
        {
          array_[index] = res;
          overflow = false;
        }
        else
        {
          overflow = true;
          array_[index] = 0;
          for (unsigned i = index + 1; i < SIZE; i++)
          {
            if (array_[i] < BASE - 1)
            {
              array_[i]++;
              overflow = false;
              break;
            }
            array_[i] = 0;
          }
        }
        if (array_[SIZE - 1] >= INTEGER_MAX_OVER)
        {
          overflow = true;
        }
      }
    }

    if (overflow)
    {
      Stream::Error ostr;
      ostr << FNS << " overflow while ceiling " << str() << " on " <<
        fraction << " digit";
      throw Overflow(ostr);
    }

    return *this;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::is_zero() const throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    for (unsigned i = 0; i != SIZE; i++)
    {
      if (array_[i])
      {
        return false;
      }
    }
    return true;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::is_nonnegative() const
    throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    return !negative_;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::is_nonpositive() const
    throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);

    return negative_;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator ==(
    const Decimal& right) const throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);
    DEV_ASSERT(right.array_[0] != INVALID_FLAG_);

    if (negative_ == right.negative_)
    {
      for (unsigned i = 0; i != SIZE; i++)
      {
        if (array_[i] != right.array_[i])
        {
          return false;
        }
      }
    }
    else
    {
      for (unsigned i = 0; i != SIZE; i++)
      {
        if (array_[i] || right.array_[i])
        {
          return false;
        }
      }
    }
    return true;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator <(
    const Decimal& test) const throw ()
  {
    DEV_ASSERT(array_[0] != INVALID_FLAG_);
    DEV_ASSERT(test.array_[0] != INVALID_FLAG_);

    unsigned diff_index;
#ifdef DEBUG_DECIMAL
    std::cerr << "LESS " << negative_ << " " << is_zero() << " " <<
      test.negative_ << " " << test.is_zero() << " " <<
      is_less_than_(test, diff_index) << " " <<
      test.is_less_than_(*this, diff_index) << "\n";
#endif
    return negative_ ? test.negative_ ?
      test.is_less_than_(*this, diff_index) :
        !is_zero() || !test.is_zero() :
        !test.negative_ && is_less_than_(test, diff_index);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator !=(
    const Decimal& right) const throw ()
  {
    return !operator ==(right);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator >(
    const Decimal& test) const throw ()
  {
    return test < *this;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator >=(
    const Decimal& test) const throw ()
  {
    return !(*this < test);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator <=(
    const Decimal& test) const throw ()
  {
    return !(test < *this);
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>&
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator +=(
    const Decimal& summand) throw (eh::Exception, Overflow)
  {
    add(*this, summand, *this);
    return *this;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>&
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator -=(
    const Decimal& subtrahend) throw (eh::Exception, Overflow)
  {
    sub(*this, subtrahend, *this);
    return *this;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator +(
    const Decimal& summand) const throw (eh::Exception, Overflow)
  {
    Decimal ret;
    add(*this, summand, ret);
    return ret;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::operator -(
    const Decimal& subtrahend) const throw (eh::Exception, Overflow)
  {
    Decimal ret;
    sub(*this, subtrahend, ret);
    return ret;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::add(const Decimal& summand1,
    const Decimal& summand2, Decimal& target)
    throw (eh::Exception, Overflow)
  {
    DEV_ASSERT(summand1.array_[0] != INVALID_FLAG_);
    DEV_ASSERT(summand2.array_[0] != INVALID_FLAG_);

    if (summand1.negative_ == summand2.negative_)
    {
      if (internal_add_(summand1, summand2, target))
      {
        Stream::Error ostr;
        ostr << FNS << "overflow summing " << summand1.str() << " and " <<
          summand2.str() << " (over " << INTEGER_RANK <<
          " digits in integer)";
        throw Overflow(ostr);
      }
      target.negative_ = summand1.negative_;
    }
    else
    {
      unsigned diff_index;
      if (summand1.is_less_than_(summand2, diff_index))
      {
        internal_sub_(summand2, summand1, target, diff_index);
        target.negative_ = summand2.negative_;
      }
      else
      {
        internal_sub_(summand1, summand2, target, diff_index);
        target.negative_ = summand1.negative_;
      }
    }
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::sub(const Decimal& minuend,
    const Decimal& subtrahend, Decimal& target)
    throw (eh::Exception, Overflow)
  {
    DEV_ASSERT(minuend.array_[0] != INVALID_FLAG_);
    DEV_ASSERT(subtrahend.array_[0] != INVALID_FLAG_);

    if (minuend.negative_ == subtrahend.negative_)
    {
      unsigned diff_index;
      if (minuend.is_less_than_(subtrahend, diff_index))
      {
        internal_sub_(subtrahend, minuend, target, diff_index);
        target.negative_ = !subtrahend.negative_;
      }
      else
      {
        internal_sub_(minuend, subtrahend, target, diff_index);
        target.negative_ = minuend.negative_;
      }
    }
    else
    {
      if (internal_add_(minuend, subtrahend, target))
      {
        Stream::Error ostr;
        ostr << FNS << "overflow subtracting " << subtrahend.str() <<
          " from " << minuend.str() << " (over " << INTEGER_RANK <<
          " digits in integer)";
        throw Overflow(ostr);
      }
      target.negative_ = minuend.negative_;
    }
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::mul_elements_(
    Element multiplier, Element factor,
    Element& minor, Element& major) throw ()
  {
    uint64_t h, l;
    DecimalHelper::mul<std::numeric_limits<Element>::digits10 <=
      std::numeric_limits<uint64_t>::digits10 / 2>(
        multiplier, factor, BASE, h, l);
    minor = static_cast<Element>(l);
    major = static_cast<Element>(h);
#ifdef DEBUG_DECIMAL
    std::cerr << "ME:" <<
      " " << static_cast<unsigned long long>(multiplier) <<
      " " << static_cast<unsigned long long>(factor) <<
      " " << static_cast<unsigned long long>(minor) <<
      " " << static_cast<unsigned long long>(major) <<
      "\n";
#endif
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::div_elements_(
    Element major, Element minor, Element divisor,
    Element& quotient, Element& remainder) throw ()
  {
    uint64_t q, r;
    DecimalHelper::div<std::numeric_limits<Element>::digits10 <=
      std::numeric_limits<uint64_t>::digits10 / 2>(
        major, minor, BASE, divisor, q, r);
    quotient = static_cast<Element>(q);
    remainder = static_cast<Element>(r);
#ifdef DEBUG_DECIMAL
    std::cerr << "DE:" <<
      " " << static_cast<unsigned long long>(major) <<
      " " << static_cast<unsigned long long>(minor) <<
      " " << static_cast<unsigned long long>(divisor) <<
      " " << static_cast<unsigned long long>(quotient_high) <<
      " " << static_cast<unsigned long long>(quotient_low) <<
      " " << static_cast<unsigned long long>(remainder) <<
      "\n";
#endif
  }


  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::mul(const Decimal& factor1,
    const Decimal& factor2, DecimalMulRemainder dmr)
    throw (eh::Exception, Overflow)
  {
    DEV_ASSERT(factor1.array_[0] != INVALID_FLAG_);
    DEV_ASSERT(factor2.array_[0] != INVALID_FLAG_);

    Decimal target;

    MulTmpArray mul_tmp;

    // Multiplication
    {
      bool exceeds = false;
      for (unsigned i = 0; i != SIZE; i++)
      {
        if (!factor2.array_[i])
        {
          continue;
        }

        Element overflow = 0;
        for (unsigned j = 0; j != SIZE; j++)
        {
          if (factor1.array_[j])
          {
            Element result, over;
            mul_elements_(factor1.array_[j], factor2.array_[i],
              result, over);
            if (mul_tmp.add(result + overflow, i + j))
            {
              exceeds = true;
              break;
            }
            overflow = over;
          }
          else
          {
            if (overflow)
            {
              if (mul_tmp.add(overflow, i + j))
              {
                exceeds = true;
                break;
              }
              overflow = 0;
            }
          }
        }
        if (exceeds)
        {
          break;
        }
        if (overflow)
        {
          if (mul_tmp.add(overflow, i + SIZE))
          {
            exceeds = true;
            break;
          }
        }
      }

      if (exceeds)
      {
        Stream::Error ostr;
        ostr << FNS << "overflow multiplying " << factor1.str() <<
          " and " << factor2.str() << " (over " << INTEGER_RANK <<
          " digits in integer)";
        throw Overflow(ostr);
      }
    }

    // Rounding
    if (FRACTION_RANK && dmr != DMR_FLOOR)
    {
      if (dmr == DMR_ROUND ? mul_tmp.round() : mul_tmp.ceil())
      {
        Stream::Error ostr;
        ostr << FNS << "overflow increment after multiplying " <<
          factor1.str() << " and " << factor2.str() <<
          " (over " << INTEGER_RANK << " digits in integer)";
        throw Overflow(ostr);
      }
    }

    mul_tmp.export_to(target);

    target.negative_ = factor1.negative_ != factor2.negative_;

    return target;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::div_(const Decimal& dividend,
    const Decimal& divisor, Decimal& quotient)
    throw (eh::Exception, Overflow)
  {
    DEV_ASSERT(dividend.array_[0] != INVALID_FLAG_);
    DEV_ASSERT(divisor.array_[0] != INVALID_FLAG_);

    DivTmpDivisor divisor_tmp(divisor);
    divisor_tmp.shrink();
#ifdef DEBUG_DECIMAL
    std::cerr << "\nDivisor: " << divisor_tmp.dump() << "\n";
#endif
    Element max_div = divisor_tmp.max_element();
    if (!max_div)
    {
      Stream::Error ostr;
      ostr << FNS << "division by zero";
      throw Overflow(ostr);
    }

    DivTmpDividend dividend_tmp(dividend);
    dividend_tmp.shrink();
#ifdef DEBUG_DECIMAL
    std::cerr << "Dividend: " << dividend_tmp.dump() << "\n";
#endif

    quotient.negative_ = dividend.negative_ != divisor.negative_;

    if (dividend_tmp.size() < divisor_tmp.size())
    {
#ifdef DEBUG_DECIMAL
      std::cerr << "Special case\n";
#endif
      std::fill(quotient.array_, quotient.array_ + SIZE, 0);
      return true;
    }

    if (divisor_tmp.size() == 1)
    {
#ifdef DEBUG_DECIMAL
      std::cerr << "Quick\n";
#endif
      Element r;
      dividend_tmp.div(max_div, r);
#ifdef DEBUG_DECIMAL
      std::cerr << "Remainder: " << static_cast<unsigned long long>(r) <<
        " Resulted: " << dividend_tmp.dump() << "\n";
#endif
      if (dividend_tmp.export_to(quotient))
      {
        Stream::Error ostr;
        ostr << FNS << "overflow dividing " << dividend.str() << " by " <<
          divisor.str() << " (over " << INTEGER_RANK <<
          " digits in integer)";
        throw Overflow(ostr);
      }
    }
    else
    {
#ifdef DEBUG_DECIMAL
      std::cerr << "Slow " << dividend_tmp.size() << " " <<
        divisor_tmp.size() << "\n";
#endif

      Element scale = BASE / (max_div + 1);
      assert(scale > 0 && scale < BASE);
      if (scale > 1)
      {
#ifdef DEBUG_DECIMAL
        std::cerr << "scale = " << static_cast<unsigned long long>(scale) <<
          "\n";
#endif
        dividend_tmp.mul(scale);
        divisor_tmp.mul(scale);
        max_div = divisor_tmp.max_element();
#ifdef DEBUG_DECIMAL
        std::cerr << "Dividend: " << dividend_tmp.dump() << "\n";
        std::cerr << "Divisor: " << divisor_tmp.dump() << "\n";
#endif
      }

      Element pre_max_div = divisor_tmp.pre_max_element();
#ifdef DEBUG_DECIMAL
      std::cerr << "divs:" <<
        " " << static_cast<unsigned long long>(max_div) <<
        " " << static_cast<unsigned long long>(pre_max_div) <<
        "\n";
#endif
      int i = dividend_tmp.initial_size() - divisor_tmp.initial_size();
#ifdef DEBUG_DECIMAL
      std::cerr << "Expected result size:" << i << "\n";
#endif
      if (static_cast<unsigned>(i) > SIZE)
      {
        Stream::Error ostr;
        ostr << FNS << "overflow dividing " << dividend.str() << " by " <<
          divisor.str() << " (over " << INTEGER_RANK <<
          " digits in integer)";
        throw Overflow(ostr);
      }
      if (static_cast<unsigned>(i) < SIZE)
      {
        std::fill(quotient.array_ + i + 1, quotient.array_ + SIZE, 0);
      }
      for (; i >= 0; i--)
      {
        Element guess = dividend_tmp.guess_next_quotient(
          i + divisor_tmp.initial_size(), max_div, pre_max_div);

        if (dividend_tmp.apply_next_quotient(i, guess, divisor_tmp))
        {
          guess--;
          dividend_tmp.fix_next_quotient(i, divisor_tmp);
        }

#ifdef DEBUG_DECIMAL
        std::cerr << "Q " << i << " = " <<
          static_cast<unsigned long long>(guess) << "\n";
#endif
        if (static_cast<unsigned>(i) == SIZE)
        {
          if (guess)
          {
            Stream::Error ostr;
            ostr << FNS << "overflow dividing " << dividend.str() <<
              " by " << divisor.str() << " (over " << INTEGER_RANK <<
              " digits in integer)";
            throw Overflow(ostr);
          }
          continue;
        }

        if (static_cast<unsigned>(i) == SIZE - 1 &&
          guess >= INTEGER_MAX_OVER)
        {
          Stream::Error ostr;
          ostr << FNS << "overflow dividing " << dividend.str() << " by " <<
            divisor.str() << " (over " << INTEGER_RANK <<
            " digits in integer)";
          throw Overflow(ostr);
        }
        quotient.array_[i] = guess;

        dividend_tmp.shrink();
      }

      if (scale > 1)
      {
        Element junk;
        dividend_tmp.div(scale, junk);
        assert(!junk);
      }
    }

#ifdef DEBUG_DECIMAL
    std::cerr << "quotent " << quotient.str() << "\n";
#endif

    return false;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::div(const Decimal& dividend,
    const Decimal& divisor, Decimal& remainder)
    throw (eh::Exception, Overflow)
  {
    Decimal quotient;
    if (div_(dividend, divisor, quotient))
    {
      remainder = dividend;
    }
    else
    {
      sub(dividend, mul(quotient, divisor, DMR_FLOOR), remainder);
    }
    return quotient;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::div(const Decimal& dividend,
    const Decimal& divisor, DecimalDivRemainder ddr)
    throw (eh::Exception, Overflow)
  {
    Decimal quotient;
    div_(dividend, divisor, quotient);
    if (ddr == DDR_CEIL && dividend != mul(quotient, divisor, DMR_FLOOR))
    {
      if (internal_add_(quotient, EPSILON, quotient))
      {
        Stream::Error ostr;
        ostr << FNS << "overflow while dividing (over " << INTEGER_RANK <<
          " digits in integer)";
        throw Overflow(ostr);
      }
    }
    return quotient;
  }

  template <typename Element, const unsigned TOTAL_RANK,
   const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::internal_add_(
    const Decimal& summand1, const Decimal& summand2, Decimal& target)
    throw ()
  {
    Element overflow = 0;
    for (unsigned i = 0; i < Decimal::SIZE; i++)
    {
      Element num = summand1.array_[i] + summand2.array_[i] + overflow;
      if (num >= BASE)
      {
        target.array_[i] = num - BASE;
        overflow = 1;
      }
      else
      {
        target.array_[i] = num;
        overflow = 0;
      }
    }
    return overflow != 0 || target.array_[SIZE - 1] >= INTEGER_MAX_OVER;
  }

  template <typename Element, const unsigned TOTAL_RANK,
   const unsigned FRACTION_RANK>
  void
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::internal_sub_(
    const Decimal& minuend, const Decimal& subtrahend, Decimal& target,
    unsigned diff_index) throw ()
  {
    //The Austrian method // minuend > subtrahend
    Element underflow = 0;
    for (unsigned i = 0; i <= diff_index; i++)
    {
      Element real_subtrahend = subtrahend.array_[i] + underflow;
      if (minuend.array_[i] < real_subtrahend)
      {
        target.array_[i] = BASE - (real_subtrahend - minuend.array_[i]);
        underflow = 1;
      }
      else
      {
        target.array_[i] = minuend.array_[i] - real_subtrahend;
        underflow = 0;
      }
    }
    if (diff_index < SIZE)
    {
      std::fill(target.array_ + diff_index + 1, target.array_ + SIZE, 0);
    }
    assert(!underflow);
  }

  template <typename Element, const unsigned TOTAL_RANK,
   const unsigned FRACTION_RANK>
  bool
  Decimal<Element, TOTAL_RANK, FRACTION_RANK>::is_less_than_(
    const Decimal& test, unsigned& diff_index) const throw ()
  {
    unsigned i = SIZE - 1;
    do
    {
      if (array_[i] < test.array_[i])
      {
        diff_index = i;
        return true;
      }
      if (array_[i] > test.array_[i])
      {
        diff_index = i;
        return false;
      }
    }
    while (i--);
    diff_index = 0;
    return false;
  }

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Decimal<Element, TOTAL_RANK, FRACTION_RANK>
    Decimal<Element, TOTAL_RANK, FRACTION_RANK>::ZERO(false, 0, 0);

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Decimal<Element, TOTAL_RANK, FRACTION_RANK>
    Decimal<Element, TOTAL_RANK, FRACTION_RANK>::EPSILON(
      false, FRACTION_RANK ? 0 : 1, FRACTION_RANK ? 1 : 0);

  template <typename Element, const unsigned TOTAL_RANK,
    const unsigned FRACTION_RANK>
  const Decimal<Element, TOTAL_RANK, FRACTION_RANK>
    Decimal<Element, TOTAL_RANK, FRACTION_RANK>::MAXIMUM(maximum_());

  template <typename Hash, typename Element, const unsigned TOTAL,
    const unsigned FRACTION>
  void
  hash_add(Hash& hash,
    const Decimal<Element, TOTAL, FRACTION>& key)
    throw ()
  {
    DEV_ASSERT(key.array_[0] !=
      (Decimal<Element, TOTAL, FRACTION>::INVALID_FLAG_));

    hash.add(key.array_, sizeof(key.array_));
  }
}
