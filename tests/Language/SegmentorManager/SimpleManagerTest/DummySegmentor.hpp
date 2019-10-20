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



#ifndef _TESTS_LANGUAGE_SEGMETOR_MANAGER_SIMPLE_MANAGER_TEST_DUMMY_SEGMENTOR_HPP_
#define _TESTS_LANGUAGE_SEGMETOR_MANAGER_SIMPLE_MANAGER_TEST_DUMMY_SEGMENTOR_HPP_

#include <Language/SegmentorCommons/SegmentorInterface.hpp>
#include <Stream/MemoryStream.hpp>
#include <sstream>

class DummySegmentor: public Language::Segmentor::SegmentorInterface
{
public:

  DummySegmentor(int my_id, std::ostream& out) throw ();

  virtual void
  segmentation(Language::Segmentor::WordsList& result,
    const char* phrase, size_t phrase_len) const throw (SegmException);

  virtual void
  put_spaces(std::string& result, const char* phrase,
    size_t phrase_len) const throw (SegmException);

private:
  virtual
  ~DummySegmentor() throw ();

  int my_id_;
  std::ostream& out_;
};


//
// class DummySegmentor
//

DummySegmentor::DummySegmentor(int my_id, std::ostream& out) throw ():
  my_id_(my_id),
  out_(out)
{
}

DummySegmentor::~DummySegmentor() throw ()
{
}

void
DummySegmentor::segmentation(Language::Segmentor::WordsList& result, const char* phrase,
  size_t phrase_len) const throw (SegmException)
{
  try
  {
    out_ << "#" << my_id_;
    size_t half = phrase_len / 2;
    result.clear();
    if (half > 0)
    {
      result.push_back(std::string(phrase, half));
    }
    if (phrase_len - half > 0)
    {
      result.push_back(std::string(phrase + half, phrase_len - half));
    }
  }
  catch (const eh::Exception& e)
  {
    Stream::Error error;
    error << "DummySegmentor::segmentation: eh::Exception caught: "
          << e.what();

    throw SegmException(error);
  }
}

void
DummySegmentor::put_spaces(std::string& res, const char* phrase,
  size_t phrase_len) const throw (SegmException)
{
  std::string result;

  try
  {
    out_ << "#" << my_id_;

    if (!phrase || !phrase_len)
    {
      res.clear();
      return;
    }

    std::string(phrase, phrase_len).swap(result);

    if (phrase_len < 2)
    {
      return;
    }

    std::string::size_type first = 0;
    std::string::size_type second = 0;
    while(second < phrase_len)
    {
      first = result.find(' ', second);
      if (first == std::string::npos)
      {
        if (second == 0)
        {
          first = 0;
        }
        else
        {
          first = second;
        }
      }
      else
      {
        ++first;
      }

      second = result.find(' ', first);
      if (second == std::string::npos)
      {
        second = phrase_len;
      }

      if (second - first > 1)
      {
        break;
      }
      ++second;
    }

    if (first != std::string::npos && second <= phrase_len && second - first > 1)
    {
      result.assign(phrase, first + 1);
      result += ' ';
      result.append(phrase + first + 1, second - first - 1);
    }

    result.swap(res);
  }
  catch (const eh::Exception& e)
  {
    Stream::Error error;
    error << FNS << "eh::Exception caught: " << e.what();
    throw SegmException(error);
  }
}

#endif
