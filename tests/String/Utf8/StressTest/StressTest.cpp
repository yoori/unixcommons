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



// StressTest.cpp : Defines the entry point for the console application.
// Test UTF-8 incorrect usage issues
// We test stop parser on incorrect UTF-8 texts. And some extreme cases..

#include <sstream>
#include <fstream>
#include <iostream>
#include <map>
#include <tr1/array>

#include <eh/Exception.hpp>
#include <Generics/Rand.hpp>

#include <String/StringManip.hpp>
#include <String/UTF8Case.hpp>
#include <String/UnicodeSymbol.hpp>
#include <Stream/BzlibStreams.hpp>
#include "../Common/UTF8TreeLoader.hpp"

DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

using namespace String;

class TestContext
{
public:

  template<typename Arg1, typename Arg2>
  void
  check_equal(const Arg1& a, const Arg2& b) const throw (Exception);

  template<typename Arg1, typename Arg2>
  void
  test_equal(const Arg1& a, const Arg2& b) const throw (eh::Exception);

  void
  set_operation(const char* name) throw (eh::Exception);
  void
  set_operation(const String::SubString& name) throw (eh::Exception);

private:
  std::string operation_;
} test_context;

#include "StressTest.hpp"

template<typename Arg1, typename Arg2>
void
TestContext::check_equal(const Arg1& a, const Arg2& b) const
  throw (Exception)
{
  if (a != b)
  {
    Stream::Error ostr;
    ostr << "Opfail: " << operation_ << ". '"<< a << "' != '" << b << "'";
    throw Exception(ostr);
  }
}

template<typename Arg1, typename Arg2>
void
TestContext::test_equal(const Arg1& a, const Arg2& b) const
  throw (eh::Exception)
{
  if (a != b)
  {
    std::cerr << "Opfail: " << operation_ << ". '"<< a << "' != '" << b <<
      "'" << std::endl;
  }
}

void
TestContext::set_operation(const char* name) throw (eh::Exception)
{
  operation_ = name;
}

void
TestContext::set_operation(const String::SubString& name) throw (eh::Exception)
{
  name.assign_to(operation_);
}

bool
load_file(const char* fn, std::string& result) throw (eh::Exception)
{
  try
  {
    Stream::BzlibInStream ifs(fn);

    std::getline(ifs, result, '\0'); // read reason phrase

    std::cout << "File " << fn << " loaded. Size=" << result.size()
      << std::endl;
    return true;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "File " << fn << " open error. "
      << ex.what() << std::endl;
    return false;
  }
}

struct Replacement
{
  std::string str;
  size_t symbols;
};
typedef std::map<UnicodeSymbol, Replacement> Utf8Dictionary;

void
load_reflections(const char* fn, Utf8Dictionary& dict)
  throw (eh::Exception)
{
  std::ifstream ifs(fn);
  if (!ifs.good())
  {
    Stream::Error ostr;
    ostr << "File " << fn << " open error";
    throw Exception(ostr);
  }
  char line[1024];
  for (int ln = 1; !ifs.eof(); ln++)
  {
    ifs.getline(line, 1024, '\n');
    if (line[0] == '#' || line[0] == '\0' ||
      line[0] == '\n' || line[0] == '\r')
    {
      continue;
    }
    Stream::Parser sstr(line);
    UnicodeSymbol first;
    Replacement second;
    second.symbols = 0;
    sstr >> first;
    if (!sstr)
    {
      std::cerr << fn << ":" << ln << ": failed to read UnicodeSymbol";
      continue;
    }
    for (;;)
    {
      UnicodeSymbol tmp;
      sstr >> tmp;
      if (!sstr)
      {
        break;
      }
      second.str.append(tmp.c_str());
      second.symbols++;
    }
    if (second.str.empty())
    {
      continue; // or failed on broken file ?
    }
    // todo: we should use values ascending and boost time of inserts
    if (!dict.insert(Utf8Dictionary::value_type(first, second)).second)
    {
      std::cerr << "File " << fn << " contain content errors" << std::endl;
      return;
    }
  }
#if 0
  std::ofstream ofs((std::string(fn) + ".tmp").c_str());
  for (Utf8Dictionary::const_iterator itor(dict.begin()); itor != dict.end();
    ++itor)
  {
    ofs << itor->first << " " << itor->second << "\n";
  }
#endif
}

