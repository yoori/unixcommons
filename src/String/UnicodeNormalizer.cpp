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



// @file String/UnicodeNormalizer.cpp


#include <cstddef>
#include <algorithm>

#include <String/UnicodeNormalizer.hpp>

/**
 * Hangul composition constants
 * see 3.12 Conjoining Jamo Behavior
 * wch in [AC00, D7A3]
 */
namespace
{
  const wchar_t S_BASE = 0xAC00;
  const wchar_t L_BASE = 0x1100;  // Leading consonant
                                  // (When not occurring in clusters,
                                  // the term leading consonant is
                                  // equivalent to syllable-initial
                                  // character)
  const wchar_t V_BASE = 0x1161;  // Vowel
  const wchar_t T_BASE = 0x11A7;  // Trailing consonant.
                                  // (When not occurring in clusters,
                                  // the term trailing consonant is
                                  // equivalent to syllable-final character
  const int L_COUNT = 19;
  const int V_COUNT = 21;
  const int T_COUNT = 28;
  const size_t N_COUNT = V_COUNT * T_COUNT; // 588
  const int S_COUNT = L_COUNT * N_COUNT;         // 11172

  template <typename Table>
  wchar_t*
  decompose(const Table* body, const uint16_t* indices,
    wchar_t wch, wchar_t start, wchar_t* output) throw ()
  {
    indices += wch - start;
    const Table* cur = &body[indices[0]];
    const Table* const END = &body[indices[1]];

    if (cur != END)
    {
      if (!*cur)
      {
        return 0;
      }
      do
      {
        *output++ = *cur++;
      }
      while (cur != END);
    }
    return output;
  }

  inline
  wchar_t*
  decompose(const uint64_t* body, wchar_t wch, wchar_t start,
    wchar_t* output) throw ()
  {
    start = wch - start;
    if (!(body[start >> 6] & (1ull << (start & 0x3F))))
    {
      return 0;
    }
    *output++ = wch;
    return output;
  }

  template <typename Table>
  wchar_t*
  decompose(const Table* body, wchar_t wch, wchar_t start, wchar_t* output)
    throw ()
  {
    wch = body[wch - start];
    if (!wch)
    {
      return 0;
    }
    *output++ = wch;
    return output;
  }
}

namespace String
{
  namespace Normalizer
  {
    wchar_t*
    hangul_decompose(wchar_t wch, wchar_t* output) throw ()
    {
      if (wch < S_BASE || wch >= S_BASE + S_COUNT)
      {
        *output++ = wch;
        return output;
      }
      int s_index = wch - S_BASE;
      *output++ = static_cast<wchar_t>(L_BASE + s_index / N_COUNT);
      *output++ = static_cast<wchar_t>(
        V_BASE + (s_index % N_COUNT) / T_COUNT);
      int t = T_BASE + s_index % T_COUNT;
      if (t != T_BASE)
      {
        *output++ = static_cast<wchar_t>(t);
      }
      return output;
    }

