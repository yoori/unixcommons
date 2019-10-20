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



#include <sstream>
#include <limits>

#ifdef MECAB_LIBRARY
#include <mecab.h>
#endif

#include <String/UTF8Handler.hpp>
#include <String/UTF8Category.hpp>
#include <String/Tokenizer.hpp>

#include <Generics/ArrayAutoPtr.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Language/SegmentorCommons/SegmentorCommons.hpp>

#include "Mecab.hpp"


#ifdef MECAB_LIBRARY
namespace
{
  const char* const MECAB_DEFAULT_CONFIG = "/usr/etc/mecabrc";
  const char JAPANESE_RANGES[] =
    "\xe2\xba\x80-\xe2\xbb\xb3"
    "\xe2\xbc\x80-\xe2\xbf\x95"
    "\xe3\x80\x80-\xe3\x83\xbf"
    "\xe3\x87\xb0-\xe3\x87\xbf"
    "\xe3\x88\x80-\xe3\x8b\xbe"
    "\xe3\x90\x80-\xe4\xb6\xb5"
    "\xe4\xb8\x80-\xe9\xbf\x83"
    "\xef\xa4\x80-\xef\xab\x99"
    "\xef\xb8\xb0-\xef\xb9\x8f"
    "\xef\xbd\xa6-\xef\xbe\x9d"
    "\xef\xbe\x9e-\xef\xbe\x9f"
    ;

  typedef const String::StringManip::InverseCategory<
    String::Utf8Category> NotJapanese;
  NotJapanese NOT_JAPANESE(JAPANESE_RANGES);
}
#endif

namespace Language
{
  namespace Segmentor
  {
    namespace Japanese
    {
#ifdef MECAB_LIBRARY
      //
      // class MecabSegmentor::MecabTagger
      //

      MecabSegmentor::MecabTagger_::MecabTagger_(const char* cmd)
        throw (UniqueException, SegmException)
        : tagger_(0)
      {
        try
        {
          tagger_.reset(MeCab::createTagger(cmd));
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }

        if (!tagger_.get())
        {
          Stream::Error error;
          error << FNS << "Can't init mecab tagger: "
                << MeCab::getTaggerError();
          throw SegmException(error);
        }
      }

      inline
      MecabSegmentor::MecabTagger_::~MecabTagger_() throw ()
      {
      }

      inline
      bool
      MecabSegmentor::MecabTagger_::empty() const throw ()
      {
        return tagger_.get() == 0;
      }

      template <class Target>
      inline
      void
      MecabSegmentor::MecabTagger_::parse_to(Target& target,
        const char* phrase,
        size_t phrase_len)
        throw (eh::Exception)
      {
        const MeCab::Node* node = tagger_->parseToNode(phrase, phrase_len);
        if (!node)
        {
          Stream::Error error;
          error << FNS <<  "Can't parse phrase \""
                << String::SubString(phrase, phrase_len)
                << "\". Description: "
                << tagger_->what();
          throw SegmException(error);
        }
        while (node)
        {
          append(target, String::SubString(node->surface, node->length));
          node = node->next;
        }
      }

      //
      // class MecabSegmentor
      //

      inline
      MecabSegmentor::MecabTagger_var_
      MecabSegmentor::get_tagger_() const
        throw (SegmException)
      {
        bool create_new = false;
        {
          WriteGuard_ guard(lock_);

          if (!taggers_.empty())
          {
            MecabTagger_var_ result = taggers_.top();
            taggers_.pop();
            return result;
          }

          if (max_threads_count_)
          {
            --max_threads_count_;
            create_new = true;
          }
          else if (policy_ == TLVP_WAITING)
          {
            ++waiting_num_;
          }
        }

        if (create_new)
        {
          return init_new_tagger_();
        }
        else if (policy_ == TLVP_WAITING)
        {
          waiting_sem_->acquire();
          return get_tagger_();
        }
        else
        {
          Stream::Error error;
          error << FNS << "Can't create a new mecab tagger, "
            "because limit is reached.";
          throw SegmException(error);
        }
      }

      inline
      void
      MecabSegmentor::put_tagger_(MecabTagger_var_& tagger) const
        throw (SegmException)
      {
        try
        {
          WriteGuard_ guard(lock_);

          if (policy_ == TLVP_WAITING && waiting_num_ > 0)
          {
            --waiting_num_;
            waiting_sem_->release();
          }

          taggers_.push(tagger);
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }
      }

