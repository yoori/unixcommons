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



// PerformanceTest.cpp :
//   Defines the entry point for the test console application.
//

#include <iostream>
#include <iomanip>

#include <Generics/Uncopyable.hpp>
#include <Generics/Time.hpp>
#include <String/UTF8Case.hpp>
#include <String/UTF8Handler.hpp>
#include <String/UTF8IsProperty.hpp>

#include "Additional.hpp"

namespace
{
  DECLARE_EXCEPTION(TestException, eh::DescriptiveException);

  size_t RepetitionCount = 5000;
  std::string str;    // input data storage
  std::string sstr;   // operational space

  const char ascii[] = "ascii text string @#$%^12345";
  const char utf8_2bytes[] = "Текст с 2-х байтными октетами.";
  const char utf8_3bytes[] = "전세계의호텔전세계의호텔전세계의호텔";
  const char ut8_multilang[] = "전세계의호텔 Стартовая строка English "
                               "امودود ЛВВЫРАЛЫ 13122125234378";

  const char* const TEXT_CORPUS[] =
  {
    // ASCII text 1 byte into code point sequence
    "The multiply operation creates five separate copies"
    " of the 8-bit byte pattern to fan-out into a 64-bit"
    " value. The AND operation selects the bits that are"
    " in the correct (reversed) positions, relative to "
    "each 10-bit groups of bits. The multiply and the AND"
    " operations copy the bits from the original byte so "
    "they each appear in only one of the 10-bit sets. The"
    " reversed positions of the bits from the original "
    "byte coincide with their relative positions within "
    "any 10-bit set. The last step, which involves modulus"
    "division by 2^10 - 1, has the effect of merging "
    "together each set of 10 bits (from positions 0-9, "
    "10-19, 20-29, ...) in the 64-bit value. They do not "
    "overlap, so the addition steps underlying the modulus"
    " division behave like or operations.",
    // Russian text 2 bytes into code point sequence
    "Алгебра событий (в теории вероятностей) — алгебра "
    "подмножеств пространства элементарных событий Ω, "
    "элементами которого служат элементарные события. "
    "Как и положено алгебре множеств алгебра событий "
    "содержит невозможное событие (пустое множество) "
    "и замкнута относительно теоретико-множественных "
    "операций, производимых в конечном числе. Достаточно"
    " потребовать, чтобы алгебра событий была замкнута "
    "относительно",
    // Korean text, 3 bytes into code point sequence
    "툴바에 버튼을 추가하여 원하는 사이트를 검색하거나 "
    "뉴스 헤드라인을 훑어보십시오. 갤러리에서 버튼을 "
    "선택할 수 있습니다. 사용자만의 버튼도 간단히 만들"
    " 수 있습니다. 홈페이지에 시계를 추가하세요. 색상을"
    " 변경하려면 '편집'을 클릭합니다. 여러 탭에 다른 "
    "뉴스 섹션을 표시하는. 툴바 4 및 버튼 갤러리가 곧 "
    "여러 개의 언어로 제공됩니다. 웹 사이트가 여러 언어를"
    " 지원하는 경우 버튼 XML 파일에서 해당 언어로 된 버튼"
    " 제목 및 설명을 제공할 수 있습니다. 각 제목 및 설명에"
    " 대한 언어를 지정하려면 각 <title> 및 <description> "
    "태그에 언어 속성을 포함시키십시오.",
    // Deseret text, 4 bytes into code point sequence
    "𐐙𐐲𐑌𐐼𐐲𐑋𐐯𐑌𐐻𐐲𐑊𐐨, 𐐿𐐲𐑋𐐹𐐷𐐭𐐻𐑉𐑆 𐐾𐐲𐑅𐐻 𐐼𐐨𐑊 𐐶𐐮𐑃"
    " 𐑌𐐲𐑋𐐺𐑉𐑆. 𐐜𐐩 𐑅𐐻𐐬𐑉 𐑊𐐯𐐻𐑉𐑆 𐐰𐑌𐐼 𐐲𐑄𐑉 𐐿𐐯𐑉𐐲𐐿𐐻𐑉𐑆"
    " 𐐺𐐴 𐐲𐑅𐐴𐑌𐐨𐑍 𐐪 𐑌𐐲𐑋𐐺𐑉 𐑁𐐬𐑉 𐐨𐐽 𐐶𐐲𐑌. 𐐒𐐨𐑁𐐬𐑉 "
    "𐐏𐐭𐑌𐐮𐐿𐐬𐐼 𐐶𐐲𐑆 𐐮𐑌𐑂𐐯𐑌𐐻𐐲𐐼, 𐑄𐐯𐑉 𐐶𐐲𐑉 𐐸𐐲𐑌𐐼𐑉𐐯𐐼𐑆"
    " 𐐲𐑂 𐐼𐐮𐑁𐑉𐐲𐑌𐐻 𐐯𐑌𐐿𐐬𐐼𐐨𐑍 𐑅𐐮𐑅𐐻𐐲𐑋𐑆 𐑁𐐬𐑉 𐐲𐑅𐐴𐑌𐐨𐑍 "
    "𐑄𐐨𐑆 𐑌𐐲𐑋𐐺𐑉𐑆. 𐐤𐐬 𐑅𐐨𐑍𐑊 𐐯𐑌𐐿𐐬𐐼𐐨𐑍 𐐿𐐳𐐼",
    // Synthetic text, 4 bytes into code point representation
    "𚐜𚐝𚐞𚐟𚐠𚐡𚐢𚐣𚐤𚐥𚐦𚐧𚐨𚐩𚐪𚐫𚐬𚐭𚐮𚐯𚐰𚐱𚐲𚐳𚐴𚐵𚐶𚐷𚐸𚐹𚐺𚐻𚐼𚐽"
    "𚐾𚐿𚑀𚑁𚑂𚑃𚑄𚑅𚑆𚑇𚑈𚑉𚑊𚑋𚑌𚑍𚑎𚑏𚑐𚑑𚑒𚑓𚑔𚑕𚑖𚑗𚑘𚑙𚑚𚑛𚑜𚑝𚑞𚑟"
    "𚑠𚑡𚑢𚑣𚑤𚑥𚑦𚑧𚑨𚑩𚑪𚑫𚑬𚑭𚑮𚑯𚑰𚑱𚑲𚑳𚑴𚑵𚑶𚑷𚑸𚑹𚑺𚑻𚑼𚑽𚑾𚑿𚒀𚒁"
    "𚒂𚒃𚒄𚒅𚒆𚒇𚒈𚒉𚒊𚒋𚒌𚒍𚒎𚒏𚒐𚒑𚒒𚒓𚒔𚒕𚒖𚒗𚒘𚒙𚒚𚒛𚒜𚒝𚒞𚒟𚒠𚒡𚒢𚒣"
    "𚒤𚒥𚒦𚒧𚒨𚒩𚒪𚒫𚒬𚒭𚒮𚒯𚒰𚒱𚒲𚒳𚒴𚒵𚒶𚒷𚒸𚒹𚒺𚒻𚒼𚒽𚒾",
//    0

  };

