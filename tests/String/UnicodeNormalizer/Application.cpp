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



#if 0
// @file UnicodeNormalizer/Application.cpp


#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <fstream>
#include <sstream>

#include <Generics/Rand.hpp>
#include <Generics/Time.hpp>
#include <String/UnicodeSymbol.hpp>
#include <eh/Exception.hpp>
#include <String/UnicodeNormalizer.hpp>
#include <String/UnicodeSymbol.hpp>
#include <String/UTF8Case.hpp>
#include <String/SubString.hpp>
#include <Stream/FlagsSaver.hpp>

using namespace String;
using namespace Normalizer;

//// Max code unit: 10FFFD
namespace
{
  DECLARE_EXCEPTION(TestException, eh::DescriptiveException);

  // Standard for non-zero canonical classes
  typedef std::vector<wchar_t> Mapping;
  typedef std::map<wchar_t, Mapping> Map;
  Map standard;

  typedef std::map<wchar_t, uint8_t> CanonicalMap;
  CanonicalMap std_canonical_map;

  typedef std::vector<wchar_t> CanonicalSet;
  CanonicalSet std_canonical_set;

  typedef std::pair<wchar_t, wchar_t> ComposeArgument;
  typedef std::map<ComposeArgument, wchar_t> ComposeMap;
  ComposeMap std_compose_map;

  struct TestInfoRecord
  {
    std::wstring input;
    std::wstring NFC;
    std::wstring NFD;
    std::wstring NFKC;
    std::wstring NFKD;
  };
  typedef std::deque<TestInfoRecord> ConformanceData;
  ConformanceData std_conformance_data_part0;
  ConformanceData std_conformance_data_part1;
  ConformanceData std_conformance_data_part2;

  void
  print(const std::wstring& wstr) throw (eh::Exception)
  {
    std::cerr << std::hex;
    for (std::size_t j = 0; j < wstr.size(); ++j)
    {
      std::cerr << (int)(wstr[j]) << " ";
    }
  }


  class DataLoader
  {
  public:
    DataLoader() throw (eh::Exception);

    static void
    open_file(std::ifstream& ifs, const char* name)
      throw (eh::Exception);
  } data_loader;

  DataLoader::DataLoader() throw (eh::Exception)
  {
    std::ifstream ifs;
    open_file(ifs, "DecompositionMapRFC3491.txt");

    // Loading standard for mapping
    while (ifs)
    {
      std::string line;
      std::getline(ifs, line);

      if (line.empty() || !isalnum(line[0]))
      {
        continue;
      }
      char* end = 0;
      wchar_t key = strtol(line.c_str(), &end, 16);
      Mapping mapping;
      while (*end != '\0')
      {
        ++end;
        char* end_ptr = 0;
        wchar_t value = strtol(end, &end_ptr, 16);
        mapping.push_back(value);
        end = end_ptr;
      }
      standard[key] = mapping;
    }
    ifs.close();
    std::cout << "Loaded " << standard.size() << " mapped elements."
      << std::endl;

    open_file(ifs, "CodeUnitCombiner.txt");

    // Loading standard canonical classes
    while (ifs)
    {
      std::string line;
      std::getline(ifs, line);

      if (line.empty() || !isalnum(line[0]))
      {
        continue;
      }
      char* end = 0;
      wchar_t key = strtol(line.c_str(), &end, 16);
      char* end_ptr = 0;
      uint8_t value = strtol(end, &end_ptr, 10);
      std_canonical_map[key] = value;
      std_canonical_set.push_back(key);
    }
    ifs.close();
    std::cout << "Loaded " << std_canonical_map.size()
      << " combiners." << std::endl;

    open_file(ifs, "CanonicalReverseMapping.txt");

    // Loading standard canonical classes
    while (ifs)
    {
      std::string line;
      std::getline(ifs, line);

      if (line.empty() || !isalnum(line[0]))
      {
        continue;
      }
      char* end = 0;
      wchar_t starter = strtol(line.c_str(), &end, 16);
      char* end_ptr = 0;
      wchar_t combiner = strtol(end, &end_ptr, 16);
      wchar_t value = strtol(end_ptr, &end, 16);

      std_compose_map[ComposeArgument(starter, combiner)] = value;
    }
    ifs.close();
    std::cout << "Loaded " << std_compose_map.size()
      << " canonical composition mapping elements." << std::endl;
    // Load NormalisationTest of UTR#15
    open_file(ifs, "NormalizationTest.txt");

    // Loading standard canonical classes
    const AsciiStringManip::CharCategory USEFULL(
      AsciiStringManip::HEX_NUMBER, "@");
    ConformanceData* parts[3] =
    {
      &std_conformance_data_part0,
      &std_conformance_data_part1,
      &std_conformance_data_part2
    };
    std::size_t current_part_index = 0;
    ConformanceData* current_part = *parts;

    while (ifs)
    {
      std::string line;
      std::getline(ifs, line);

      if (line.empty() || !USEFULL.is_owned(line[0]))
      {
        continue;
      }
      if (line[0] == '@')
      {
        current_part = parts[current_part_index++];
        continue;
      }
      Stream::Parser sstr(line);

      unsigned long one = 0;
      char divider;
      TestInfoRecord record;
      std::wstring* fields[] =
      {
        &record.input, &record.NFC, &record.NFD, &record.NFKC, &record.NFKD
      };

      for (std::size_t i = 0; i < sizeof(fields) / sizeof(*fields); ++i)
      {
        for (;;)
        {
          sstr >> std::hex >> one;
          *fields[i] += one;
          if (sstr.str()[0] == ';')
          {
            sstr >> divider;
            break;
          }
        }
      }
      current_part->push_back(record);
    }
    ifs.close();
    std::cout << "Loaded " << std_conformance_data_part0.size()
      << " + " << std_conformance_data_part1.size()
      << " + " << std_conformance_data_part2.size()
      << " conformance normalization test." << std::endl;
  }

