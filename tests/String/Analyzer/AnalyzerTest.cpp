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



// @file AnalyzerTest.cpp
//

#include <iostream>
#include <vector>
#include <String/Analyzer.hpp>
#include <Generics/Rand.hpp>
#include <String/AsciiStringManip.hpp>
#include <Logger/StreamLogger.hpp>
#include <Logger/ActiveObjectCallback.hpp>

using namespace String::SequenceAnalyzer;
using namespace String::AsciiStringManip;

/**
 * Special adapter for logger.
 * Able to store error reports into logger and allow analyze awaiting
 * behavior.
 */

class TestLogger : public Logging::Null::Logger
{
public:
  virtual bool
  log(const String::SubString& text,
    unsigned long severity = INFO,
    const char* aspect = 0,
    const char* code = 0)
    throw ();

  /**
   * @return reference on last reported error
   */
  const std::string&
  get_last_error() const throw ();

  void
  clear_last_error() throw ();

protected:
  virtual
  ~TestLogger() throw ();

private:
  std::string last_error_;
};
typedef ReferenceCounting::QualPtr<TestLogger> TestLogger_var;

class Tester
{
public:
  /**
   * Raise if has been got incorrect input data for testing.
   */
  DECLARE_EXCEPTION(InvalidTestData, eh::DescriptiveException);
  /**
   * Raise if unit functionality test failed.
   */
  DECLARE_EXCEPTION(UnitTestFailed, eh::DescriptiveException);

  Tester() throw (InvalidTestData, eh::Exception);

  void
  do_test() throw (eh::Exception);

  /**
   * Check range abilities a-d deploy into a b c d
   */
  void
  do_complex_test()
    throw (eh::Exception);

private:
  typedef std::vector<char> CharactersSet;

  static CharactersSet
  create_mixer_(const CharCategory& cat)
    throw (InvalidTestData, eh::Exception);

  static CharactersSet
  create_negative_mixer_(const CharCategory& cat)
    throw (InvalidTestData, eh::Exception);

  void
  generate_lexeme_(std::string& result) throw ();

  void
  generate_separators_(std::string& result) throw ();

  /**
   * Check the boundary conditions: empty input, etc
   */
  void
  unit_test_extremal()
    throw (eh::Exception);

  /**
   * Check shield symbol functionality. "\t" -> tab character
   * Replacing work through shield_map.
   * Check input cases with shield symbol='\', map state, input state:
   * 1. shield_map.empty(), input \'regular'
   * 2. shield_map.empty(), input \'irregular'
   * 3. shield_map.empty(), input \\
   * 4. shield_map = \\ -> 'regular', \t -> 'regular', input \'mapped regular'
   * 5. shield_map = \\ -> 'regular', \t -> 'regular', input \'regular'
   * 6. shield_map = \\ -> 'regular', \t -> 'regular', input \'irregular'
   * 7. shield_map = \\ -> 'regular', \t -> 'regular', input \\
   */
  void
  unit_test_shield()
    throw (eh::Exception);

  /**
   * Check random lexeme, separators sequences.
   * And ignore_successive_separators flag.
   */
  void
  unit_test_separator()
    throw (eh::Exception);

  /**
   * Check Noncritical exception behavior
   */
  void
  unit_test_exceptions()
    throw (eh::Exception);

  /**
   * Check ignoring irregular symbols when allow_ignored_symbs=true
   * and omissions of it if allow_ignored_symbs=false.
   */
  void
  unit_test_regular()
    throw (eh::Exception);

  /**
   * Check repeat abilities `repeat`{3} deploy into repeat repeat repeat
   */
  void
  unit_test_repeat()
    throw (eh::Exception);

  /**
   * Check range abilities a-d deploy into a b c d
   */
  void
  unit_test_range()
    throw (eh::Exception);

  /**
   * Check padding abilities
   */
  void
  unit_test_padding()
    throw (eh::Exception);

private:

