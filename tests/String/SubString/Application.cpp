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
#include <vector>
#include <eh/Exception.hpp>

#include <String/SubString.hpp>

#include <Generics/Function.hpp>


typedef String::BasicSubString<const char, String::CharTraits<char>,
  String::CheckerRough<char> > RoughSubString;
typedef String::BasicSubString<const wchar_t, String::CharTraits<char>,
  String::CheckerRough<char> > WRoughSubString;

namespace
{
  const char STR[] = "STD_STR";
  const char STR_B[] = "std_str";
  std::string str(STR);
  RoughSubString substr(str);
  std::string str_b(STR_B);
  RoughSubString substr_b(str_b);
}

void
check_operators() throw (eh::Exception)
{
  if (!(substr == STR))
  {
    std::cerr << FNS << "fail 1" << std::endl;
  }
  if (!(STR == substr))
  {
    std::cerr << FNS << "fail 2" << std::endl;
  }
  if (!(substr == substr))
  {
    std::cerr << FNS << "fail 3" << std::endl;
  }
  if (!(substr == str))
  {
    std::cerr << FNS << "fail 4" << std::endl;
  }
  if (!(str == substr))
  {
    std::cerr << FNS << "fail 5" << std::endl;
  }
  if (substr != STR)
  {
    std::cerr << FNS << "fail 6" << std::endl;
  }
  if (STR != substr)
  {
    std::cerr << FNS << "fail 7" << std::endl;
  }
  if (substr != substr)
  {
    std::cerr << FNS << "fail 8" << std::endl;
  }
  if (substr != str)
  {
    std::cerr << FNS << "fail 9" << std::endl;
  }
  if (str != substr)
  {
    std::cerr << FNS << "fail 10" << std::endl;
  }
}

void
check_less() throw (eh::Exception)
{
  if (!(substr < substr_b))
  {
    std::cerr << FNS << "fail 1" << std::endl;
  }
  if (!(substr < STR_B))
  {
    std::cerr << FNS << "fail 2" << std::endl;
  }
  if (substr < substr)
  {
    std::cerr << FNS << "fail 3" << std::endl;
  }
  if (substr < str)
  {
    std::cerr << FNS << "fail 4" << std::endl;
  }
  if (str < substr)
  {
    std::cerr << FNS << "fail 5" << std::endl;
  }
  if (STR_B < substr)
  {
    std::cerr << FNS << "fail 6" << std::endl;
  }
}

namespace
{
  std::string str2test("BEEE");
}

void
check_compare() throw (eh::Exception)
{
  const std::string STANDARD("BE");
  const char* STRINGS[] =
  {
    "A", "D", "BEE", "BE"
  };
  RoughSubString range(STANDARD.data(), STANDARD.size());

  for (std::size_t i = 0; i < sizeof(STRINGS) / sizeof(STRINGS[0]); ++i)
  {
    int result = range.compare(STRINGS[i]);
    result = result < 0 ? -1 : result > 0 ? 1 : 0;
    int std_result = STANDARD.compare(STRINGS[i]);
    std_result = std_result < 0 ? -1 : std_result > 0 ? 1 : 0;
    if (result != std_result)
    {
      std::cerr << FNS << "Fail: difference comparing std::string, our="
        << result << ", std=" << std_result << std::endl;
    }
  }
  const char S[] = "BE";
  if (range.compare(0, 5, S, 1) <= 0)
  {
    std::cerr << FNS << "fail 1" << std::endl;
  }
  if (range.compare(0, 5, S, 2) != 0)
  {
    std::cerr << FNS << "fail 2" << std::endl;
  }
  try
  {
    range.compare(0, 5, 0, 2);
    std::cerr << FNS << "fail 3" << std::endl;
  }
  catch (const RoughSubString::LogicError& e)
  {
  }
  catch (...)
  {
    std::cerr << FNS << "fail 4" << std::endl;
  }
  {
    char str[] = "A";
    RoughSubString(str).compare(0, sizeof(str), str, sizeof(str));
  }
}