  void
  DataLoader::open_file(std::ifstream& ifs, const char* name)
    throw (eh::Exception)
  {
    // load standard result to do code check
    char* ev = getenv("TEST_TOP_SRC_DIR");
    std::string root_path = ev ? ev : "../../../..";
    root_path +=
      "/tests/String/UnicodeNormalizer/Data/";
    root_path += name; //"DecompositionMapRFC3491.txt";

    ifs.open(root_path.c_str());
    if (!ifs)
    {
      std::ostringstream ost;
      ost << "File " << root_path << " open error";
      throw TestException(ost.str());
    }
  }

  // Alternative - compressed data for get_combining_class
  // function. Memory less but slower than used in String library.
  const uint64_t MASKS[195] =
  {
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0xFFFFFFFFFFFFFFFFLL, 0x0000FFFF00007FFFLL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000078LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0xBBFFFFFBFFFE0000LL, 0x0000000000000016LL,
    0x0000000000000000LL, 0x00010000003FF800LL, 0x0000000000000000LL,
    0x00003D9F9FC00000LL, 0xFFFF000000020000LL, 0x00000000000007FFLL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x1000000000000000LL, 0x00000000001E2000LL, 0x1000000000000000LL,
    0x0000000000002000LL, 0x1000000000000000LL, 0x0000000000002000LL,
    0x1000000000000000LL, 0x0000000000002000LL, 0x1000000000000000LL,
    0x0000000000002000LL, 0x0000000000000000LL, 0x0000000000002000LL,
    0x0000000000000000LL, 0x0000000000602000LL, 0x0000000000000000LL,
    0x0000000000002000LL, 0x0000000000000000LL, 0x0000000000002000LL,
    0x0000000000000000LL, 0x0000000000000400LL, 0x0700000000000000LL,
    0x0000000000000F00LL, 0x0300000000000000LL, 0x0000000000000F00LL,
    0x02A0000003000000LL, 0x3C16000000000000LL, 0x00000000000000DDLL,
    0x0000000000000040LL, 0x0280000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0010000000100000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000040000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000020000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x000007E21FFF0000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL, 0x0000000000000000LL,
    0x0000FC0000000000LL, 0x0000000000000000LL, 0x0000000006000000LL,
  };

