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



// @file String/SubStringExternal.tpp

namespace String
{
  //
  // External functions
  //

  template <typename CharType, typename Traits, typename Checker>
  std::basic_ostream<typename BasicSubString<CharType, Traits, Checker>::
    BasicStringValueType>&
  operator <<(std::basic_ostream<
    typename BasicSubString<CharType, Traits, Checker>::
      BasicStringValueType>& ostr,
    const BasicSubString<CharType, Traits, Checker>& substr)
    throw (eh::Exception)
  {
    if (!substr.empty())
    {
      ostr.write(substr.data(), substr.length());
    }
    return ostr;
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator ==(const BasicSubString<CharType, Traits, Checker>& substr,
    typename BasicSubString<CharType, Traits, Checker>::ConstPointer str)
    throw (typename BasicSubString<CharType, Traits, Checker>::LogicError)
  {
    return substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator ==(
    typename BasicSubString<CharType, Traits, Checker>::ConstPointer str,
    const BasicSubString<CharType, Traits, Checker>& substr)
    throw (typename BasicSubString<CharType, Traits, Checker>::LogicError)
  {
    return substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator ==(const BasicSubString<CharType, Traits, Checker>& left_substr,
    const BasicSubString<CharType, Traits, Checker>& right_substr)
    throw ()
  {
    return left_substr.equal(right_substr);
  }

  template <typename CharType, typename Traits, typename Checker,
    typename BasicStringTraits, typename Allocator>
  bool
  operator ==(const BasicSubString<CharType, Traits, Checker>& substr,
    const std::basic_string<
      typename BasicSubString<CharType, Traits, Checker>::
        BasicStringValueType, BasicStringTraits, Allocator>& str)
    throw (eh::Exception)
  {
    return substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker,
    typename BasicStringTraits, typename Allocator>
  bool
  operator ==(const std::basic_string<
    typename BasicSubString<CharType, Traits, Checker>::
      BasicStringValueType, BasicStringTraits, Allocator>& str,
    const BasicSubString<CharType, Traits, Checker>& substr)
    throw (eh::Exception)
  {
    return substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator !=(const BasicSubString<CharType, Traits, Checker>& substr,
    typename BasicSubString<CharType, Traits, Checker>::ConstPointer str)
    throw (typename BasicSubString<CharType, Traits, Checker>::LogicError)
  {
    return !substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator !=(
    typename BasicSubString<CharType, Traits, Checker>::ConstPointer str,
    const BasicSubString<CharType, Traits, Checker>& substr)
    throw (typename BasicSubString<CharType, Traits, Checker>::LogicError)
  {
    return !substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator !=(const BasicSubString<CharType, Traits, Checker>& left_substr,
    const BasicSubString<CharType, Traits, Checker>& right_substr)
    throw ()
  {
    return !left_substr.equal(right_substr);
  }

  template <typename CharType, typename Traits, typename Checker,
    typename BasicStringTraits, typename Allocator>
  bool
  operator !=(const BasicSubString<CharType, Traits, Checker>& substr,
    const std::basic_string<
      typename BasicSubString<CharType, Traits, Checker>::
        BasicStringValueType, BasicStringTraits, Allocator>& str)
    throw (eh::Exception)
  {
    return !substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker,
    typename BasicStringTraits, typename Allocator>
  bool
  operator !=(const std::basic_string<
    typename BasicSubString<CharType, Traits, Checker>::
      BasicStringValueType, BasicStringTraits, Allocator>& str,
    const BasicSubString<CharType, Traits, Checker>& substr)
    throw (eh::Exception)
  {
    return !substr.equal(str);
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator <(const BasicSubString<CharType, Traits, Checker>& substr,
    typename BasicSubString<CharType, Traits, Checker>::ConstPointer str)
    throw (typename BasicSubString<CharType, Traits, Checker>::LogicError)
  {
    return substr.compare(str) < 0;
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator <(
    typename BasicSubString<CharType, Traits, Checker>::ConstPointer str,
    const BasicSubString<CharType, Traits, Checker>& substr)
    throw (typename BasicSubString<CharType, Traits, Checker>::LogicError)
  {
    return !operator <(substr, str);
  }

  template <typename CharType, typename Traits, typename Checker>
  bool
  operator <(const BasicSubString<CharType, Traits, Checker>& left_substr,
    const BasicSubString<CharType, Traits, Checker>& right_substr) throw ()
  {
    return left_substr.compare(right_substr) < 0;
  }

  template <typename CharType, typename Traits, typename Checker,
    typename BasicStringTraits, typename Allocator>
  bool
  operator <(const BasicSubString<CharType, Traits, Checker>& substr,
    const std::basic_string<
      typename BasicSubString<CharType, Traits, Checker>::
        BasicStringValueType, BasicStringTraits, Allocator>& str)
    throw ()
  {
    return substr.compare(str) < 0;
  }

  template <typename CharType, typename Traits, typename Checker,
    typename BasicStringTraits, typename Allocator>
  bool
  operator <(const std::basic_string<
    typename BasicSubString<CharType, Traits, Checker>::
      BasicStringValueType, BasicStringTraits, Allocator>& str,
    const BasicSubString<CharType, Traits, Checker>& substr)
    throw ()
  {
    return substr.compare(str) > 0;
  }

  template <typename Hash,
    typename CharType, typename Traits, typename Checker>
  void
  hash_add(Hash& hash,
    const BasicSubString<CharType, Traits, Checker>& value) throw ()
  {
    hash.add(value.data(), value.size());
  }
}
