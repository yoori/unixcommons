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



#include <cctype>

#include <String/AsciiStringManip.hpp>


namespace String
{
  namespace AsciiStringManip
  {
    namespace Tables
    {
      const char ASCII_TOLOWER_TABLE[256] =
      {
        '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
        '\x08', '\x09', '\x0A', '\x0B', '\x0C', '\x0D', '\x0E', '\x0F',
        '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
        '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F',
        '\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27',
        '\x28', '\x29', '\x2A', '\x2B', '\x2C', '\x2D', '\x2E', '\x2F',
        '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37',
        '\x38', '\x39', '\x3A', '\x3B', '\x3C', '\x3D', '\x3E', '\x3F',
        '\x40', '\x61', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67',
        '\x68', '\x69', '\x6A', '\x6B', '\x6C', '\x6D', '\x6E', '\x6F',
        '\x70', '\x71', '\x72', '\x73', '\x74', '\x75', '\x76', '\x77',
        '\x78', '\x79', '\x7A', '\x5B', '\x5C', '\x5D', '\x5E', '\x5F',
        '\x60', '\x61', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67',
        '\x68', '\x69', '\x6A', '\x6B', '\x6C', '\x6D', '\x6E', '\x6F',
        '\x70', '\x71', '\x72', '\x73', '\x74', '\x75', '\x76', '\x77',
        '\x78', '\x79', '\x7A', '\x7B', '\x7C', '\x7D', '\x7E', '\x7F',
        '\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87',
        '\x88', '\x89', '\x8A', '\x8B', '\x8C', '\x8D', '\x8E', '\x8F',
        '\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97',
        '\x98', '\x99', '\x9A', '\x9B', '\x9C', '\x9D', '\x9E', '\x9F',
        '\xA0', '\xA1', '\xA2', '\xA3', '\xA4', '\xA5', '\xA6', '\xA7',
        '\xA8', '\xA9', '\xAA', '\xAB', '\xAC', '\xAD', '\xAE', '\xAF',
        '\xB0', '\xB1', '\xB2', '\xB3', '\xB4', '\xB5', '\xB6', '\xB7',
        '\xB8', '\xB9', '\xBA', '\xBB', '\xBC', '\xBD', '\xBE', '\xBF',
        '\xC0', '\xC1', '\xC2', '\xC3', '\xC4', '\xC5', '\xC6', '\xC7',
        '\xC8', '\xC9', '\xCA', '\xCB', '\xCC', '\xCD', '\xCE', '\xCF',
        '\xD0', '\xD1', '\xD2', '\xD3', '\xD4', '\xD5', '\xD6', '\xD7',
        '\xD8', '\xD9', '\xDA', '\xDB', '\xDC', '\xDD', '\xDE', '\xDF',
        '\xE0', '\xE1', '\xE2', '\xE3', '\xE4', '\xE5', '\xE6', '\xE7',
        '\xE8', '\xE9', '\xEA', '\xEB', '\xEC', '\xED', '\xEE', '\xEF',
        '\xF0', '\xF1', '\xF2', '\xF3', '\xF4', '\xF5', '\xF6', '\xF7',
        '\xF8', '\xF9', '\xFA', '\xFB', '\xFC', '\xFD', '\xFE', '\xFF'
      };

