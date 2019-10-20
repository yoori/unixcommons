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





#ifndef GENERICS_DECIMAL_HPP
#define GENERICS_DECIMAL_HPP

#include <Generics/SimpleDecimal.hpp>


namespace Generics
{
  /**
   * Decimal number class
   * provide fixed point decimal number
   * @param Element type of base implementation element
   * @param TOTAL total rank
   * @param FRACTION fraction rank
   */
  template <typename Element, const unsigned TOTAL,
    const unsigned FRACTION>
  class Decimal : public DecimalBase<Element, TOTAL, FRACTION>
  {
  public:
    typedef DecimalBase<Element, TOTAL, FRACTION> Parent;

    using Parent::TOTAL_RANK;
    using Parent::FRACTION_RANK;
    using Parent::INTEGER_RANK;

  private:
    template <typename Hash, typename DiffElement,
      const unsigned DIFF_TOTAL, const unsigned DIFF_FRACTION>
    friend
    void
    hash_add(Hash& hash,
      const Decimal<DiffElement, DIFF_TOTAL, DIFF_FRACTION>& key)
      throw ();

    /**
     * decimal digits per element
     * summing of two Elements must fit Element
     */
    static const unsigned DIGITS_PER_ELEMENT =
      static_cast<Element>(-1) /
        DecimalHelper::Pow10<Element,
          std::numeric_limits<Element>::digits10>::Value >= 2 ?
        std::numeric_limits<Element>::digits10 :
        std::numeric_limits<Element>::digits10 - 1;

    /** evaluation base for this number (base of digit)*/
    static const Element BASE =
      DecimalHelper::Pow10<Element, DIGITS_PER_ELEMENT>::Value;

    /**  */
    static const unsigned MAX_SUM = static_cast<Element>(-1) / BASE;

    /** size of array of elements to represent number */
    static const unsigned SIZE =
      (TOTAL_RANK + DIGITS_PER_ELEMENT - 1) / DIGITS_PER_ELEMENT;

    /** max value of integer element at end of integer part */
    static const Element INTEGER_MAX_OVER =
      DecimalHelper::Pow10<Element,
        DIGITS_PER_ELEMENT - (SIZE * DIGITS_PER_ELEMENT - TOTAL_RANK)>::
          Value;

    /** index of end of fraction part */
    static const unsigned FRACTION_END =
      FRACTION_RANK / DIGITS_PER_ELEMENT;

    /** remainder of fraction in fraction-integer boundary */
    static const Element FRACTION_REMAINDER =
      DecimalHelper::Pow10<Element,
        FRACTION_RANK % DIGITS_PER_ELEMENT>::Value;

    /** over part in fraction-integer boundary */
    static const Element FRACTION_OVER =
      DecimalHelper::Pow10<Element,
        DIGITS_PER_ELEMENT - FRACTION_RANK % DIGITS_PER_ELEMENT>::Value;

    /** self sign */
    bool negative_;
    /** number as array of decimals with base BASE */
    Element array_[SIZE];

  public:
    /** size of array to pack value of decimal */
    static const unsigned PACK_SIZE = SIZE * sizeof(Element) + 1;

    static const Decimal ZERO;
    static const Decimal EPSILON;
    static const Decimal MAXIMUM;

    /** Base exception of this class */
    DECLARE_EXCEPTION(Exception, DecimalException);
    /** operation overflow exception */
    DECLARE_EXCEPTION(Overflow, Exception);
    /** conversion from string exception */
    DECLARE_EXCEPTION(NotNumber, Exception);
    /** conversion from signed to unsigned exception */
    DECLARE_EXCEPTION(Sign, Exception);

    /**
     * Constructor
     * Initializes the number with INVALID_FLAG_
     */
    Decimal() throw ();

    /**
     * Construct from parts
     * @tparam Integer integer type to construct from
     * @tparam Fraction fraction type to construct from
     * @param negative sign
     * @param integer integer part
     * @param fraction fraction part
     * @exception Overflow if passed values are too big
     */
    template <typename Integer, typename Fraction>
    Decimal(bool negative, Integer integer, Fraction fraction)
      throw (Overflow);

    /**
     * Construct from decimal rational.
     * The constructed number is integer / 10 ^ power.
     * The least significant digits could be lost.
     * @param integer numerator
     * @param power power of ten in denominator
     */
    template <typename Integer>
    Decimal(Integer integer, unsigned power)
      throw (Overflow);

    /**
     * Construct from SimpleDecimal
     * @param diff SimpleDecimal
     */
    template <typename DiffBase, const unsigned DIFF_TOTAL,
      const unsigned DIFF_FRACTION>
    explicit
    Decimal(const SimpleDecimal<DiffBase, DIFF_TOTAL, DIFF_FRACTION>& diff)
      throw (Overflow);

