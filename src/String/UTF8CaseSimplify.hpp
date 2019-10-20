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



// @file String/UTF8CaseSimplify.hpp
#ifndef STRING_UTF8_CASE_SIMPLIFY_HPP
#define STRING_UTF8_CASE_SIMPLIFY_HPP

#include <String/UTF8Case.hpp>
#include <String/UTF8Tables.hpp>
#include <String/UTF8Handler.hpp>


namespace String
{
  namespace ToSimplify
  {
    namespace Helper
    {
      inline
      unsigned
      status(const Plane2Bits TABLE[], const unsigned char SECOND,
        const unsigned char BASE, const unsigned char THIRD)
        throw (eh::Exception)
      {
        const unsigned char OFFSET = THIRD & 0x3F;
        return (TABLE[SECOND - BASE][OFFSET >> 5] >>
          ((OFFSET & 0x1F) << 1)) & 3;
      }

      template <typename OutputIterator>
      void
      copy(const SubString& STR, OutputIterator& dest) throw (eh::Exception)
      {
        for (SubString::SizeType i = 0; i != STR.size(); ++i)
        {
          *dest++ = STR[i];
        }
      }

      template <typename OutputIterator>
      void
      replace(const char MODIFIED, const SubString REPL[],
        OutputIterator& dest) throw (eh::Exception)
      {
        if (static_cast<unsigned char>(MODIFIED) < 0x20)
        {
          if (!MODIFIED)
          {
            return;
          }
          copy(REPL[static_cast<unsigned char>(MODIFIED)], dest);
          return;
        }

        *dest++ = MODIFIED;
      }

      template <typename OutputIterator>
      bool
      replace(const CodeUnit2Bytes& MODIFIED,
        const SubString REPL[], OutputIterator& dest) throw (eh::Exception)
      {
        if (!MODIFIED[0])
        {
          switch (MODIFIED[1])
          {
          case '\0':
            break;
          case '\xFF':
            return false;
          default:
            copy(REPL[static_cast<unsigned char>(MODIFIED[1])], dest);
            break;
          }
          return true;
        }

        *dest++ = MODIFIED[0];
        if (MODIFIED[1])
        {
          *dest++ = MODIFIED[1];
        }

        return true;
      }

      template <typename OutputIterator>
      void
      replace(const CodeUnit4Bytes& MODIFIED,
        const SubString REPL[], OutputIterator& dest) throw (eh::Exception)
      {
        if (!MODIFIED[0])
        {
          if (MODIFIED[1])
          {
            copy(REPL[static_cast<unsigned char>(MODIFIED[1])], dest);
          }
          return;
        }

        *dest++ = MODIFIED[0];
        if (MODIFIED[1])
        {
          *dest++ = MODIFIED[1];
          if (MODIFIED[2])
          {
            *dest++ = MODIFIED[2];
            if (MODIFIED[3])
            {
              *dest++ = MODIFIED[3];
            }
          }
        }
      }

      void
      out_hangul(unsigned ch, char*& dest) throw ()
      {
        unsigned char tmp = (ch >> 12) | 0xE0;
        *dest++ = reinterpret_cast<const char&>(tmp);
        tmp = ((ch >> 6) & 0x3F) | 0x80;
        *dest++ = reinterpret_cast<const char&>(tmp);
        tmp = (ch & 0x3F) | 0x80;
        *dest++ = reinterpret_cast<const char&>(tmp);
      }

      void
      decompose_hangul(const unsigned char FIRST,
        const unsigned char SECOND,
        const unsigned char THIRD, char*& dest) throw ()
      {
        unsigned ch = (((static_cast<unsigned>(FIRST) & 0x0F) << 12) |
          ((static_cast<unsigned>(SECOND) & 0x3F) << 6) |
          (static_cast<unsigned>(THIRD) & 0x3F)) - 0xAC00;
        unsigned l = ch / 588;
        unsigned v = (ch % 588) / 28;
        unsigned t = ch % 28;
        out_hangul(0x1100 + l, dest);
        out_hangul(0x1161 + v, dest);
        if (t)
        {
          out_hangul(0x11A7 + t, dest);
        }
      }
    }
  }
}

