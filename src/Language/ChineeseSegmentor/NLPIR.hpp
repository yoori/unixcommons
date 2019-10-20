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



#ifndef LANGUAGE_CHINEESE_SEGMENTOR_NLPIR_HPP
#define LANGUAGE_CHINEESE_SEGMENTOR_NLPIR_HPP

#include <Language/SegmentorCommons/SegmentorInterface.hpp>


namespace Language
{
  namespace Segmentor
  {
    namespace Chineese
    {
      class NlpirSegmentor : public UniqueSegmentorInterface<NlpirSegmentor>
      {
      public:
        explicit
        NlpirSegmentor(const char* path = "/usr/share/NLPIR")
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
        static
        const char*
        put_spaces_(const char* phrase, size_t phrase_len)
          throw (eh::Exception);

        virtual
        ~NlpirSegmentor() throw ();
      };
      typedef ReferenceCounting::ConstPtr<NlpirSegmentor>
        NlpirSegmentor_var;
    }
  }
}

#endif