      const char ASCII_TOUPPER_TABLE[256] =
      {
        '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
        '\x08', '\x09', '\x0A', '\x0B', '\x0C', '\x0D', '\x0E', '\x0F',
        '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
        '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F',
        '\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27',
        '\x28', '\x29', '\x2A', '\x2B', '\x2C', '\x2D', '\x2E', '\x2F',
        '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37',
        '\x38', '\x39', '\x3A', '\x3B', '\x3C', '\x3D', '\x3E', '\x3F',
        '\x40', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47',
        '\x48', '\x49', '\x4A', '\x4B', '\x4C', '\x4D', '\x4E', '\x4F',
        '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57',
        '\x58', '\x59', '\x5A', '\x5B', '\x5C', '\x5D', '\x5E', '\x5F',
        '\x60', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47',
        '\x48', '\x49', '\x4A', '\x4B', '\x4C', '\x4D', '\x4E', '\x4F',
        '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57',
        '\x58', '\x59', '\x5A', '\x7B', '\x7C', '\x7D', '\x7E', '\x7F',
        '\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87',
        '\x88', '\x89', '\x8A', '\x8B', '\x8C', '\x8D', '\x8E', '\x8F',
        '\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97',
        '\x98', '\x99', '\x9A', '\x9B', '\x9C', '\x9D', '\x9E', '\x9F',
        '\xA0', '\xA1', '\xA2', '\xA3', '\xA4', '\xA5', '\xA6', '\xA7',
        '\xA8', '\xA9', '\xAA', '\xAB', '\xAC', '\xAD', '\xAE', '\xAF',
        '\xB0', '\xB1', '\xB2', '\xB3', '\xB4', '\xB5', '\xB6', '\xB7',
        '\xB8', '\xB9', '\xBA', '\xBB', '\xBC', '\xBD', '\xBE', '\xBF',
        '\xC0', '\xC1', '\xC2', '\xC3', '\xC4', '\xC5', '\xC6', '\xC7',
        '\xC8', '\xC9', '\xCA', '\xCB', '\xCC', '\xCD', '\xCE', '\xCF',
        '\xD0', '\xD1', '\xD2', '\xD3', '\xD4', '\xD5', '\xD6', '\xD7',
        '\xD8', '\xD9', '\xDA', '\xDB', '\xDC', '\xDD', '\xDE', '\xDF',
        '\xE0', '\xE1', '\xE2', '\xE3', '\xE4', '\xE5', '\xE6', '\xE7',
        '\xE8', '\xE9', '\xEA', '\xEB', '\xEC', '\xED', '\xEE', '\xEF',
        '\xF0', '\xF1', '\xF2', '\xF3', '\xF4', '\xF5', '\xF6', '\xF7',
        '\xF8', '\xF9', '\xFA', '\xFB', '\xFC', '\xFD', '\xFE', '\xFF'
      };
    }
  }
}

namespace String
{
  namespace AsciiStringManip
  {
    const char HEX_DIGITS[17] = "0123456789ABCDEF";

    const CharCategory ALPHA("A-Za-z");
    const CharCategory NUMBER("0-9");
    const CharCategory ALPHA_NUM(ALPHA, NUMBER);
    const CharCategory OCTAL_NUMBER("0-7");
    const CharCategory HEX_NUMBER("0-9A-Fa-f");
    const CharCategory SPACE(isspace);
    const CharCategory REGEX_META("^.$\\|()[]*+?{}");

    namespace Category
    {
      CharTable::CharTable(const char* str, bool check_zero)
        throw ()
      {
        std::fill(table_, table_ + 256, false);

        if (check_zero)
        {
          table_[0] = true;
        }

        if (!str)
        {
          return;
        }

        char last = '\0';
        for (; *str; last = *str, str++)
        {
          if (*str == '-' && last && str[1])
          {
            char till = *++str;
            for (char ch = last; ; ch++)
            {
              table_[static_cast<uint8_t>(ch)] = true;
              if (ch == till)
              {
                break;
              }
            }
          }
          else
          {
            table_[static_cast<uint8_t>(*str)] = true;
          }
        }
      }

      CharTable::CharTable(const CharTable& first, const CharTable& second)
        throw ()
      {
        for (int i = 0; i < 256; i++)
        {
          table_[i] = first.table_[i] || second.table_[i];
        }
      }

      CharTable::CharTable(const CharTable& first, const CharTable& second,
        const CharTable& third) throw ()
      {
        for (int i = 0; i < 256; i++)
        {
          table_[i] = first.table_[i] || second.table_[i] ||
            third.table_[i];
        }
      }
    }
  }
}
