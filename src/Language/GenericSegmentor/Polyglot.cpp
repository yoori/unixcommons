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



#include <Language/GenericSegmentor/Polyglot.hpp>


namespace Language
{
  namespace Segmentor
  {
    const DefaultPolyglotSymbols::CategoryType
      DefaultPolyglotSymbols::INVALID_SYMBOLS(
        "\xE1\x84\x80-\xE1\x87\xB9" // (0x1100 - 0x11f9) Hangul
        "\xE2\xBA\x80-\xE2\xBB\xB2" // (0x2e80 - 0x2ef2) CJK Radicals
        "\xE2\xBC\x80-\xE2\xBF\x95" // (0x2f00 - 0x2fd5) Kanji Radicals
        "\xE2\xBF\xB0-\xE2\xBF\xBB" // (0x2ff0 - 0x2ffb) Ideographic Description Characters
        "\xE3\x81\x81-\xE3\x82\x9F" // (0x3041 - 0x309f) Hiragana
        "\xE3\x82\xA0-\xE3\x83\xBF" // (0x30a0 - 0x30ff) Katakana
        "\xE3\x84\x85-\xE3\x84\xAD" // (0x3105 - 0x312d) Bopomofo
        "\xE3\x84\xB1-\xE3\x86\x8E" // (0x3131 - 0x318e) Hangul
        "\xE3\x86\x90-\xE3\x86\x9F" // (0x3190 - 0x319f) Kanbun
        "\xE3\x86\xA0-\xE3\x86\xB7" // (0x31a0 - 0x31b7) Bopomofo
        "\xE3\x87\x80-\xE3\x87\xA3" // (0x31c0 - 0x31e3) CJK
        "\xE3\x87\xB0-\xE3\x87\xBF" // (0x31f0 - 0x31ff) Kanbun
        "\xE3\x88\x80-\xE3\x8B\xBE" // (0x3200 - 0x32fe) Hangul
        "\xE3\x8C\x80-\xE3\x8F\xBE" // (0x3300 - 0x33fe) Katakana
        "\xE3\x90\x80-\xE4\xB6\xB5" // (0x3400 - 0x4db5) CJK
        "\xE4\xB8\x80-\xE9\xBE\xBB" // (0x4e00 - 0x9fbb) CJK
        "\xEA\xB0\x80-\xED\x9E\xA3" // (0xac00 - 0xd7a3) Hangul
        "\xEF\xA4\x80-\xEF\xAB\x99" // (0xf900 - 0xfad9) CJK
        "\xF0\xA0\x80\x80-\xF0\xAA\x9B\x95" // (0x20000 - 0x2a6d5) CJK Extension B
        "\xF0\xAF\xA0\x80-\xF0\xAF\xA8\x9D" // (0x2f800 - 0x2fa1d) CJK Compatibility
        , true);
  }
}