void
load_reflections2(const char* fn, Utf8Dictionary& dict)
  throw (eh::Exception)
{
  std::ifstream ifs(fn);
  if (!ifs.good())
  {
    Stream::Error ostr;
    ostr << "File " << fn << " open error";
    throw Exception(ostr);
  }
  char line[1024];
  for (int ln = 1; !ifs.eof(); ln++)
  {
    ifs.getline(line, 1024, '\n');
    if (line[0] == '#' || line[0] == '\0' ||
      line[0] == '\n' || line[0] == '\r')
    {
      continue;
    }
    Replacement repl;
    UnicodeSymbol first, second;
    bool has_second = false;
    std::string str;

    repl.symbols = 0;

    Stream::Parser sstr(line);
    {
      sstr >> str;
      if (!sstr)
      {
        std::cerr << fn << ":" << ln << ": failed to read a range" <<
          std::endl;
        continue;
      }
      Stream::Parser sstr2(str);
      sstr2 >> first;
      if (!sstr2)
      {
        std::cerr << fn << ":" << ln << ": failed to read UnicodeSymbol" <<
          std::endl;
        continue;
      }
      char ch = ' ';
      sstr2 >> ch;
      if (sstr2)
      {
        if (ch != '-')
        {
          std::cerr << fn << ":" << ln << ": failed to read range symbol" <<
            std::endl;
          continue;
        }
        sstr2 >> second;
        if (!sstr2)
        {
          std::cerr << fn << ":" << ln <<
            ": failed to read second UnicodeSymbol" << std::endl;
          continue;
        }
        has_second = true;
      }
    }

    {
      std::getline(sstr, str);
      String::StringManip::trim(str, str);
      if (!sstr || str.empty())
      {
        std::cerr << fn << ":" << ln << ": failed to read a replacement" <<
          std::endl;
        continue;
      }
      switch (str[0])
      {
      case '/':
        break;
      case '+':
        repl.str = ' ';
        repl.symbols = 1;
        break;
      case '*':
        continue;
      default:
        Stream::Parser sstr2(str);
        for (;;)
        {
          UnicodeSymbol tmp;
          sstr2 >> tmp;
          if (!sstr2)
          {
            break;
          }
          repl.str.append(tmp.c_str());
          repl.symbols++;
        }
      }
    }
    if (!dict.insert(Utf8Dictionary::value_type(first, repl)).second)
    {
      std::cerr << fn << ":" << ln << " contain content errors" << std::endl;
      return;
    }
    if (has_second)
    {
      for (;;)
      {
        ++first;
        if (!dict.insert(Utf8Dictionary::value_type(first, repl)).second)
        {
          std::cerr << fn << ":" << ln << " contain content errors" << std::endl;
          return;
        }
        if (first == second)
        {
          break;
        }
      }
    }
  }

  for (wchar_t ch = 0xAC00; ch <= 0xD7A3; ch++)
  {
    Replacement val;
    val.symbols = 3;
    val.str.resize(9);
    unsigned n = ch - 0xAC00;
    unsigned l = n / 588;
    unsigned v = (n % 588) / 28;
    unsigned t = n % 28;
    unsigned long octets;
    String::UTF8Handler::wchar_to_utf8_char(0x1100 + l, &val.str[0],
      octets);
    String::UTF8Handler::wchar_to_utf8_char(0x1161 + v, &val.str[3],
      octets);
    if (t)
    {
      String::UTF8Handler::wchar_to_utf8_char(0x11A7 + t, &val.str[6],
        octets);
    }
    else
    {
      val.symbols = 2;
      val.str.resize(6);
    }
    dict[ch] = val;
  }
#if 0
  std::ofstream ofs((std::string(fn) + ".tmp").c_str());
  for (Utf8Dictionary::const_iterator itor(dict.begin()); itor != dict.end();
    ++itor)
  {
    ofs << itor->first << " " << itor->second << "\n";
  }
#endif
}