  class TestContext : Generics::Uncopyable
  {
  public:
    template<typename Arg1, typename Arg2>
    void
    test_equal(const Arg1& a, const Arg2& b) const throw (eh::Exception);

    void
    set_operation(const char* name) throw (eh::Exception);

    void
    set_operand(const char* source_string) throw (eh::Exception);
  private:
    std::string operation_;
    std::string operand_;
  } test_context;


  typedef bool PropertyFun(const char*);

  class UTF8IsPropertyOnStringPerformance
  {
  public:
    long long
    operator () () const throw (eh::Exception);

    UTF8IsPropertyOnStringPerformance(const char* str, PropertyFun f)
      throw (TestException);
  private:
    const char* ORIGINAL_STRING_;
    PropertyFun* function_;
  };

  typedef std::size_t CountingFunction(char);

  class CountingPerformanceFunctor
  {
  public:
    long long
    operator () () const throw (eh::Exception);

    CountingPerformanceFunctor(CountingFunction f) throw ();
  private:
    CountingFunction* function_;
  };

  // context object
  typedef void (*Measure)(std::string &result);

  // Functor for profiling
  class ProfFunctor
  {
  public:
    ProfFunctor(Measure pf, std::string& result)
      throw (eh::Exception);

    long long
    operator () () throw (eh::Exception);
  private:
    Measure measuring_functor_;
    std::string& result_;
    Generics::CPUTimer timer_;
  };

  struct CountingTestCase
  {
    CountingFunction* checking_call;
    const char NAME[32];
  };

  CountingTestCase counting_test_cases[] =
  {
    {&Test::get_octet_count_if, "get_octet_count_if"},
    {&Test::get_octet_count_inside_static, "get_octet_count_inside_static"},
    {&Test::get_octet_count_inside, "get_octet_count_inside"},
    {&Test::get_octet_count_outdoor, "get_octet_count_outdoor"},
  };

