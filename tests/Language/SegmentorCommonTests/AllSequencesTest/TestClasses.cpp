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




#include "TestClasses.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <String/UTF8Handler.hpp>
#include <String/UTF8Category.hpp>
#include <String/UnicodeSymbol.hpp>
#include <Language/SegmentorCommonTests/Commons/TextGenerator.hpp>
#include <Generics/Function.hpp>
#include <Stream/MemoryStream.hpp>
#include <cassert>

namespace
{
  /**
   * set_next is move char to next char
   * @param us is char to move next
   */
  inline
  bool
  set_next(unsigned char& uc)
    throw ()
  {
    if (uc == 255)
    {
      uc = 0;
    }
    else
    {
      ++uc;
    }

    return uc;
  }

} //namespace

////// class Segment

Segment::Segment(Language::Segmentor::SegmentorInterface_var segmentor, 
  unsigned long start_border,  
  unsigned long finish_border,
  TestScenarios scenario,
  bool print_utf8_transforms,
  bool symbols_only)
  throw (SegmentError)
  : segmentor_(segmentor),
    start_border_(start_border),
    finish_border_(finish_border),
    print_utf8_transforms_(print_utf8_transforms),
    symbols_only_(symbols_only)
{
  switch (scenario)
  {
  case TS_ALL:
    {
      if (symbols_only_ && finish_border_ > 1)
      {
        Stream::Error err;
        err << FNS << " Segment init error:"
          " upper border for symbol (>1).";
        throw SegmentError(err);
      }
      scenario_ = &Segment::check_all_;
      break;
    }
  case TS_STANDARD_UTF8:
    {
      if (symbols_only_ && finish_border_ > 4)
      {
        Stream::Error err;
        err << FNS << " Segment init error:"
          " upper border for std utf8 (>4).";
        throw SegmentError(err);
      }
      scenario_ = 
        &Segment::check_with_walker_<SegmentorTestCommons::Utf8CharWalker>;
      break;
    }
  case TS_NON_STANDARD_UTF8:
    {
      if (symbols_only_ && finish_border_ > 6)
      {
        Stream::Error err;
        err << FNS << " Segment init error:"
          " upper border for non std utf8 (>6).";
        throw SegmentError(err);
      }
      scenario_ = 
        &Segment::check_with_walker_<SegmentorTestCommons::PseudoUtf8CharWalker>;
      break;
    }
  case TS_SEPARATORS:
    {
      if (symbols_only_ && finish_border_ > 4)
      {
        Stream::Error err;
        err << FNS << " Segment init error:"
          " invalid borders for separators.";
        throw SegmentError(err);
      }
      scenario_ = &Segment::check_separators_;
      break;
    }
  case TS_PHRASES:
    {
      scenario_ = &Segment::check_phrases_;
      break;
    }
  case TS_PHRASES_SEQ:
    {
      scenario_ = &Segment::check_phrases_seq_;
      break;
    }
  default:
    {
      Stream::Error err;
      err << FNS << " Segment init error:"
        " got unexpected scenario id.";
      throw SegmentError(err);
    }
  }
}