void
check_constructor() throw (eh::Exception)
{
  try
  {
    RoughSubString s(static_cast<char*>(0), 1234);
    std::cerr << "Expected LogicError, but nothing thrown." << std::endl;
  }
  catch (const RoughSubString::LogicError& e)
  {
  }
  catch (...)
  {
    std::cerr << "Expected LogicError, but unknown type thrown."
      << std::endl;
  }
  try
  {
    RoughSubString s2(0);
    std::cerr << "Expected LogicError, but nothing thrown." << std::endl;
  }
  catch (const RoughSubString::LogicError& e)
  {
  }
  catch (...)
  {
    std::cerr << "Expected LogicError, but unknown type thrown."
      << std::endl;
  }
}

void
check_copy_constructible() throw (eh::Exception)
{
  const char S1[] = "str1";
  const char S2[] = "str2";
  const char LS1[] = "str            1";
  const char LS2[] = "str            2";
  const RoughSubString str1(S1, sizeof(S1));
  const RoughSubString str2(S2, sizeof(S2));
  const RoughSubString long_str1(LS1, sizeof(LS1));
  const RoughSubString long_str2(LS2, sizeof(LS2));

  RoughSubString copy_str1(str1);
  RoughSubString copy_str2(str2);
  RoughSubString copy_long_str1(long_str1);
  RoughSubString copy_long_str2(long_str2);

  {
    RoughSubString s(str1);
    s = copy_long_str1;
    if (s != long_str1)
    {
      std::cerr << FNS <<"fail 1" << std::endl;
    }
  }
  {
    copy_str1.swap(copy_str2);
    if (!(copy_str1 == str2 && copy_str2 == str1))
    {
      std::cerr << FNS << "fail 2" << std::endl;
    }
    copy_str1.swap(copy_str2);
  }
  {
    copy_long_str1.swap(copy_long_str2);
    if (!(copy_long_str1 == long_str2 && copy_long_str2 == long_str1))
    {
      std::cerr << FNS << "fail 3" << std::endl;
    }
    copy_long_str1.swap(copy_long_str2);
  }
  {
    copy_str1.swap(copy_long_str1);
    if (!(copy_str1 == long_str1 && copy_long_str1 == str1))
    {
      std::cerr << FNS << "fail 4" << std::endl;
    }
    copy_str1.swap(copy_long_str1);
  }
  {
    copy_long_str1.swap(copy_str1);
    if (!(copy_str1 == long_str1 && copy_long_str1 == str1))
    {
      std::cerr << FNS << "fail 4" << std::endl;
    }
    copy_long_str1.swap(copy_str1);
  }
  {
    std::vector<RoughSubString> sstr_vector;
    sstr_vector.push_back(copy_str1);
    sstr_vector.push_back(copy_long_str1);
    sstr_vector.push_back(copy_str2);
    sstr_vector.push_back(copy_long_str2);
    if (!(sstr_vector[0] == str1 &&
          sstr_vector[1] == long_str1 &&
          sstr_vector[2] == str2 &&
          sstr_vector[3] == long_str2))
    {
      std::cerr << FNS << "fail 5" << std::endl;
    }
  }
}

