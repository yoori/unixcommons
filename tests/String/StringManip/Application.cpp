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



#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <list>
#include <vector>
#include <cstdlib>
#include <eh/Exception.hpp>

#include <String/StringManip.hpp>
#include <String/AsciiStringManip.hpp>
#include <String/UnicodeSymbol.hpp>
#include <String/UTF8IsProperty.hpp>

#include <Generics/Rand.hpp>

namespace String
{
  namespace Test
  {
    struct test_case
    {
      const char* test_str;
      std::string trim_result;
    };

    DECLARE_EXCEPTION(InvalidArguments, eh::DescriptiveException);
    class StringManipTest
    {
    public:

      StringManipTest(int argc, char* argv[]) throw(InvalidArguments);

      int run() throw();
    private:
      typedef std::string StdString;
      typedef std::vector<StdString> Strings;

      Strings test_strs_;
      bool interactive_;

      static test_case test_cases[];
    };
  }
}

//////////////////////////////////////////////////////////////////////////
// Implementation 
//////////////////////////////////////////////////////////////////////////

namespace String
{
  namespace Test
  {
    test_case StringManipTest::test_cases[] =
    {
      {"test string 1", "test string 1"},
      {" test string 2", "test string 2"},
      {"test string 3 ", "test string 3"},
      {" test string 4", "test string 4"},
      {0, ""},
      {"  ", ""}
    };

    StringManipTest::StringManipTest(int argc, char* argv[])
      throw(InvalidArguments)
    {
      if(argc>1) //interactive
      {
        interactive_ = true;
        for(int i=1;i<argc;i++)
        {
          test_strs_.push_back(argv[i]);
        }
      }
      else
      {
        interactive_ = false;
        for(size_t i=0; i<sizeof(test_cases)/sizeof(test_cases[0]);i++)
        {
          if(test_cases[i].test_str)
          {
            test_strs_.push_back(test_cases[i].test_str);
          }
          else
          {
            test_strs_.resize(test_strs_.size() + 1);
          }
        }
      }
    }

    int StringManipTest::run() throw()
    {
      if(interactive_)
      {
        for(Strings::iterator i = test_strs_.begin();
          i!= test_strs_.end();
          ++i)
        {
          std::cout << " trim '" << *i << "' '";
          String::StringManip::trim(*i, *i);
          std::cout << *i << "'" << std::endl;
        }
      }
      else
      {
        size_t j = 0;
        for(Strings::iterator i = test_strs_.begin();
          i!= test_strs_.end();
          ++i, j++)
        {
          String::StringManip::trim(*i, *i);
          if(*i != test_cases[j].trim_result)
          {
            std::cerr << "error checking Generics::StringManip::trim: '"
              << test_cases[j].test_str << "' expected '"  
              << test_cases[j].trim_result << "' got '"
              << *i << "'" << std::endl;
            return 1;
          }
        }
      }
      return 0;
    }
  }
}
//////////////////////////////////////////////////////////////////////////

void
str_append(std::string& str, int size, const char* chars, int length)
  throw (eh::Exception)
{
  while (size-- > 0)
  {
    char ch = chars[rand() % length];
    str.push_back(ch);
  }
}

void
str_append(std::string& str, int size, const char* chars)
  throw (eh::Exception)
{
  str_append(str, size, chars, strlen(chars));
}

void
generate_string(std::string& str) throw (eh::Exception)
{
  str_append(str, rand() % 16, " \t");
  str_append(str, rand() % 128, "acbnp439hf1-34djc,12394i 1293ier1923ie =23ie ");
  str_append(str, rand() % 16, " \t");
  //std::cout << "Generated '" << str << "'\n";
}

void
create_string(std::string& str) throw (eh::Exception)
{
  for (size_t length = rand() % 128; length--;)
  {
    str.push_back(rand() % 255 + 1);
  }
}

