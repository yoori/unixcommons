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




#ifndef STRING_REGEX_HPP
#define STRING_REGEX_HPP

#include <vector>

#include <pcre.h>

#include <String/SubString.hpp>

#include <Generics/Allocator.hpp>


namespace String
{
  /**
   * Wrapper for pcre library
   */
  class RegEx
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Constructor
     * Compiles regexp if required
     * @param regex regular expression
     * @param options compilation options (see pcreapi(3))
     * @param allocator custom allocator for expression and compiled regex
     */
    explicit
    RegEx(const String::SubString& regex = String::SubString(),
      int options = 0, Generics::Allocator::Base* allocator = 0)
      throw (Exception, eh::Exception);

    /**
     * Copy constructor
     * Increases compiled regexp reference count
     * @param side source regexp
     */
    RegEx(const RegEx& side)
      throw (Exception, eh::Exception);

    /**
     * Destructor
     * Decreases compiled regexp reference count and deletes if appropriate
     */
    ~RegEx() throw ();

    /**
     * Assignment operator
     * Increases compiled regexp reference count
     * @param side source regexp
     * @return reference to destination regexp
     */
    RegEx&
    operator =(const RegEx& side)
      throw (Exception, eh::Exception);


    /**
     * Reinitializes regexp with a new regular expression
     * @param regex regular expression
     * @param options compilation options (see pcreapi(3))
     * @param allocator custom allocator for expression and compiled regex
     */
    void
    set_expression(const String::SubString& regex, int options = 0,
      Generics::Allocator::Base* allocator = 0)
      throw (Exception, eh::Exception);


    typedef std::vector<SubString> Result;

    /**
     * Returns total number of substrings in regular expressions
     * @return expected number of substrings
     */
    int
    sub_strings() const throw (Exception);

    /**
     * Performes execution of compiled regular expression
     * and returns all of the found substrings
     * @param result resulted list of substrings
     * @param subject string to match
     * @param options execution options (see pcreapi(3))
     * @return if match occurred or not
     */
    bool
    search(Result& result, const String::SubString& subject,
      int options = 0) const
      throw (Exception, eh::Exception);

    /**
     * Performes execution of compiled regular expression and returns
     * all of the found substrings, in the sense of /g Perl regexp
     * modifier.
     * @param result resulted list of substrings
     * @param subject string to match
     * @param options execution options (see pcreapi(3))
     */
    void
    gsearch(Result& result, const String::SubString& subject,
      int options = 0) const
      throw (Exception, eh::Exception);

    /**
     * Performes "quick" execution of compiled regular expression
     * Neither exceptions nor implicit memory allocation is preformed
     * @param subject string to match
     * @param options execution options (see pcreapi(3))
     * @return if subject matches compiled regular expression or not
     */
    bool
    match(const String::SubString& subject, int options = 0) const
      throw ();

    /**
     * Compiled regular expression
     * @return original regular expression
     */
    String::SubString
    expression() const throw ();


  private:
    /**
     * Provides data members initialization
     */
    void
    init_() throw ();

    /**
     * Provides data members clearance
     */
    void
    clear_() throw ();

    Generics::Allocator::SmartBase_var allocator_;
    char* expr_;
    size_t expr_len_;
    size_t expr_size_;
    pcre* re_;
    size_t re_size_;
    int substrcount_;
  };

  /**
   * Template version of RegEx with custom allocator.
   * Default allocator constructor is used.
   */
  template <typename Alloc = std::allocator<char> >
  class BasicRegEx : public RegEx
  {
  public:
    /**
     * Constructor
     * Compiles regexp if required
     * @param regex regular expression
     * @param options compilation options (see pcreapi(3))
     */
    explicit
    BasicRegEx(const String::SubString& regex = String::SubString(),
      int options = 0) throw (Exception, eh::Exception);

  private:
    static Generics::Allocator::Base_var alloc_;
  };
}

////////////////////////
// INLINES
////////////////////////

namespace String
{
  //
  // RegEx class
  //

  inline
  void
  RegEx::init_() throw ()
  {
    expr_ = 0;
    expr_len_ = 0;
    expr_size_ = 0;
    re_ = 0;
    re_size_ = 0;
    substrcount_ = 0;
  }

  inline
  void
  RegEx::clear_() throw ()
  {
    if (re_)
    {
      if (pcre_refcount(re_, -1) == 0)
      {
        allocator_->deallocate(expr_, expr_size_);
        allocator_->deallocate(re_, re_size_);
      }
    }
    allocator_.reset();
    init_();
  }

  inline
  RegEx::RegEx(const String::SubString& regex, int options,
    Generics::Allocator::Base* allocator)
    throw (Exception, eh::Exception)
  {
    init_();

    if (regex.data())
    {
      set_expression(regex, options, allocator);
    }
  }

  inline
  RegEx::RegEx(const RegEx& side)
    throw (Exception, eh::Exception)
  {
    init_();

    *this = side;
  }

  inline
  RegEx::~RegEx() throw ()
  {
    clear_();
  }

  inline
  int
  RegEx::sub_strings() const throw (Exception)
  {
    if (!re_)
    {
      Stream::Error ostr;
      ostr << FNS << "Expression is not compiled";
      throw Exception(ostr);
    }

    return substrcount_;
  }

  inline
  String::SubString
  RegEx::expression() const throw ()
  {
    return SubString(expr_, expr_len_);
  }


  //
  // BasicRegEx class
  //

  template <typename Alloc>
  Generics::Allocator::Base_var BasicRegEx<Alloc>::alloc_(
    Generics::Allocator::Template<Alloc>::allocator());

  template <typename Alloc>
  BasicRegEx<Alloc>::BasicRegEx(const String::SubString& regex,
    int options) throw (Exception, eh::Exception)
    : RegEx(regex, options, alloc_)
  {
  }
}

#endif
