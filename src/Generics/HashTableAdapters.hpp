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





#ifndef GENERICS_HASH_TABLE_ADAPTERS_HPP
#define GENERICS_HASH_TABLE_ADAPTERS_HPP

#include <String/SubString.hpp>

#include <Generics/Hash.hpp>


/*
 * XXX!
 * StringHashAdapter and SubStringHashAdapter must return equal hash
 * values for equal strings.
 * XXX!
 */


namespace Generics
{
  class StringHashAdapter
  {
  public:
    typedef std::string text_type;

  public:
    StringHashAdapter(const char* text = 0) throw (eh::Exception);
    StringHashAdapter(const String::SubString& text) throw (eh::Exception);
    StringHashAdapter(const std::string& text) throw (eh::Exception);
    StringHashAdapter(size_t hash, const char* text)
      throw (eh::Exception);

    StringHashAdapter(const char* buffer, size_t buffer_len)
      throw (eh::Exception);

    StringHashAdapter(StringHashAdapter&&) throw ();

    StringHashAdapter&
    assign(size_t hash, const char* text)
      throw (eh::Exception);

    StringHashAdapter&
    assign(const char* text) throw (eh::Exception);

    bool
    operator ==(const StringHashAdapter& src) const throw (eh::Exception);
    bool
    operator <(const StringHashAdapter& src) const throw (eh::Exception);
    bool
    operator >(const StringHashAdapter& src) const throw (eh::Exception);

    size_t
    hash() const throw ();

    /**
     * @return The string on which was calculated hash
     */
    const std::string&
    text() const throw ();
    operator const std::string&() const throw ();

  protected:
    void
    hash_i() throw (eh::Exception);

  protected:
    std::string text_;
    size_t hash_;
  };

  class SubStringHashAdapter
  {
  public:
    typedef String::SubString text_type;

  public:
    SubStringHashAdapter(const String::SubString& text =
      String::SubString())
      throw ();

    template <typename Traits, typename Alloc>
    SubStringHashAdapter(const std::basic_string<char, Traits, Alloc>& text)
      throw ();

    SubStringHashAdapter(size_t hash, const String::SubString& text)
      throw ();

    bool
    operator ==(const SubStringHashAdapter& src) const
      throw ();

    bool
    operator <(const SubStringHashAdapter& src) const
      throw ();

    size_t
    hash() const
      throw ();

    operator String::SubString() const
      throw ();

    const String::SubString&
    text() const throw ();

  protected:
    void
    calc_hash_()
      throw ();

  protected:
    String::SubString text_;
    size_t hash_;
  };

  template <class T>
  class NumericHashAdapter
  {
  public:
    NumericHashAdapter() throw (eh::Exception);
    NumericHashAdapter(const T& value) throw (eh::Exception);

    bool
    operator ==(const NumericHashAdapter& src) const throw (eh::Exception);
    bool
    operator <(const NumericHashAdapter& src) const throw (eh::Exception);
    bool
    operator >(const NumericHashAdapter& src) const throw (eh::Exception);

    size_t
    hash() const throw ();

    const T&
    value() const throw (eh::Exception);