void
test_trim_string(const std::string& str) throw (eh::Exception)
{
  std::string trimmed1, trimmed2(str);
  String::StringManip::trim(str, trimmed1);
  String::StringManip::trim(trimmed2, trimmed2);
  std::string::size_type first = std::string::npos;
  for (std::string::size_type i = 0; i < str.length(); i++)
  {
    if (!std::isspace(str[i]))
    {
      first = i;
      break;
    }
  }
  std::string::size_type last = std::string::npos;
  for (std::string::size_type i = 0; i < str.length(); i++)
  {
    if (!std::isspace(str[i]))
    {
      last = i;
    }
  }
  std::string expected(first == std::string::npos ? "" :
    str.substr(first, last - first + 1));
  if ((first == std::string::npos &&
      (!trimmed1.empty() || !trimmed2.empty())) ||
      (first != std::string::npos &&
      trimmed1 != expected) || trimmed2 != expected)
  {
    std::cerr << "Error in trim function: '" << str << "' => '" <<
      trimmed1 << "' and '" << trimmed2 << "' (expected '" <<
      expected << "')\n";
  }
}

void
test_trim(void) throw (eh::Exception)
{
  test_trim_string("");
  test_trim_string(" \t \t\t ");
  for (int i = 0; i < 1000; i++)
  {
    std::string str;
    generate_string(str);
    test_trim_string(str);
  }
}

void
charcheck_error(const char* where, char ch) throw (eh::Exception)
{
  std::cerr << "Error in " << where << " character '" <<
    ch << "'" << std::endl;
}