    /**
     * Construct from string
     * @param str string of decimal number in format [+|-]abcd[.[efg]]
     * @exception Overflow if passed string is bigger value
     * @exception NotNumber if passed string contains not digits
     */
    explicit
    Decimal(const String::SubString& str) throw (Overflow, NotNumber);

    /**
     * Construct from general. Firstly converted to string.
     * @param num general number to construct from
     * @exception Overflow if passed num is bigger value
     * @exception NotNumber if passed num is invalid
     */
    template <typename General>
    explicit
    Decimal(General num) throw (Overflow, NotNumber);

    /**
     * Construct from different Decimal
     * @param diff different Decimal
     */
    template <typename DiffElement, const unsigned DIFF_TOTAL,
      const unsigned DIFF_FRACTION>
    explicit
    Decimal(const Decimal<DiffElement, DIFF_TOTAL, DIFF_FRACTION>& diff)
      throw (Overflow);

    /**
     * Integer representation of this number
     * @tparam ToInteger integer type to convert to
     * @return integer part of this number
     * @exception Overflow if return value can't fit return type
     * @exception Sign if value is negative but conversion is
     * unapplicable
     */
    template <typename ToInteger>
    ToInteger
    integer() const throw (Overflow, Sign);

    /**
     * Integer representation of this number
     * @tparam ToInteger integer type to convert to
     * @param val integer part of this number
     * @exception Overflow if return value can't fit return type
     * @exception Sign if value is negative but conversion is
     * inapplicable
     */
    template <typename ToInteger>
    void
    to_integer(ToInteger& val) const throw (Overflow, Sign);

    /**
     * Floating representation of this number
     * Precision loss is possible
     * @tparam ToFloating floating type to convert to
     * @return integer part of this number
     */
    template <typename ToFloating>
    ToFloating
    floating() const throw ();

    /**
     * Floating representation of this number
     * @tparam ToFloating integer type to return to
     * @param val integer part of this number
     */
    template <typename ToFloating>
    void
    to_floating(ToFloating& val) const throw ();

    /**
     * String representation of this number
     * @return string representation of this number in format [-]abcd[.efg]
     */
    std::string
    str() const throw (eh::Exception);

    /**
     * Internal dump of this number
     * @return Internal dump of this number
     */
    std::string
    dump() const throw (eh::Exception);

    /**
     * Packs current value into PACK_SIZE bytes long buffer
     * @param buffer pointer to PACK_SIZE bytes long buffer
     */
    void
    pack(void* buffer) const throw ();

    /**
     * Unpacks current value from PACK_SIZE bytes long buffer
     * @param buffer pointer to PACK_SIZE bytes long buffer
     */
    void
    unpack(const void* buffer) throw ();

    /**
     * Revert sign of this number
     * @return this
     */
    Decimal&
    negate() throw ();

    /**
     * Makes floor of absolute value of this
     * @param fraction fraction rank for floor (zero means integer)
     * @return this
     */
    Decimal&
    floor(unsigned fraction) throw ();

    /**
     * Makes ceil of absolute value of this
     * @param fraction fraction rank for ceil (zero means integer)
     * @return this
     * @exception Overflow if result is too big
     */
    Decimal&
    ceil(unsigned fraction) throw (eh::Exception, Overflow);

    /**
     * Test on zero
     * @return true if number is zero
     */
    bool
    is_zero() const throw ();

    /**
     * Test on greater than or equal to zero
     * @return true if number greater than or equal to zero
     */
    bool
    is_nonnegative() const throw ();

    /**
     * Test on less than or equal to zero
     * @return true if number less than or equal to zero
     */
    bool
    is_nonpositive() const throw ();

    /**
     * Test on equality
     * @param test value to compare for equality
     * @return true if equal or false otherwise
     */
    bool
    operator ==(const Decimal& test) const throw ();

    /**
     * Test on not equality
     * @param test value to compare for inequality
     * @return true if not equal or false otherwise
     */
    bool
    operator !=(const Decimal& test) const throw ();

    /**
     * Test on minority
     * @param test value to compare for minority
     * @return true if less than or false otherwise
     */
    bool
    operator <(const Decimal& test) const throw ();

    /**
     * Test on minority or equality
     * @param test value to compare for minority or equality
     * @return true if less than or equal to or false otherwise
     */
    bool
    operator <=(const Decimal& test) const throw ();

    /**
     * Test on majority
     * @param test value to compare for majority
     * @return true if greater than or false otherwise
     */
    bool
    operator >(const Decimal& test) const throw ();