    wchar_t*
    decompose_2008(wchar_t wch, wchar_t* output) throw ()
    {
      if (wch <= 0x33FF)
      {
        return decompose(IDNA2008Decomposition::MAPPING_0000_33FF,
          IDNA2008Decomposition::MAPPING_INDEX_0000_33FF, wch, 0x0000,
          output);
      }

      bool prohibited = false;

      if (wch < 0xE01F0)
      {
        // 3400-E01EF
        if (wch < 0x11740)
        {
          // 3400-1173F
          if (wch < 0xAC00)
          {
            // 3400-ABFF
            if (wch < 0xA48D)
            {
              // 3400-A48C
              if (wch < 0x4DC0)
              {
                // 3400-4DBF
                prohibited = wch >= 0x4DB6;
              }
              else
              {
                // 4DC0-A48C
                prohibited = wch >= 0x9FD6 && wch <= 0x9FFF;
              }
            }
            else
            {
              // A48D-ABFF
              if (wch < 0xA4D0)
              {
                // A48D-A4CF
                prohibited = wch < 0xA490 || wch > 0xA4C6;
              }
              else
              {
                // A4D0-ABFF
                if (wch < 0xA640)
                {
                  // A4D0-A63F
                  prohibited = wch >= 0xA62C;
                }
                else
                {
                  // A640-ABFF
                  return decompose(IDNA2008Decomposition::MAPPING_A640_ABFF,
                    wch, 0xA640, output);
                }
              }
            }
          }
          else
          {
            // AC00-1173F
            if (wch < 0xD7FC)
            {
              // AC00-D7FB
              if (wch < 0xD7B0)
              {
                // AC00-D7AF
                if (wch < 0xD7A4)
                {
                  // AC00-D7A3
                  return hangul_decompose(wch, output);
                }
                else
                {
                  // D7A4-D7AF
                  prohibited = true;
                }
              }
              else
              {
                // D7B0-D7FB
                prohibited = wch >= 0xD7C7 && wch <= 0xD7CA;
              }
            }
            else
            {
              // D7FC-1173F
              if (wch < 0x10400)
              {
                // D7FC-103FF
                if (wch < 0xF900)
                {
                  // D7FC-F8FF
                  prohibited = true;
                }
                else
                {
                  // F900-103FF
                  if (wch < 0xFFEF)
                  {
                    // F900-FFEE
                    return decompose(
                      IDNA2008Decomposition::MAPPING_F900_FFEE,
                      IDNA2008Decomposition::MAPPING_INDEX_F900_FFEE, wch,
                      0xF900, output);
                  }
                  else
                  {
                    // FFEF-103FF
                    return decompose(
                      IDNA2008Decomposition::MAPPING_FFEF_103FF,
                      wch, 0xFFEF, output);
                  }
                }
              }
              else
              {
                // 10400-1173F
                if (wch < 0x10C80)
                {
                  // 10400-10C7F
                  if (wch < 0x10428)
                  {
                    // 10400-10427
                    return decompose(
                      IDNA2008Decomposition::MAPPING_10400_10427,
                      wch, 0x10400, output);
                  }
                  else
                  {
                    // 10428-10C7F
                    return decompose(
                      IDNA2008Decomposition::MAPPING_10428_10C7F,
                      wch, 0x10428, output);
                  }
                }
                else
                {
                  // 10C80-1173F
                  if (wch < 0x10CB3)
                  {
                    // 10C80-10CB2
                    return output;
                  }
                  else
                  {
                    // 10CB3-1173F
                    return decompose(
                      IDNA2008Decomposition::MAPPING_10CB3_1173F,
                      wch, 0x10CB3, output);
                  }
                }
              }
            }
          }
        }
        else
        {
          // 11740-E01EF
          if (wch < 0x1D800)
          {
            // 11740-1D7FF
            if (wch < 0x13000)
            {
              // 11740-12FFF
              if (wch < 0x1239A)
              {
                // 11740-12399
                if (wch < 0x11900)
                {
                  // 11740-118FF
                  if (wch < 0x118C0)
                  {
                    // 11740-118BF
                    if (wch < 0x118A0)
                    {
                      // 11740-1189F
                      prohibited = true;
                    }
                    else
                    {
                      // 118A0-118BF
                      return decompose(
                        IDNA2008Decomposition::MAPPING_118A0_118BF,
                        wch, 0x118A0, output);
                    }
                  }
                  else
                  {
                    // 118C0-118FF
                    prohibited = wch >= 0x118F3 && wch <= 0x118FE;
                  }
                }
                else
                {
                  // 11900-12399
                  if (wch < 0x11AF9)
                  {
                    // 11900-11AF8
                    prohibited = wch < 0x11AC0;
                  }
                  else
                  {
                    // 11AF9-12399
                    prohibited = wch < 0x12000;
                  }
                }
              }
              else
              {
                // 1239A-12FFF
                if (wch < 0x12544)
                {
                  // 1239A-12543
                  return decompose(
                    IDNA2008Decomposition::MAPPING_1239A_12543,
                    wch, 0x1239A, output);
                }
                else
                {
                  // 12544-12FFF
                  prohibited = true;
                }
              }
            }
            else
            {
              // 13000-1D7FF
              if (wch < 0x16A39)
              {
                // 13000-16A38
                if (wch < 0x14647)
                {
                  // 13000-14646
                  prohibited = wch >= 0x1342F && wch <= 0x143FF;
                }
                else
                {
                  // 14647-16A38
                  prohibited = wch < 0x16800;
                }
              }
              else
              {
                // 16A39-1D7FF
                if (wch < 0x1D000)
                {
                  // 16A39-1CFFF
                  if (wch < 0x1B000)
                  {
                    // 16A39-1AFFF
                    if (wch < 0x16FA0)
                    {
                      // 16A39-16F9F
                      if (wch < 0x16F00)
                      {
                        // 16A39-16EFF
                        if (wch < 0x16B90)
                        {
                          // 16A39-16B8F
                          return decompose(
                            IDNA2008Decomposition::MAPPING_16A39_16B8F,
                            wch, 0x16A39, output);
                        }
                        else
                        {
                          // 16B90-16EFF
                          prohibited = true;
                        }
                      }
                      else
                      {
                        // 16F00-16F9F
                        return decompose(
                          IDNA2008Decomposition::MAPPING_16F00_16F9F,
                          wch, 0x16F00, output);
                      }
                    }
                    else
                    {
                      // 16FA0-1AFFF
                      prohibited = true;
                    }
                  }
                  else
                  {
                    // 1B000-1CFFF
                    if (wch < 0x1BC00)
                    {
                      // 1B000-1BBFF
                      prohibited = wch >= 0x1B002;
                    }
                    else
                    {
                      // 1BC00-1CFFF
                      if (wch < 0x1BCA0)
                      {
                        // 1BC00-1BC9F
                        return decompose(
                          IDNA2008Decomposition::MAPPING_1BC00_1BC9F,
                          wch, 0x1BC00, output);
                      }
                      else
                      {
                        // 1BCA0-1CFFF
                        if (wch < 0x1BCA4)
                        {
                          // 1BCA0-1BCA3
                          return output;
                        }
                        else
                        {
                          // 1BCA4-1CFFF
                          prohibited = true;
                        }
                      }
                    }
                  }
                }
                else
                {
                  // 1D000-1D7FF
                  return decompose(
                    IDNA2008Decomposition::MAPPING_1D000_1D7FF,
                    IDNA2008Decomposition::MAPPING_INDEX_1D000_1D7FF, wch,
                    0x1D000, output);
                }
              }
            }
          }
          else
          {
            // 1D800-E01EF
            if (wch < 0x2A700)
            {
              // 1D800-2A6FF
              if (wch < 0x1F9C1)
              {
                // 1D800-1F9C0
                if (wch < 0x1EE00)
                {
                  // 1D800-1EDFF
                  if (wch < 0x1E8C5)
                  {
                    // 1D800-1E8C4
                    if (wch < 0x1DAB0)
                    {
                      // 1D800-1DAAF
                      if (wch < 0x1DA8C)
                      {
                        // 1D800-1DA8B
                      }
                      else
                      {
                        // 1DA8C-1DAAF
                        return decompose(
                          IDNA2008Decomposition::MAPPING_1DA8C_1DAAF,
                          wch, 0x1DA8C, output);
                      }
                    }
                    else
                    {
                      // 1DAB0-1E8C4
                      prohibited = wch < 0x1E800;
                    }
                  }
                  else
                  {
                    // 1E8C5-1EDFF
                    prohibited = wch < 0x1E8C7 || wch >= 0x1E8D7;
                  }
                }
                else
                {
                  // 1EE00-1F9C0
                  return decompose(
                    IDNA2008Decomposition::MAPPING_1EE00_1F9C0,
                    IDNA2008Decomposition::MAPPING_INDEX_1EE00_1F9C0, wch,
                    0x1EE00, output);
                }
              }
              else
              {
                // 1F9C1-2A6FF
                prohibited = wch < 0x20000 || wch > 0x2A6D6;
              }
            }
            else
            {
              // 2A700-E01EF
              if (wch < 0x2B81E)
              {
                // 2A700-2B81D
                prohibited = wch >= 0x2B735 && wch <= 0x2B73F;
              }
              else
              {
                // 2B81E-E01EF
                if (wch < 0x2FA1E)
                {
                  // 2B81E-2FA1D
                  if (wch < 0x2F800)
                  {
                    // 2B81E-2F7FF
                    prohibited = wch < 0x2B820 || wch > 0x2CEA1;
                  }
                  else
                  {
                    // 2F800-2FA1D
                    return decompose(
                      IDNA2008Decomposition::MAPPING_2F800_2FA1D,
                      wch, 0x2F800, output);
                  }
                }
                else
                {
                  // 2FA1E-E01EF
                  if (wch < 0xE0100)
                  {
                    // 2FA1E-E00FF
                    prohibited = true;
                  }
                  else
                  {
                    // E0100-E01EF
                    return output;
                  }
                }
              }
            }
          }
        }
      }
      else
      {
        prohibited = true;
      }

      if (prohibited)
      {
        return 0;
      }

      *output++ = wch;
      return output;
    }