  static void
  print_(const std::string& lex)
    throw (eh::Exception);

  TestLogger_var logger_;
  ReferenceCounting::FixedPtr<Generics::ActiveObjectCallback>
    last_error_callback_;

  const CharCategory regulars_;
  const CharactersSet regulars_mixer_;
  const CharactersSet irregulars_mixer_;
  const CharCategory separators_;
  const CharactersSet separators_mixer_;

  AnalyzerParams params_;

  typedef std::list<std::string> Result;
  Result result_;

};


//
// TestLogger class
//
TestLogger::~TestLogger() throw ()
{
}

bool
TestLogger::log(const String::SubString& text,
  unsigned long /*severity*/,
  const char* /*aspect*/,
  const char* /*code*/)
  throw()
{
  text.assign_to(last_error_);
  return true;
}

const std::string&
TestLogger::get_last_error() const throw ()
{
  return last_error_;
}

void
TestLogger::clear_last_error() throw ()
{
  last_error_.clear();
}


//
// Test body below
//

int
main()
{
  try
  {
    std::cout << "Analyzer test started.." << std::endl;

    Tester tester;
    // unit testing
    tester.do_test();
    // complex testing
    tester.do_complex_test();
    std::cout << "Test complete" << std::endl;
  }
  catch (eh::Exception& e)
  {
    std::cout << "\nFAIL: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "\nFAIL: unknown exception" << std::endl;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Implementations

void
init_params(AnalyzerParams& params)
  throw (eh::Exception)
{
  params.shield_symbol = '\\'; //The '\' symbol

  params.main_separators = CharSet(", \n\r\t");
  params.ignore_successive_separators = true;

  params.regular_symbs = CharSet("a-zA-Z0-9_.");
  params.regular_range_symbs = CharSet("a-zA-Z0-9_.");

  params.allow_ignored_symbs = true; // false;
  params.allow_recursion = true;
  params.recursion_max_depth = 10000;
  params.allow_repeat = false;
//  params.num_retries_symb = CharPair('*');
  params.allow_repeat = true;
  params.num_retries_symb = CharPair('{', '}');
  params.retry_part_symb = CharPair('`');

  params.allow_range = true;
  params.immediate_range_mode = false;
  params.range_part_symb = CharPair('[', ']');
  params.range_separators = CharSet(", ");
  params.range_symbol = '-';
  params.allow_padding = true; // false;
  params.use_int_range = true;
  params.int_range_bounds.clear();
  params.int_range_bounds.add(0, 1000);
  params.default_int_range_start = 0;

  // reserved, must be false
  params.use_char_range = false;
  params.use_str_range = false;
}


void
init_complex_test_params(AnalyzerParams& params)
throw (eh::Exception)
{
  params.shield_symbol = '\\'; //The '\' symbol
  params.shield_map.insert(
    std::pair<char, std::string>('%', "BAD%TEXT"));
  params.shield_map.insert(
    std::pair<char, std::string>('\\', std::string()));

  // Set separators ", \n\r\t"
  params.main_separators = CharSet(", \n\r\t");
  params.ignore_successive_separators = true;

  // Set regular symbols "a-z0-9_."
  params.regular_symbs = CharSet("a-zA-Z0-9_.");
  params.regular_range_symbs = CharSet("a-zA-Z0-9_.");
  params.allow_ignored_symbs = true;

  params.allow_repeat = true;
  params.allow_repeat = true;
  params.num_retries_symb = CharPair('{', '}');
  params.retry_part_symb = CharPair('`');

  params.allow_recursion = true;
  params.recursion_max_depth = 10;
  params.allow_range = true;
  params.use_int_range = true;
  params.range_part_symb = CharPair('[', ']');
  params.range_separators = CharSet(", ");
  params.range_symbol = '-';
  params.immediate_range_mode = true;

  params.int_range_bounds.add(0, 1000);
  params.default_int_range_start = 0;
  params.use_char_range = false;  // not realize yet
  params.use_str_range = false; // doesn't work now


  params.allow_padding = false;
  // reserved, must be false
  params.use_char_range = false;
  params.use_str_range = false;
}

//
// class Tester
//

Tester::Tester() throw (InvalidTestData, eh::Exception)
  : logger_(new TestLogger),
    last_error_callback_(new Logging::ActiveObjectCallbackImpl(logger_)),
    regulars_("a-zA-Z0-9_."),
    regulars_mixer_(create_mixer_(regulars_)),
    irregulars_mixer_(create_negative_mixer_(regulars_)),
    separators_(", \n\r\t"),
    separators_mixer_(create_mixer_(separators_))
{
  init_params(params_);
}

void
Tester::do_test() throw (eh::Exception)
{
  unit_test_extremal();
  unit_test_shield();
  unit_test_separator();
  unit_test_exceptions();
  unit_test_regular();
  unit_test_repeat();
  unit_test_range();
  unit_test_padding();
}

void
Tester::do_complex_test()
  throw (eh::Exception)
{
  const char FUN[] = "complex test_analyzer(): ";
  std::cout << FUN << "started" << std::endl;
  AnalyzerParams params;
  init_complex_test_params(params);

  // complex test
  Analyzer analyzer(params, last_error_callback_);
  //  Stream::Parser is("lexeme, [1-3], `repeat`{3}, \\\\\\");
// Troubles: parsing result
// [0-3] - range type cannot be defined, but [1-3] - OK
// repeat{0}{3} != repeat{3}{0}
// `r`{2}[1-4][5-5,6] != `r`{2}[1-4][5-6]
// output of `[1-3]`{3}{0}{3}  not empty.
// \\% have output without map.
// \\% and allow irregular output.
// \\ empty output born empty lexeme
  Stream::Parser istr("lexeme, [0-1], [1-1], "
    "`r`{1}[1-2][[0, 1-2,[3-4, [[5-6], [7-8, 9]]]]], \\\\, `[1-3]`{3}, "
    "c[1[[1-3]{2}, 15, text, \\%]]{1}{1}{1}");
  typedef std::list<std::string> Result;
  Result result;
  analyzer.process_char_sequence(istr, result);

  Result awaiting_result;
  awaiting_result.push_back("lexeme");
  awaiting_result.push_back("0");
  awaiting_result.push_back("1");
  awaiting_result.push_back("1");

  char buffer[16] = {'r'};
  for (unsigned i = 10; i < 30; ++i)
  {
    snprintf(buffer + 1, sizeof(buffer) - 2, "%u", i);
    awaiting_result.push_back(buffer);
  }
  awaiting_result.push_back("");
  for (std::size_t j = 0; j < 3; ++j)
  {
    for (unsigned i = 1; i < 4; ++i)
    {
      snprintf(buffer, sizeof(buffer) - 1, "%u", i);
      awaiting_result.push_back(buffer);
    }
  }

  awaiting_result.push_back("c11");
  awaiting_result.push_back("c11");
  awaiting_result.push_back("c12");
  awaiting_result.push_back("c12");
  awaiting_result.push_back("c13");
  awaiting_result.push_back("c13");
  awaiting_result.push_back("c115");
  awaiting_result.push_back("c1text");
  awaiting_result.push_back("c1BAD%TEXT");

  if (result != awaiting_result)
  {
    std::cerr << FUN << "case 1 failed" << std::endl;
    std::for_each(result.begin(), result.end(), print_);
  }
}

Tester::CharactersSet
Tester::create_mixer_(const CharCategory& cat)
  throw (InvalidTestData, eh::Exception)
{
  CharactersSet mixer;
  for (unsigned char uch = 0; uch != 255; uch++)
  {
    char ch = String::AsciiStringManip::convert(uch);
    if (cat.is_owned(ch))
    {
      mixer.push_back(ch);
    }
  }
  if (mixer.empty())
  {
    throw InvalidTestData("Empty characters subset");
  }
  return mixer;
}

Tester::CharactersSet
Tester::create_negative_mixer_(const CharCategory& cat)
  throw (InvalidTestData, eh::Exception)
{
  CharactersSet mixer;
  for (unsigned char uch = 0; uch != 255; uch++)
  {
    char ch = String::AsciiStringManip::convert(uch);
    if (!cat.is_owned(ch))
    {
      mixer.push_back(ch);
    }
  }
  if (mixer.size() == 256)
  {
    throw InvalidTestData("Empty negative characters subset");
  }
  return mixer;
}

void
Tester::generate_lexeme_(std::string& result) throw ()
{
  result.clear();
  std::size_t len = Generics::safe_rand(1, 10);
  std::size_t max_index = regulars_mixer_.size() - 1;

  for (std::size_t i = 0; i < len; ++i)
  {
    result += regulars_mixer_[Generics::safe_rand(0, max_index)];
  }
}

void
Tester::generate_separators_(std::string& result) throw ()
{
  result.clear();
  std::size_t len = Generics::safe_rand(1, 4);
  std::size_t max_index = separators_mixer_.size() - 1;

  for (std::size_t i = 0; i < len; ++i)
  {
    result += separators_mixer_[Generics::safe_rand(0, max_index)];
  }
}

void
Tester::unit_test_extremal()
  throw (eh::Exception)
{
  const char FUN[] = "unit_test_extremal(): ";
  std::cout << FUN << "started" << std::endl;

  Analyzer analyzer(params_, last_error_callback_);
  {
    Stream::Parser istr("");
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    std::cerr << FUN << "case 1 failed, result size=" << result_.size()
      << std::endl;
    result_.clear();
  }

  {
    Stream::Parser istr("");
    analyzer.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    std::cerr << FUN << "case 2 failed, result size=" << result_.size()
      << std::endl;
    result_.clear();
  }

  {
    Stream::Parser istr("\0");
    analyzer.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    std::cerr << FUN << "case 3 failed, result size=" << result_.size()
      << std::endl;
    result_.clear();
  }
}

void
Tester::unit_test_shield()
  throw (eh::Exception)
{
  const char FUN[] = "unit_test_shield(): ";
  std::cout << FUN << "started" << std::endl;
  params_.shield_symbol = '\\';
  params_.shield_map.clear();
  Analyzer analyzer(params_, last_error_callback_);
  std::string input;

  input = '\\';
  input += regulars_mixer_[0]; // must be save in output according
  {                            // documentation.
    Stream::Parser istr(input);
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    std::cerr << FUN << "case 1 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }

  input = '\\';
  input += irregulars_mixer_[0];
  {
    Stream::Parser istr(input);
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (!result_.empty())  // empty results!
  {
    // current time return one empty string.
    std::cerr << FUN << "case 2 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }

  input = "\\\\";
  input += regulars_mixer_[0];
  {
    Stream::Parser istr(input);
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    // current time return one empty string.
    std::cerr << FUN << "case 3 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }

  if (regulars_mixer_.size() < 3)
  {
    throw InvalidTestData("Not enough regular symbols");
  }

  params_.shield_map.insert(
    std::pair<char, std::string>(regulars_mixer_[0], "tab"));
  params_.shield_map.insert(
    std::pair<char, std::string>(regulars_mixer_[1], "second"));
  Analyzer analyzer_filled(params_, last_error_callback_);

  input = '\\';
  input += regulars_mixer_[0];
  {
    Stream::Parser istr(input);
    result_.clear();
    analyzer_filled.process_char_sequence(istr, result_);
  }
  if (result_.size() != 1 || result_.front() != "tab")
  {
    // current time return one empty string.
    std::cerr << FUN << "case 4 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }

  input = '\\';
  input += regulars_mixer_[1];
  {
    Stream::Parser istr(input);
    result_.clear();
    analyzer_filled.process_char_sequence(istr, result_);
  }
  if (result_.size() != 1 || result_.front() != "second")
  {
    // current time return one empty string.
    std::cerr << FUN << "case 5 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }

  input = '\\';
  input += regulars_mixer_[2];
  {
    Stream::Parser istr(input);
    result_.clear();
    analyzer_filled.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    // current time return one empty string.
    std::cerr << FUN << "case 6 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }

  input = '\\';
  input += irregulars_mixer_[0];
  {
    Stream::Parser istr(input);
    result_.clear();
    analyzer_filled.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    // current time return one empty string.
    std::cerr << FUN << "case 7 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }

  input = "\\\\";
  {
    Stream::Parser istr(input);
    result_.clear();
    analyzer_filled.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    // present time, return one empty string.
    std::cerr << FUN << "case 8 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
  }
}

void
Tester::unit_test_separator()
  throw (eh::Exception)
{
  const char FUN[] = "unit_test_separator(): ";
  std::cout << FUN << "started" << std::endl;

  std::string lexeme;
  std::string input;
  Result awaiting_result;
  Result awaiting_result_count_separators;
  lexeme = ",,";
  input += lexeme;
  for (std::size_t j = 1; j < lexeme.size(); ++j)
  {
    awaiting_result_count_separators.push_back("");
  }
  for (std::size_t i = 0; i < 10; ++i)
  {
    generate_lexeme_(lexeme);
    awaiting_result.push_back(lexeme);
    awaiting_result_count_separators.push_back(lexeme);
    input += lexeme;
    generate_separators_(lexeme);
    input += lexeme;
    for (std::size_t j = 1; j < lexeme.size(); ++j)
    {
      awaiting_result_count_separators.push_back("");
    }
  }
  //generate_lexeme_(lexeme);
  //awaiting_result.push_back(lexeme);
  //awaiting_result_count_separators.push_back(lexeme);
  //input += lexeme;

  params_.ignore_successive_separators = true;
  {
    Analyzer analyzer(params_, last_error_callback_);
    Stream::Parser istr(input);
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (result_ != awaiting_result)
  {
    std::cerr << FUN << "case 1 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
    std::cerr << "Awaiting is:" << std::endl;
    std::for_each(awaiting_result.begin(), awaiting_result.end(), print_);
    std::cerr << std::endl;
  }

  params_.ignore_successive_separators = false;
  {
    Analyzer analyzer(params_, last_error_callback_);
    Stream::Parser istr(input);
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (result_ != awaiting_result_count_separators)
  {
    std::cerr << FUN << "case 2 failed" << std::endl;
    std::for_each(result_.begin(), result_.end(), print_);
    std::cerr << "Awaiting is:" << std::endl;
    std::for_each(awaiting_result_count_separators.begin(),
      awaiting_result_count_separators.end(), print_);
    std::cerr << std::endl;
  }
  params_.ignore_successive_separators = false;
}

void
Tester::unit_test_exceptions()
  throw (eh::Exception)
{
  const char FUN[] = "unit_test_exceptions(): ";
  std::cout << FUN << "started" << std::endl;
  char buffer[2] = "\0";

  params_.allow_ignored_symbs = false;
  Analyzer analyzer(params_, last_error_callback_);
  for (unsigned char uch = 1; uch != 255; uch++)
  {
    char ch = String::AsciiStringManip::convert(uch);
    buffer[0] = ch;
    Stream::Parser istr(buffer);
    result_.clear();
    try
    {
      logger_->clear_last_error();
      analyzer.process_char_sequence(istr, result_);
      if (!params_.regular_symbs.is_owned(ch) &&
        !params_.main_separators.is_owned(ch))
      {
        if (logger_->get_last_error().empty())
        {
          std::cerr << FUN << "Error information should have been put "
            "by callback call, ch=" << ch << ", ascii code="
            << static_cast<int>(ch)
            << std::endl;
        }
      }
    }
    catch (const eh::Exception& e)
    {
      continue;
    }
  }

}

void
Tester::unit_test_regular()
  throw (eh::Exception)
{
  const char FUN[] = "unit_test_regular(): ";
  std::cout << FUN << "started" << std::endl;
  char buffer[2] = "\0";

  params_.allow_ignored_symbs = true;
  Analyzer analyzer(params_, last_error_callback_);
  for (unsigned char uch = 0; uch != 255; uch++)
  {
    char ch = String::AsciiStringManip::convert(uch);
    buffer[0] = ch;
    Stream::Parser istr(buffer);
    result_.clear();
    try
    {
      analyzer.process_char_sequence(istr, result_);
    }
    catch (const eh::Exception& e)
    {
      ((ch == params_.shield_symbol ||
        ch == params_.retry_part_symb.first()) ?
        std::cout : std::cerr) << FUN << "Character " << ch << ", code: "
        << std::hex << static_cast<unsigned>(ch) << std::dec
        << ". Exception: " << e.what()
        << std::endl;
      continue;
    }

    if (regulars_.is_owned(ch))
    {
      if (result_.size() != 1 || result_.front() != buffer)
      {
        std::cerr << FUN << "case 1 failed, regular character code="
          << std::hex << static_cast<unsigned>(ch) << std::dec
          << " cannot be processed correctly" << std::endl;
      }
    }
    else
    {
      if (!result_.empty())
      {
        std::cerr << FUN << "case 2 failed, irregular character "
          << buffer << ", code="
          << std::hex << static_cast<unsigned>(ch) << std::dec
          << " present in output=" << result_.front() << std::endl;
      }
    }
  }

  Analyzer analyzer_ignorable(params_, last_error_callback_);
  params_.ignored_symbs =
    CharSet(params_.ignored_symbs, CharSet("!\"#$%&'()*+"));
  for (char ch = '!'; ch != '+'; ++ch)
  {
    buffer[0] = ch;
    Stream::Parser istr(buffer);
    result_.clear();
    logger_->clear_last_error();
    analyzer_ignorable.process_char_sequence(istr, result_);
    if (logger_->get_last_error().empty())
    {
      std::cerr << FUN << "Error information should have been put "
        "by callback call" << std::endl;
    }
    if (!result_.empty())
    {
      std::cerr << FUN << "case 3 failed, ignored character code="
        << std::hex << static_cast<unsigned>(ch) << std::dec
        << " give some output=" << result_.front() << std::endl;
    }
  }
  params_.allow_ignored_symbs = false;
}

void
Tester::unit_test_repeat()
  throw (eh::Exception)
{
  const char FUN[] = "unit_test_repeat(): ";
  std::cout << FUN << "started" << std::endl;

  Result awaiting_result;
  awaiting_result.push_back("repeat");
  awaiting_result.push_back("repeat");
  awaiting_result.push_back("repeat");

  params_.allow_repeat = true;
  params_.num_retries_symb = CharPair('{', '}');
  params_.retry_part_symb = CharPair('`');
  {
    Analyzer analyzer(params_, last_error_callback_);
    Stream::Parser istr("`repeat`{3}");
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (result_ != awaiting_result)
  {
    std::cerr << FUN << "case 1 failed, results is:\n";
    std::for_each(result_.begin(), result_.end(), print_);
  }

  params_.num_retries_symb = CharPair('b');
  params_.retry_part_symb = CharPair('c', 'x');
  {
    Analyzer analyzer2(params_, last_error_callback_);
    Stream::Parser istr("crepeatxb3b");
    result_.clear();
    analyzer2.process_char_sequence(istr, result_);
  }
  if (result_ != awaiting_result)
  {
    std::cerr << FUN << "case 2 failed, results is:\n";
    std::for_each(result_.begin(), result_.end(), print_);
  }

  params_.allow_repeat = false;
  {
    Analyzer analyzer3(params_, last_error_callback_);
    Stream::Parser istr("`repeat`{3}");
    result_.clear();
    analyzer3.process_char_sequence(istr, result_);
  }
  if (!result_.empty())
  {
    std::cerr << FUN << "case 3 failed, results is:\n";
    std::for_each(result_.begin(), result_.end(), print_);
  }
}

void
Tester::unit_test_range()
  throw (eh::Exception)
{
  const char FUN[] = "unit_test_range(): ";
  std::cout << FUN << "started" << std::endl;

  Result awaiting_result;
  awaiting_result.push_back("1");
  awaiting_result.push_back("2");
  awaiting_result.push_back("3");
  awaiting_result.push_back("4");

  params_.allow_range = true;
  params_.use_int_range = true;
  params_.range_part_symb = CharPair('[', ']');
  params_.range_separators = CharSet(", ");
  params_.range_symbol = '-';

  params_.int_range_bounds.add(0, 1000);
  params_.default_int_range_start = 0;
  params_.use_char_range = false;  // not realized yet
  params_.use_str_range = false; // doesn't work now

  {
    Analyzer analyzer(params_, last_error_callback_);
    Stream::Parser istr("[1-4]");
    analyzer.process_char_sequence(istr, result_);
  }
  if (result_ != awaiting_result)
  {
    std::cerr << FUN << "case 1 failed, results is:\n";
    std::for_each(result_.begin(), result_.end(), print_);
  }

  {
    Analyzer analyzer(params_, last_error_callback_);
    Stream::Parser istr("[1-1]");
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (result_.size() != 1 && result_.front() != "1")
  {
    std::cerr << FUN << "case 2 failed, results is:\n";
    std::for_each(result_.begin(), result_.end(), print_);
  }

  {
    Analyzer analyzer(params_, last_error_callback_);
    Stream::Parser istr("[0-1]");
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  Result::const_iterator it = result_.begin();
  if (!(result_.size() == 2 && *it == "0" && *(++it) == "1"))
  {
    std::cerr << FUN << "case 3 failed, results is:\n";
    std::for_each(result_.begin(), result_.end(), print_);
  }
}

void
Tester::unit_test_padding()
throw (eh::Exception)
{
  const char FUN[] = "unit_test_padding(): ";
  std::cout << FUN << "started" << std::endl;
  Result awaiting_result;
  awaiting_result.push_back("_1");
  awaiting_result.push_back("_2");
  awaiting_result.push_back("_3");
  awaiting_result.push_back("_4");
  awaiting_result.push_back("_5");
  awaiting_result.push_back("_6");
  awaiting_result.push_back("_7");
  awaiting_result.push_back("_8");
  awaiting_result.push_back("_9");
  awaiting_result.push_back("10");
  awaiting_result.push_back("11");
  awaiting_result.push_back("12");
  awaiting_result.push_back("13");
  awaiting_result.push_back("14");
  awaiting_result.push_back("15");
  awaiting_result.push_back("16");
  awaiting_result.push_back("17");
  awaiting_result.push_back("18");
  awaiting_result.push_back("19");
  awaiting_result.push_back("20");
  awaiting_result.push_back("21");
  awaiting_result.push_back("22");

  params_.allow_padding = true;
  params_.padding_symb = '_';
  {
    Analyzer analyzer(params_, last_error_callback_);
    Stream::Parser istr("[_1-22]");
    result_.clear();
    analyzer.process_char_sequence(istr, result_);
  }
  if (result_ != awaiting_result)
  {
    std::cerr << FUN << "case 1 failed, results is:\n";
    std::for_each(result_.begin(), result_.end(), print_);
  }
}

void
Tester::print_(const std::string& lex)
  throw (eh::Exception)
{
  std::cerr << "len=" << lex.size() <<", lex: " << lex << std::endl;
}