    /**
     * Test on majority or equality
     * @param test value to compare for majority or equality
     * @return true if greater than or equal to or false otherwise
     */
    bool
    operator >=(const Decimal& test) const throw ();

    /**
     * Add summand to this
     * @param summand summand
     * @return this
     * @exception Overflow if result is too big
     */
    Decimal&
    operator +=(const Decimal& summand) throw (eh::Exception, Overflow);

    /**
     * Substruct subtrahend from this
     * @param subtrahend subtrahend
     * @return this
     * @exception Overflow if result is too big
     */
    Decimal&
    operator -=(const Decimal& subtrahend)
      throw (eh::Exception, Overflow);

    /**
     * Add summand to value of this
     * @param summand summand
     * @return new value result of summation
     * @exception Overflow if result is too big
     */
    Decimal
    operator +(const Decimal& summand) const
      throw (eh::Exception, Overflow);

    /**
     * Subtract subtrahend from value of this
     * @param subtrahend subtrahend
     * @return new value result of substraction
     * @exception Overflow if result is too big
     */
    Decimal
    operator -(const Decimal& subtrahend) const
      throw (eh::Exception, Overflow);

    /**
     * Do multiplication of decimals
     * @param factor1 the first factor
     * @param factor2 the second factor
     * @param dmr remainder processing behaviour
     * @return result of multiplication
     * @exception Overflow if result is too big
     */
    static
    Decimal
    mul(const Decimal& factor1, const Decimal& factor2,
      DecimalMulRemainder dmr) throw (eh::Exception, Overflow);

    /**
     * Do division of decimals
     * @param dividend dividend
     * @param divisor divisor
     * @param remainder remainder
     * @return quotient
     * @exception Overflow if result is too big
     */
    static
    Decimal
    div(const Decimal& dividend, const Decimal& divisor,
      Decimal& remainder) throw (eh::Exception, Overflow);

    /**
     * Do division of decimals
     * @param dividend dividend
     * @param divisor divisor
     * @param ddr remainder processing behaviour
     * @return quotient
     * @exception Overflow if result is too big
     */
    static
    Decimal
    div(const Decimal& dividend, const Decimal& divisor,
      DecimalDivRemainder ddr = DDR_FLOOR) throw (eh::Exception, Overflow);

    /**
     * Make sum of decimals
     * @param summand1 the first summand
     * @param summand2 the second summand
     * @param target result of operation
     * @exception Overflow if result is too big
     */
    static
    void
    add(const Decimal& summand1, const Decimal& summand2,
      Decimal& target) throw (eh::Exception, Overflow);

    /**
     * Make substruction of decimals
     * @param minuend minuend
     * @param subtrahend subtrahend
     * @param target result of operation
     * @exception Overflow if result is too big
     */
    static
    void
    sub(const Decimal& minuend, const Decimal& subtrahend,
      Decimal& target) throw (eh::Exception, Overflow);

  private:
    static const Element INVALID_FLAG_ = -1;

    /**
     * Maximum
     * @return maximal value
     */
    static
    Decimal
    maximum_() throw ();

  private:
    /**
     * Construct from parts helper
     * @tparam Integer integer type to construct from
     * @tparam Fraction fraction type to construct from
     * @param negative sign
     * @param integer integer part
     * @param fraction fraction part
     * @exception Overflow if passed values too big
     */
    template <typename Integer, typename Fraction>
    void
    construct_(bool negative, Integer integer, Fraction fraction)
      throw (Overflow);

    /**
     * Construct from decimal rational helper.
     * The constructed number is integer / 10 ^ power.
     * The least significant digits could be lost.
     * @param integer numerator
     * @param power power of ten in denominator
     */
    template <typename Integer>
    void
    construct_(Integer integer, unsigned power)
      throw (Overflow);

    /**
     * Construct from string helper
     * @param str string of decimal number in format [+|-][abcd[.[efg]]]
     * @exception Overflow if passed string is bigger value
     * @exception NotNumber if passed string contains not digits
     */
    void
    construct_(const String::SubString& str)
      throw (Overflow, NotNumber);

    /**
     * Sum array parts of decimals
     * @param summand1 the first summand
     * @param summand2 the second summand
     * @param target result of operation
     * @return true if result is too big
     */
    static
    bool
    internal_add_(const Decimal& summand1, const Decimal& summand2,
      Decimal& target) throw ();

    /**
     * Substruct array parts of decimals
     * minuend > subtrahend
     * @param minuend minuend
     * @param subtrahend subtrahend
     * @param target result of operation
     * @param diff_index the superior index in array
     * where minuend[index] != subtrahend[index]
     */
    static
    void
    internal_sub_(const Decimal& left, const Decimal& right,
      Decimal& target, unsigned diff_index) throw ();