    wchar_t*
    decompose_2003(wchar_t wch, wchar_t* output) throw ()
    {
      wchar_t* res = decompose_2008(wch, output);
      if (res)
      {
        return res;
      }

      if (wch == 0x1806)
      {
        return output;
      }

      using Normalizer::IDNA2003Decomposition::MAP;
      const uint32_t* const END = MAP + sizeof(MAP) / sizeof(*MAP);
      const uint32_t* const ELEM = std::lower_bound(MAP, END, wch,
          std::less<uint32_t>());
      if (ELEM != END && static_cast<wchar_t>(*ELEM) == wch)
      {
        *output++ = wch;
        return output;
      }

      return 0;
    }


    int
    get_non_zero_combining_class(uint32_t wch) throw ()
    {
      return (*Combining::COMBINING_CLASS_INDEX[wch >> 8])[wch & 0xFF];
    }

    int
    get_combining_class(uint32_t wch) throw ()
    {
      return wch > 0x1D244 ? 0 : get_non_zero_combining_class(wch);
    }

    bool
    is_starter(uint32_t wch) throw ()
    {
      return get_combining_class(wch) == 0;
    }

    bool
    canonical_order(wchar_t a, wchar_t b) throw ()
    {
      return get_non_zero_combining_class(a) <
        get_non_zero_combining_class(b);
    }

