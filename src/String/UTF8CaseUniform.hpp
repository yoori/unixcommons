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



// @file String/UTF8CaseUniform.hpp
#ifndef STRING_UTF8_CASE_UNIFORM_HPP
#define STRING_UTF8_CASE_UNIFORM_HPP

#include <String/UTF8Case.hpp>
#include <String/UTF8Tables.hpp>
#include <String/UTF8Handler.hpp>


bool
String::ToUniform::to_uniform(Helper::Iterator it, char*& dest,
  size_t& counter) throw ()
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
          // #c3.9f 73 73
          // #c4.b0 69 cc.87
          // #c5.89 ca.bc 6e
          // #c5.bf 73
          // #c7.b0 6a cc.8c
          // #c8.ba e2.b1.a5
          // #c8.be e2.b1.a6
          // #ce.90 ce.b9 cc.88 cc.81
          // #ce.b0 cf.85 cc.88 cc.81
          // #d6.87 d5.a5 d6.82
          switch (FIRST)
          {
          case 0xC3:
            {
              *dest++ = '\x73';
              *dest++ = '\x73';
              counter++;
              break;
            }
          case 0xC4:
            {
              *dest++ = '\x69';
              *dest++ = '\xCC';
              *dest++ = '\x87';
              counter++;
              break;
            }
          case 0xC5:
            {
              if (SECOND == 0x89)
              {
                *dest++ = '\xCA';
                *dest++ = '\xBC';
                *dest++ = '\x6E';
                counter++;
              }
              else
              {
                *dest++ = '\x73';
              }
              break;
            }
          case 0xC7:
            {
              *dest++ = '\x6A';
              *dest++ = '\xCC';
              *dest++ = '\x8C';
              counter++;
              break;
            }
          case 0xC8:
            {
              *dest++ = '\xE2';
              *dest++ = '\xB1';
              *dest++ = MODIFIED[1];
              break;
            }
          case 0xCE:
            {
              if (SECOND == 0x90)
              {
                *dest++ = '\xCE';
                *dest++ = '\xB9';
              }
              else
              {
                *dest++ = '\xCF';
                *dest++ = '\x85';
              }
              *dest++ = '\xCC';
              *dest++ = '\x88';
              *dest++ = '\xCC';
              *dest++ = '\x81';
              counter += 2;
              break;
            }
          case 0xD6:
            {
              *dest++ = '\xD5';
              *dest++ = '\xA5';
              *dest++ = '\xD6';
              *dest++ = '\x82';
              counter++;
              break;
            }
          }
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
              switch (SECOND)
              {
              case 0xBA:
                {
                  if (THIRD == 0x9E)
                  {
                    *dest++ = '\x73';
                    *dest++ = '\x73';
                  }
                  else
                  {
                    *dest++ = MODIFIED[1];
                    *dest++ = THIRD == 0x9A ? '\xCA' : '\xCC';
                    *dest++ = MODIFIED[2];
                  }
                  counter++;
                  break;
                }
              case 0xBD:
                {
                  *dest++ = '\xCF';
                  *dest++ = '\x85';
                  *dest++ = '\xCC';
                  *dest++ = '\x93';
                  counter++;
                  if (THIRD > 0x90)
                  {
                    *dest++ = MODIFIED[1];
                    *dest++ = MODIFIED[2];
                    counter++;
                  }
                  break;
                }
              case 0xBE:
                {
                  if (THIRD < 0xB3)
                  {
                    *dest++ = '\xE1';
                    *dest++ = MODIFIED[1];
                    *dest++ = MODIFIED[2];
                    *dest++ = '\xCE';
                    *dest++ = '\xB9';
                    counter++;
                  }
                  else
                  {
                    if (THIRD == 0xBE)
                    {
                      *dest++ = '\xCE';
                      *dest++ = '\xB9';
                    }
                    else
                    {
                      *dest++ = '\xCE';
                      *dest++ = MODIFIED[1];
                      *dest++ = MODIFIED[2];
                      *dest++ = MODIFIED[3];
                      counter++;
                      if (THIRD == 0xB7)
                      {
                        *dest++ = '\xCE';
                        *dest++ = '\xB9';
                        counter++;
                      }
                    }
                  }
                  break;
                }
              case 0xBF:
                {
                  const Table_3_E1_BF& MODIFIED =
                    TABLE_3_E1_BF[THIRD & 0x3F];
                  for (size_t i = 0; i < MODIFIED.substr.size(); i++)
                  {
                    *dest++ = MODIFIED.substr[i];
                  }
                  counter += MODIFIED.symbols;
                  break;
                }
              }
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
            break;
          }
        case 0xEF:
          if ((SECOND & 0xC0) != 0x80)
          {
            return false;
          }
          switch (SECOND)
          {
          case 0xAC:
            if (THIRD <= 0x86)
            {
              if (THIRD < 0x83)
              {
                *dest++ = '\x66';
                *dest++ = THIRD == 0x80 ? '\x66' :
                  THIRD == 0x81 ? '\x69' : '\x6C';
              }
              else
              {
                if (THIRD < 0x85)
                {
                  *dest++ = '\x66';
                  *dest++ = '\x66';
                  *dest++ = THIRD == 0x83 ? '\x69' : '\x6C';
                  counter++;
                }
                else
                {
                  *dest++ = '\x73';
                  *dest++ = '\x74';
                }
              }
              counter++;
              continue;
            }
            else
            {
              if (THIRD >= 0x93 && THIRD <= 0x97)
              {
                const CodeUnit2Bytes& MODIFIED =
                  TABLE_3_EF_AC[THIRD & 0x7];
                *dest++ = '\xD5';
                *dest++ = MODIFIED[0];
                *dest++ = '\xD5';
                *dest++ = MODIFIED[1];
                counter++;
                continue;
              }
            }
            break;
          case 0xBC:
            if (THIRD >= 0xA1 && THIRD <= 0xBA)
            {
              *dest++ = reinterpret_cast<const char&>(FIRST);
              *dest++ = '\xBD';
              *dest++ = TABLE_3_EF_BC[THIRD & 0x1F];
              continue;
            }
            break;
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
                  *dest++ = FOURTH >= 0x98 && FOURTH <= 0xA7 ? '\x91' :
                    reinterpret_cast<const char&>(THIRD);
                  *dest++ = TABLE_4_F0[FOURTH & 0x3F];
                  continue;
                }
              case 0xB2:
                {
                  if (FOURTH < 0xB3)
                  {
                    *dest++ = reinterpret_cast<const char&>(FIRST);
                    *dest++ = reinterpret_cast<const char&>(SECOND);
                    *dest++ = '\xB3';
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
