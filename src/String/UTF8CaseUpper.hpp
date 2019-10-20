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



// @file String/UTF8CaseUpper.hpp
#ifndef STRING_UTF8_CASE_UPPER_HPP
#define STRING_UTF8_CASE_UPPER_HPP

#include <String/UTF8Case.hpp>
#include <String/UTF8Tables.hpp>
#include <String/UTF8Handler.hpp>


bool
String::ToUpper::to_upper(Helper::Iterator it, char*& dest, size_t& counter)
  throw ()
{
  for (counter = 0; !it.exhausted(); ++counter)
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
          return false;
        }
        const unsigned char SECOND = it.forward();
        if ((SECOND & 0xC0) != 0x80)
        {
          return false;
        }
        const unsigned char SLOT_NUMBER =
          FIRST - static_cast<unsigned char>(0xC2);
        if (SLOT_NUMBER > 20)
        {
          *dest++ = reinterpret_cast<const char&>(FIRST);
          *dest++ = reinterpret_cast<const char&>(SECOND);
          continue;
        }

        const CodeUnit2Bytes& MODIFIED =
          TABLE_2[SLOT_NUMBER][SECOND & 0x3F];
        if (MODIFIED[0] == 0)
        {
          // SPECIAL
          // #c4.b1 49
          // #c5.bf 53
          // #c8.bf e2.b1.be
          // #c9.80 e2.b1.bf
          // #c9.90 e2.b1.af
          // #c9.91 e2.b1.ad
          // #c9.92 e2.b1.b0
          // #c9.9c ea.9e.ab
          // #c9.a1 ea.9e.ac
          // #c9.a5 ea.9e.8d
          // #c9.a6 ea.9e.aa
          // #c9.ab e2.b1.a2
          // #c9.ac ea.9e.ad
          // #c9.b1 e2.b1.ae
          // #c9.bd e2.b1.a4
          // #ca.87 ea.9e.b1
          // #ca.9d ea.9e.b2
          // #ca.9e ea.9e.b0
          if (FIRST < 0xC5)
          {
            *dest++ = '\x49';
            continue;
          }
          if (FIRST == 0xC5)
          {
            *dest++ = '\x53';
            continue;
          }
          if (FIRST == 0xCA || (SECOND >= 0x9C && SECOND <= 0xA6) ||
            SECOND == 0xAC)
          {
            *dest++ = '\xEA';
            *dest++ = '\x9E';
            *dest++ = MODIFIED[1];
            continue;
          }
          *dest++ = '\xE2';
          *dest++ = '\xB1';
          *dest++ = MODIFIED[1];
          continue;
        }
        *dest++ = MODIFIED[0];
        *dest++ = MODIFIED[1];
        continue;
      }
    case 3:
      {
        if (it.exhausted())
        {
          return false;
        }
        const unsigned char SECOND = it.forward();
        if (it.exhausted())
        {
          return false;
        }
        const unsigned char THIRD = it.forward();
        if ((THIRD & 0xC0) != 0x80)
        {
          return false;
        }

        if (FIRST == 0xE0 ? (SECOND & 0xE0) != 0xA0 :
          FIRST == 0xED ? (SECOND & 0xE0) != 0x80 :
          (SECOND & 0xC0) != 0x80)
        {
          return false;
        }

        switch (FIRST)
        {
        case 0xE1:
          {
            const CodeUnit4Bytes& MODIFIED =
              TABLE_3_E1[SECOND & 0x3F][THIRD & 0x3F];
            if (MODIFIED[0] == 0)
            {
              // SPECIAL
              // #e1.be.be ce.99
              *dest++ = '\xCE';
              *dest++ = '\x99';
              continue;
            }
            *dest++ = MODIFIED[0];
            *dest++ = MODIFIED[1];
            *dest++ = MODIFIED[2];
            continue;
          }
        case 0xE2:
          {
            const CodeUnit2Bytes& MODIFIED =
              TABLE_3_E2[SECOND & 0x3F][THIRD & 0x3F];
            if (SECOND == 0xB4 &&
              (THIRD < 0xA6 || THIRD == 0xA7 || THIRD == 0xAD))
            {
              *dest++ = '\xE1';
              *dest++ = MODIFIED[0];
              *dest++ = MODIFIED[1];
              continue;
            }
            if (MODIFIED[0] == 0)
            {
              // SPECIAL
              // #e2.b1.a5 c8.ba
              // #e2.b1.a6 c8.be
              *dest++ = '\xC8';
              *dest++ = THIRD == 0xA5 ? '\xBA' : '\xBE';
              continue;
            }
            *dest++ = reinterpret_cast<const char&>(FIRST);
            *dest++ = MODIFIED[0];
            *dest++ = MODIFIED[1];
            continue;
          }
        case 0xEA:
          {
            const unsigned char SLOT_NUMBER =
              SECOND - static_cast<unsigned char>(0x99);
            if (SLOT_NUMBER < 6)
            {
              const CodeUnit2Bytes& MODIFIED =
                TABLE_3_EA[SLOT_NUMBER][THIRD & 0x3F];
              *dest++ = reinterpret_cast<const char&>(FIRST);
              *dest++ = MODIFIED[0];
              *dest++ = MODIFIED[1];
              continue;
            }
            else
            {
              switch (SECOND)
              {
              case 0xAD:
                {
                  if (THIRD >= 0xB0)
                  {
                    *dest++ = '\xE1';
                    *dest++ = '\x8E';
                    *dest++ = reinterpret_cast<const char&>(THIRD) - 0x10;
                    continue;
                  }
                  if (THIRD == 0x93)
                  {
                    *dest++ = reinterpret_cast<const char&>(FIRST);
                    *dest++ = '\x9E';
                    *dest++ = '\xB3';
                    continue;
                  }
                  break;
                }
              case 0xAE:
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
                  continue;
                }
              default:
                break;
              }
            }
            break;
          }
        case 0xEF:
          if ((SECOND & 0xC0) != 0x80)
          {
            return false;
          }
          if (SECOND == 0xBD && THIRD >= 0x81 && THIRD <= 0x9A)
          {
            *dest++ = reinterpret_cast<const char&>(FIRST);
            *dest++ = '\xBC';
            *dest++ = TABLE_3_EF[THIRD & 0x3F];
            continue;
          }
          break;
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
          return false;
        }
        const unsigned char SECOND = it.forward();
        if (it.exhausted())
        {
          return false;
        }
        const unsigned char THIRD = it.forward();
        if (it.exhausted())
        {
          return false;
        }
        if ((THIRD & 0xC0) != 0x80)
        {
          return false;
        }
        const unsigned char FOURTH = it.forward();
        if ((FOURTH & 0xC0) != 0x80)
        {
          return false;
        }

        if (FIRST < 0xF4)
        {
          if (FIRST == 0xF0)
          {
            if (SECOND == 0x90)
            {
              switch (THIRD)
              {
              case 0x90:
                {
                  *dest++ = reinterpret_cast<const char&>(FIRST);
                  *dest++ = reinterpret_cast<const char&>(SECOND);
                  *dest++ = reinterpret_cast<const char&>(THIRD);
                  *dest++ = TABLE_4_F0_90[FOURTH & 0x3F];
                  continue;
                }
              case 0x91:
                {
                  if (FOURTH <= 0x8F)
                  {
                    *dest++ = reinterpret_cast<const char&>(FIRST);
                    *dest++ = reinterpret_cast<const char&>(SECOND);
                    *dest++ = '\x90';
                    *dest++ = TABLE_4_F0_91[FOURTH & 0x3F];
                    continue;
                  }
                  break;
                }
              case 0xB3:
                {
                  if (FOURTH < 0xB3)
                  {
                    *dest++ = reinterpret_cast<const char&>(FIRST);
                    *dest++ = reinterpret_cast<const char&>(SECOND);
                    *dest++ = '\xB2';
                    *dest++ = reinterpret_cast<const char&>(FOURTH);
                    continue;
                  }
                  break;
                }
              default:
                break;
              }
            }
            else
            {
              if (SECOND == 0x91)
              {
                if (THIRD == 0xA3 && FOURTH <= 0x9F)
                {
                  *dest++ = reinterpret_cast<const char&>(FIRST);
                  *dest++ = reinterpret_cast<const char&>(SECOND);
                  *dest++ = '\xA2';
                  *dest++ = reinterpret_cast<const char&>(FOURTH) + 0x20;
                  continue;
                }
              }
            }
            if (SECOND < 0x90 || SECOND > 0xBF)
            {
              return false;
            }
          }
          else
          {
            if ((SECOND & 0xC0) != 0x80)
            {
              return false;
            }
          }
        }
        else
        {
          if (FIRST == 0xF4)
          {
            if ((SECOND & 0xF0) != 0x80)
            {
              return false;
            }
          }
          else
          {
            return false;
          }
        }
        *dest++ = reinterpret_cast<const char&>(FIRST);
        *dest++ = reinterpret_cast<const char&>(SECOND);
        *dest++ = reinterpret_cast<const char&>(THIRD);
        *dest++ = reinterpret_cast<const char&>(FOURTH);
        break;
      }
    default:
      return false;
    }
  }
  return true;
}

#endif