  protected:
    T value_;
  };
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace Generics
{
  //
  // StringHashAdapter class
  //
  inline
  StringHashAdapter::StringHashAdapter(const char* text)
    throw (eh::Exception)
    : text_(text ? text : "")
  {
    hash_i();
  }

  inline
  StringHashAdapter::StringHashAdapter(StringHashAdapter&& init) throw ()
    : text_(std::move(init.text_)),
      hash_(init.hash_)
  {}
  
  inline
  StringHashAdapter::StringHashAdapter(const String::SubString& text)
    throw (eh::Exception)
    : text_(text.str())
  {
    hash_i();
  }

  inline
  StringHashAdapter::StringHashAdapter(const std::string& text)
    throw (eh::Exception)
    : text_(text)
  {
    hash_i();
  }

  inline
  StringHashAdapter::StringHashAdapter(size_t hash, const char* text)
    throw (eh::Exception)
    : text_(text), hash_(hash)
  {
  }

  inline
  StringHashAdapter::StringHashAdapter(const char* buffer, size_t buffer_len)
    throw (eh::Exception)
    : text_(buffer, buffer_len)
  {
    hash_i();
  }


  inline
  StringHashAdapter&
  StringHashAdapter::assign(size_t hash, const char* text)
    throw (eh::Exception)
  {
    text_.assign(text);
    hash_ = hash;
    return *this;
  }

  inline
  StringHashAdapter&
  StringHashAdapter::assign(const char* text) throw (eh::Exception)
  {
    text_.assign(text);
    hash_i();

    return *this;
  }

  inline
  bool
  StringHashAdapter::operator ==(const StringHashAdapter& src) const
    throw (eh::Exception)
  {
    return text_ == src.text_;
  }

  inline
  bool
  StringHashAdapter::operator <(const StringHashAdapter& src)
    const throw (eh::Exception)
  {
    return text_ < src.text_;
  }

  inline
  bool
  StringHashAdapter::operator >(const StringHashAdapter& src)
    const throw (eh::Exception)
  {
    return text_ > src.text_;
  }

  inline
  size_t
  StringHashAdapter::hash() const throw ()
  {
    return hash_;
  }

  inline
  void
  StringHashAdapter::hash_i() throw (eh::Exception)
  {
    Murmur64Hash hash(hash_);
    hash_add(hash, text_);
  }

  inline
  const std::string&
  StringHashAdapter::text() const throw ()
  {
    return text_;
  }

  inline
  StringHashAdapter::operator const std::string&() const
    throw ()
  {
    return text_;
  }

//
// SubStringHashAdapter class
//
  inline
  SubStringHashAdapter::SubStringHashAdapter(const String::SubString& text)
    throw ()
    : text_(text)
  {
    calc_hash_();
  }

  template <typename Traits, typename Alloc>
  SubStringHashAdapter::SubStringHashAdapter(
    const std::basic_string<char, Traits, Alloc>& text) throw ()
    : text_(text)
  {
    calc_hash_();
  }

  inline
  SubStringHashAdapter::SubStringHashAdapter(size_t hash,
    const String::SubString& text)
    throw ()
    : text_(text), hash_(hash)
  {
  }

  inline
  bool
  SubStringHashAdapter::operator ==(const SubStringHashAdapter& src) const
    throw ()
  {
    return text_ == src.text_;
  }

  inline
  bool
  SubStringHashAdapter::operator <(const SubStringHashAdapter& src) const
    throw ()
  {
    return text_ < src.text_;
  }

  inline
  size_t
  SubStringHashAdapter::hash() const throw ()
  {
    return hash_;
  }

  inline
  SubStringHashAdapter::operator String::SubString() const throw ()
  {
    return text_;
  }

  inline
  const String::SubString&
  SubStringHashAdapter::text() const throw ()
  {
    return text_;
  }

  inline
  void
  SubStringHashAdapter::calc_hash_() throw ()
  {
    Murmur64Hash hash(hash_);
    hash_add(hash, text_);
  }

//
// NumericHashAdapter template
//
  template <class T>
  NumericHashAdapter<T>::NumericHashAdapter() throw (eh::Exception)
    : value_(0)
  {
  }

  template <class T>
  NumericHashAdapter<T>::NumericHashAdapter(const T& value)
    throw (eh::Exception)
    : value_(value)
  {
  }

  template <class T>
  bool
  NumericHashAdapter<T>::operator ==(const NumericHashAdapter& src) const
    throw (eh::Exception)
  {
    return value_ == src.value_;
  }

  template <class T>
  bool
  NumericHashAdapter<T>::operator <(const NumericHashAdapter& src) const
    throw (eh::Exception)
  {
    return value_ < src.value_;
  }

  template <class T>
  bool
  NumericHashAdapter<T>::operator >(const NumericHashAdapter& src) const
    throw (eh::Exception)
  {
    return value_ > src.value_;
  }

  template <class T>
  size_t
  NumericHashAdapter<T>::hash() const throw ()
  {
    return static_cast<size_t>(value_);
  }

  template <class T>
  const T&
  NumericHashAdapter<T>::value() const throw (eh::Exception)
  {
    return value_;
  }

  inline
  std::ostream&
  operator <<(std::ostream& ostr, const StringHashAdapter& str)
    throw (eh::Exception)
  {
    ostr << str.text();
    return ostr;
  }

  inline
  std::istream&
  operator >>(std::istream& istr, StringHashAdapter& str)
    throw (eh::Exception)
  {
    std::string str_;
    istr >> str_;
    str = StringHashAdapter(str_.c_str());
    return istr;
  }
}

#endif