    /**
     * Check array parts if this < test
     * @param test decimal to compare with
     * @param diff_index the superior index in array
     * where this[index] != test[index]
     * @return true if this < test
     */
    bool
    is_less_than_(const Decimal& test, unsigned& diff_index) const
      throw ();

    /**
     * Multiply elements in base of BASE
     * @param multiplier value to multiply
     * @param factor value by multiply
     * @param minor minor part of result
     * @param major major part of result
     */
    static
    void
    mul_elements_(Element multiplier, Element factor,
      Element& minor, Element& major) throw ();

    /**
     * Divide elements in base of BASE
     * @param major major part of dividend
     * @param minor minor part of dividend
     * @param divisor divisor
     * @param quotient quotient
     * @param remainder remainder of operation
     */
    static
    void
    div_elements_(Element major, Element minor, Element divisor,
      Element& quotient, Element& remainder) throw ();

    /**
     * Do division of decimals
     * @param dividend dividend
     * @param divisor divisor
     * @param quotient quotient
     * @return if remainder equals to dividend or must be calculated
     * @exception Overflow if result is too big
     */
    static
    bool
    div_(const Decimal& dividend, const Decimal& divisor,
      Decimal& quotient) throw (eh::Exception, Overflow);

    // Types for multiplication operation
    class MulTmpArray
    {
    public:
      MulTmpArray() throw ();
      bool
      add(Element value, unsigned index) throw ();
      bool
      round() throw ();
      bool
      ceil() throw ();
      void
      export_to(Decimal& result) const throw ();
      std::string
      dump() const throw (eh::Exception);

    private:
      static const unsigned TMP_FRACTION_RANK =
        2 * FRACTION_RANK;
      static const unsigned TMP_TOTAL_RANK =
        INTEGER_RANK + TMP_FRACTION_RANK;
      static const unsigned TMP_SIZE =
        TMP_TOTAL_RANK / DIGITS_PER_ELEMENT +
          (TMP_TOTAL_RANK % DIGITS_PER_ELEMENT ? 1 : 0);
      static const Element TMP_INTEGER_MAX_OVER =
        DecimalHelper::Pow10<Element, DIGITS_PER_ELEMENT -
          (TMP_SIZE * DIGITS_PER_ELEMENT - TMP_TOTAL_RANK)>::Value;

      Element tmp_array_[TMP_SIZE];
    };
    friend class MulTmpArray;



    // Types for division operation
    template <const unsigned DIV_TMP_SIZE>
    class DivTmpArrayBase
    {
    public:
      DivTmpArrayBase() throw ();

      void
      shrink() throw ();

      unsigned
      size() throw ();

      unsigned
      initial_size() throw ();

      void
      mul(Element multiplicator) throw ();
      void
      div(Element multiplicator, Element& remainder) throw ();

      std::string
      dump() const throw (eh::Exception);

    protected:
      Element tmp_array_[DIV_TMP_SIZE];
      unsigned size_;
      unsigned initial_size_;
    };
    template <const unsigned DIV_TMP_SIZE>
    friend class DivTmpArrayBase;

    class DivTmpDivisor;

    static const unsigned DIV_TMP_FRACTION_RANK =
      2 * FRACTION_RANK;
    static const unsigned DIV_TMP_TOTAL_RANK =
      INTEGER_RANK + DIV_TMP_FRACTION_RANK + DIGITS_PER_ELEMENT;
    static const unsigned DIV_TMP_SIZE =
      DIV_TMP_TOTAL_RANK / DIGITS_PER_ELEMENT +
        (DIV_TMP_TOTAL_RANK % DIGITS_PER_ELEMENT ? 1 : 0);

    class DivTmpDividend : public DivTmpArrayBase<DIV_TMP_SIZE>
    {
    public:
      DivTmpDividend(const Decimal& dividend) throw ();

      Element
      guess_next_quotient(unsigned index, Element max_div,
        Element pre_max_div) throw ();

      bool
      apply_next_quotient(unsigned index, Element guess,
        const DivTmpDivisor& divisor) throw ();

      void
      fix_next_quotient(unsigned index, const DivTmpDivisor& divisor)
        throw ();

      bool
      export_to(Decimal& result) throw ();
    };
    friend class DivTmpDividend;

    class DivTmpDivisor : public DivTmpArrayBase<SIZE + 1>
    {
    public:
      DivTmpDivisor(const Decimal& divider) throw ();

      Element
      max_element() throw ();
      Element
      pre_max_element() throw ();

      friend class
      DivTmpDividend;
    };
  };
}

#include <Generics/Decimal.tpp>

#endif //GENERICS_DECIMAL_HPP
