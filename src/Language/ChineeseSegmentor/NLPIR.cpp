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



#include <fstream>

#include <Sync/Key.hpp>

#include <String/StringManip.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Language/ChineeseSegmentor/NLPIR.hpp>

#define OS_LINUX
#include <NLPIR.h>


namespace
{
  void
  free_nlpir(void* nlpir) throw ();

  Sync::Key<CNLPIR> nlpir_key(free_nlpir);

  void
  free_nlpir(void* nlpir) throw ()
  {
    delete static_cast<CNLPIR*>(nlpir);
    nlpir_key.set_data(0);
  }
};

namespace Language
{
  namespace Segmentor
  {
    namespace Chineese
    {
      NlpirSegmentor::NlpirSegmentor(const char* path)
        throw (UniqueException, SegmException)
      {
        std::string license;
        bool file_exists = false;
        try
        {
          std::ifstream in((path ? std::string(path) + "/" : std::string()) +
            "Data/NLPIR.code");
          if (in)
          {
            file_exists = true;
            std::getline(in, license);
          }
        }
        catch (const eh::Exception& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "Failed to read license code: " << ex.what();
          throw SegmException(ostr);
        }
        if (!NLPIR_Init(path, UTF8_CODE, file_exists ? license.c_str() : 0))
        {
          Stream::Error ostr;
          ostr << FNS << "Failed to initialize NLPIR";
          throw SegmException(ostr);
        }
      }

      NlpirSegmentor::~NlpirSegmentor() throw ()
      {
        NLPIR_Exit();
      }

      const char*
      NlpirSegmentor::put_spaces_(const char* phrase, size_t phrase_len)
        throw (eh::Exception)
      {
        CNLPIR* nlpir = nlpir_key.get_data();
        if (!nlpir)
        {
          nlpir = new CNLPIR;
          try
          {
            nlpir_key.set_data(nlpir);
          }
          catch (const eh::Exception&)
          {
            delete nlpir;
            throw;
          }
        }
        const char* r = nlpir->ParagraphProcess(
          std::string(phrase, phrase_len).c_str(), 0);
        if (!r)
        {
          Stream::Error ostr;
          ostr << FNS << "Failed to process paragraph";
          throw SegmException(ostr);
        }
        return r;
      }

      void
      NlpirSegmentor::segmentation(WordsList& result, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        try
        {
          std::string r(put_spaces_(phrase, phrase_len));
          result.clear();
          String::StringManip::Splitter<
            String::AsciiStringManip::Char3Category<' ', '\t', '\n'> >
            tokenizer(r);
          for (String::SubString token; tokenizer.get_token(token);)
          {
            result.push_back(token.str());
          }
        }
        catch (const SegmException&)
        {
          throw;
        }
        catch (const eh::Exception& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "Generic failure: " << ex.what();
          throw SegmException(ostr);
        }
      }

      void
      NlpirSegmentor::put_spaces(std::string& res, const char* phrase,
        size_t phrase_len) const throw (SegmException)
      {
        try
        {
          res = put_spaces_(phrase, phrase_len);
        }
        catch (const SegmException&)
        {
          throw;
        }
        catch (const eh::Exception& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "Generic failure: " << ex.what();
          throw SegmException(ostr);
        }
      }
    }
  }
}
