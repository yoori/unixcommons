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



#ifndef LANGUAGE_SEGMENTOR_COMMONS_SEGMENTORCOMMONS_HPP
#define LANGUAGE_SEGMENTOR_COMMONS_SEGMENTORCOMMONS_HPP

#include <string>

#include <String/SubString.hpp>

#include <Language/SegmentorCommons/SegmentorInterface.hpp>


namespace Language
{
  namespace Segmentor
  {
    inline
    void
    append(std::string& target, const String::SubString& str)
      throw (eh::Exception)
    {
      if (!str.empty())
      {
        if (!target.empty() && *target.rbegin() != ' ' &&
          *str.begin() != ' ')
        {
          target += ' ';
        }
        str.append_to(target);
      }
    }

    inline
    void
    append(WordsList& target, const String::SubString& str)
      throw (eh::Exception)
    {
      if (!str.empty())
      {
        target.push_back(str.str());
      }
    }
  }//namespace Segmentor
}//namespace Language

#endif