void
check_erase() throw (eh::Exception)
{
  const char c_str[] = "Hello, World!";
  RoughSubString str(c_str);
  if (str != c_str)
  {
    std::cerr << FNS << "fail 1" << std::endl;
  }
  for (std::size_t i = 0; i <= sizeof(c_str); ++i)
  {
    str.erase_front(1);
    switch (i)
    {
      case 0:
        if (str[i] != 'e')
        {
          std::cerr << FNS << i << " fail 2 " << str[i] << std::endl;
        }
        break;
      case sizeof(c_str) - 3:
        if (str[0] != '!')
        {
          std::cerr << FNS << i << " fail 3 " << str[0] << std::endl;
        }
    }
  }
  if (str.size())
  {
    std::cerr << FNS << "fail 0" << std::endl;
  }
  str = RoughSubString(c_str);
  for (std::size_t i = 0; i <= sizeof(c_str); ++i)
  {
    str.erase_back(1);
    switch (i)
    {
      case 0:
        if (str[i] != 'H')
        {
          std::cerr << FNS << i << "fail 5 " << str[i] << std::endl;
        }
        break;
      case sizeof(c_str) - 1:
        if (str[0] != 'H')
        {
          std::cerr << FNS << i << " fail 6 " << str[0] << std::endl;
        }
    }
  }
  if (str.size())
  {
    std::cerr << FNS << "fail 7" << std::endl;
  }
}

void
check_copy() throw (eh::Exception)
{
  RoughSubString s("foo");
  char dest[4];
  dest[0] = dest[1] = dest[2] = dest[3] = 1;
  s.copy(dest, 4);
  int pos = 0;
  if (dest[pos++] != 'f')
  {
    std::cerr << FNS << "fail 1" << std::endl;
  }
  if (dest[pos++] != 'o')
  {
    std::cerr << FNS << "fail 2" << std::endl;
  }
  if (dest[pos++] != 'o')
  {
    std::cerr << FNS << "fail 3" << std::endl;
  }
  if (dest[pos++] != 1)
  {
    std::cerr << FNS << "fail 4" << std::endl;
  }

  dest[0] = dest[1] = dest[2] = dest[3] = 1;
  s.copy(dest, 4, 2);
  pos = 0;
  if (dest[pos++] != 'o')
  {
    std::cerr << FNS << "fail 5 " << dest[0] << std::endl;
  }
  if (dest[pos++] != 1)
  {
    std::cerr << FNS << "fail 6" << std::endl;
  }
}

void
check_assign() throw (eh::Exception)
{
  RoughSubString s;
  const char C_STR[] = "test string for assign";

  s.assign(C_STR, C_STR + 22);
  if (s != "test string for assign")
  {
    std::cerr << FNS << "fail 1" << std::endl;
  }

  RoughSubString s2("other test string");
  s.assign(s2);
  if (s != s2)
  {
    std::cerr << FNS << "fail 2" << std::endl;
  }

  RoughSubString str1;
  RoughSubString str2;

  // short string optim:
  str1 = RoughSubString("123456");
  // longer than short string:
  str2 = RoughSubString("1234567890123456789012345678901234567890");

  if (str1[5] != '6')
  {
    std::cerr << FNS << "fail 3" << std::endl;
  }
  if (str2[29] != '0')
  {
    std::cerr << FNS << "fail 4" << std::endl;
  }

  str1.assign(str2, 5, RoughSubString::NPOS);
  if (str1[0] != '6')
  {
    std::cerr << FNS << "fail 5" << std::endl;
  }
}

void
check_out() throw (eh::Exception)
{
  RoughSubString str("RoughSubString");
  {
    std::ostringstream ostr;
    ostr << str;
    if (!ostr.good())
    {
      std::cerr << FNS << "fail 1" << std::endl;
    }
    if (str != ostr.str())
    {
      std::cerr << FNS << "fail 2" << std::endl;
    }
  }
}

void
check_traits() throw (eh::Exception)
{
  RoughSubString s1, s2;
  if (!s1.compare("str1"))
  {
    std::cerr << FNS << "fail 1 " << std::endl;
  }
  if (s1.compare(s2))
  {
    std::cerr << FNS << "fail 2 " << std::endl;
  }
  if (s1.compare(0, 2, s2))
  {
    std::cerr << FNS << "fail 3 " << std::endl;
  }
  if (s1.compare(0, 2, s2, 0, 5))
  {
    std::cerr << FNS << "fail 4 " << std::endl;
  }
}