bool
Segment::equal_ignore_spaces(const char *orig, size_t orig_len,
                             const char *with_spaces, size_t with_spaces_len)
  throw ()
{
  if (with_spaces_len < orig_len)
  {
    return false;
  }
  
  const char *orig_end = orig + orig_len;
  const char *with_spaces_end = with_spaces + with_spaces_len;
  while (orig < orig_end)
  {
    if (*with_spaces == *orig)
    {
      ++with_spaces;
      ++orig;
    }
    else if (*with_spaces == ' ')
    {
      ++with_spaces;
      if (--with_spaces_len < orig_len)
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  
  return (with_spaces == with_spaces_end);
}

void
Segment::check_all_(std::istream&, std::ostream& estrm) const
  throw (SegmentError)
{
  unsigned char test[finish_border_ + 7];
  memset(test, 0, sizeof(test));

  unsigned char test_null[sizeof(test)];
  memset(test_null, 0, sizeof(test_null));

  char* test_ptr = reinterpret_cast<char*>(test);

  for (size_t i = start_border_; i <= finish_border_; ++i)
  {
    std::cout << "Start processing of sequences of " 
              << i << " byte(s) length." << std::endl;
    do
    {
      // update char string as increment of big number (MSB)
      for(size_t j = i - 1; !set_next(test[j]) && j > 0; j--);

      try
      {
        std::string result;
        segmentor_->put_spaces(result, test_ptr, i);

        if (print_utf8_transforms_ &&
            !equal_ignore_spaces(test_ptr, i,
                                 result.c_str(), result.size()))
        {
          // print transformation where input was changed, ignore spaces
          // for general purposes
          std::cout << "transformation: '";
          SegmentorTestCommons::hex_dump(std::cout, test_ptr, i);
          std::cout << "' => '";
          SegmentorTestCommons::hex_dump(std::cout, result.c_str(), result.size());
          std::cout << "'" << std::endl;
        }
      }
      catch (const eh::Exception& e)
      {
        estrm << "exception: '"
              << std::string(test_ptr, i) << "' => \"" << e.what() << '\"'
              << std::endl;
      }
    }
    while (memcmp(test, test_null, i));
  }
}

void
Segment::check_separators_(std::istream&, std::ostream& estrm) const
  throw (SegmentError)
{
  SegmentorTestCommons::Utf8CharWalker test_str(start_border_);

  size_t octets = 0;
  while ((octets = test_str.octets()) <= finish_border_)
  {
    std::cout << "Start processing of sequences of " 
              << octets << " byte(s) length." << std::endl;
    do
    {
      try
      {
        std::string result;
        segmentor_->put_spaces(result, test_str, octets);

        if (result.empty() || result == " ")
        {
          estrm << "transformation: '"
                << test_str << "' => '" << result << "\' ";
          test_str.dump(estrm);
          estrm << " (U+" << std::hex << test_str.code() << ')'
                << std::endl;
        }
      }
      catch (const eh::Exception& e)
      {
        estrm << "exception: '" << test_str
              << "' => \"" << e.what() << '\"'
              << std::endl;
      }
    }
    while (test_str.next());
  }
}

void
Segment::check_phrases_(std::istream& istrm, std::ostream& estrm) const
  throw (SegmentError)
{
  std::string word_from, word_to;
  for (int line_num = 0; !istrm.eof(); ++line_num)
  {
    try
    {
      getline(istrm, word_from, ' ');
      if (word_from.empty() || istrm.eof())
      {
        break;
      }
      getline(istrm, word_to);
      
      const char* from_cstr = word_from.c_str();
      size_t from_size = word_from.size();

      std::string result;
      segmentor_->put_spaces(result, from_cstr, from_size);

      if (word_to != result)
      {
        //this is error condition - expected not equal existing
        estrm << "phrase " << line_num << " : "
              << word_from << " -> " << word_to  << " != " << result
              << ((word_from != result) ? " * " : " ^ ")
              << std::endl;
      }
    }
    catch (const eh::Exception& e)
    {
      estrm << "exception in " << line_num << " : "
            << word_from << " -> " << word_to
            << "(\"" << e.what() << "\")"
            << std::endl;
    }
  }
}

void
Segment::check_phrases_seq_(std::istream& istrm, std::ostream& estrm) const
  throw (SegmentError)
{
  std::string word_from, word_to;
  for (int line_num = 0; !istrm.eof(); ++line_num)
  {
    try
    {
      getline(istrm, word_from, ' ');
      if (word_from.empty() || istrm.eof())
      {
        break;
      }
      getline(istrm, word_to);
      
      const char* from_cstr = word_from.c_str();
      size_t from_size = word_from.size();

      Language::Segmentor::WordsList list;
      segmentor_->segmentation(list, from_cstr, from_size);

      std::string result;
      result.reserve(1024);
      for (Language::Segmentor::WordsList::const_iterator i = list.begin();
           i != list.end(); ++i)
      {
        if (!result.empty())
        {
          result += " ";
        }
        result += *i;        
      }
        
      if (word_to != result)
      {
        //this is error condition - expected not equal existing
        estrm << "phrase " << line_num << " : "
              << word_from << " -> " << word_to  << " != " << result
              << ((word_from != result) ? " * " : " ^ ")
              << std::endl;
      }
    }
    catch (const eh::Exception& e)
    {
      estrm << "exception in " << line_num << " : "
            << word_from << " -> " << word_to
            << "(\"" << e.what() << "\")"
            << std::endl;
    }
  }
}
