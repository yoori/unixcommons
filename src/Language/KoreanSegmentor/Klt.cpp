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



#include <Generics/ArrayAutoPtr.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Language/KoreanSegmentor/Klt.hpp>

#include "Korean.hpp"


#ifdef KLT_LIBRARY
extern "C"
{
  #include <index/ham-ndx.h>
  #include <index/get-tok.h>

  /**X
   * @param word KSC5601/UTF-8 input sentence
   * @param length
   * @param out ptr. array of keywords
   * @param hamout morph. analysis result
   * @param mode running mode of HAM
   */
  extern PREFIX int
  get_tokens_TS(HAM_PUCHAR word, int length, TOKEN_STR out[],
    HAM_PMORES hamout, HAM_PRUNMODE mode);
}

namespace
{
  const char* const KLT_DEFAULT_CONFIG = "/opt/KLT/hdic/KLT2000.ini";
}
#endif


namespace Language
{
  namespace Segmentor
  {
    namespace Korean
    {
#ifdef KLT_LIBRARY

      //
      // Variables
      //

      HAM_RUNMODE klt_mode; // HAM running mode: 'header/runmode.h'

      //
      // class KltSegmentor
      //

      KltSegmentor::KltSegmentor(const char* config_file,
        const char* additional_params)
        throw (UniqueException, SegmException)
      {
        if (open_HAM_index(&klt_mode,
          const_cast<char*>(additional_params ? additional_params : ""),
           const_cast<char*>(config_file ? config_file : KLT_DEFAULT_CONFIG)))
        {
          Stream::Error ostr;
          ostr << FNS << "can't load dictionary \"" << klt_mode.dicpath <<
            "\", KLT error code: " << klt_mode.err_code;
          throw SegmException(ostr);
        }

        klt_mode.hcode_out = klt_mode.hcode_in = 2;
        klt_mode.index.stopw = 0;
      }

      KltSegmentor::~KltSegmentor() throw ()
      {
        close_HAM_index(&klt_mode);
      }

      void
      KltSegmentor::segmentation(WordsList& result, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        try
        {
          result.clear();

          if (!phrase || !phrase_len)
          {
            return;
          }

          HAM_MORES hamout;
          Generics::ArrayAutoPtr<TOKEN_STR> out(phrase_len);

          String::SubString input(phrase, phrase_len);
          String::StringManip::Splitter<const NotHangul&> tokenizer(
            input, NOT_HANGUL);
          const char* pos = input.begin();
          String::SubString token;
          while (tokenizer.get_token(token))
          {
            if (token.begin() != pos)
            {
              result.push_back(std::string(pos, token.begin()));
            }

            size_t kwd_count = get_tokens_TS(
              reinterpret_cast<HAM_PUCHAR>(const_cast<char*>(token.begin())),
              token.length(), out.get(), &hamout, &klt_mode);

            for (size_t i = 0; i < kwd_count; ++i)
            {
              std::string klt_token(
                reinterpret_cast<const char*>(out.get()[i].token),
                out.get()[i].length);

              // Remove everything after first zero byte.
              klt_token.resize(strlen(klt_token.c_str()));

              result.push_back(klt_token);
            }

            pos = token.end();
          }

          if (tokenizer.is_error())
          {
            Stream::Error ostr;
            ostr << FNS << "invalid UTF-8 character in the input: " << input;
            throw SegmException(ostr);
          }

          if (pos != input.end())
          {
            result.push_back(std::string(pos, input.end()));
          }
        }
        catch (const eh::Exception& e)
        {
          Stream::Error ostr;
          ostr << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(ostr);
        }
      }

      void
      KltSegmentor::put_spaces(std::string& res, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        try
        {
          std::string result;
          result.reserve(phrase_len + phrase_len);
          WordsList gathered_words;

          segmentation(gathered_words, phrase, phrase_len);

          WordsList::const_iterator it = gathered_words.begin();
          WordsList::const_iterator end = gathered_words.end();
          for (; it != end; ++it)
          {
            if (!result.empty())
            {
              result += ' ';
            }
            result += *it;
          }
          result.swap(res);
        }
        catch (const SegmException& e)
        {
          Stream::Error ostr;
          ostr << FNS << "Language::Segmentor::SegmentorInterface::"
            "SegmException caught: " << e.what();
          throw SegmException(ostr);
        }
        catch (const eh::Exception& e)
        {
          Stream::Error ostr;
          ostr << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(ostr);
        }
      }

#else

      KltSegmentor::KltSegmentor(const char*, const char*)
        throw (UniqueException, SegmException)
      {
      }

      KltSegmentor::~KltSegmentor() throw ()
      {
      }

      void
      KltSegmentor::segmentation(WordsList&, const char*, size_t) const
        throw (SegmException)
      {
      }

      void
      KltSegmentor::put_spaces(std::string&, const char*, size_t) const
        throw (SegmException)
      {
      }

#endif
    } //namespace Korean
  } //namespace Segmentor
} //namespace Language
