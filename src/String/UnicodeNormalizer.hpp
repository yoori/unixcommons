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



// @file String/UnicodeNormalizer.hpp


#ifndef UNICODE_NORMALIZER_HPP
#define UNICODE_NORMALIZER_HPP

#include <cstdint>

#include <String/SubString.hpp>


namespace String
{
  /**
   * Performs NFC/NFKC normalization and checks for prohibited IDNA symbols
   * @param input IDNA host name
   * @param output Normalized IDNA host name
   * @param idna2008 if IDNA2008 rules are applied (IDNA2003 otherwise)
   * @return if transformation was successful or not
   */
  bool
  lower_and_normalize(const String::WSubString& input,
    std::wstring& output, bool idna2008) throw (eh::Exception);

  namespace Normalizer
  {
    /**
     * Perform decomposition and case folding according to RFC-3454
     * @param wch The wide character to be decomposed
     * @param output The pointer to memory when result will be write
     * @return Modified value of output point beyond written data, in
     * other words new value for output point to empty part of buffer
     * or NULL if wch is prohibited
     */
    wchar_t*
    decompose_2003(wchar_t wch, wchar_t* output) throw ();

    /**
     * Perform decomposition and case folding according to RFC-5894
     * @param wch The wide character to be decomposed
     * @param output The pointer to memory when result will be write
     * @return Modified value of output point beyond written data, in
     * other words new value for output point to empty part of buffer
     * or NULL if wch is prohibited
     */
    wchar_t*
    decompose_2008(wchar_t wch, wchar_t* output) throw ();

    /**
     * At first run starter_pos must be equal to output. While we are
     * processing string it becomes less. And output pointer will lag behind
     * the first.
     * @param first The pointer to input data
     * @param last The pointer behind the input data
     * @param output The pointer to available position in input data to write
     * composition result
     * @return The output pointer that point to position in input string
     * available for result writing by next call compose.
     * 2. Return the first pointer - point to the next portion of input data
     * that we can try to combine
     */
    wchar_t*
    compose(wchar_t*& first, const wchar_t* last, wchar_t* output)
      throw ();

    /**
     * @param first The pointer to begin of input data
     * @param last The pointer beyond the input data
     * @return The new last position, pointer to beyond of data.
     */
    wchar_t*
    compose_string(wchar_t* first, const wchar_t* last) throw ();

    /**
     * Calculate Canonical Combining Class of code unit,
     * according Unicode Database.
     * @param wch The character to get combining class
     * @return The canonical class correspond wch character, according
     * to Unicode database.
     */
    int
    get_combining_class(unsigned wchar_t wch) throw ();

    /**
     * Decompose and canonical order input data.
     * @param input The pointer to begin of input string
     * @param last The pointer to end of input string
     * @param output The pointer to store result of decomposition and sorting,
     * memory should be enough to do this operations, overflow isn't check.
     * @return The pointer beyond the written data, new value for output
     * return 0 to indicate error state - first symbol of decomposed string
     * is Combiner. Decomposition should start from Starter
     */
    wchar_t*
    normalize(const wchar_t* input, const wchar_t* last, wchar_t* output)
      throw ();

    namespace Combining
    {
      typedef const uint8_t CombiningClassBlock[256];
      extern const CombiningClassBlock* const COMBINING_CLASS_INDEX[];
    }

    namespace Composition
    {
      struct CompositeHashRecord
      {
        const uint32_t value;
        const uint32_t starter;
        const uint32_t combiner;
      };
      extern const CompositeHashRecord COMPOSITE_HASH[6584];
    }

    namespace IDNA2003Decomposition
    {
      extern const uint32_t MAP[357];
    }

    namespace IDNA2008Decomposition
    {
      extern const uint16_t MAPPING_0000_33FF[];
      extern const uint16_t MAPPING_INDEX_0000_33FF[];
      extern const uint16_t MAPPING_A640_ABFF[];
      extern const uint32_t MAPPING_F900_FFEE[];
      extern const uint16_t MAPPING_INDEX_F900_FFEE[];
      extern const uint64_t MAPPING_FFEF_103FF[];
      extern const uint32_t MAPPING_10400_10427[];
      extern const uint64_t MAPPING_10428_10C7F[];
      extern const uint64_t MAPPING_10CB3_1173F[];
      extern const uint32_t MAPPING_118A0_118BF[];
      extern const uint64_t MAPPING_1239A_12543[];
      extern const uint64_t MAPPING_16A39_16B8F[];
      extern const uint64_t MAPPING_16F00_16F9F[];
      extern const uint64_t MAPPING_1BC00_1BC9F[];
      extern const uint32_t MAPPING_1D000_1D7FF[];
      extern const uint16_t MAPPING_INDEX_1D000_1D7FF[];
      extern const uint64_t MAPPING_1DA8C_1DAAF[];
      extern const uint32_t MAPPING_1EE00_1F9C0[];
      extern const uint16_t MAPPING_INDEX_1EE00_1F9C0[];
      extern const uint32_t MAPPING_2F800_2FA1D[];
    }
  }
}

#endif // UNICODE_NORMALIZER_HPP