  inline bool
  get_NZ_canonical_class(wchar_t wch) throw ()
  {
    if (wch < 0x30C0)
    {
      // long long MASK[195] contain info about first 12480 characters
      return (static_cast<uint64_t>(1) << (wch & 0x3F)) & MASKS[wch >> 6];
    }
    if (wch > 0x1D1AD)
    {
      return 0;
    }
    return (*COMBINING_CLASS_INDEX[wch >> 8])[wch & 0xFF] != 0;
  }

  const WSubString SAMPLES[] =
  {
    WSubString(L"Madsen", 6),
    WSubString(L"", static_cast<size_t>(0)),
    WSubString(L"", 1),
    WSubString(L"A", 1),
    WSubString(L"A", 2),
    WSubString(L"Йёлжик", 6),
    WSubString(L"높였다", 3),
    WSubString(
      L"\xAA8C5\x317\x5FBEC\x346\x2AF73\x302\x1F44\x334\x6AEAB\x1D16F", 10),
    WSubString(L"\x41\x1806\x31A\x20D4\xFE21\x32D\xF7B", 7),
    WSubString(L"\x41\x200D\x340\x32C\x655\x742\x5BF\xB725A\xB725A", 9),
    WSubString(L"\x1F44\x334\x6AEAB\x1D16F", 4),
    WSubString(L"\x1F44", 1),
    WSubString(L"\x1F44\x334", 2),
    WSubString(L"\x41\x334\x1F44", 3),
    WSubString(L"\x41\x300\x301", 3),
    WSubString(L"\x41\x300\x323", 3),
    WSubString(L"\x41\x323\x300", 3),
    WSubString(L"\x41\x300\x301\x41", 4),
    WSubString(L"\x41\x300\x323\x41", 4),
    WSubString(L"\x41\x323\x300\x41", 4),
    WSubString(L"\x41\x300\x301", 3),
    WSubString(L"\x41\x300\x323", 3),
    WSubString(L"\x41\x323\x300", 3),
    WSubString(L"\x41\x1806\x31A\x20D4\xFE21\x32D\xF7B\x407B1\x6EA\x5B5", 10),

    // disappears chars
    WSubString(L"\x41\x300\x301\xAD", 4),
    WSubString(L"\x41\x300\x323\xAD", 4),
    WSubString(L"\x41\x323\x300\xAD", 4),
    WSubString(L"\xAD\x41\x300\x301", 4),
    WSubString(L"\xAD\x41\x300\x323", 4),
    WSubString(L"\xAD\x41\x323\x300", 4),
    WSubString(L"\xFE00\xAD\xFEFF", 3),
    WSubString(L"\xFE00\x41\x323\x300\xFEFF", 4),
    WSubString(L"\xFE00\x41\x300\x323\xFEFF", 4),
    WSubString(L"\x41\x323\xFE00\xFEFF\x300", 5),
    WSubString(L"\x41\x300\xFE00\xFEFF\x323", 5),

    WSubString(L"\xFE00\x41\x323\x41\xFEFF", 4),
    WSubString(L"\xFE00\x41\x323\xFEFF", 4),
    WSubString(L"\x41\x323\x41\xFEFF\x300", 5),
    WSubString(L"\x41\x300\xFE00\x41\x323", 5),
  };

}

void
do_decomposition_test() throw (eh::Exception)
{
  wchar_t result_buf[64];
  memset(result_buf, 0, sizeof(result_buf));
  std::cerr << std::hex << std::uppercase << std::setfill('0');
  for (wchar_t wch = 0; wch <= 0x10FFFD; ++wch)
  {
    wchar_t* end = decompose(wch, result_buf);
    *end = 0;
    // cannot check correctness of hangul decomposition
    if (wch >= 0xAC00 && wch <= 0xD7A3)
    {
      continue;
    }
    if (standard.find(wch) != standard.end())
    {
      const Mapping& mapping = standard[wch];
      if (wmemcmp(&mapping[0], result_buf, mapping.size()) ||
        result_buf[mapping.size()])
      {
        std::cerr << "Test fail: 1, wch=" << wch << ", result="
          << result_buf[0] << std::endl;
      }
      if (mapping.empty() && *result_buf)
      {
        std::cerr << "Test fail: 2, wch=" << wch << std::endl;
      }
    }
    else
    {
      if (wch != *result_buf && !result_buf[1])
      {
        std::cerr << "Test fail: 3 wch=" << wch << ", result="
          << result_buf[0] << ", next=" << result_buf[1] << std::endl;
      }
    }
  }
}

