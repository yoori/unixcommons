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



// @file String/UTF8CaseLower.hpp
#ifndef STRING_UTF8_CASE_LOWER_HPP
#define STRING_UTF8_CASE_LOWER_HPP

#include <String/UTF8Case.hpp>
#include <String/UTF8Tables.hpp>
#include <String/UTF8Handler.hpp>


bool
String::ToLower::to_lower(Helper::Iterator it, char*& dest, size_t& counter)
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
          FIRST - static_cast<unsigned char>(0xC3);
        if (SLOT_NUMBER > 18)
        {
          *dest++ = reinterpret_cast<const char&>(FIRST);
          *dest++ = reinterpret_cast<const char&>(SECOND);
          continue;
        }

        const CodeUnit2Bytes& MODIFIED = TABLE_2[SLOT_NUMBER][SECOND & 0x3F];
        if (MODIFIED[0] == 0)
        {
          // SPECIAL
          // #c4.b0 69
          // #c8.ba e2.b1.a5
          // #c8.be e2.b1.a6
          if (FIRST == 0xC4)
          {
            *dest++ = '\x69';
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
              // #e1.ba.9e c3.9f
              *dest++ = '\xC3';
              *dest++ = '\x9F';
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
            if (MODIFIED[0] == 0)
            {
              // SPECIAL
              // #e2.84.a6 cf.89
              // #e2.84.aa 6b
              // #e2.84.ab c3.a5
              // #e2.b1.a2 c9.ab
              // #e2.b1.a3 e1.b5.bd
              // #e2.b1.a4 c9.bd
              // #e2.b1.ad c9.91
              // #e2.b1.ae c9.b1
              // #e2.b1.af c9.90
              // #e2.b1.b0 c9.92
              // #e2.b1.be c8.bf
              // #e2.b1.bf c9.80
              if (SECOND == 0x84)
              {
                if (THIRD < 0xAA)
                {
                  *dest++ = '\xCF';
                  *dest++ = '\x89';
                  continue;
                }
                if (THIRD == 0xAA)
                {
                  *dest++ = '\x6B';
                  continue;
                }
                *dest++ = '\xC3';
                *dest++ = '\xA5';
                continue;
              }
              const CodeUnit2Bytes& MODIFIED_SP =
                TABLE_3_SP_E2[THIRD & 0x1F];
              *dest++ = MODIFIED_SP[0];
              *dest++ = MODIFIED_SP[1];
              if (THIRD == 0xA3)
              {
                *dest++ = '\xBD';
              }
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
              if (MODIFIED[0] == 0)
              {
                // SPECIAL
                // #ea.9d.bd e1.b5.b9
                // #ea.9e.8d c9.a5
                // #ea.9e.aa c9.a6
                // #ea.9e.ab c9.9c
                // #ea.9e.ac c9.a1
                // #ea.9e.ad c9.ac
                // #ea.9e.b0 ca.9e
                // #ea.9e.b1 ca.87
                // #ea.9e.b2 ca.9d
                if (SECOND == 0x9D)
                {
                  *dest++ = '\xE1';
                  *dest++ = '\xB5';
                  *dest++ = '\xB9';
                }
                else
                {
                  *dest++ = THIRD < 0xB0 ? '\xC9' : '\xCA';
                  *dest++ = MODIFIED[1];
                }
                continue;
              }
              *dest++ = reinterpret_cast<const char&>(FIRST);
              *dest++ = MODIFIED[0];
              *dest++ = MODIFIED[1];
              continue;
            }
            break;
          }
        case 0xEF:
          if (SECOND == 0xBC && THIRD >= 0xA1 && THIRD <= 0xBA)
          {
            *dest++ = reinterpret_cast<const char&>(FIRST);
            *dest++ = '\xBD';
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
              if (THIRD == 0x90)
              {
                *dest++ = reinterpret_cast<const char&>(FIRST);
                *dest++ = reinterpret_cast<const char&>(SECOND);
                *dest++ = FOURTH >= 0x98 && FOURTH <= 0xA7 ? '\x91' :
                  reinterpret_cast<const char&>(THIRD);
                *dest++ = TABLE_4_F0[FOURTH & 0x3F];
                continue;
              }
              if (THIRD == 0xB2)
              {
                *dest++ = reinterpret_cast<const char&>(FIRST);
                *dest++ = reinterpret_cast<const char&>(SECOND);
                *dest++ = FOURTH < 0xB3 ? '\xB3' :
                  reinterpret_cast<const char&>(THIRD);
                *dest++ = reinterpret_cast<const char&>(FOURTH);
                continue;
              }
            }
            else
            {
              if (SECOND == 0x91)
              {
                if (THIRD == 0xA2 && FOURTH >= 0xA0)
                {
                  *dest++ = reinterpret_cast<const char&>(FIRST);
                  *dest++ = reinterpret_cast<const char&>(SECOND);
                  *dest++ = '\xA3';
                  *dest++ = reinterpret_cast<const char&>(FOURTH) - 0x20;
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
