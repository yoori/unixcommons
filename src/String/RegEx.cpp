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



#include <String/StringManip.hpp>
#include <String/RegEx.hpp>


namespace String
{
  RegEx&
  RegEx::operator =(const RegEx& side)
    throw (Exception, eh::Exception)
  {
    if (this != &side)
    {
      clear_();
      if (side.re_)
      {
        allocator_ = side.allocator_;
        expr_ = side.expr_;
        expr_size_ = side.expr_size_;
        re_ = side.re_;
        re_size_ = side.re_size_;
        substrcount_ = side.substrcount_;

        pcre_refcount(side.re_, 1);
      }
    }

    return *this;
  }

  void
  RegEx::set_expression(const String::SubString& regex, int options,
    Generics::Allocator::Base* allocator)
    throw (Exception, eh::Exception)
  {
    if (!regex.data())
    {
      Stream::Error ostr;
      ostr << FNS << "Couldn't compile expression. Null pointer passed.";
      throw Exception(ostr);
    }

    Generics::Allocator::Base_var alloc(
      ReferenceCounting::add_ref(allocator ? allocator :
        Generics::Allocator::Base::get_default_allocator()));

    size_t expr_size = regex.size() + 1;
    char* expr = static_cast<char*>(alloc->allocate(expr_size));
    std::copy(regex.data(), regex.data() + regex.size(), expr);
    expr[regex.size()] = '\0';

    const char* error;
    int erroffset;
    pcre* re = pcre_compile(expr, options, &error, &erroffset, 0);
    if (!re)
    {
      alloc->deallocate(expr, expr_size);

      Stream::Error ostr;
      ostr << FNS << "Couldn't compile expression '" << regex <<
        "', Reason: " << error << ". At position: " << erroffset;
      throw Exception(ostr);
    }

    size_t re_len;
    pcre_fullinfo(re, 0, PCRE_INFO_SIZE, &re_len);
    size_t re_size = re_len;
    pcre* rea;
    try
    {
      rea = static_cast<pcre*>(alloc->allocate(re_size));
    }
    catch (...)
    {
      pcre_free(re);
      alloc->deallocate(expr, expr_size);
      throw;
    }
    memcpy(rea, re, re_len);
    pcre_free(re);

    clear_();

    allocator_ = alloc;
    expr_ = expr;
    expr_len_ = regex.size();
    expr_size_ = expr_size_;
    re_ = rea;
    re_size_ = re_size_;
    pcre_fullinfo(re_, 0, PCRE_INFO_CAPTURECOUNT, &substrcount_);
    substrcount_++;

    pcre_refcount(re_, 1);
  }

  bool
  RegEx::search(Result& result, const String::SubString& subject,
    int options) const
    throw (Exception, eh::Exception)
  {
    if (!re_)
    {
      Stream::Error ostr;
      ostr << FNS << "Expression is not compiled";
      throw Exception(ostr);
    }

    Generics::ArrayAutoPtr<int> ovector(3 * substrcount_);
    if (pcre_exec(re_, 0, subject.data(), subject.size(),
      0, options, &ovector[0], 3 * substrcount_) <= 0)
    {
      return false;
    }

    result.resize(substrcount_);
    for (int i = 0; i < substrcount_; i++)
    {
      int start = ovector[2 * i];
      int end = ovector[2 * i + 1];
      if (start != end)
      {
        result[i] = subject.substr(start, end - start);
      }
    }

    return true;
  }

  void
  RegEx::gsearch(Result& result, const String::SubString& subject,
    int options) const
    throw (Exception, eh::Exception)
  {
    if (!re_)
    {
      Stream::Error ostr;
      ostr << FNS << "Expression is not compiled";
      throw Exception(ostr);
    }

    // If there are no capturing parenthesis in the regexp, the whole
    // match is returned.
    const int first_capture = substrcount_ > 1 ? 1 : 0;

    Generics::ArrayAutoPtr<int> ovector(3 * substrcount_);

    result.clear();
    size_t offset = 0;
    while (offset <= subject.size() &&
      pcre_exec(re_, 0, subject.data(), subject.size(),
        offset, options, &ovector[0], 3 * substrcount_) > 0)
    {
      size_t res_offset = result.size() - first_capture;
      result.resize(res_offset + substrcount_);
      for (int i = first_capture; i < substrcount_; ++i)
      {
        int start = ovector[2 * i];
        int end = ovector[2 * i + 1];

        // For both zero-length match (start == end >= 0) and not
        // executed capture (start == end == -1) there's an empty
        // string in the result already.
        if (end > start)
        {
          result[res_offset + i] = subject.substr(start, end - start);
        }
      }
      offset = ovector[1];
      // Provide progress for zero-length match.
      if (ovector[1] == ovector[0])
      {
        ++offset;
      }
    }
  }

  bool
  RegEx::match(const String::SubString& subject, int options) const
    throw ()
  {
    if (!re_)
    {
      return false;
    }

    int ovector[90];
    return pcre_exec(re_, 0, subject.data(), subject.size(),
      0, options, ovector, 90) > 0;
  }
}