void
print_hangul_decomposition() throw (eh::Exception)
{
  wchar_t result_buf[64];
  Stream::FlagsSaver flags_saver(std::cout);
  std::cout << std::hex << std::uppercase << std::setfill('0');
  for (wchar_t wch = 0xAC00; wch <= 0xD7A3; ++wch)
  {
    wchar_t* end = decompose(wch, result_buf);
    *end = 0;
    std::cout << wch << " ---> ";
    for (wchar_t* ptr = result_buf; ptr != end; ++ptr)
    {
      std::cout << *ptr << " ";
    }
    std::cout << std::endl;
  }
}

void
do_get_canonical_test() throw (eh::Exception)
{
  const char FUN[] = "do_get_canonical_test(): ";
  for (wchar_t wch = 0; wch <= 0x10FFFD; ++wch)
  {
    if (std_canonical_map.find(wch) != std_canonical_map.end())
    {
      if (get_canonical_class(wch) != std_canonical_map[wch])
      {
        std::cerr << FUN << "failed, wch=" << wch <<
          ", get_canonical(wch)=" << get_canonical_class(wch)
          << std::endl;
      }
    }
    else
    {
      if (get_canonical_class(wch) != 0)
      {
        std::cerr << FUN << "failed, wch=" << wch <<
          ", get_canonical(wch) != 0, but = " << get_canonical_class(wch)
          << std::endl;
      }
    }
  }
}

/**
 * Get some properties of starters and combiners decomposition
 */
void
do_test_properties() throw (eh::Exception)
{
  wchar_t result[64];
  std::cout << std::hex << std::uppercase;
  for (wchar_t wch = 0; wch <= 0x10FFFF; ++wch)
  {
    result[0] = 0;
    wchar_t* end = decompose(wch, result);
    if (get_canonical_class(*result) != 0 && wch != *result)
    {
//      std::cout << "Decomposition started from !=0 combining class, wch = "
//      << wch << std::endl;
    }
    if (wch != *result)
    {
      bool have_starter = false;
      for (wchar_t* ptr = result; ptr != end; ++ptr)
      {
        if (get_canonical_class(*ptr) == 0)
        {
          have_starter = true;
        }
      }
      if (!have_starter)
      {
        std::cout << "wch=" << wch << ", do not have starter" << std::endl;
      }
    }
  }
  for (wchar_t wch = 0; wch <= UnicodeSymbol::MAX_CODE_UNIT; ++wch)
  {
    result[0] = 0;
    /*wchar_t* end = */decompose(wch, result);
    if (get_canonical_class(wch) == 0 && get_canonical_class(*result) != 0)
    {
      std::cout << "Starter decomposed doesn't started from zero " << wch
        << std::endl;
    }
  }
  for (UnicodeSymbol sym(L'\0'); sym < UnicodeSymbol::MAX_CODE_UNIT; ++sym)
  {
    wchar_t wch = sym;
    result[0] = 0;
    wchar_t* end = decompose(wch, result);
    if (end != result && get_canonical_class(wch) != 0 &&
      get_canonical_class(*result) == 0)
    {
      std::cout << "Combine decomposed started from started " << wch
        << std::endl;
    }

    if (get_canonical_class(wch) != 0)
    {
      bool have_starter = false;
      for (wchar_t* ptr = result; ptr != end; ++ptr)
      {
        if (get_canonical_class(*ptr) == 0)
        {
          have_starter = true;
        }
      }
      if (have_starter)
      {
        std::cout << "wch=" << wch << ", have starter" << std::endl;
      }
    }
  }
}

bool
is_ordered(const WSubString& sample, wchar_t* result, wchar_t* endof_result)
  throw (eh::Exception)
{
  unsigned int previous_class = 0, current_class;
  for (wchar_t* ptr = result; ptr != endof_result; ++ptr)
  {
    current_class = get_canonical_class(*ptr);
    if (current_class != 0 && previous_class > current_class)
    {
      std::cerr << "Sort failed, " << endof_result - result
        << ", prev=" << previous_class << ", curr=" << current_class
        << ", offset=" << ptr - result << std::endl;

      for (std::size_t i = 0; i < sample.size(); ++i)
      {
        std::cerr << static_cast<unsigned>(sample[i]) << ",";
      }
      std::cerr << std::endl << "Result:" << std::endl;
      for (wchar_t* ptr = result; ptr != endof_result; ++ptr)
      {
        std::cerr << static_cast<unsigned>(*ptr) << ",";
      }
      std::cerr << std::endl;
      return false;
    }
    previous_class = current_class;
  }
  return true;
}