void
stress_test() throw (eh::Exception)
{
  using namespace Generics;

  std::string destination;
  test_context.set_operation("Standards checking");
  for (std::size_t i = 0; i < 4; ++i)
  {
    case_change<Upper>(String::SubString(tds[i].sample), destination);
    test_context.check_equal(destination, tds[i].sample_upper);
  }
  const char Eth[] = "C-buffer.";
  const char EthL[] = "c-buffer.";
  const char EthU[] = "C-BUFFER.";

  // C buffers testing

  char buf[256];
  std::copy(static_cast<const char*>(Eth),
    static_cast<const char*>(Eth)+sizeof(Eth), static_cast<char*>(buf));
  using namespace String;

  test_context.set_operation("case_change<Uniform>");
  case_change<Uniform>(String::SubString(), destination);
  test_context.check_equal(destination, "");
  case_change<Uniform>(String::SubString(&buf[0], 1u), destination);
  test_context.check_equal(destination, "c");
  case_change<Uniform>(String::SubString(&buf[0], 3u), destination);
  test_context.check_equal(destination, "c-b");
  case_change<Uniform>(String::SubString(&buf[0], 4u), destination);
  test_context.check_equal(destination, "c-bu");
  case_change<Uniform>(String::SubString(&buf[0]), destination);
  test_context.check_equal(destination, std::string(EthL));
  case_change<Upper>(String::SubString(buf), destination);
  test_context.check_equal(destination, EthU);
  strcpy(buf, destination.c_str());
  case_change<Uniform>(String::SubString(buf), destination);
  test_context.check_equal(destination, EthL);

  const char bad_octet[] = {'\xf4', '\x8f', '\0'};   // broken utf8 octet.
  char b2[] = {'\xf4', '\x8f', '\0'};                // broken utf8 octet, mutable

  std::string invalid = std::string(Eth) + bad_octet + "rest string";

  test_context.check_equal(
    case_change<Uniform>(String::SubString(b2), destination), false);
  test_context.check_equal(strcmp(bad_octet, b2), 0);

  // transform methods stop on bad utf8 octet. Check this feature,
  // and it give guarantees that buffer overrun impossible.
  std::string sdest;

  sdest = ""; case_change<Uniform>(invalid, sdest);
  test_context.check_equal(EthL, sdest);
}

void
ill_formed_test() throw (eh::Exception)
{
  using namespace String;
  std::string z;
  load_file((root_path + "bad_UTF8_octets.txt.bz2").c_str(), z);
  if (z.size() < 3)
  {
    return;
  }

  //using namespace Generics;
  std::string sL;
  const char* end = z.data() + z.size();

  for (const char* cit = &z[3],* itn = cit;
      itn != end;
      ++itn, cit = itn)
  {
    while (itn != end && *itn != '\n')
    {
      ++itn;
    }
    test_context.set_operation("case_change<Uniform>(cit, itn, sL)");
    test_context.test_equal(
      case_change<Uniform>(String::SubString(cit, itn), sL), false);
    test_context.set_operation("case_change<Lower>(cit, itn, sL)");
    test_context.test_equal(
      case_change<Lower>(String::SubString(cit, itn), sL), false);
    test_context.set_operation("case_change<Upper>(cit, itn, sL)");
    test_context.test_equal(
      case_change<Upper>(String::SubString(cit, itn), sL), false);

    if (itn == end)
    {
      break;
    }
  }
}