bool
String::ToSimplify::to_simplify(String::Helper::Iterator it, char*& dest,
  size_t& counter) throw ()
{
  counter = 0;

  while (!it.exhausted())
  {
    const unsigned char FIRST = it.forward();
    switch (UTF8Handler::get_octet_count(FIRST))
    {
    case 1:
      *dest++ = TABLE_1[FIRST];
      continue;

    case 2:
      {
        if (it.exhausted())
        {
          return true;
        }
        const unsigned char SECOND = it.forward();
        if ((SECOND & 0xC0) != 0x80)
        {
          *dest++ = ' ';
          it.backward(1);
          continue;
        }
        Helper::replace(TABLE_2[FIRST - 0xC2][SECOND & 0x3F],
          TABLE_2_, dest);
        continue;
      }

    case 3:
      {
        if (it.exhausted())
        {
          return true;
        }
        const unsigned char SECOND = it.forward();
        if (it.exhausted())
        {
          return true;
        }
        const unsigned char THIRD = it.forward();

        if ((THIRD & 0xC0) != 0x80)
        {
          *dest++ = ' ';
          it.backward(2);
          continue;
        }

        if (FIRST == 0xE0)
        {
          if ((SECOND & 0xE0) != 0xA0)
          {
            *dest++ = ' ';
            it.backward(2);
            continue;
          }

          switch (Helper::status(TABLE_3_E0, SECOND, 0xA0, THIRD))
          {
          case 0:
            continue;
          case 1:
            break;
          case 2:
            *dest++ = ' ';
            continue;
          case 3:
            switch (SECOND)
            {
            case 0xA4:
              {
                *dest++ = '\xE0';
                *dest++ = '\xA4';
                *dest++ = reinterpret_cast<const char&>(THIRD) - 1;
                break;
              }
            case 0xA5:
              {
                *dest++ = '\xE0';
                *dest++ = '\xA4';
                *dest++ = TABLE_3_E0_[THIRD - 0x98];
                break;
              }
            case 0xA7:
              {
                *dest++ = '\xE0';
                *dest++ = '\xA6';
                *dest++ = THIRD == 0x9F ? '\xAF' :
                  reinterpret_cast<const char&>(THIRD) + 5;
                break;
              }
            case 0xA8:
              {
                *dest++ = '\xE0';
                *dest++ = '\xA8';
                *dest++ = THIRD == 0xB3 ? '\xB2' : '\xB8';
                break;
              }
            case 0xA9:
              {
                *dest++ = '\xE0';
                *dest++ = '\xA8';
                *dest++ = THIRD == 0x9B ? '\x9C' :
                  THIRD == 0x9E ? '\xAB' :
                    reinterpret_cast<const char&>(THIRD) - 3;
                break;
              }
            case 0xAD:
              {
                *dest++ = '\xE0';
                *dest++ = '\xAC';
                *dest++ = reinterpret_cast<const char&>(THIRD) + 5;
                break;
              }
            case 0xAE:
              {
                *dest++ = '\xE0';
                *dest++ = '\xAE';
                *dest++ = '\x92';
                *dest++ = ' ';
                break;
              }
            case 0xB8:
              {
                *dest++ = ' ';
                *dest++ = '\xE0';
                *dest++ = '\xB8';
                *dest++ = '\xB2';
                break;
              }
            case 0xBA:
              {
                *dest++ = ' ';
                *dest++ = '\xE0';
                *dest++ = '\xBA';
                *dest++ = '\xB2';
                break;
              }
            case 0xBB:
              {
                *dest++ = '\xE0';
                *dest++ = '\xBA';
                *dest++ = '\xAB';
                *dest++ = '\xE0';
                *dest++ = '\xBA';
                *dest++ = THIRD == 0x9C ? '\x99' : '\xA1';
                break;
              }
            case 0xBD:
              {
                *dest++ = '\xE0';
                *dest++ = '\xBD';
                switch (THIRD)
                {
                case 0x83:
                  {
                   *dest++ = '\x82';
                   break;
                  }
                case 0x8D:
                  {
                   *dest++ = '\x8C';
                   break;
                  }
                case 0x92:
                  {
                   *dest++ = '\x91';
                   break;
                  }
                case 0x97:
                  {
                   *dest++ = '\x96';
                   break;
                  }
                case 0x9C:
                  {
                   *dest++ = '\x9B';
                   break;
                  }
                case 0xA9:
                  {
                   *dest++ = '\x80';
                   break;
                  }
                }
                *dest++ = ' ';
                break;
              }
            }
            continue;
          }
        }
        else
        {
          if ((SECOND & (FIRST == 0xED ? 0xE0 : 0xC0)) != 0x80)
          {
            *dest++ = ' ';
            it.backward(2);
            continue;
          }

          switch (FIRST)
          {
          case 0xE1:
            {
              if (SECOND < 0xB4)
              {
                switch (Helper::status(TABLE_3_E1_1, SECOND, 0x80, THIRD))
                {
                case 0:
                  continue;
                case 1:
                  break;
                case 2:
                  *dest++ = ' ';
                  continue;
                case 3:
                  switch (SECOND)
                  {
                  case 0x80:
                    {
                      *dest++ = '\xE1';
                      *dest++ = '\x80';
                      *dest++ = '\xA5';
                      *dest++ = ' ';
                      break;
                    }
                  case 0x82:
                    {
                      *dest++ = '\xE2';
                      *dest++ = '\xB4';
                      *dest++ = reinterpret_cast<const char&>(THIRD) - 0x20;
                      break;
                    }
                  case 0x83:
                    {
                      if (THIRD == 0xBC)
                      {
                        *dest++ = '\xE1';
                        *dest++ = '\x83';
                        *dest++ = '\x9C';
                      }
                      else
                      {
                        *dest++ = '\xE2';
                        *dest++ = '\xB4';
                        *dest++ = reinterpret_cast<const char&>(THIRD) +
                          0x20;
                      }
                      break;
                    }
                  case 0x8F:
                    {
                      *dest++ = reinterpret_cast<const char&>(FIRST);
                      *dest++ = reinterpret_cast<const char&>(SECOND);
                      *dest++ = reinterpret_cast<const char&>(THIRD) - 0x8;
                      continue;
                    }
                  case 0x9A:
                  case 0xA0:
                    {
                      *dest++ = ' ';
                      break;
                    }
                  case 0xAC:
                    {
                      *dest++ = '\xE1';
                      *dest++ = '\xAC';
                      *dest++ = reinterpret_cast<const char&>(THIRD) - 1;
                      *dest++ = ' ';
                      break;
                    }
                  }
                  continue;
                }
              }
              else
              {
                if (Helper::replace(
                  TABLE_3_E1_2[SECOND - 0xB4][THIRD & 0x3F],
                  TABLE_3_E1_2_, dest))
                {
                  continue;
                }
              }
              break;
            }

          case 0xE2:
            {
              if (SECOND < 0x91)
              {
                if (SECOND < 0x85)
                {
                  Helper::replace(TABLE_3_E2_1[SECOND - 0x80][THIRD & 0x3F],
                    TABLE_3_E2_1_, dest);
                  continue;
                }
                else
                {
                  if (SECOND < 0x87)
                  {
                    Helper::replace(
                      TABLE_3_E2_2[SECOND - 0x85][THIRD & 0x3F], 0, dest);
                  }
                  else
                  {
                    *dest++ = ' ';
                  }
                  continue;
                }
              }
              else
              {
                if (SECOND < 0xB0)
                {
                  if (SECOND < 0x94)
                  {
                    Helper::replace(
                      TABLE_3_E2_3[SECOND - 0x91][THIRD & 0x3F],
                      0, dest);
                  }
                  else
                  {
                    if ((SECOND == 0x9D && THIRD >= 0xB6) ||
                      (SECOND == 0x9E && THIRD <= 0x93))
                    {
                      break;
                    }
                    *dest++ = ' ';
                  }
                }
                else
                {
                  Helper::replace(
                    TABLE_3_E2_4[SECOND - 0xB0][THIRD & 0x3F],
                    0, dest);
                }
                continue;
              }
              break;
            }

          case 0xE3:
            {
              if (SECOND < 0x90)
              {
                Helper::replace(TABLE_3_E3[SECOND - 0x80][THIRD & 0x3F],
                  TABLE_3_E3_, dest);
                continue;
              }
              break;
            }

          case 0xE4:
            {
              if ((SECOND == 0xB6 && THIRD >= 0xB6) || SECOND == 0xB7)
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0xE9:
            {
              if (SECOND == 0xBF && THIRD >= 0x96)
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0xEA:
            {
              if (SECOND < 0xA0)
              {
                if (SECOND < 0x99)
                {
                  if (SECOND < 0x94)
                  {
                    if (SECOND == 0x92 ? THIRD >= 0x8D :
                      SECOND == 0x93 && (THIRD <= 0x8F || THIRD >= 0xBE))
                    {
                      *dest++ = ' ';
                      continue;
                    }
                  }
                  else
                  {
                    if (SECOND == 0x98)
                    {
                      if (THIRD >= 0x8D && (THIRD <= 0x8F || THIRD >= 0xAC))
                      {
                        *dest++ = ' ';
                        continue;
                      }
                    }
                  }
                }
                else
                {
                  Helper::replace(TABLE_3_EA_1[SECOND - 0x99][THIRD & 0x3F],
                    0, dest);
                  continue;
                }
              }
              else
              {
                if (SECOND < 0xB0)
                {
                  switch (Helper::status(TABLE_3_EA_2, SECOND, 0xA0, THIRD))
                  {
                  case 0:
                    continue;
                  case 1:
                    break;
                  case 2:
                    *dest++ = ' ';
                    continue;
                  case 3:
                    if (SECOND == 0xAD)
                    {
                      if (THIRD < 0xB0)
                      {
                        Helper::copy(TABLE_3_EA_2_[THIRD - 0x9C], dest);
                      }
                      else
                      {
                        *dest++ = '\xE1';
                        *dest++ = '\x8E';
                        *dest++ = reinterpret_cast<const char&>(THIRD) - 0x10;
                      }
                    }
                    else
                    {
                      *dest++ = '\xE1';
                      if (THIRD < 0x90)
                      {
                        *dest++ = '\x8E';
                        *dest++ = reinterpret_cast<const char&>(THIRD) + 0x30;
                      }
                      else
                      {
                        *dest++ = '\x8F';
                        *dest++ = reinterpret_cast<const char&>(THIRD) - 0x10;
                      }
                    }
                    continue;
                  }
                }
                else
                {
                  Helper::decompose_hangul(FIRST, SECOND, THIRD, dest);
                  continue;
                }
              }
              break;
            }

          case 0xEB:
          case 0xEC:
            {
              Helper::decompose_hangul(FIRST, SECOND, THIRD, dest);
              continue;
            }

          case 0xED:
            {
              if (SECOND < 0x9E)
              {
                Helper::decompose_hangul(FIRST, SECOND, THIRD, dest);
                continue;
              }
              else
              {
                if (SECOND == 0x9E)
                {
                  if (THIRD <= 0xA3)
                  {
                    Helper::decompose_hangul(FIRST, SECOND, THIRD, dest);
                    continue;
                  }
                  else
                  {
                    if (THIRD <= 0xAF)
                    {
                      *dest++ = ' ';
                      continue;
                    }
                  }
                }
                else
                {
                  if (SECOND == 0x9F)
                  {
                    if ((THIRD >= 0x87 && THIRD <= 0x8A) || THIRD >= 0xBC)
                    {
                      *dest++ = ' ';
                      continue;
                    }
                  }
                  else
                  {
                    *dest++ = ' ';
                    continue;
                  }
                }
              }
              break;
            }

          case 0xEE:
            {
              *dest++ = ' ';
              continue;
            }

          case 0xEF:
            {
              if (SECOND < 0xA4)
              {
                *dest++ = ' ';
              }
              else
              {
                Helper::replace(TABLE_3_EF[SECOND - 0xA4][THIRD & 0x3F],
                  TABLE_3_EF_, dest);
              }
              continue;
            }
          }
        }
        *dest++ = reinterpret_cast<const char&>(FIRST);
        *dest++ = reinterpret_cast<const char&>(SECOND);
        *dest++ = reinterpret_cast<const char&>(THIRD);
        continue;
      }

    case 4:
      {
        if (it.exhausted())
        {
          return true;
        }
        const unsigned char SECOND = it.forward();
        if (it.exhausted())
        {
          return true;
        }
        const unsigned char THIRD = it.forward();
        if (it.exhausted())
        {
          return true;
        }
        const unsigned char FOURTH = it.forward();
        if ((FOURTH & 0xC0) != 0x80)
        {
          *dest++ = ' ';
          it.backward(3);
          continue;
        }

        if (FIRST == 0xF0)
        {
          if (SECOND < 0x90 || SECOND > 0xBF || (THIRD & 0xC0) != 0x80)
          {
            *dest++ = ' ';
            it.backward(3);
            continue;
          }
          switch (SECOND)
          {
          case 0x90:
            {
              switch (Helper::status(TABLE_4_F0_90_1, THIRD, 0x80, FOURTH))
              {
              case 0:
                continue;
              case 1:
                break;
              case 2:
                *dest++ = ' ';
                continue;
              case 3:
                {
                  if (THIRD == 0x90)
                  {
                    *dest++ = '\xF0';
                    *dest++ = '\x90';
                    const CodeUnit2Bytes& MODIFIED =
                      TABLE_4_F0_90_2[0][FOURTH & 0x3F];
                    *dest++ = MODIFIED[0];
                    *dest++ = MODIFIED[1];
                  }
                  else
                  {
                    *dest++ = reinterpret_cast<const char&>(FIRST);
                    *dest++ = reinterpret_cast<const char&>(SECOND);
                    *dest++ = '\xB3';
                    *dest++ = reinterpret_cast<const char&>(FOURTH);
                  }
                  continue;
                }
              }
              break;
            }

          case 0x91:
            {
              if (THIRD < 0x9D)
              {
                switch (Helper::status(TABLE_4_F0_91, THIRD, 0x80, FOURTH))
                {
                case 0:
                  continue;
                case 1:
                  break;
                case 2:
                  *dest++ = ' ';
                  continue;
                case 3:
                  {
                    *dest++ = '\xF0';
                    *dest++ = '\x91';
                    *dest++ = '\x82';
                    *dest++ = FOURTH == 0xAB ? '\xA5' :
                      reinterpret_cast<const char&>(FOURTH) - 1;
                    continue;
                  }
                }
              }
              else
              {
                if (THIRD == 0xA2)
                {
                  if (FOURTH >= 0xA0)
                  {
                    *dest++ = reinterpret_cast<const char&>(FIRST);
                    *dest++ = reinterpret_cast<const char&>(SECOND);
                    *dest++ = '\xA3';
                    *dest++ = reinterpret_cast<const char&>(FOURTH) - 0x20;
                    continue;
                  }
                }
                else
                {
                  if (THIRD == 0xA3)
                  {
                    if (FOURTH < 0xB3 || FOURTH > 0xBE)
                    {
                      break;
                    }
                  }
                  else
                  {
                    if (THIRD == 0xAB)
                    {
                      if (FOURTH < 0xB9)
                      {
                        break;
                      }
                    }
                  }
                }
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0x92:
            {
              if (THIRD < 0x90 ?
                THIRD > 0x8E || (THIRD == 0x8E && FOURTH >= 0x9A) :
                THIRD > 0x91 ?
                  THIRD > 0x95 || (THIRD == 0x95 && FOURTH >= 0x84 ) :
                  THIRD == 0x91 && FOURTH >= 0xAF)
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0x93:
            {
              if (THIRD > 0x90 || (THIRD == 0x90 && FOURTH >= 0xAF))
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0x94:
            {
              if (THIRD < 0x90 || THIRD > 0x99 ||
                (THIRD == 0x99 && FOURTH >= 0x87))
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0x96:
            {
              if (THIRD < 0xA8)
              {
                if (THIRD < 0xA0)
                {
                  *dest++ = ' ';
                  continue;
                }
              }
              else
              {
                switch (Helper::status(TABLE_4_F0_96, THIRD, 0xA8,
                  FOURTH))
                {
                case 0:
                  continue;
                case 1:
                  break;
                case 2:
                  *dest++ = ' ';
                  continue;
                case 3:
                  continue;
                }
              }
              break;
            }

          case 0x9B:
            {
              if (THIRD < 0x81)
              {
                if (FOURTH >= 0x82)
                {
                  *dest++ = ' ';
                  continue;
                }
              }
              else
              {
                if (THIRD < 0xB0 || THIRD > 0xB2)
                {
                  *dest++ = ' ';
                  continue;
                }
                else
                {
                  if (THIRD == 0xB0)
                  {
                    break;
                  }
                  else
                  {
                    switch (Helper::status(TABLE_4_F0_9B, THIRD, 0xB1,
                      FOURTH))
                    {
                    case 0:
                      continue;
                    case 1:
                      break;
                    case 2:
                      *dest++ = ' ';
                      continue;
                    case 3:
                      continue;
                    }
                  }
                }
              }
              break;
            }

          case 0x9D:
            {
              if (THIRD < 0x90)
              {
                switch (Helper::status(TABLE_4_F0_9D_1, THIRD, 0x80,
                  FOURTH))
                {
                case 0:
                  continue;
                case 1:
                  break;
                case 2:
                  *dest++ = ' ';
                  continue;
                case 3:
                  continue;
                }
              }
              else
              {
                if (THIRD < 0xA0)
                {
                  Helper::replace(
                    TABLE_4_F0_9D_2[THIRD - 0x90][FOURTH & 0x3F], 0, dest);
                  continue;
                }
                else
                {
                  *dest++ = ' ';
                  continue;
                }
              }
              break;
            }

          case 0x9E:
            {
              if (THIRD <= 0xA3)
              {
                if (THIRD == 0xA3)
                {
                  switch (Helper::status(TABLE_4_F0_9E_1, THIRD, 0xA3,
                    FOURTH))
                  {
                  case 0:
                    continue;
                  case 1:
                    break;
                  case 2:
                    *dest++ = ' ';
                    continue;
                  case 3:
                    continue;
                  }
                }
                else
                {
                  if (THIRD < 0xA0)
                  {
                    *dest++ = ' ';
                    continue;
                  }
                }
              }
              else
              {
                if (THIRD >= 0xB8 && THIRD < 0xBB)
                {
                  Helper::replace(TABLE_4_F0_9E_2[THIRD - 0xB8][FOURTH & 0x3F],
                    0, dest);
                  continue;
                }
                else
                {
                  *dest++ = ' ';
                  continue;
                }
              }
              break;
            }

          case 0x9F:
            {
              if (THIRD >= 0x84 && THIRD < 0x8A)
              {
                Helper::replace(TABLE_4_F0_9F[THIRD - 0x84][FOURTH & 0x3F],
                  TABLE_4_F0_9F_, dest);
              }
              else
              {
                *dest++ = ' ';
              }
              continue;
            }

          case 0xAA:
            {
              if (THIRD == 0x9B && FOURTH >= 0x97)
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0xAB:
            {
              if ((THIRD == 0x9C && FOURTH >= 0xB5) ||
                (THIRD == 0xA0 && FOURTH >= 0x9E && FOURTH <= 0x9F))
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0xAC:
            {
              if (THIRD > 0xBA || (THIRD == 0xBA && FOURTH >= 0xA2))
              {
                *dest++ = ' ';
                continue;
              }
              break;
            }

          case 0xAF:
            {
              if (THIRD >= 0xA0 && THIRD <= 0xA8)
              {
                Helper::replace(TABLE_4_F0_AF[THIRD - 0xA0][FOURTH & 0x3F],
                  0, dest);
              }
              else
              {
                *dest++ = ' ';
              }
              continue;
            }

          default:
            if (!(SECOND >= 0x97 && (SECOND >= 0xB0 || SECOND <= 0x9A ||
              (SECOND >= 0xAD && SECOND <= 0xAE))))
            {
              break;
            }
            // FALLTHROUGH

          case 0x95:
          case 0x9C:
          case 0xAE:
            {
              *dest++ = ' ';
              continue;
            }
            break;
          }
        }
        else
        {
          if (FIRST < 0xF4)
          {
            if ((SECOND & 0xC0) != 0x80 || (THIRD & 0xC0) != 0x80)
            {
              it.backward(3);
            }
            *dest++ = ' ';
            continue;
          }

          if (FIRST == 0xF4)
          {
            if ((SECOND & 0xF0) != 0x80 || (THIRD & 0xC0) != 0x80)
            {
              it.backward(3);
            }
            *dest++ = ' ';
            continue;
          }

          *dest++ = ' ';
          it.backward(3);
          continue;
        }

        *dest++ = reinterpret_cast<const char&>(FIRST);
        *dest++ = reinterpret_cast<const char&>(SECOND);
        *dest++ = reinterpret_cast<const char&>(THIRD);
        *dest++ = reinterpret_cast<const char&>(FOURTH);
        break;
      }

    default:
      *dest++ = ' ';
    }
  }

  return true;
}

#endif