void
do_canonical_order_test() throw (eh::Exception)
{
  const char FUN[] = "do_canonical_order_test(): ";
  wchar_t result[2048];
  std::cerr << std::hex << std::uppercase;

  for (std::size_t i = 0; i < sizeof(SAMPLES) / sizeof(SAMPLES[0]); ++i)
  {
    wchar_t* endof_result =
      normalize(SAMPLES[i].begin(), SAMPLES[i].end(), result);
    is_ordered(SAMPLES[i], result, endof_result);
  }
  std::cout << FUN << "Text corpus checked" << std::endl;

  for (std::size_t i = 0; i < 100000; ++i)
  {
    wchar_t sample[64];
    // fill sample by (combiner, random wchar) pairs
    for (std::size_t i = 0; i < sizeof(sample) / sizeof(wchar_t); ++i)
    {
      sample[i] = (i % 2) ? std_canonical_set.at(
        Generics::safe_rand(0, std_canonical_set.size() - 1)) :
          static_cast<wchar_t>(UnicodeSymbol::random());
    }
    wchar_t* endof_result =
      normalize(sample, sample + sizeof(sample) / sizeof(wchar_t), result);
    if (endof_result)
    {
      is_ordered(WSubString(sample, sizeof(sample) / sizeof(sample[0])),
        result, endof_result);
    }
  }
  std::cout << FUN << "complete" << std::endl;
}

void
do_canonical_perf_test() throw (eh::Exception)
{
  const char FUN[] = "do_canonical_perf_test(): ";
  std::cout << FUN << "started.." << std::endl;
  for (wchar_t wch = 0; wch <= 0x10FFFF; ++wch)
  {
    if ((get_canonical_class(wch) != 0) != get_NZ_canonical_class(wch))
    {
      std::cerr << FUN << "failed, wch=" << wch;
      if (get_canonical_class(wch))
      {
        std::cerr << ", not zero CC, NZ_CC=" << get_NZ_canonical_class(wch);
      }
      else
      {
        std::cerr << ", CC=0, NZ_CC" << get_NZ_canonical_class(wch);
      }
      std::cerr << std::endl;
    }
  }

  wchar_t BUF[10240];
  for (std::size_t i = 0; i < sizeof(BUF) / sizeof(BUF[0]); ++i)
  {
    BUF[i] = Generics::safe_rand(0x30C0);
  }

  const std::size_t MEASURE_COUNT = 100000;
  Generics::CPUTimer timer;
  timer.start();
  for (std::size_t j = 0; j < MEASURE_COUNT; ++j)
  {
    for (std::size_t i = 0; i < sizeof(BUF) / sizeof(BUF[0]); ++i)
    {
      get_NZ_canonical_class(BUF[i]);
    }
  }
  timer.stop();
  std::cout << FUN << "Compressed function: " << timer.elapsed_time()
    << std::endl;
  timer.start();
  for (std::size_t j = 0; j < MEASURE_COUNT; ++j)
  {
    for (std::size_t i = 0; i < sizeof(BUF) / sizeof(BUF[0]); ++i)
    {
      //get_canonical_class(BUF[i]) != 0;
    }
  }
  timer.stop();
  std::cout << FUN << "Full function: " << timer.elapsed_time()
    << std::endl;

}

unsigned short
hash(wchar_t starter, wchar_t combiner) throw ()
{
  // 0..5, 16..20, 22..24
  // combiner 31..16, starter 15..0
  // 1^25, 21^5^26,

  combiner ^= ((starter >> 8) & 2);  // 1^25
  starter ^= (combiner & (1 << 5)); // 21^5
  starter ^= ((starter >> 5) & (1 << 5)); // 21^26
  combiner &= 0x1F;     //   1F = 00011111       5
  return combiner | ((starter & 0x1FF ) << 5); // 0..5, 16..20,   1F = 0011111     5
}