template <typename Action>
void
one_utf8_check(const char* operation, const UnicodeSymbol& symbol,
  const std::string& symbol_str, const Utf8Dictionary& dict,
    bool check_counter = true)
  throw (eh::Exception)
{
  std::string result;
  Utf8Dictionary::const_iterator looked_up_value;

  Stream::Dynamic ostr;
  char buf[32];
  snprintf(buf, sizeof(buf), "U+%04X",
    static_cast<unsigned>(static_cast<wchar_t>(symbol)));
  ostr << operation << "(" << symbol_str << ") " << buf;
  test_context.set_operation(ostr.str());
  size_t counter;
  test_context.test_equal(true,
    case_change<Action>(symbol_str, result, &counter));
  looked_up_value = dict.find(symbol);
  if (looked_up_value == dict.end())
  {
    test_context.test_equal(symbol_str, result);
    if (check_counter)
    {
      test_context.test_equal(1u, counter);
    }
  }
  else
  {
    test_context.test_equal(looked_up_value->second.str, result);
    if (check_counter)
    {
      test_context.test_equal(looked_up_value->second.symbols, counter);
    }
  }
}

void
all_utf8_space_test() throw (eh::Exception)
{
  using namespace String;
  Utf8Dictionary dict_to_lower;
  Utf8Dictionary dict_to_upper;
  Utf8Dictionary dict_to_uniform;
  Utf8Dictionary dict_simplify;

  load_reflections((root_path + "to_lower.txt").c_str(), dict_to_lower);
  load_reflections((root_path + "to_upper.txt").c_str(), dict_to_upper);
  load_reflections((root_path + "to_uniform.txt").c_str(), dict_to_uniform);
  load_reflections2((root_path + "simplify.txt").c_str(), dict_simplify);

  UnicodeSymbol symbol(L'\0');
  std::string symbol_str;
  const UnicodeSymbol LAST("\xF4\x8F\xBF\xBF");

  for (; symbol <= LAST; ++symbol)
  {
    symbol_str = symbol.c_str();
    if (symbol_str.empty())
    {
      symbol_str.push_back('\0');
    }
    // uniform
    one_utf8_check<Uniform>("case_change<Uniform>", symbol, symbol_str,
      dict_to_uniform);
    // lower
    one_utf8_check<Lower>("case_change<Lower>", symbol, symbol_str,
      dict_to_lower);
    // upper
    one_utf8_check<Upper>("case_change<Upper>", symbol, symbol_str,
      dict_to_upper);
    // simplify
    one_utf8_check<Simplify>("case_change<Simplify>", symbol, symbol_str,
      dict_simplify, false);
  }
}

void
random_buffer_test() throw (eh::Exception)
{
  typedef std::tr1::array<char, 100> data_type;
  data_type buf;
  using namespace String;
  std::cout << "Random test started" << std::endl;
  std::string res;
  for (std::size_t i = 0; i < 100000; ++i)
  {
    // gcc bug, below compile but core dumped (doesn't work), investigate...
    //      std::for_each(buf.begin(), buf.end(), random_byte);// randomize buffer
    for (std::size_t i = 0; i < buf.size(); ++i)
    {
      do
      {
        buf[i] = Generics::safe_rand(257);
      }
      while (buf[i]==13 || buf[i] == 10);
    }
    buf.back() = 0;
    case_change<Lower>(String::SubString(buf.data()), res);
    case_change<Uniform>(String::SubString(buf.data()), res);
    case_change<Upper>(String::SubString(buf.data()), res);
    case_change<Simplify>(String::SubString(buf.data()), res);
  }
  std::cout << "Random test done" << std::endl;
}


int
main()
{
  std::cout << "Functional and stress UTF-8 API test started...\n";
  char* ev = getenv("TEST_TOP_SRC_DIR");
  root_path = ev ? ev : ".";
  root_path += "/tests/String/Utf8/Data/";
  try
  {
    stress_test();
    ill_formed_test();
    all_utf8_space_test();
    random_buffer_test();
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "Exception raised: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception occurred" << std::endl;
  }

  return 0;
}