    wchar_t*
    normalize(const wchar_t* input, const wchar_t* last, wchar_t* output,
      wchar_t* (*decomposer)(wchar_t, wchar_t*) throw ()) throw ()
    {
      // output will point to first raw, unwritten byte!
      wchar_t* first_combiner = output;
      wchar_t* beyond_last = output;
      bool first_symbol_flag = true;

      for (; input != last; ++input)
      {
        beyond_last = decomposer(*input, output);
        if (!beyond_last)
        {
          return 0;
        }
        if (beyond_last != output)
        {
          if (is_starter(*output))  // found end-starter
          {
            first_symbol_flag = false;
            if (first_combiner < output)
            {
              std::stable_sort(first_combiner, output, canonical_order);
            }

            if (is_starter(beyond_last[-1]))
            {
              first_combiner = beyond_last;
            }
            else if (is_starter(beyond_last[-2]))
            {
              first_combiner = beyond_last - 1;
            }
            else
            {
              // +1 to exclude met starter from further sort
              first_combiner = output + 1;
            }
          }
          else
          {
            if (first_symbol_flag)
            {
              return 0;
            }
          }
          output = beyond_last;
        }
      }
      // check rest of string - we couldn't found end-starter
      // and result string can be without starters anywhere
      if (first_combiner != beyond_last)
      {
        std::stable_sort(first_combiner, beyond_last, canonical_order);
      }
      return output;
    }

    unsigned
    hash(wchar_t starter, wchar_t combiner) throw ()
    {
      register unsigned hash = starter;
      hash ^= (hash & 0x800) >> 6;
      hash ^= (hash & 0x200) >> 4;
      hash ^= combiner & 0x20;
      hash &= 0x5FF;
      hash ^= (combiner & 0x1F) << 9;
      return hash % 6584;
    }

