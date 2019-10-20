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



#ifndef LANGUAGE_BLOGIC_NORMALIZETRIGGER_HPP
#define LANGUAGE_BLOGIC_NORMALIZETRIGGER_HPP

#include <vector>
#include <string>

#include <String/SubString.hpp>
#include <String/AsciiStringManip.hpp>

#include <Language/SegmentorCommons/SegmentorInterface.hpp>


namespace Language
{
  namespace Trigger
  {
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);


    /**
     * Normalizes trigger according to specification (leaves letters
     * and digits only).
     * @param trigger trigger to normalize
     * @param result normalized trigger
     * @param segmentor optional segmentor
     */
    void
    normalize(const String::SubString& trigger, std::string& result,
      const Segmentor::SegmentorInterface* segmentor = 0)
      throw (eh::Exception, Exception);


    struct Trigger
    {
      struct Part
      {
        String::SubString part;
        bool quotes;
      };

      typedef std::vector<Part> Parts;

      std::string trigger;
      bool exact;
      Parts parts;
    };

    /**
     * Normalizes trigger according to specification (leaves letters
     * and digits only).
     * @param trigger trigger to normalize
     * @param result normalized trigger, containing split to hard ones.
     * @param segmentor optional segmentor
     */
    void
    normalize(const String::SubString& trigger, Trigger& result,
      const Segmentor::SegmentorInterface* segmentor = 0)
      throw (eh::Exception, Exception);


    /**
     * Normalizes phrase according to trigger specification (leaves letters
     * and digits only).
     * @param phrase phrase to normalize
     * @param result normalized phrase
     * @param segmentor optional segmentor
     */
    void
    normalize_phrase(const String::SubString& phrase, std::string& result,
      const Language::Segmentor::SegmentorInterface* segmentor = 0)
      throw (eh::Exception, Exception);
  }
}

#endif
