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



#ifndef GENERICS_COMMON_DECIMAL_HPP
#define GENERICS_COMMON_DECIMAL_HPP

#include <ios>
#include <limits>
#include <cstdint>
#include <cassert>
#include <cmath>

#include <eh/Exception.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  /**
   * Base exception for all Decimal/SimpleDecimal instantiations
   */
  DECLARE_EXCEPTION(DecimalException, eh::DescriptiveException);

  enum DecimalMulRemainder
  {
    DMR_FLOOR,
    DMR_ROUND,
    DMR_CEIL
  };

  enum DecimalDivRemainder
  {
    DDR_FLOOR,
    DDR_CEIL
  };

  namespace DecimalHelper
  {
    template <typename T, const unsigned POWER>
    struct Pow10
    {
      static const T Value = 10 * Pow10<T, POWER - 1>::Value;
    };

    template <typename T, const unsigned POWER>
    const T Pow10<T, POWER>::Value;

    template <typename T>
    struct Pow10<T, 0>
    {
      static const T Value = 1;
    };

    template <typename T>
    const T Pow10<T, 0>::Value;

    template <typename Integer, const bool UNSIGNED>
    struct Splitter
    {
      static
      void
      split(Integer& integer, bool& negative) throw ();
    };

    template <typename Integer>
    struct Splitter<Integer, true>
    {
      static
      void
      split(Integer& integer, bool& negative) throw ();
    };

    template <typename Integer, const bool UNSIGNED>
    void
    Splitter<Integer, UNSIGNED>::split(Integer& integer, bool& negative)
      throw ()
    {
      if (integer < 0)
      {
        negative = true;
        assert(integer >= -std::numeric_limits<Integer>::max());
        integer = -integer;
      }
      else
      {
        negative = false;
      }
    }

    template <typename Integer>
    void
    Splitter<Integer, true>::split(Integer& /*integer*/, bool& negative)
      throw ()
    {
      negative = false;
    }

    template <typename Integer>
    void
    split(Integer& integer, bool& negative) throw ()
    {
      Splitter<Integer, !std::numeric_limits<Integer>::is_signed>::split(
        integer, negative);
    }

    /**
     * assemble decimal "digit" from array
     */
    template <typename Base>
    Base
    assemble_decimal(unsigned digits, const unsigned char* num) throw ()
    {
      Base ret = 0;
      for (unsigned i = 0; i < digits; ++i)
      {
        ret = ret * 10 + *num++;
      }
      return ret;
    }

    /**
     * disassemble decimal "digit" into array
     */
    template <typename Base>
    void
    disassemble_decimal(unsigned digits, Base elem, unsigned char* num)
      throw ()
    {
      if (digits)
      {
        unsigned i = digits - 1;
        do
        {
          num[i] = elem % 10;
          elem /= 10;
        }
        while (i--);
      }
    }

    /**
     * @param pow The exponent
     * @return The N-th degree of ten, barring overflow type Base
     */
    template <typename Base>
    Base
    pow10(unsigned pow) throw ()
    {
      if (pow < 20)
      {
        static const uint64_t READY_POWERS[20] =
        {
          1,
          10,
          100,
          1000,
          10000,
          100000,
          1000000,
          10000000,
          100000000,
          1000000000,
          10000000000,
          100000000000,
          1000000000000,
          10000000000000,
          100000000000000,
          1000000000000000,
          10000000000000000,
          100000000000000000,
          1000000000000000000,
          10000000000000000000ull,
        };
        return READY_POWERS[pow];
      }
      Base res = 1;
      Base base = 10;
      for (;;)
      {
        if (pow & 1)
        {
          res *= base;
        }
        pow >>= 1;
        if (!pow)
        {
          break;
        }
        base *= base;
      }

      return res;
    }

    inline
    bool
    exceeds(unsigned long long a, unsigned long long b) throw ()
    {
      return a >= b;
    }

    template <typename Iterator>
    void
    skip(Iterator& cur, Iterator& end, std::ios_base::iostate& iostate)
      throw ()
    {
      bool eof;
      while (!(eof = ++cur == end) &&
        static_cast<unsigned>(*cur - '0') < 10)
      {
      }
      if (eof)
      {
        iostate |= std::ios_base::eofbit;
      }
      iostate |= std::ios_base::failbit;
    }

    /**
     * Stream construct from char iterators
     * @param cur Iterator locate begin of the string of decimal number
     *   in format [+|-][abcd[.[efg]]]
     * @param end Iterator locate end of input string with decimal number
     * @param iostate Return state of construction used failbit and eofbit
     * @param data Return extracted value
     * @param negative_sign data Return extracted negative sign
     * @return If not null pointer returned, this mean NotNumber exception
     *   state and returned description can be used to raise exception.
     *   Overflow state set in iostate::failbit
     */
    template <const unsigned TOTAL, const unsigned FRACTION, typename Base,
      typename Iterator>
    const char*
    extract_decimal(Iterator cur, Iterator end,
      std::ios_base::iostate& iostate, Base& data, bool& negative_sign)
      throw ()
    {
      bool eof = cur == end;
      if (eof)
      {
        iostate |= std::ios_base::failbit | std::ios_base::eofbit;
        return "empty string passed";
      }
      bool negative = false;
      unsigned digit = *cur - '0';
      switch (digit)
      {
      case static_cast<unsigned>('-' - '0'):
        negative = true;
      case static_cast<unsigned>('+' - '0'):
        eof = ++cur == end;
        if (eof)
        {
          iostate |= std::ios_base::failbit | std::ios_base::eofbit;
          return "empty number passed";
        }
        digit = *cur - '0';
      }
      if (digit > 9)
      {
        iostate |= std::ios_base::failbit;
        return "empty number passed";
      }

      Base decimal = 0;
      for (;;)
      {
        if (digit < 10)
        {
          if (decimal < Pow10<Base, TOTAL - FRACTION>::Value / 10)
          {
            decimal = decimal * 10 + digit;
          }
          else
          {
            skip(cur, end, iostate);
            return "number of digits in integer part of string "
              "is bigger than expected";
          }
        }
        else
        {
          break;
        }
        eof = ++cur == end;
        if (eof)
        {
          break;
        }
        digit = *cur - '0';
      }
      if (FRACTION && !eof && digit == static_cast<unsigned>('.' - '0'))
      {
        int digits_left = FRACTION;
        while (!(eof = ++cur == end))
        {
          unsigned digit = *cur - '0';
          if (digit < 10)
          {
            if (digits_left)
            {
              --digits_left;
              decimal = decimal * 10 + digit;
            }
            else
            {
              skip(cur, end, iostate);
              return "number of digits in fractional part of string "
                "is bigger than expected";
            }
          }
          else
          {
            break;
          }
        }

        if (FRACTION > 1 && digits_left > 0)
        {
          decimal *= pow10<Base>(digits_left);
        }
      }
      else
      {
        decimal *= Pow10<Base, FRACTION>::Value;
      }

      if (eof)
      {
        iostate |= std::ios_base::eofbit;
      }
      // write values here to be exceptions-safe
      negative_sign = negative;
      data = decimal;
      return 0;
    }

    /**
     * tmp = factor1 * factor2;
     * quotient = tmp / divisor;
     * remainder = tmp % divisor;
     * returns true if quotient <= std::numeric_limits<CalcType>::max()
     */
    bool
    muldiv(uint64_t factor1, uint64_t factor2, uint64_t divisor,
      uint64_t& quotient, uint64_t& remainder) throw ()
      __attribute__((always_inline));

    inline
    bool
    muldiv(uint64_t factor1, uint64_t factor2, uint64_t divisor,
      uint64_t& quotient, uint64_t& remainder) throw ()
    {
      uint64_t a, b, c;
      __asm__ __volatile__(
        "mulq %%rdx\n"
        "cmpq %%rcx, %%rdx\n"
        "jae 1f\n"
        "divq %%rcx\n"
        "xorq %%rcx, %%rcx\n"
        "1:\n"
        : "=a" (a), "=d" (b), "=c" (c)
        : "a" (factor1), "d" (factor2), "c" (divisor)
      );
      quotient = a;
      remainder = b;
      return c;
    }

    template <const bool>
    void
    mul(uint64_t factor1, uint64_t factor2, uint64_t base,
      uint64_t& major, uint64_t& minor) throw ()
    {
      major = factor1 * factor2;
      minor = major % base;
      major /= base;
    }

    template <>
    void
    mul<false>(uint64_t factor1, uint64_t factor2, uint64_t base,
      uint64_t& major, uint64_t& minor) throw ();

    template <const bool>
    void
    div(uint64_t major, uint64_t minor, uint64_t base, uint64_t divisor,
      uint64_t& quotient, uint64_t& remainder) throw ()
    {
      quotient = major * base + minor;
      remainder = quotient % divisor;
      quotient /= divisor;
    }

    template <>
    void
    div<false>(uint64_t major, uint64_t minor, uint64_t base,
      uint64_t divisor, uint64_t& quotient, uint64_t& remainder) throw ();
  }

  /**
   * Checks that the base type is an expected integer.
   */
  template <typename BaseType>
  class DecimalIntegerCheck
  {
  private:
    static_assert(std::numeric_limits<BaseType>::is_integer,
      "base type must be integer");
    static_assert(std::numeric_limits<BaseType>::is_bounded,
      "base type must be bounded");
  };

  /**
   */
  template <typename BaseType, const unsigned TOTAL,
    const unsigned FRACTION>
  class DecimalRanks : private DecimalIntegerCheck<BaseType>
  {
  public:
    typedef BaseType Base;
    static const unsigned TOTAL_RANK = TOTAL;
    static const unsigned FRACTION_RANK = FRACTION;
    static const unsigned INTEGER_RANK = TOTAL_RANK - FRACTION_RANK;

  private:
    static_assert(TOTAL_RANK, "TOTAL must be positive");
    static_assert(FRACTION_RANK <= TOTAL_RANK,
      "FRACTION must be less than or equal to TOTAL");

  protected:
    /**
     * Destructor
     */
    ~DecimalRanks() throw () = default;
  };

  /**
   * Base type for Decimal and SimpleDecimal classes
   * Contains common constraints
   */
  template <typename BaseType, const unsigned TOTAL,
    const unsigned FRACTION>
  class DecimalBase : public DecimalRanks<BaseType, TOTAL, FRACTION>
  {
  private:
    static_assert(!std::numeric_limits<BaseType>::is_signed,
      "BaseType must be unsigned");

  protected:
    /**
     * Destructor
     */
    ~DecimalBase() throw () = default;
  };

  /**
   */
  template <typename Parent>
  class SimpleDecimalBase : public Parent
  {
  protected:
    typedef uint64_t CalcType;

  private:
    static_assert(Parent::TOTAL_RANK <= static_cast<unsigned>(
      std::numeric_limits<typename Parent::Base>::digits10),
      "TOTAL_RANK must fit type");
    static_assert(Parent::TOTAL_RANK <= static_cast<unsigned>(
      std::numeric_limits<CalcType>::digits10),
      "TOTAL_RANK must fit CalcType");

  protected:
    static const typename Parent::Base MAX_VALUE_ =
      DecimalHelper::Pow10<typename Parent::Base, Parent::TOTAL_RANK>::
        Value;

    static const typename Parent::Base MAX_FRACTION_ =
      DecimalHelper::Pow10<typename Parent::Base, Parent::FRACTION_RANK>::
        Value;

    static const typename Parent::Base MAX_INTEGER_ =
      DecimalHelper::Pow10<typename Parent::Base, Parent::INTEGER_RANK>::
        Value;

  protected:
    /**
     * Destructor
     */
    ~SimpleDecimalBase() throw () = default;
  };

  /**
   * Converts Decimal to more narrow SimpleDecimal
   * @param s SimpleDecimal to convert to
   * @param d Decimal to convert from
   */
  template <typename SimpleDecimal, typename Decimal>
  void
  narrow_decimal(SimpleDecimal& s, const Decimal& d) throw (eh::Exception)
  {
    static_assert(Decimal::FRACTION_RANK >= SimpleDecimal::FRACTION_RANK,
      "Decimal FRACTION must not be less than SimpleDecimal one");
    static_assert(
      static_cast<unsigned>(std::numeric_limits<int64_t>::digits10) >=
        SimpleDecimal::TOTAL_RANK,
      "SimpleDecimal TOTAL must not exceed int64_t");
    static_assert(
      static_cast<unsigned>(std::numeric_limits<int64_t>::digits10) >=
        SimpleDecimal::FRACTION_RANK,
      "SimpleDecimal FRACTION must not exceed int64_t");

    s = SimpleDecimal(Decimal::mul(d,
      Decimal(DecimalHelper::Pow10<int64_t,
        SimpleDecimal::FRACTION_RANK>::Value, 0), DMR_FLOOR).
          template integer<int64_t>(), SimpleDecimal::FRACTION_RANK);
  }

  template <typename SimpleDecimal, typename Float>
  SimpleDecimal
  convert_float(Float value) throw (eh::Exception)
  {
    int state = std::fpclassify(value);
    if (state == FP_ZERO)
    {
      return SimpleDecimal::ZERO;
    }
    if (state != FP_NORMAL)
    {
      Stream::Error ostr;
      ostr << FNS << "provided value is not normal";
      throw typename SimpleDecimal::Exception(ostr);
    }
    const int64_t POW =
      DecimalHelper::Pow10<int64_t, SimpleDecimal::FRACTION_RANK>::Value;
    if ((value >= 0 ? value : -value) >=
      std::numeric_limits<int64_t>::max() / POW)
    {
      Stream::Error ostr;
      ostr << FNS << "provided value is too big";
      throw typename SimpleDecimal::Overflow(ostr);
    }
    return SimpleDecimal(static_cast<int64_t>(value * POW),
      SimpleDecimal::FRACTION_RANK);
  }
}

namespace Generics
{
  //
  // DecimalRanks class
  //

  template <typename BaseType, const unsigned TOTAL,
    const unsigned FRACTION>
  const unsigned DecimalRanks<BaseType, TOTAL, FRACTION>::TOTAL_RANK;
  template <typename BaseType, const unsigned TOTAL,
    const unsigned FRACTION>
  const unsigned DecimalRanks<BaseType, TOTAL, FRACTION>::FRACTION_RANK;
  template <typename BaseType, const unsigned TOTAL,
    const unsigned FRACTION>
  const unsigned DecimalRanks<BaseType, TOTAL, FRACTION>::INTEGER_RANK;


  //
  // SimpleDecimalBase class
  //

  template <typename Parent>
  const typename Parent::Base SimpleDecimalBase<Parent>::MAX_VALUE_;
  template <typename Parent>
  const typename Parent::Base SimpleDecimalBase<Parent>::MAX_FRACTION_;
  template <typename Parent>
  const typename Parent::Base SimpleDecimalBase<Parent>::MAX_INTEGER_;
}

#endif