      MecabSegmentor::MecabSegmentor(const char* config_file,
        size_t max_threads_count, ThreadsLimitViolationPolicy policy)
          throw (SegmException)
        : policy_(policy),
          max_threads_count_(max_threads_count? max_threads_count: 1),
          waiting_sem_(0), waiting_num_(0)
      {
        try
        {
          std::ostringstream buf;
          buf << "-r " <<
            (config_file ? config_file : MECAB_DEFAULT_CONFIG) <<
            " -O wakati" " -g " << std::numeric_limits<size_t>::max();
          buf.str().swap(command_line_);

          if (policy_ == TLVP_WAITING)
          {
            waiting_sem_.reset(new Sync::Semaphore(0));
          }
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }

        //at least one tagger should be initialized
        MecabTagger_var_ buf = get_tagger_();
        put_tagger_(buf);
      }

      MecabSegmentor::MecabTagger_var_
      MecabSegmentor::init_new_tagger_() const
        throw (SegmException)
      {
        try
        {
          MecabTagger_var_ tagger = new MecabTagger_(command_line_.c_str());
          return tagger;
        }
        catch (const SegmException& e)
        {
          throw;
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }
      }

      MecabSegmentor::~MecabSegmentor() throw ()
      {
      }

      template <class Target>
      inline
      void
      MecabSegmentor::put_parsed_(Target& target, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        try
        {
          MecabTagger_var_ tagger = get_tagger_();

          String::SubString input(phrase, phrase_len);
          String::StringManip::Splitter<const NotJapanese&> tokenizer(
            input, NOT_JAPANESE);

          const char* pos = input.begin();
          String::SubString token;
          while (tokenizer.get_token(token)) // foreach japanese tokens
          {
            // we can have not japanese text before token
            if (token.begin() != pos)
            {
              append(target, String::SubString(pos, token.begin()));
            }

            tagger->parse_to(target, token.begin(), token.length());

            // prepare for next token
            pos = token.end();

          }//while // foreach japanese tokens

          // we can have text after tokens
          if (pos != input.end())
          {
            append(target, String::SubString(pos, input.end()));
          }

          put_tagger_(tagger);
        }
        catch (const SegmException&)
        {
          throw;
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }
      }

      void
      MecabSegmentor::segmentation(WordsList& result, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        put_parsed_(result, phrase, phrase_len);
      }

      void
      MecabSegmentor::put_spaces(std::string& result, const char* phrase,
        size_t phrase_len) const
        throw (SegmException)
      {
        try
        {
          if (!phrase || !phrase_len)
          {
            result.clear();
            return;
          }

          const std::string::size_type MAX_OUTPUT_LENGTH =
            phrase_len + phrase_len + 10; //fixme

          std::string result_str;
          result_str.reserve(MAX_OUTPUT_LENGTH);

          put_parsed_(result_str, phrase, phrase_len);

          result_str.swap(result);
        }
        catch (const SegmException&)
        {
          throw;
        }
        catch (const eh::Exception& e)
        {
          Stream::Error error;
          error << FNS << "eh::Exception caught: " << e.what();
          throw SegmException(error);
        }
      }

      bool
      MecabSegmentor::is_valid_utf8_(const char* str, size_t str_len) const
        throw ()
      {
        bool valid_utf8 = true;
        unsigned long count = 0;
        unsigned long count_buf = 0;
        while (valid_utf8 && count < str_len)
        {
          count_buf = String::UTF8Handler::get_octet_count(str[count]);
          if (!count_buf || count_buf + count > str_len)
          {
            return false;
          }

          valid_utf8 = valid_utf8 &&
            String::UTF8Handler::is_correct_utf8_sequence(str + count,
              count_buf);
          count += count_buf;
        }

        return valid_utf8;
      }

#else

      MecabSegmentor::MecabSegmentor(const char*, size_t,
        ThreadsLimitViolationPolicy) throw (UniqueException, SegmException)
      {
      }

      MecabSegmentor::~MecabSegmentor() throw ()
      {
      }

      void
      MecabSegmentor::segmentation(WordsList&, const char*, size_t) const
        throw (SegmException)
      {
      }

      void
      MecabSegmentor::put_spaces(std::string&, const char*, size_t) const
        throw (SegmException)
      {
      }
#endif
    } //namespace Japanese
  } //namespace Segmentor
} //namespace Language
