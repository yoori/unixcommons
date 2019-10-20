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



#ifndef LANGUAGE_SEGMENTORMANAGER_SEGMENTORFILTER_HPP
#define LANGUAGE_SEGMENTORMANAGER_SEGMENTORFILTER_HPP

#include <String/StringManip.hpp>
#include <String/UTF8Category.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Language/SegmentorCommons/SegmentorInterface.hpp>
#include <Language/SegmentorCommons/SegmentorCommons.hpp>


namespace Language
{
  namespace Segmentor
  {
    template <typename Category>
    class FilterSegmentor : public SegmentorInterface
    {
    public:
      FilterSegmentor(const SegmentorInterface* segmentor,
        const Category& filter) throw ();

      virtual
      void
      segmentation(WordsList& result, const char* phrase,
        size_t phrase_len) const throw (SegmException);

      virtual
      void
      put_spaces(std::string& result, const char* phrase,
        size_t phrase_len) const throw (SegmException);

    protected:
      virtual
      ~FilterSegmentor() throw ();

    private:
      const SegmentorInterface_var SEGMENTOR_;
      const Category& FILTER_;
    };

    template <typename Segmentor, typename CategoryWrapper>
    class AutomaticFilterSegmentor :
      public FilterSegmentor<typename CategoryWrapper::CategoryType>
    {
    public:
      AutomaticFilterSegmentor() throw (eh::Exception);
      template <typename T>
      AutomaticFilterSegmentor(T data) throw (eh::Exception);
      template <typename T1, typename T2>
      AutomaticFilterSegmentor(T1 data1, T2 data2) throw (eh::Exception);
      template <typename T1, typename T2, typename T3>
      AutomaticFilterSegmentor(T1 data1, T2 data2, T3 data3)
        throw (eh::Exception);
      template <typename T1, typename T2, typename T3, typename T4>
      AutomaticFilterSegmentor(T1 data1, T2 data2, T3 data3, T4 data4)
        throw (eh::Exception);

    protected:
      virtual
      ~AutomaticFilterSegmentor() throw ();
    };
  }
}

namespace Language
{
  namespace Segmentor
  {
    //
    // FilterSegmentor class
    //

    template <typename Category>
    FilterSegmentor<Category>::FilterSegmentor(
      const SegmentorInterface* segmentor,
      const Category& filter) throw ()
      : SEGMENTOR_(ReferenceCounting::add_ref(segmentor)),
        FILTER_(filter)
    {
    }

    template <typename Category>
    FilterSegmentor<Category>::~FilterSegmentor() throw ()
    {
    }

    template <typename Category>
    void
    FilterSegmentor<Category>::segmentation(WordsList& result,
      const char* phrase, size_t phrase_len) const throw (SegmException)
    {
      result.clear();

      if (!phrase || !phrase_len)
      {
        return;
      }

      try
      {
        String::SubString input(phrase, phrase_len);
        String::StringManip::Splitter<const Category&> tokenizer(
          input, FILTER_);
        const char* pos = input.begin();
        String::SubString token;
        while (tokenizer.get_token(token))
        {
          if (token.begin() != pos)
          {
            result.emplace_back(pos, token.begin());
          }

          WordsList tmp;
          SEGMENTOR_->segmentation(tmp, token.begin(), token.length());
          result.splice(result.end(), tmp);

          pos = token.end();
        }

        if (tokenizer.is_error())
        {
          Stream::Error error;
          error << FNS << "invalid UTF-8 character in the input: " << input;
          throw SegmException(error);
        }

        if (pos != input.end())
        {
          result.emplace_back(pos, input.end());
        }
      }
      catch (const SegmException&)
      {
        throw;
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "eh::Exception caught: " << ex.what();
        throw SegmException(ostr);
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "unknown exception caught";
        throw SegmException(ostr);
      }
    }

    template <typename Category>
    void
    FilterSegmentor<Category>::put_spaces(std::string& res,
      const char* phrase, size_t phrase_len) const throw (SegmException)
    {
      try
      {
        String::SubString input(phrase, phrase_len);
        String::StringManip::Splitter<const Category&> tokenizer(
          input, FILTER_);
        const char* pos = input.begin();

        std::string result;
        result.reserve(2 * input.size());

        String::SubString token;
        while (tokenizer.get_token(token))
        {
          if (token.begin() != pos)
          {
            Language::Segmentor::append(result,
              String::SubString(pos, token.begin()));
          }

          std::string tmp;
          SEGMENTOR_->put_spaces(tmp, token.begin(), token.length());
          Language::Segmentor::append(result, tmp);

          pos = token.end();
        }

        if (pos != input.end())
        {
          Language::Segmentor::append(result,
            String::SubString(pos, input.end()));
        }

        result.swap(res);
      }
      catch (const SegmException&)
      {
        throw;
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "eh::Exception caught: " << ex.what();
        throw SegmException(ostr);
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "unknown exception caught";
        throw SegmException(ostr);
      }
    }


    //
    // AutomaticFilterSegmentor class
    //

    template <typename Segmentor, typename CategoryWrapper>
    AutomaticFilterSegmentor<Segmentor, CategoryWrapper>::
      AutomaticFilterSegmentor() throw (eh::Exception)
      : FilterSegmentor<typename CategoryWrapper::CategoryType>(
          SegmentorInterface_var(new Segmentor()),
          CategoryWrapper::INVALID_SYMBOLS)
    {
    }

    template <typename Segmentor, typename CategoryWrapper>
    template <typename T>
    AutomaticFilterSegmentor<Segmentor, CategoryWrapper>::
      AutomaticFilterSegmentor(T data) throw (eh::Exception)
      : FilterSegmentor<typename CategoryWrapper::CategoryType>(
          SegmentorInterface_var(new Segmentor(data)),
          CategoryWrapper::INVALID_SYMBOLS)
    {
    }

    template <typename Segmentor, typename CategoryWrapper>
    template <typename T1, typename T2>
    AutomaticFilterSegmentor<Segmentor, CategoryWrapper>::
      AutomaticFilterSegmentor(T1 data1, T2 data2) throw (eh::Exception)
      : FilterSegmentor<typename CategoryWrapper::CategoryType>(
          SegmentorInterface_var(new Segmentor(data1, data2)),
          CategoryWrapper::INVALID_SYMBOLS)
    {
    }

    template <typename Segmentor, typename CategoryWrapper>
    template <typename T1, typename T2, typename T3>
    AutomaticFilterSegmentor<Segmentor, CategoryWrapper>::
      AutomaticFilterSegmentor(T1 data1, T2 data2, T3 data3)
      throw (eh::Exception)
      : FilterSegmentor<typename CategoryWrapper::CategoryType>(
          SegmentorInterface_var(new Segmentor(data1, data2, data3)),
          CategoryWrapper::INVALID_SYMBOLS)
    {
    }

    template <typename Segmentor, typename CategoryWrapper>
    template <typename T1, typename T2, typename T3, typename T4>
    AutomaticFilterSegmentor<Segmentor, CategoryWrapper>::
      AutomaticFilterSegmentor(T1 data1, T2 data2, T3 data3, T4 data4)
      throw (eh::Exception)
      : FilterSegmentor<typename CategoryWrapper::CategoryType>(
          SegmentorInterface_var(new Segmentor(data1, data2, data3, data4)),
          CategoryWrapper::INVALID_SYMBOLS)
    {
    }

    template <typename Segmentor, typename CategoryWrapper>
    AutomaticFilterSegmentor<Segmentor, CategoryWrapper>::
      ~AutomaticFilterSegmentor() throw ()
    {
    }
  }
}

#endif
