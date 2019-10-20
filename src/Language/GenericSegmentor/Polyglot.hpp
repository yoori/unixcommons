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



#ifndef LANGUAGE_GENERIC_SEGMENTOR_POLYGLOT_HPP
#define LANGUAGE_GENERIC_SEGMENTOR_POLYGLOT_HPP

#include <memory>

#include <Generics/Function.hpp>

#include <Language/SegmentorCommons/SegmentorInterface.hpp>

#include <Language/SegmentorManager/SegmentorFilter.hpp>

#include <Language/Polyglot/DictionaryLoader.hpp>
#include <Language/Polyglot/Tokenizer.hpp>


namespace Language
{
  namespace Segmentor
  {
    template <typename Tokenizer, typename Dictionary,
      typename SuffixDictionary>
    class PolyglotSegmentorWrap :
      public UniqueSegmentorInterface<
        PolyglotSegmentorWrap<Tokenizer, Dictionary, SuffixDictionary>>
    {
    public:
      typedef typename UniqueSegmentorInterface<
        PolyglotSegmentorWrap<Tokenizer, Dictionary, SuffixDictionary>>::
          UniqueException UniqueException;
      typedef typename UniqueSegmentorInterface<
        PolyglotSegmentorWrap<Tokenizer, Dictionary, SuffixDictionary>>::
          SegmException SegmException;

      explicit
      PolyglotSegmentorWrap(const char* config_file)
        throw (UniqueException, SegmException);

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
      ~PolyglotSegmentorWrap() throw ();

    private:
      Dictionary dict_;
      SuffixDictionary suffix_dict_;
      std::unique_ptr<Tokenizer> tokenizer_;
    };

    struct DefaultPolyglotSymbols
    {
      typedef String::StringManip::InverseCategory<String::Utf8Category>
        CategoryType;
      static const CategoryType INVALID_SYMBOLS;
    };

    typedef
      AutomaticFilterSegmentor<
        PolyglotSegmentorWrap<
          Polyglot::Tokenizer,
          Polyglot::Dictionary,
          Polyglot::SuffixDictionary>,
        DefaultPolyglotSymbols>
      PolyglotSegmentor;

    typedef
      AutomaticFilterSegmentor<
        PolyglotSegmentorWrap<
          Polyglot::NormalizeTokenizer,
          Polyglot::DictionaryWithNorm,
          Polyglot::SuffixDictionary>,
        DefaultPolyglotSymbols>
      NormalizePolyglotSegmentor;

    typedef ReferenceCounting::ConstPtr<PolyglotSegmentor>
      PolyglotSegmentor_var;

    typedef ReferenceCounting::ConstPtr<NormalizePolyglotSegmentor>
      NormalizePolyglotSegmentor_var;
  } //namespace Segmentor
} //namespace Language

namespace Language
{
  namespace Segmentor
  {
    /**X
     * class PolyglotSegmentor
     */
    template <typename Tokenizer, typename Dictionary,
      typename SuffixDictionary>
    PolyglotSegmentorWrap<Tokenizer, Dictionary, SuffixDictionary>::
      PolyglotSegmentorWrap(const char* config_file)
        throw (UniqueException, SegmException)
    {
      try
      {
        Polyglot::DictionaryLoader::load(config_file, dict_);
        Polyglot::DictionaryLoader::load_suffixes(config_file, suffix_dict_);
        tokenizer_.reset(new Tokenizer(dict_, suffix_dict_));
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error error;
        error << FNS <<
          "can't initialize dictionary: eh::Exception caught: " <<
          ex.what();
        throw SegmException(error);
      }
      catch (...)
      {
        Stream::Error error;
        error << FNS <<
          "can't initialize dictionary: unknown exception caught";
        throw SegmException(error);
      }
    }

    template <typename Tokenizer, typename Dictionary,
      typename SuffixDictionary>
    PolyglotSegmentorWrap<Tokenizer, Dictionary, SuffixDictionary>::
      ~PolyglotSegmentorWrap() throw ()
    {
    }

    template <typename Tokenizer, typename Dictionary,
      typename SuffixDictionary>
    void
    PolyglotSegmentorWrap<Tokenizer, Dictionary, SuffixDictionary>::
      segmentation(WordsList& result, const char* phrase,
        size_t phrase_len) const throw (SegmException)
    {
      try
      {
        tokenizer_->segment(String::SubString(phrase, phrase_len), result);
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error error;
        error << FNS << "eh::Exception caught: " << ex.what();
        throw SegmException(error);
      }
      catch (...)
      {
        Stream::Error error;
        error << FNS << "unknown Exception";
        throw SegmException(error);
      }
    }

    template <typename Tokenizer, typename Dictionary,
      typename SuffixDictionary>
    void
    PolyglotSegmentorWrap<Tokenizer, Dictionary, SuffixDictionary>::
      put_spaces(std::string& result, const char* phrase,
        size_t phrase_len) const throw (SegmException)
    {
      try
      {
        tokenizer_->put_spaces(result,
          String::SubString(phrase, phrase_len));
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error error;
        error << FNS << "eh::Exception caught: " << ex.what();
        throw SegmException(error);
      }
      catch (...)
      {
        Stream::Error error;
        error << FNS << "unknown Exception";
        throw SegmException(error);
      }
    }
  } //namespace Segmentor
} //namespace Language

#endif