void
test_charcheck() throw (eh::Exception)
{
  String::AsciiStringManip::CharCategory all("\001-\177", true);
  String::AsciiStringManip::CharCategory none("");

  for (char ch = 0; ch < 127; ch++)
  {
    if (String::AsciiStringManip::ALPHA.is_owned(ch) !=
      ((ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z')))
    {
      charcheck_error("ALPHA", ch);
    }
    if (String::AsciiStringManip::ALPHA_NUM.is_owned(ch) !=
      ((ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z') ||
        (ch >= '0' && ch <= '9')))
    {
      charcheck_error("ALPHA_NUM", ch);
    }
    if (!all.is_owned(ch))
    {
      charcheck_error("all", ch);
    }
    if (none.is_owned(ch))
    {
      charcheck_error("none", ch);
    }
  }
}

struct Token
{
  std::string token;
  char separator;
};
typedef std::list<Token> Tokens;

bool
operator ==(const Token& token1, const Token& token2)
{
  return token1.separator == token2.separator &&
    token1.token == token2.token;
}

template <typename T1, typename T2>
bool
is_equal(T1 b1, T1 e1, T2 b2, T2 e2)
{
  for (; b1 != e1 && b2 != e2; ++b1, ++b2)
  {
    if (!(*b1 == *b2))
    {
      return false;
    }
  }
  return b1 == e1 && b2 == e2;
}

char
get_separator(const char*, char ch) throw ()
{
  return ch;
}

char
get_separator(const char* end, const char* str) throw ()
{
  return str == end ? '\0' : *str;
}

template <typename Tokenizer>
bool
get_token(Tokenizer& tokenizer, std::string& result)
  throw (eh::Exception)
{
  String::SubString token;
  if (tokenizer.get_token(token))
  {
    token.assign_to(result);
    return true;
  }
  return false;
}

template <typename Tokenizer>
void
create_tokens(Tokenizer& tokenizer, const char* end, Tokens& tokens)
  throw (eh::Exception)
{
  std::string token;
  while (get_token(tokenizer, token))
  {
    Token new_token = { token,
      get_separator(end, tokenizer.get_separator()) };
    tokens.push_back(new_token);
  }
}

void
append_delims(std::string& str, bool at_least_one = false)
  throw (eh::Exception)
{
  str_append(str, rand() % 5 + at_least_one, " \t\n");
}

void
append_normal(std::string& str) throw (eh::Exception)
{
  str_append(str, rand() % 20 + 15, "30mi23-09t356=1.v1=43-r.,v1-E");
}

template <typename T>
std::string
create_string(T b, T e)
{
  std::ostringstream ostr;

  for (; b != e; ++b)
  {
    std::string delim;
    append_delims(delim);

    ostr << delim << b->token;
    if (b->separator)
    {
      ostr << b->separator;
    }
  }

  return ostr.str();
}

template <typename T>
void
out_tokens(T b, T e)
{
  for (; b != e; ++b)
  {
    std::cout << "'" << b->token << "'" << (int)b->separator << std::endl;
  }
}

template <typename Tokenizer>
bool
test_tokenizer(Tokenizer& tokenizer, const String::SubString& str,
  const Tokens& tokens) throw (eh::Exception)
{
  Tokens result;
  create_tokens(tokenizer, str.end(), result);

  if (!is_equal(tokens.begin(), tokens.end(), result.begin(), result.end()))
  {
    std::cerr << "Tokenizer test failed" << std::endl;
    return false;
  }

  return true;
}

template <typename Tokenizer, typename Category>
bool
test_tokens(const String::SubString& str, const Tokens& tokens,
  Category& category) throw (eh::Exception)
{
  Tokenizer tokenizer(str, category);
  if (!test_tokenizer(tokenizer, str, tokens))
  {
    return false;
  }
  return true;
}

bool
test_tokens(const Tokens& tokens) throw (eh::Exception)
{
  static const String::AsciiStringManip::CharCategory DELIM(" \t\n");

  std::string str = create_string(tokens.begin(), tokens.end());

  {
    String::StringManip::Splitter<> tokenizer(str);
    if (!test_tokenizer(tokenizer, str, tokens))
    {
      return false;
    }
  }

  if (!test_tokens<String::StringManip::CharSplitter>(str, tokens, DELIM))
  {
    return false;
  }

  return true;
}

void
create_random_tokens(Tokens& tokens) throw (eh::Exception)
{
  for (int i = 50; i >= 0; i--)
  {
    Token token;
    append_normal(token.token);
    if (i || rand() & 16)
    {
      std::string delim;
      append_delims(delim, true);
      token.separator = delim[0];
    }
    else
    {
      token.separator = '\0';
    }
    tokens.push_back(token);
  }
}

void
test_tokenizer() throw (eh::Exception)
{
  {
    Tokens tokens;
    if (!test_tokens(tokens))
    {
      return;
    }
  }
  for (int i = 0; i < 100; i++)
  {
    Tokens tokens;
    create_random_tokens(tokens);
    if (!test_tokens(tokens))
    {
      return;
    }
  }
}

int
string_manip_test(int argc, char** argv) throw()
{
  try
  {
    String::Test::StringManipTest test(argc, argv);
    return test.run();
  }
  catch(const String::Test::InvalidArguments& e)
  {
    std::cerr << "Caught InvalidArguments exception. Description: "
      << e.what() << std::endl;
    return 1;
  }
}


template <const bool PADDING>
void
test_base64()
{
  char all_chars[256];
  for (int i = 0; i < 256; i++)
  {
    all_chars[i] = static_cast<char>(i);
  }

  for (int i = 0; i < 100; i++)
  {
    std::string original;
    str_append(original, rand() % 128, all_chars, 256);

    try
    {
      std::string encoded;
      String::StringManip::base64mod_encode(encoded, original.data(),
        original.size(), PADDING);

      {
        std::string decoded;
        String::StringManip::base64mod_decode(decoded, encoded,
          PADDING);

        if (decoded != original)
        {
          std::cerr << "Failed to encode/decode base64 '" << original <<
            "', got '" << decoded << "', encoded '" << encoded << "'" <<
            std::endl;
        }
      }

      for (std::string::size_type i = 0; i < encoded.length(); i++)
      {
        std::string decoded;

        try
        {
          String::StringManip::base64mod_decode(decoded,
            String::SubString(encoded.data(), i), PADDING);
        }
        catch (const String::StringManip::InvalidFormatException&)
        {
          continue;
        }

        if ((PADDING && (i % 4)) || decoded == original)
        {
          std::cerr << "Failed to fail in base64 encode/decode '" <<
            original << "', got '" << decoded << "', encoded '" <<
            encoded << "'" << i << "/" << encoded.size() << std::endl;
        }
      }
    }
    catch (const eh::Exception& ex)
    {
      std::cerr << "test_base64(): Problem with '" << original << "': " <<
        ex.what() << std::endl;
    }
  }

  {
    String::SubString src("ABCDEFGHIJKLMNOPQRSTUVWXYZ\xFE\xEF\xFF");
    std::string dst;
    String::StringManip::base64_encode(dst, src.data(), src.size(),
      PADDING);
    String::SubString tst("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVr+7/8=");
    if (dst != tst.substr(0, tst.size() - 1 + PADDING))
    {
      std::cerr << "base64_encode failure" << std::endl;
    }
  }

  {
    String::SubString str("!@)#I@!#$)JKT#$)M#$)FK#F$)#FK#$F");

    for (size_t l = 1; l <= str.size(); l++)
    {
      String::SubString src = str.substr(0, l);
      uint8_t data = Generics::safe_rand() &
        ((1 << String::StringManip::base64mod_fill_size(src.size())) - 1);
      std::string enc;
      String::StringManip::base64mod_encode(enc, src.data(), src.size(),
        PADDING, data);
      std::string dec;
      uint8_t got = 0;
      try
      {
        String::StringManip::base64mod_decode(dec, enc, PADDING);
        if (data)
        {
          std::cerr << "base64mod_decode not failed with fill" << std::endl;
        }
      }
      catch (const String::StringManip::InvalidFormatException&)
      {
        if (!data)
        {
          std::cerr << "base64mod_decode failed with no fill" << std::endl;
        }
      }
      try
      {
        String::StringManip::base64mod_decode(dec, enc, PADDING, &got);
        if (got != data)
        {
          std::cerr << "base64mod_decode faild with wrong fill" <<
            std::endl;
        }
      }
      catch (const String::StringManip::InvalidFormatException&)
      {
        std::cerr << "base64mod_decode with fill failed" << std::endl;
      }
    }
  }
}

void
test_js_encode() throw (eh::Exception)
{
  const char SRC[] =
    "\xE2\xE2\x80\xE2\x80\xA7\xE2\x80\xA8\x80\xA9\xE2\x80\xA9"
    "abcd\xE2\x80\xA8\xE2\x80\xA8\xE2";
  const char DST[] =
    "\xE2\xE2\x80\xE2\x80\xA7\\u2028\x80\xA9\\u2029"
    "abcd\\u2028\\u2028\xE2";
  try
  {
    std::string dst;
    String::StringManip::js_encode(SRC, dst);
    if (dst != DST)
    {
      std::cerr << "test_js_code(): invalid result" << std::endl;
    }
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "test_js_code(): " << ex.what() << std::endl;
  }
  for (int i = 0; i < 100; i++)
  {
    std::string str;
    try
    {
      str.clear();
      create_string(str);
      std::string res;
      String::StringManip::js_encode(str.c_str(), res);
    }
    catch (const eh::Exception& ex)
    {
      std::cerr << "test_js_code(): Problems with '" << str << "': " <<
        ex.what() << std::endl;
    }
  }
}

void
test_json_encode() throw (eh::Exception)
{
  for (int i = 0; i < 100; i++)
  {
    std::string str;
    try
    {
      str.clear();
      create_string(str);
      String::StringManip::json_escape(str);
    }
    catch (const eh::Exception& ex)
    {
      std::cerr << "test_json_code(): Problems with '" << str << "': " <<
        ex.what() << std::endl;
    }
  }
}

void
test_js_unicode_encode() throw (eh::Exception)
{
  for (int i = 0; i < 100; i++)
  {
    std::string str;
    try
    {
      str.clear();
      generate_string(str);
      std::string res;
      String::StringManip::js_unicode_encode(str.c_str(), res);
      std::string dec;
      String::StringManip::js_unicode_decode(res, dec);
      if (str != dec)
      {
        std::cerr << "Failed to decode '" << res << "' to '" << str <<
          "' but got '" << dec << "'" << std::endl;
      }
    }
    catch (const eh::Exception& ex)
    {
      std::cerr << "test_js_unicode_code(): Problems with '" << str <<
        "': " << ex.what() << std::endl;
    }
  }
}

void
test_xml()
{
  char chars[126];
  for (int i = 0; i < 126; i++)
  {
    chars[i] = static_cast<char>(i + 1);
  }

  for (int i = 0; i < 100; i++)
  {
    std::string original;
    str_append(original, rand() % 1024, chars, 126);

    try
    {
      std::string encoded;
      String::StringManip::xml_encode(original.c_str(), encoded);

      {
        std::string decoded;
        String::StringManip::xml_decode(encoded, decoded);

        if (decoded != original)
        {
          std::cerr << "Failed to encode/decode xml '" << original <<
            "', got '" << decoded << "', encoded '" << encoded << "'" <<
            std::endl;
        }
      }
    }
    catch (const eh::Exception& ex)
    {
      std::cerr << "test_xml(): Problem with '" << original << "': " <<
        ex.what() << std::endl;
    }
  }
}

bool
test_csv_encode(const char* input, const char* expected,
  const char separator) throw (eh::Exception)
{
  std::string encoded;
  String::StringManip::csv_encode(input, encoded, separator);
  if(encoded != expected)
  {
    std::cerr << "Failed to encode <<" << input <<">> csv string, "
      << "expected: " << expected << " , "
      << "got: " << encoded
      << std::endl;
    return false;
  }
  return true;
}

void
test_csv_encode() throw (eh::Exception)
{
  static struct TestCase
  {
    const char* input;
    const char* expected;
    const char  separator;
  } cases[] = {
    {0, "", ','},
    {"", "", ','},
    {"some words", "some words", ','},
    {",", "\",\"", ','},
    {";", "\";\"", ';'},
    {"some \n words", "\"some \n words\"", ','},
    {"some \r words", "\"some \r words\"", ','},
    {"some \r\n words", "\"some \r\n words\"", ','},
    {"some \n\r words", "\"some \n\r words\"", ','},
    {"\"", "\"\"\"\"", ','},
    {"\"\"", "\"\"\"\"\"\"", ','},
    {"\"text", "\"\"\"text\"", ','},
    {"text\"", "\"text\"\"\"", ','},
    {"\"text\"", "\"\"\"text\"\"\"", ','},
    {"prefix\"text\"sufix", "\"prefix\"\"text\"\"sufix\"", ','},
    {"pre\"txt\", post", "\"pre\"\"txt\"\", post\"", ','},
    {"pre\"txt1, txt2\", post", "\"pre\"\"txt1, txt2\"\", post\"", ','},
    {"pre\"txt1, txt2\", post", "\"pre\"\"txt1, txt2\"\", post\"", ','},
    {"pre\"txt\"\n post", "\"pre\"\"txt\"\"\n post\"", ','},
    {"pre\"txt1\n txt2\"\n post", "\"pre\"\"txt1\n txt2\"\"\n post\"", ','},
    {"pre\"txt1\n txt2\", post",  "\"pre\"\"txt1\n txt2\"\", post\"", ','},
    {0, 0, 0}
  };
  for (int i = 0; cases[i].expected != 0; ++i)
  {
    test_csv_encode(cases[i].input, cases[i].expected, cases[i].separator);
  }
}

class TestFlattenCaseGenerator
{
public:
  void
  generate() throw (eh::Exception);

  void
  check() throw (eh::Exception);

  static void
  checking(const std::string& res,
    const std::string& src,
    const char *standard) throw (eh::Exception);

private:
  std::string input_;
  std::string standard_;
};

void
TestFlattenCaseGenerator::generate() throw (eh::Exception)
{
  standard_.clear();
  input_.clear();
  bool put_space_before = false;
  for (std::size_t i = Generics::safe_integral_rand(10); i != 0; --i)
  {
    if (Generics::safe_integral_rand(1))
    {
      input_ += " ";
      if (!put_space_before)
      {
        standard_ += " ";
        put_space_before = true;
      }
    }
    else
    {
      String::UnicodeSymbol put_it;
      do 
      {
        put_it = String::UnicodeSymbol::random();
      }
      while (String::is_space(put_it.c_str()));
      input_.append(put_it.c_str(), put_it.length());
      standard_.append(put_it.c_str(), put_it.length());
      put_space_before = false;
    }
  }
}

void
TestFlattenCaseGenerator::check() throw (eh::Exception)
{
  std::string dest;
  String::StringManip::flatten(dest, input_);
  if (dest != standard_)
  {
    std::cerr << "flatten functional doesn't work: input="
      << input_ << std::endl
      << "result=" << dest << std::endl << "standard="
      << standard_ << std::endl;
  }
}

void
TestFlattenCaseGenerator::checking(const std::string& res,
                                   const std::string& src,
                                   const char *standard)
  throw (eh::Exception)
{
  if (res != standard)
  {
    std::cerr << "flatten functional trouble:\n"
      << "Source: " << src << std::endl
      << "Result: " << res << std::endl
      << "Standard: " << standard << std::endl;
  }
}

void
check_flatten() throw (eh::Exception)
{
  using namespace String::Test;

  std::string dest;

  std::string src("    ");
  String::StringManip::flatten(dest, src);
  TestFlattenCaseGenerator::checking(dest, src, " ");

  src = "A";
  String::StringManip::flatten(dest, src);
  TestFlattenCaseGenerator::checking(dest, src, "A");

  src = "   A";
  String::StringManip::flatten(dest, src);
  TestFlattenCaseGenerator::checking(dest, src, " A");

  src = "A   A";
  String::StringManip::flatten(dest, src);
  TestFlattenCaseGenerator::checking(dest, src, "A A");

  src = "A   ";
  String::StringManip::flatten(dest, src);
  TestFlattenCaseGenerator::checking(dest, src, "A ");

  src = "";
  String::StringManip::flatten(dest, src);
  TestFlattenCaseGenerator::checking(dest, src, "");

  std::string src0("A  \xD7\x9B");
  String::StringManip::flatten(dest, src0);
  TestFlattenCaseGenerator::checking(dest, src0, "A \xD7\x9B");

  std::string src1("A  \xD7\x9B. ");
  String::StringManip::flatten(dest, src1);
  TestFlattenCaseGenerator::checking(dest, src1, "A \xD7\x9B. ");

  std::string src2("Test\xC2\xA0\xE1\x9A\x80\xE3\x80\x80\xE2\x80\x87 AAB"
    "   \t  _Rpl\t  . ");
  String::StringManip::flatten(
    dest, src2, String::SubString("R", 1));
  TestFlattenCaseGenerator::checking(dest, src2, "TestRAABR_RplR.R");

  String::StringManip::flatten(dest, src2);
  TestFlattenCaseGenerator::checking(dest, src2, "Test AAB _Rpl . ");
}

void
check_random_flatten() throw (eh::Exception)
{
  TestFlattenCaseGenerator checker;
  std::cout << "check_random_flatten start" << std::endl;
  for (std::size_t i = 0; i < 100; ++i)
  {
    checker.generate();
    checker.check();
  }
}

void
check_mark() throw (eh::Exception)
{
  const char* SRCS[] = 
  {
    "^.$|()[]*+?{}\\",
    "mmm^.$|()[]*+?{}",
    "^.$|()[]*+?{}mmm",
    "mmm_^_mmm",
    "^",
    "mmm",
    ""
  };
  const char* STANDARDS[] =
  {
    "m^m.m$m|m(m)m[m]m*m+m?m{m}m\\",
    "mmmm^m.m$m|m(m)m[m]m*m+m?m{m}",
    "m^m.m$m|m(m)m[m]m*m+m?m{m}mmm",
    "mmm_m^_mmm",
    "m^",
    "mmm",
    ""
  };
  std::string result;

  for (std::size_t i = 0; i < sizeof(SRCS) / sizeof(SRCS[0]); ++i)
  {
    String::StringManip::mark(SRCS[i], result,
      String::AsciiStringManip::REGEX_META, 'm');
    if (result != STANDARDS[i])
    {
      std::cerr << "Marker trouble: result=" << result << "\nstandard="
        << STANDARDS[i] << std::endl;
    }
  }
}

void
check_replace() throw (eh::Exception)
{
  const char* data[][2] =
  {
    { "", "" },
    { "a", "a" },
    { "#", "#" },
    { "ab", "ab" },
    { "##", "#" },
    { "#a#", "#a#" },
    { "##a", "#a" },
    { "a##", "a#" },
    { "###", "##" },
    { "####", "##" },
    { "abc##def##ghi", "abc#def#ghi" },
    { "###abcde##fgh##ijklmn###opqrst##uvwxy##z###",
      "##abcde#fgh#ijklmn##opqrst#uvwxy#z##" },
  };

  for (size_t i = 0; i < sizeof(data) / sizeof(*data); i++)
  {
    std::string r;
    String::StringManip::replace(String::SubString(data[i][0]), r,
      String::SubString("##"), String::SubString("#"));
    if (r != data[i][1])
    {
      std::cerr << "replace returned >>" << r << "<< instead of >>" <<
        data[i][1] << "<< for >>" << data[i][0] << "<<" << std::endl;
    }
  }
}

template <typename Integer>
void
test_str_to_int(const char* type) throw (eh::Exception)
{
  for (Integer i = std::numeric_limits<Integer>::min();;)
  {
    Stream::Stack<128> ostr;
    ostr << i;
    Integer j;
    if (!String::StringManip::str_to_int(ostr.str(), j) || j != i)
    {
      std::cerr << "str_to_int failed with " <<
        type << " " << i << std::endl;
    }
    if (i == std::numeric_limits<Integer>::max())
    {
      break;
    }
    ++i;
  }

  for (long long i = 1; i < 100; i++)
  {
    Integer k;
    {
      Stream::Stack<128> ostr;
      long long j = std::numeric_limits<Integer>::min() - i;
      ostr << j;
      if (String::StringManip::str_to_int(ostr.str(), k))
      {
        std::cerr << "str_to_int not failed with " <<
          type << " " << j << std::endl; 
      }
    }
    {
      Stream::Stack<128> ostr;
      long long j = std::numeric_limits<Integer>::max() + i;
      ostr << j;
      if (String::StringManip::str_to_int(ostr.str(), k))
      {
        std::cerr << "str_to_int not failed with " <<
          type << " " << j << std::endl; 
      }
    }
  }
}

void
test_str_to_int() throw (eh::Exception)
{
  test_str_to_int<bool>("bool");
  test_str_to_int<short>("short");
  test_str_to_int<unsigned short>("unsigned short");
}

const struct
{
  const char* src;
  size_t octets;
  const char* dst;
} UTF8_SUBSTR[] =
{
  { "abc", 10, "abc" },
  { "abc", 0, "" },
  { "abc", 1, "a" },
  { "abc", 2, "ab" },
  { "abc", 3, "abc" },
  { "\x80", 0, "" },
  { "\x80", 1, 0 },
  { "\x80", 2, 0 },
  { "\xE3\x91", 1, "" },
  { "\xE3\x91", 2, "" },
  { "\xE3\x91", 3, 0 },
  { "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8", 0, "" },
  { "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8", 2, "" },
  { "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8", 4,
    "\xE3\x91\x98" },
  { "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8", 6,
    "\xE3\x91\x98\xDD\x85" },
  { "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8", 8,
    "\xE3\x91\x98\xDD\x85\xE5\x92\x98" },
  { "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8", 10,
    "\xE3\x91\x98\xDD\x85\xE5\x92\x98" },
  { "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8", 12,
    "\xE3\x91\x98\xDD\x85\xE5\x92\x98\xF1\x85\x99\xB8" },
  { "\xE3\x91\x98\xDD\x85\xE5" "a\x98\xF1\x85\x99\xB8", 12, 0 },
  { 0, 0, 0 }
};

void
test_utf8_substr() throw (eh::Exception)
{
  for (size_t i = 0; UTF8_SUBSTR[i].src; i++)
  {
    String::SubString dst;
    if (String::StringManip::utf8_substr(
      String::SubString(UTF8_SUBSTR[i].src), UTF8_SUBSTR[i].octets, dst))
    {
      if (UTF8_SUBSTR[i].dst)
      {
        if (UTF8_SUBSTR[i].dst != dst)
        {
          std::cerr << FNS << i << " got '" << dst << "' but not '" <<
            UTF8_SUBSTR[i].dst << "'" << std::endl;
        }
      }
      else
      {
        std::cerr << FNS << i << " got '" << dst << "' but not error " <<
          std::endl;
      }
    }
    else
    {
      if (UTF8_SUBSTR[i].dst)
      {
        std::cerr << FNS << i << " got error but not '" <<
          UTF8_SUBSTR[i].dst << "'" << std::endl;
      }
    }
  }
}

void
test_hex() throw (eh::Exception)
{
  struct Hex
  {
    const char* data;
    size_t size;
    const char* noskip;
    const char* skip;
  };

  static const Hex hex[] =
  {
    { "", 0, "", "" },
    { "\x0F", 1, "0F", "F" },
    { "\xFE", 1, "FE", "FE" },
    { "\xEF\x00", 2, "EF00", "EF00" },
    { "\x00\xCD\x00", 3, "00CD00", "CD00" },
    { "\x00\x00\x00\x00", 4, "00000000", "0" },
    { "\x07\x00\xAB\x00\x89\x00", 6, "0700AB008900", "700AB008900" },
    { "\x00\x00\xAB\x00\x89\x00\x00", 7, "0000AB00890000", "AB00890000" },
  };

  for (size_t i = 0; i < sizeof(hex) / sizeof(*hex); i++)
  {
    const Hex& h = hex[i];
    std::string result = String::StringManip::hex_encode(
      reinterpret_cast<const unsigned char*>(h.data), h.size, false);
    if (result != h.noskip)
    {
      std::cerr << "Failed hex_encode(false) got '" << result <<
        "' instead of '" << h.noskip << "'" << std::endl;
    }
    result = String::StringManip::hex_encode(
      reinterpret_cast<const unsigned char*>(h.data), h.size, true);
    if (result != h.skip)
    {
      std::cerr << "Failed hex_encode(true) got '" << result <<
        "' instead of '" << h.skip << "'" << std::endl;
    }

    Generics::ArrayByte data;
    size_t size = String::StringManip::hex_decode(
      String::SubString(h.noskip), data, false);
    if (size != h.size || memcmp(data.get(), h.data, size))
    {
      std::cerr << "Failed hex_decode(noskip, false)" << std::endl;
    }
    data.reset(0);
    size = String::StringManip::hex_decode(
      String::SubString(h.noskip), data, true);
    if (size != h.size || memcmp(data.get(), h.data, size))
    {
      std::cerr << "Failed hex_decode(noskip, true)" << std::endl;
    }
    if (*h.data)
    {
      data.reset(0);
      try
      {
        size = String::StringManip::hex_decode(
          String::SubString(h.skip), data, false);
        if (*h.noskip == '0')
        {
          std::cerr << "Errorneously succeded hex_decode(skip, false)" <<
            std::endl;
        }
        else
        {
          if (size != h.size || memcmp(data.get(), h.data, size))
          {
            std::cerr << "Failed hex_decode(skip, false) " << std::endl;
          }
        }
      }
      catch (const String::StringManip::InvalidFormatException&)
      {
        if (*h.noskip != '0')
        {
          std::cerr << "Errorneously failed hex_decode(skip, false)" <<
            std::endl;
        }
      }
      data.reset(0);
      size = String::StringManip::hex_decode(
        String::SubString(h.skip), data, true);
      if (size != h.size || memcmp(data.get(), h.data, size))
      {
        std::cerr << "Failed hex_decode(skip, true) " << std::endl;
      }
    }
  }
}

int
main(int argc, char** argv)
{
  return 0;

  srand(time(0));
  
  try
  {
    std::cout << "StringManip test started.." << std::endl;
    check_mark();
    check_replace();
    check_flatten();
    check_random_flatten();
    std::string s1, s2;
    String::StringManip::trim(s1, s2);

    test_trim();
    test_charcheck();
    test_tokenizer();
    string_manip_test(argc, argv);
    test_base64<true>();
    test_base64<false>();
    test_js_encode();
    test_json_encode();
    test_js_unicode_encode();
    test_xml();
    test_csv_encode();
    test_str_to_int();
    test_utf8_substr();
    test_hex();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (eh::Exception& e)
  {
    std::cerr << "\nFAIL: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "\nFAIL: unknown exception" << std::endl;
    throw;
  }
  return 0;
}