void
do_hash_test() throw (eh::Exception)
{
  const char FUN[] = "do_hash_test(): ";
  typedef std::set<unsigned short> UniqCheck;
  UniqCheck uset;

  for (ComposeMap::const_iterator cit = std_compose_map.begin();
    cit != std_compose_map.end(); ++cit)
  {
    uset.insert(hash(cit->first.first, cit->first.second));
  }
  (uset.size() == std_compose_map.size() ? std::cout : std::cerr)
    << FUN << "Hashed points=" << std::dec << uset.size()
    << ", points into mapping=" << std_compose_map.size() << std::endl;
}

inline bool
is_composed(wchar_t* first, wchar_t* last, wchar_t* next)
{
  return next <= last && first + 1 == next;
}

void
do_composition_test() throw (eh::Exception)
{
  const char FUN[] = "do_composition_test(): ";
  wchar_t buf[2] = {L'\x003C', L'\x0338'};
  std::cout << std::hex << std::uppercase << FUN << "started"
    << std::endl;

  // alone test
  std::cout << buf[0] << " " << buf[1] << std::endl;
  wchar_t* src= buf;
  wchar_t *next = compose(src, &(buf[2]), src);
  std::cout << buf[0] << " " << buf[1] << std::endl;
  bool composed = is_composed(buf, &(buf[2]), next);
  std::cout << (composed ? "YES: " : "NO: ") << buf[0]
    << " " << buf[1] << std::endl;
//  return;

  // complete test, all possible pair
  std::size_t counter = 0;

  for (wchar_t wch = 0; wch < 0x3400; ++wch)
  {
    for (wchar_t comb = 0; comb < 0x3400; ++comb)
    {
      buf[0] = wch;
      buf[1] = comb;
      wchar_t* src= buf;
      wchar_t *next = compose(src, &(buf[2]), src);
      if (src < &buf[2])
      {
        next = compose(src, &(buf[2]), next);
      }
      bool composed = is_composed(buf, &(buf[2]), next);

      if (composed)
      {
        const wchar_t L_BASE = 0x1100;
        const int L_COUNT = 19;
        const wchar_t S_BASE = 0xAC00;
        const int S_COUNT = 11172;
        if ((wch >= L_BASE && wch <= L_BASE + L_COUNT) ||
          (wch >= S_BASE && wch <= S_BASE + S_COUNT))
        {
          // exclude Hangul, from checking
          continue;
        }
        ComposeMap::iterator cit =
          std_compose_map.find(ComposeArgument(wch, comb));
        if (cit == std_compose_map.end())
        {
          std::cerr << FUN << "calculated composite not found in standard"
            << std::endl;
          std::cerr << std::hex << std::uppercase
            << "starter=" << wch << ", combiner=" << comb
            << ", result=" << *buf << std::endl;
        }
        else
        {
//          std_compose_map.erase(cit);
          counter++;
        }
      }
    }
  }
  if (counter != std_compose_map.size())
  {
    std::cerr << FUN << "not all standard composite calculated, only "
      << counter << std::endl;
/*    for (ComposeMap::iterator it = std_compose_map.begin();
        it != std_compose_map.end(); ++it)
    {
      std::cout << it->first.first << " " << it->first.second << std::endl;
    }*/
  }
  else
  {
    std::cout << FUN << "composites matched" << std::endl;
  }
}

void
do_composition_string_test() throw (eh::Exception)
{
  const char FUN[] = "do_composition_string_test(): ";
  std::cout << FUN << "started" << std::endl;
  wchar_t wstr[10000];

  // check compose safety on random input
  for (std::size_t i = 0; i < 1000; ++i)
  {
    for (std::size_t j = 0; j < 1000; ++j)
    {
      wstr[j] = Generics::safe_rand();
    }
    compose_string(wstr, wstr + 1000);
  }
  std::cout << std::hex << std::uppercase;
  std::size_t counter = 0;
  wchar_t standard[std_compose_map.size()];
  // put all composable pairs into wstr
  // then compose wstr, and await for corresponding result
  // for each pair.
  for (ComposeMap::iterator it = std_compose_map.begin();
    it != std_compose_map.end();
    ++it)
  {
    standard[counter / 2] = it->second;
    wstr[counter++] = it->first.first;
    wstr[counter++] = it->first.second;
  }
  compose_string(wstr, wstr + counter);
  ComposeMap::iterator it = std_compose_map.begin();
  for (std::size_t i = 0; i < std_compose_map.size(); ++i, ++it)
  {
    if (standard[i] != wstr[i])
    {
      std::cerr << std::hex << "Not standard composite at " << i
        << "position\nAwaiting " << standard[i] << ", really "
        << wstr[i] << std::endl;
      break;
    }
  }

  std::cout << FUN << "done" << std::endl;
}