  typedef void ElementalTestingFunction(std::string&);

  struct SingleTestCase
  {
    ElementalTestingFunction* checking_call;
    const char NAME[32];
  };

  typedef bool IsFunction(const char* str);

  struct TestIsPropertyCases
  {
    IsFunction* checking_call;
    const char  name[32];
  };

  inline bool
  empty(const char*) throw ();

  TestIsPropertyCases test_is_property_cases[] =
  {
    {&empty, "empty"},
    {&String::is_digit, "is_digit"},
    {&String::is_letter, "is_letter"},
    {&String::is_lower_letter, "is_lower_letter"},
    {&String::is_title_letter, "is_title_letter"},
    {&String::is_upper_letter, "is_upper_letter"},
  };

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementations

using namespace String;

template<typename Arg1, typename Arg2>
void
TestContext::test_equal(const Arg1& a, const Arg2& b) const
  throw (eh::Exception)
{
  if (a != b)
  {
    std::cerr << "Requirements fail on operation: "
      << operation_ << ".\nInput source = " <<
      operand_ << "\nValue " << a << "!=" << b << std::endl;
  }
}

void
TestContext::set_operation(const char* name) throw (eh::Exception)
{
  operation_ = name;
}

void
TestContext::set_operand(const char* source_string) throw (eh::Exception)
{
  operand_ = source_string;
}

/**
 * Review the str RepetitionCount times, and call PropertyFun for
 * each UTF8 sequence.
 */

long long
UTF8IsPropertyOnStringPerformance::operator () () const
  throw (eh::Exception)
{
  Generics::CPUTimer tmr;
  tmr.start();
  for (std::size_t i = 0; i < RepetitionCount; ++i)
  {
    for (const char* p = ORIGINAL_STRING_; *p;
      p+=String::UTF8Handler::get_octet_count(*p))
    {
      function_(p);
    }
  }
  tmr.stop();
  return tmr.elapsed_time().microseconds();
}

UTF8IsPropertyOnStringPerformance::UTF8IsPropertyOnStringPerformance(
  const char* str, PropertyFun f)
  throw (TestException)
  : ORIGINAL_STRING_(str), function_(f)
{
  if (!ORIGINAL_STRING_)
  {
    throw TestException("Null sample string");
  }
}

namespace
{
  inline bool
  empty(const char*) throw ()
  {
    return false;
  }
}

void
is_property_performance_test() throw (eh::Exception)
{
  double std_dev = 0.;
  std::cout << "IsProperty evaluation parameters:"
    << std::fixed << std::endl;
  // Do rc - series and compute average and dispersion
  const std::size_t rc = 10;
  for (std::size_t i = 0;
       i < sizeof(TEXT_CORPUS) / sizeof(TEXT_CORPUS[0]);
       ++i)
  {
    std::cout << "Sample number " << i+1 << std::endl;
    for (std::size_t j = 0;
         j < sizeof(test_is_property_cases) /
             sizeof(test_is_property_cases[0]);
         ++j)
    {
      std::cout << '\t' << test_is_property_cases[j].name << '='
        << Test::acc_avg(rc,
          UTF8IsPropertyOnStringPerformance(TEXT_CORPUS[i],
            test_is_property_cases[j].checking_call),
          std_dev);
      std::cout << " Standard deviation=" << std_dev << std::endl;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Testing functions implementations
// call emulation, here is simple task to do string lowercase
// All methods must initialize input per each call.
//////////////////////////////////////////////////////////////////////////

void
new_copy_to_Lower(std::string &result) throw (eh::Exception)
{
  sstr = str;
  case_change<Lower>(sstr, result);
}

void
new_copy_to_Upper(std::string &result) throw (eh::Exception)
{
  sstr = str;
  case_change<Upper>(sstr, result);
}

void
new_copy_to_Uniform(std::string &result) throw (eh::Exception)
{
  sstr = str;
  case_change<Uniform>(sstr, result);
}

void
new_copy_to_Simplify(std::string &result) throw (eh::Exception)
{
  sstr = str;
  case_change<Simplify>(sstr, result);
}

SingleTestCase single_test_cases[] =
{
  {&new_copy_to_Lower, "new_copy_to_Lower"},
  {&new_copy_to_Uniform, "new_copy_to_Uniform"},
  // reset previous result, because lower functions != upper functions.
  {&new_copy_to_Upper, "new_copy_to_Upper"},
  {&new_copy_to_Simplify, "new_copy_to_Simplify"},
};

// end testing calls.
//////////////////////////////////////////////////////////////////////////

ProfFunctor::ProfFunctor(Measure pf, std::string& result)
  throw (eh::Exception)
  : measuring_functor_(pf), result_(result)
{
}

long long
ProfFunctor::operator()() throw (eh::Exception)
{
  timer_.start();
  for (std::size_t i = 0; i < RepetitionCount; ++i)
  {
    measuring_functor_(result_);
  }
  timer_.stop();
  return timer_.elapsed_time().microseconds();
}

void
single_performance_test() throw (eh::Exception)
{
  double std_dev = 0.;
  std::string result;
  result.reserve(2048);
  std::cout << "Single samples parameters:" << std::endl;
  // Do rc - series and compute average and dispersion
  const std::size_t rc = 10;
  std::string prev_result;
  for (std::size_t i = 0;
       i < sizeof(TEXT_CORPUS) / sizeof(TEXT_CORPUS[0]);
       ++i)
  {
    std::cout << "Process text from corpus number " << i+1 << std::endl;
    str = TEXT_CORPUS[i];
    test_context.set_operand(TEXT_CORPUS[i]);

    for (std::size_t j = 0;
         j < sizeof(single_test_cases) / sizeof(single_test_cases[0]);
         ++j)
    {
      test_context.set_operation(single_test_cases[j].NAME);
      std::cout << std::fixed << std::showpoint << std::setprecision(2);
      std::cout << '\t' << single_test_cases[j].NAME << '='
        << Test::acc_avg(rc, ProfFunctor(
        single_test_cases[j].checking_call, result), std_dev);
      std::cout << " Standard deviation=" << std_dev << std::endl;
#if 0
      if (prev_result.empty())
      {
        prev_result = result;
      }
      else
      {
        test_context.test_equal(prev_result, result);
        prev_result = result;
      }
      if (j == 5)
      {
        // and next we want test to_lower methods and clear previous
        // results, avoid fails (results to_lower != to_upper)
        prev_result.clear();
      }
#endif
    }
    prev_result.clear();
  } // for
}

//////////////////////////////////////////////////////////////////////////
// Arrays versus if

long long
CountingPerformanceFunctor::operator () () const
  throw (eh::Exception)
{
  Generics::CPUTimer tmr;
  tmr.start();
  for (std::size_t i = 0; i < RepetitionCount; ++i)
  {
    for (std::string::const_iterator it(sstr.begin()); it != sstr.end();
      it+=function_(*it));
  }
  tmr.stop();
  return tmr.elapsed_time().microseconds();
}

CountingPerformanceFunctor::CountingPerformanceFunctor(
  CountingFunction f) throw ()
  : function_(f)
{
}

void
test_octets_counting() throw (eh::Exception)
{
  using namespace Test;
  const std::size_t rc=10;
  double std_dev = 0.;

  for (std::size_t i = 0;
       i < sizeof(counting_test_cases) / sizeof(counting_test_cases[0]);
       ++i)
  {
    std::cout << '\t' << counting_test_cases[i].NAME << '='
    << acc_avg(rc,
         CountingPerformanceFunctor(counting_test_cases[i].checking_call),
         std_dev);
    std::cout << " Standard deviation=" << std_dev << std::endl;
  }
}

void
performance_arrays_test() throw (eh::Exception)
{
  std::cout << "If versus Arrays testing..." << std::endl;
  std::cout << "get_octets performance for ASCII input:" << std::endl;
  sstr = ascii;   test_octets_counting();
  std::cout << "get_octets performance for 2-bytes octets input:"
    << std::endl;
  sstr = utf8_2bytes;  test_octets_counting();
  std::cout << "get_octets performance for 3-bytes octets input:"
    << std::endl;
  sstr = utf8_3bytes;  test_octets_counting();
  std::cout << "get_octets performance for multilingual octets input:"
    << std::endl;
  sstr = ut8_multilang;  test_octets_counting();
}

//////////////////////////////////////////////////////////////////////////
// Performance comparison
// Task solved with OLD API and with NEW API

//////////////////////////////////////////////////////////////////////////
// Special feature for command line interception.
//
int
main(int argc, char *argv[] )
{
  std::cout << "UTF-8 API performance test started..."
    << std::endl;
  try
  {
    if (argc > 1)
    {
      RepetitionCount = atoi(argv[1]);
    }
    std::setlocale(LC_CTYPE, "");
    is_property_performance_test();
    single_performance_test();
    performance_arrays_test();
    std::cout << "SUCCESS" << std::endl;
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
