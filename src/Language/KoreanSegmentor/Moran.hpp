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



#ifndef LANGUAGE_KOREAN_SEGMENTOR_MORAN_HPP
#define LANGUAGE_KOREAN_SEGMENTOR_MORAN_HPP

#include <Language/SegmentorCommons/SegmentorInterface.hpp>


namespace Language
{
  namespace Segmentor
  {
    namespace Korean
    {
      class MoranSegmentor : public UniqueSegmentorInterface<MoranSegmentor>
      {
      public:
        explicit
        MoranSegmentor(const char* config_file) throw (UniqueException);

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
        ~MoranSegmentor() throw ();

        bool
        is_valid_utf8_(const char* str, size_t str_len) const throw ();
      };
      typedef ReferenceCounting::ConstPtr<MoranSegmentor>
        MoranSegmentor_var;
    } //namespace Korean
  } //namespace Segmentor
} //namespace Language

#endif