void
check_equal() throw (eh::Exception)
{
#if __GNUC__ < 4 || __GNUC__ == 4 && __GNUC_MINOR__ <= 1
  char c_str[] = "str***";
  RoughSubString s(c_str, 3);
  c_str[3]  = '1';
  if (s == "st")
  {
    std::cerr << FNS << "fail 1" << std::endl;
  }
  if (s != "str")
  {
    std::cerr << FNS << "fail 2" << std::endl;
  }
  if (s == "str1")
  {
    std::cerr << FNS << "fail 3" << std::endl;
  }
  if (!substr.equal(STR))
  {
    std::cerr << FNS << "fail 4" << std::endl;
  }
  if (!substr.equal(str))
  {
    std::cerr << FNS << "fail 5" << std::endl;
  }
#endif
}

template <typename T>
void
check_find(const char* type, T s0, T sn, T s12, T s4) throw (eh::Exception)
{
  const char STR[] = "Saample";
  RoughSubString s(STR, sizeof(STR));
  RoughSubString s_null;
  if (s.rfind(s0) != 0)
  {
    std::cerr << FNS << "fail 0" << type << std::endl;
  }
  if (s.rfind(s12) != 2)
  {
    std::cerr << FNS << "fail 1" << type << std::endl;
  }
  if (s.rfind(sn) != RoughSubString::NPOS)
  {
    std::cerr << FNS << "fail 2" << type << std::endl;
  }
  if (s.rfind(s12, sizeof(STR)) != 2 ||
      s.rfind(s12, sizeof(STR) + 100) != 2 ||
      s.rfind(s12, sizeof(STR) - 1) != 2 ||
      s.rfind(s12, 1) != 1)
  {
    std::cerr << FNS << "fail 3" << type << std::endl;
  }
  if (s_null.rfind('\0') != RoughSubString::NPOS ||
      s_null.rfind('\0', 0) != RoughSubString::NPOS)
  {
    std::cerr << FNS << "fail 4" << type << std::endl;
  }
  if (s.find(s4, 1) != 4 || s.find(s4, 4) != 4 ||
    s.find(s4, 5) != RoughSubString::NPOS)
  {
    std::cerr << FNS << "fail 5" << std::endl;
  }
}

void
check_find() throw (eh::Exception)
{
 check_find("char", 'S', 'A', 'a', 'p');
 check_find("string", RoughSubString("S"), RoughSubString("A"), RoughSubString("a"),
   RoughSubString("p"));
}

void
check_compile_constrain() throw ()
{
  String::SubString a;
#if 0
  a != 0;
#endif
#if 0
  0 != a;
#endif
#if 0
  a == 0;
#endif
#if 0
  0 == a;
#endif
#if 0
  a < 0;
#endif
#if 0
  0 < a;
#endif
#if 0
  String::SubString b(0, 1234);
#endif
}

template <typename T1, typename T2>
void
check_plus(const T1& s1, const T2& s2) throw (eh::Exception)
{
  if (s1 + s2 != "123")
  {
    std::cerr << FNS << "addition failed" << std::endl;
  }
}

void
check_plus() throw (eh::Exception)
{
  check_plus(String::SubString("1"), String::SubString("23"));
  check_plus(std::string("12"), String::SubString("3"));
  check_plus(String::SubString("1"), std::string("23"));
}

int
main(int /*argc*/, char** /*argv*/)
{
  try
  {
    std::cout << "SubStringManip test started.." << std::endl;

    check_operators();
    check_less();
    check_constructor();
    check_copy_constructible();
    check_assign();
    check_compare();
    check_erase();
    check_copy();
    check_traits();
    check_out();
    check_equal();
    check_find();
    check_compile_constrain();
    check_plus();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (eh::Exception& e)
  {
    std::cerr << "\nFAIL: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "\nFAIL: unknown exception" << std::endl;
  }
  return 0;
}