    wchar_t*
    compose_string(wchar_t* first, const wchar_t* last) throw ()
    {
      wchar_t* write_pos = first;

      while (first != last)
      {
        write_pos = compose(first, last, write_pos);
      }
      return write_pos;
    }

    /**
     * @param first The pointer must point to second element of pair
     * - combiner.
     * @return Possible to continue composing if do not meet blocking char.
     * false if don't have compose or blocking char occurred
     */
    bool
    pair_compose(wchar_t& maybe_starter, int& last_class,
      wchar_t*& first, wchar_t*& output) throw ()
    {
      wchar_t& maybe_combiner = *first;
      int combiner_class = get_combining_class(maybe_combiner);

      // see:  09C7 09BE --> 09CB decomposition
      //       Cl_0 Cl_0 --> Cl_0
      if (combiner_class > last_class || combiner_class == 0)
      {
        /**
         * Notice an important feature of Hangul composition: whenever the
         * source string is not in Normalization Form D or Normalization
         * Form KD, one cannot just detect character sequences of
         * the form <L, V> and <L, V, T>.
         * It is also necessary to catch the sequences of the form <LV, T>.
         * To guarantee uniqueness, these sequences must also be composed.
         * This is illustrated in step 2.
         */

        // 1. check to see if two current characters are L and V
        if (maybe_starter >= L_BASE && maybe_starter < L_BASE + L_COUNT &&
          maybe_combiner >= V_BASE && maybe_combiner < V_BASE + V_COUNT)
        {
          // make syllable of form LV
          maybe_starter = S_BASE + ((maybe_starter - L_BASE) * V_COUNT +
            (maybe_combiner - V_BASE)) * T_COUNT;
          first++;
          return true;
        }

        // 2. check to see if two current characters are LV and T
        if (maybe_starter >= S_BASE && maybe_starter < S_BASE + S_COUNT &&
          ((maybe_starter - S_BASE) % T_COUNT) == 0 &&
          maybe_combiner >= T_BASE && maybe_combiner < T_BASE + T_COUNT)
        {
          // make syllable of form LVT
          maybe_starter += maybe_combiner - T_BASE;
          first++;
          return true;
        }

        // usual composite check
        const Composition::CompositeHashRecord& record =
          Composition::COMPOSITE_HASH[hash(maybe_starter, maybe_combiner)];
        if (static_cast<wchar_t>(record.starter) == maybe_starter &&
          static_cast<wchar_t>(record.combiner) == maybe_combiner)
        {
          maybe_starter = record.value;
          first++;
          return true;
        }
      }
      // else blocking char or starter, if blocking char - further we
      // can meet non blocking char, and should continue composing,
      // if starter - we should go to work with them.
      if (combiner_class == 0)
      {
        return false;
      }
      // simple save, if didn't made primary composite
      last_class = combiner_class;
      *output = maybe_combiner;
      if (combiner_class)
      {
        // finish writing only for combiners, starter after composition.
        output++;
        first++;
      }

      return combiner_class;
    }

    /**
     * Write one char at least
     * @return if cannot be composed, needs forward to next starter!
     */
    wchar_t*
    compose(wchar_t*& first, const wchar_t* last, wchar_t* output) throw ()
    {
      *output = *first++;
      if (first == last)
      {
        return ++output;
      }
      wchar_t& maybe_starter = *output++;
      int last_class = get_combining_class(maybe_starter);

      while (first < last &&
        pair_compose(maybe_starter, last_class, first, output))
      {
      }

      return output;
    }
  }

  bool
  lower_and_normalize(const String::WSubString& input,
    std::wstring& output, bool idna2008) throw (eh::Exception)
  {
    output.resize(input.size() * 18);
    if (input.empty())
    {
      return false;
    }
    wchar_t* out = &output[0];
    out = Normalizer::normalize(input.begin(), input.end(), out,
      idna2008 ? Normalizer::decompose_2008 : Normalizer::decompose_2003);
    if (!out)
    {
      output.clear();
      return false;
    }
    out = Normalizer::compose_string(&output[0], out);
    output.resize(out - &output[0]);
    return true;
  }
}