std::wstring
fold_normalize(const std::wstring& wstr) throw (eh::Exception)
{
  std::string utf8;
  StringManip::wchar_to_utf8(wstr.c_str(), utf8);
  std::string lower_utf8;
  case_change<Uniform>(utf8, lower_utf8);
  return (StringManip::utf8_to_wchar(lower_utf8.c_str())).get();
}

#if 0
void
do_full_test() throw (eh::Exception)
{
  const char FUN[] = "do_full_test(): ";
  std::cout << FUN << "started" << std::endl;

  wchar_t wstr[10000] =
  {
    0x0F73, //0x0CCB, //0x323,// 0x323
  };

  std::wstring result;
  lower_and_normalize(wstr, wstr + 1, result);
  std::cout << "Simple check result:";
  print(result);
  std::cout << std::endl;

  for (std::size_t i = 0; i < 1000; ++i)
  {
    for (std::size_t j = 0; j < 1000; ++j)
    {
      wstr[j] = Generics::safe_rand();
    }
    lower_and_normalize(wstr, wstr + 1000, result);
  }

  std::size_t fails_counter = 0;

  ConformanceData* parts[3] =
  {
    &std_conformance_data_part0,
    &std_conformance_data_part1,
    &std_conformance_data_part2
  };

  for (std::size_t j = 0; j < 3; ++j)
  {
    std::cout << "Part " << j << " is testing..." << std::endl;
    for (ConformanceData::const_iterator cit = parts[j]->begin();
      cit != parts[j]->end(); ++cit)
    {
      lower_and_normalize(cit->input.data(),
        cit->input.data() + cit->input.size(), result);
      //    case_change<Uniform>
      if (result != fold_normalize(cit->NFKC))
      {
        std::cerr << "\nNon conformed realization: s1="
          << result.size() << ", s2=" << cit->NFKC.size()
          << ", input=";
        print(cit->input);
        std::cerr << std::endl << "result=";
        print(result);
        std::cerr << std::endl << "folded standard=";
        print(fold_normalize(cit->NFKC));
        std::cerr << std::endl << "ORIGINAL standard=";
        print(cit->NFKC);
        std::cerr << std::endl;
        fails_counter++;
      }
    }
  }

  std::cout << FUN << "total fails=" << fails_counter << std::endl;
  std::cout << FUN << "done" << std::endl;
}
#endif

int
main()
{
  try
  {
    std::cout << std::hex << std::uppercase;
    do_decomposition_test();
    do_get_canonical_test();
    do_canonical_order_test();
//    do_canonical_perf_test();
    do_hash_test();
//    do_composition_test();
    do_composition_string_test();
    //do_full_test();
//    do_test_properties();

    std::size_t total_memory =
      sizeof(MAPPING_INDEX_0000_33FE) +
      sizeof(MAPPING_BODY_0000_33FE) +
      sizeof(MAPPING_INDEX_F800_FFFF) +
      sizeof(MAPPING_BODY_F800_FFFF) +
      sizeof(MAPPING_10400_1043F) +
      sizeof(MAPPING_INDEX_1D100_1D1FF) +
      sizeof(MAPPING_BODY_1D100_1D1FF) +
      sizeof(MAPPING_1D400_1D7FF) +
      sizeof(MAPPING_2F800_2FBFF);

    std::cout << "SIZE=" << std::dec << total_memory << std::endl;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "Exception raised: " << ex.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unknown exception raised" << std::endl;
    return 1;
  }

  return 0;
}
#else
int
main()
{
  return 0;
}
#endif
