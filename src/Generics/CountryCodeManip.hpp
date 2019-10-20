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



#ifndef GENERICS_COUNTRYCODEMANIP_HPP
#define GENERICS_COUNTRYCODEMANIP_HPP

#include <cstdint>

#include <String/SubString.hpp>

#include <Generics/Uncopyable.hpp>
#include <Generics/GnuHashTable.hpp>


namespace Generics
{
  class CountryCodeMap : private Uncopyable
  {
  public:
    /**
     * Constructor loads strings with data into hash table.
     * Should not be used without needs.
     */
    CountryCodeMap() throw (eh::Exception);

    /**
     * Check
     * @param code SubString point to country code string
     * @return true if str comply ISO3166 standard or
     * ISO3166 extensions or ISO3166 triple country codes.
     * return false if don't comply or str is nul.
     */
    bool
    is_country_code(const String::SubString& code) const throw ();

  private:
    /**
     * Interprets the
     * @param str as the number. Used first 3 chars from str only.
     * Different architectures give different numbers. (Byte order).
     * @return number that encoded in str.
     */
    static
    uint32_t
    get_country_code_(const String::SubString& str) throw ();

    typedef Generics::GnuHashSet<NumericHashAdapter<uint32_t> >
      CountryMap_;
    CountryMap_ country_map_;
  };
}

#endif
