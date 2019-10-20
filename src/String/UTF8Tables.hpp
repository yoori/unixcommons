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





#ifndef STRING_UTF8_TABLES_HPP
#define STRING_UTF8_TABLES_HPP

#include <cstdint>

#include <String/SubString.hpp>


namespace String
{
  namespace Detail
  {
    typedef const char Plane1Byte[64];

    typedef const char CodeUnit2Bytes[2];

    typedef const CodeUnit2Bytes Plane2Bytes[64];

    typedef const char CodeUnit4Bytes[4];

    typedef const CodeUnit4Bytes Plane4Bytes[64];

    typedef const uint64_t Plane2Bits[2];
  }


  namespace ToLower
  {
    using namespace Detail;

    extern const char TABLE_1[128];

    extern const Plane2Bytes TABLE_2[19];

    extern const Plane4Bytes TABLE_3_E1[64];

    extern const Plane2Bytes TABLE_3_E2[64];

    extern const Plane2Bytes TABLE_3_EA[6];

    extern const char TABLE_3_EF[64];

    extern const CodeUnit2Bytes TABLE_3_SP_E2[32];

    extern const char TABLE_4_F0[64];
  }

  namespace ToUpper
  {
    using namespace Detail;

    extern const char TABLE_1[128];

    extern const Plane2Bytes TABLE_2[21];

    extern const Plane4Bytes TABLE_3_E1[64];

    extern const Plane2Bytes TABLE_3_E2[64];

    extern const Plane2Bytes TABLE_3_EA[6];

    extern const char TABLE_3_EF[64];

    extern const char TABLE_4_F0_90[64];

    extern const char TABLE_4_F0_91[16];
  }

  namespace ToUniform
  {
    using namespace Detail;

    extern const char TABLE_1[128];

    extern const Plane2Bytes TABLE_2[21];

    extern const Plane4Bytes TABLE_3_E1[64];

    struct Table_3_E1_BF
    {
      SubString substr;
      size_t symbols;
    };
    extern const Table_3_E1_BF TABLE_3_E1_BF[64];

    extern const Plane2Bytes TABLE_3_E2[64];

    extern const Plane2Bytes TABLE_3_EA[6];

    extern const CodeUnit2Bytes TABLE_3_EF_AC[8];

    extern const char TABLE_3_EF_BC[32];

    extern const CodeUnit2Bytes TABLE_3_SP_E2[32];

    extern const char TABLE_4_F0[64];
  }

  namespace ToSimplify
  {
    using namespace Detail;

    extern const char TABLE_1[128];

    extern const Plane2Bytes TABLE_2[30];
    extern const String::SubString TABLE_2_[12];

    extern const Plane2Bits TABLE_3_E0[32];
    extern const char TABLE_3_E0_[8];

    extern const Plane2Bits TABLE_3_E1_1[52];
    extern const Plane2Bytes TABLE_3_E1_2[12];
    extern const String::SubString TABLE_3_E1_2_[13];

    extern const Plane1Byte TABLE_3_E2_1[5];
    extern const String::SubString TABLE_3_E2_1_[26];
    extern const Plane4Bytes TABLE_3_E2_2[2];
    extern const Plane4Bytes TABLE_3_E2_3[3];
    extern const Plane4Bytes TABLE_3_E2_4[16];

    extern const Plane4Bytes TABLE_3_E3[16];
    extern const String::SubString TABLE_3_E3_[222];

    extern const Plane4Bytes TABLE_3_EA_1[7];
    extern const Plane2Bits TABLE_3_EA_2[16];
    extern const String::SubString TABLE_3_EA_2_[4];

    extern const Plane4Bytes TABLE_3_EF[28];
    extern const String::SubString TABLE_3_EF_[132];

    extern const Plane2Bits TABLE_4_F0_90_1[64];
    extern const Plane2Bytes TABLE_4_F0_90_2[1];

    extern const Plane2Bits TABLE_4_F0_91[29];

    extern const Plane2Bits TABLE_4_F0_96[24];

    extern const Plane2Bits TABLE_4_F0_9B[2];

    extern const Plane2Bits TABLE_4_F0_9D_1[16];
    extern const Plane2Bytes TABLE_4_F0_9D_2[16];

    extern const Plane2Bits TABLE_4_F0_9E_1[1];
    extern const Plane2Bytes TABLE_4_F0_9E_2[3];

    extern const Plane4Bytes TABLE_4_F0_9F[6];
    extern const String::SubString TABLE_4_F0_9F_[12];

    extern const Plane4Bytes TABLE_4_F0_AF[9];
  }
}

#endif
