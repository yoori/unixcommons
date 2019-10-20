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

#include <Language/SegmentorCommons/SegmentorCommons.hpp>
#include <Language/KoreanSegmentor/Moran.hpp>

#include "Korean.hpp"


/*
 * ! WARNING !
 * moran segmentor is unstable, it can segfault
 */

namespace
{
#ifdef MORAN_LIBRARY
  const unsigned int MAX_POS_LENGTH  = 10; /* POS tag length */
  const unsigned int MAX_WORD_LENGTH = 100; /* Max Keyword Length at UTF8 */
  const unsigned int MAX_INPUT_STR   = 10240; /* Max Input at One Time */
  const unsigned int MAX_INDEX_TERMS = 500; /* Max Keyword at One Time */

  extern "C"
  {
    struct pos_info
    {
      int id; /* Keyword ID */
      int token_id; /* Token ID */
      int word_position; /* Position in Unicode String */
      int moran; /* Analysis Type */
      int where; /* Debugging Code */
      int length; /* Length of Key Word */
      char pos[MAX_POS_LENGTH]; /* POS of Word */
      unsigned short word[MAX_WORD_LENGTH]; /* Key Word in UCS2 */
    };

    /* Lexicon Loading from the file db_file_name ex) "RunEnv/moran.dbs" */
    void MorAn16_open_dbs(char *db_file_name);

    /* Code Conversion : UTF8 -> UCS2 return UCS2 length */
    int UTF8toUCS2(char *utf8_string, unsigned short *ucs_string);

    /* Code Conversion : UCS2 -> UTF8 return UTF8 length */
    int UCS2toUTF8(unsigned short *ucs_string, char *utf8_string);

    /* Index Word Extraction from UCS2 string to Word Structure with config,
       config NULL means Default setting */
    int MorAn16_korStr2indexStr(
      unsigned short *source, struct  pos_info *output, void *config, int mode);

    /* Destroy Lexicon */
    void MorAn16_close_dbs();
  }

  //
  // class MoranParser
  //

  class MoranParser
  {
    //UCS2 string
    Generics::ArrayAutoPtr<unsigned short> source_;
    //Set of tokens in UCS2
    Generics::ArrayAutoPtr<pos_info> output_;
    //Token in UTF8
    Generics::ArrayAutoPtr<char> kwd_;

  public:
    MoranParser (const char* phrase, size_t phrase_len)
      throw (eh::Exception);

    template <class Target>
    void
    parse_to(Target& target) throw (eh::Exception);
  };

  inline
  MoranParser::MoranParser(const char* phrase, size_t phrase_len)
    throw (eh::Exception)
    : source_(phrase_len + 1),
      output_(phrase_len),
      kwd_(2 * MAX_WORD_LENGTH + 1)
  {
    std::string input(phrase, phrase_len);
    UTF8toUCS2(const_cast<char*>(input.c_str()), source_.get());
  }

  template <class Target>
  inline
  void
  MoranParser::parse_to(Target& target)
    throw (eh::Exception)
  {
    size_t n = 0;
    n = MorAn16_korStr2indexStr(source_.get(), output_.get(), 0, 1);

    for (size_t i = 0; i < n; ++i)
    {
      UCS2toUTF8(output_[i].word, kwd_.get());
      Language::Segmentor::append(target, String::SubString(kwd_.get()));
    }
  }

  template <class Target>
  inline
  void
  parse_to(Target& target, const char* phrase, size_t phrase_len)
    throw (eh::Exception)
  {
    String::SubString input(phrase, phrase_len);
    String::StringManip::Splitter<const NotHangul&> tokenizer(input,
      Language::Segmentor::Korean::NOT_HANGUL);
    const char* pos = input.begin();
    String::SubString token;

    while (tokenizer.get_token(token))
    {
      if (token.begin() != pos)
      {
        Language::Segmentor::append(target, String::SubString(pos, token.begin()));
      }

      MoranParser parser(token.begin(), token.length());
      parser.parse_to(target);

      pos = token.end();

    }//while

    if (pos != input.end())
    {
      Language::Segmentor::append(target, String::SubString(pos, input.end()));
    }
  }
#endif//MORAN_LIBRARY
}

namespace Language
{
  namespace Segmentor
  {
    namespace Korean
    {
      //
      // class MoranSegmentor
      //
#ifdef MORAN_LIBRARY
      MoranSegmentor::MoranSegmentor(const char* config_file)
        throw (UniqueException)
      {
        MorAn16_open_dbs(const_cast<char*>(config_file));
      }

      MoranSegmentor::~MoranSegmentor() throw ()
      {
        MorAn16_close_dbs();
      }

      void
      MoranSegmentor::segmentation(WordsList& result, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        try
        {
          result.clear();

          if (!phrase || *phrase == '\0' || !phrase_len)
          {
            return;
          }

          if (!is_valid_utf8_(phrase, phrase_len))
          {
            String::SubString phrase_str(phrase, phrase_len);
            append(result, phrase_str);
            return;
          }

          parse_to(result, phrase, phrase_len);
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }
      }

      void
      MoranSegmentor::put_spaces(std::string& res, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        try
        {
          if (!phrase || *phrase == '\0' || !phrase_len)
          {
            res.clear();
            return;
          }

          std::string result;
          result.reserve(phrase_len + phrase_len);

          if (!is_valid_utf8_(phrase, phrase_len))
          {
            String::SubString phrase_str(phrase, phrase_len);
            append(result, phrase_str);
            return;
          }

          parse_to(result, phrase, phrase_len);

          result.swap(res);
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }
      }

      bool
      MoranSegmentor::is_valid_utf8_(const char* str, size_t str_len) const
        throw ()
      {
        bool valid_utf8 = true;
        unsigned long count = 0;
        unsigned long count_buf = 0;
        while (valid_utf8 && count < str_len)
        {
          count_buf = String::UTF8Handler::get_octet_count(str[count]);
          //UTF8 symbol of size 4 cannot be converted into ucs2
          if (!count_buf || count_buf > 3 || count_buf + count > str_len)
          {
            return false;
          }

          valid_utf8 = valid_utf8 &&
            String::UTF8Handler::is_correct_utf8_sequence(str + count, count_buf);
          count += count_buf;
        }

        return valid_utf8;
      }
#else //not MORAN_LIBRARY
      MoranSegmentor::MoranSegmentor(const char* /*config_file*/)
        throw (UniqueException)
      {
      }
      MoranSegmentor::~MoranSegmentor() throw ()
      {
      }
      void
      MoranSegmentor::segmentation(WordsList& /*result*/,
        const char* /*phrase*/, size_t /*phrase_len*/) const
        throw (SegmException)
      {
      }
      void
      MoranSegmentor::put_spaces(std::string& /*result*/,
        const char* /*phrase*/, size_t /*phrase_len*/) const
        throw (SegmException)
      {
      }
      bool
      MoranSegmentor::is_valid_utf8_(const char* /*str*/,
        size_t /*str_len*/) const throw ()
      {
        return false;
      }

#endif //MORAN_LIBRARY

    } //namespace Korean
  } //namespace Segmentor
} //namespace Language
