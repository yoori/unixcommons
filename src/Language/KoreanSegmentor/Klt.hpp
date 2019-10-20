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



#ifndef LANGUAGE_KOREAN_SEGMENTOR_KLT_HPP
#define LANGUAGE_KOREAN_SEGMENTOR_KLT_HPP

#include <Language/SegmentorCommons/SegmentorInterface.hpp>


namespace Language
{
  namespace Segmentor
  {
    namespace Korean
    {
      class KltSegmentor : public UniqueSegmentorInterface<KltSegmentor>
      {
      public:
        explicit
        KltSegmentor(const char* config_file = 0,
          const char* additional_params = 0)
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
        ~KltSegmentor() throw ();
      };
      typedef ReferenceCounting::ConstPtr<KltSegmentor>
        KltSegmentor_var;
    } //namespace Korean
  } //namespace Segmentor
} //namespace Language

#endif
