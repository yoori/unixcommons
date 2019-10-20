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

#include <Language/GenericSegmentor/Polyglot.hpp>

#include <Language/BLogic/NormalizeTrigger.hpp>


//#define DP

const char* tests[][2] =
{
  {
    "     aaa  \"   bbb   ccc    ddd  \"  eee   ",
    "aaa \"bbb ccc ddd\" eee"
  },
  {
    "aaa\\\"\\\"\\\\\"bbb ccc\\\"\\\\\\ddd\\e\\ ",
    "aaa \"bbb ccc\" \"ddd e\""
  },
  {
    "AaAa \"BBb aAa\" CCC",
    "aaaa \"bbb aaa\" ccc"
  },
  {
    "Ddd \"bBb AAA\" AAAA",
    "aaaa \"bbb aaa\" ddd"
  },
  {
    "DDd\\ AAAA\"aaa bbb\"aaaa\\ zzz",
    "\"aaa bbb\" aaaa ddd zzz"
  },
  {
    "aaa\\ \\ \\ bbb",
    "aaa bbb"
  },
  {
    "\"aa xx\" aa\\ \\ bb",
    "\"aa xx\" bb"
  },
  {
    "\\\\ \\-",
    ""
  },
  {
    "\\ \\s\\ ",
    "s"
  },
  {
    "\\\\\\",
    ""
  },
  {
    "\\\\\\\\",
    ""
  },
  {
    "\"aaa\"",
    "aaa"
  },
  {
    "\"aaa \\\\\\\"\"\"",
    "aaa"
  },
  {
    "\"a a\" \"A A\" ",
    "\"a a\""
  },
  {
    "-b a",
    "a b"
  },
  {
    "a -b",
    "a b"
  },
  {
    "a \\-b",
    "a b"
  },
  {
    "\\-b \\-a",
    "a b"
  },
  {
    "-a -b",
    "a b"
  },
  {
    "\"",
    0
  },
  {
    "a\"b\"c\"d",
    0
  },
  {
    "\xBE""a",
    "a"
  },
  {
    " \t --a",
    "a"
  },
  {
    "-\\-a",
    "a"
  },
  {
    "\\-a",
    "a"
  },
  {
    "\xC4\xB1",
    "i"
  },
  {
    "\"a\" a",
    "a"
  },
  {
    "a \"a\"",
    "a"
  },
  {
    "a \"a b\"",
    "\"a b\""
  },
  {
    "\"a b\" b",
    "\"a b\""
  },
  {
    "a b \"a b c\" c b \"c\"",
    "\"a b c\""
  },
  {
    "a b \"a b c\" \"b c\" c b \"c\" \"bc\" bc",
    "\"a b c\" bc"
  },
  {
    "   [   exact   match \t   TrIgGeR ]    ",
    "[exact match trigger]"
  },
  {
    " - [ aaa ]",
    0
  },
  {
    " aaa ]",
    0
  },
  {
    " - bbb [ aaa",
    0
  },
  {
    " [ aaa ",
    0
  },
  {
    " [ aaa [",
    0
  },
  {
    "[",
    0
  },
  {
    "]",
    0
  },
  {
    "[a b e+d]",
    "[a b e d]"
  },
  {
    "a",
    "a"
  },
  {
    "a bc",
    "a bc"
  },
  {
    "\"a bc\"",
    "\"a bc\""
  },
  {
    0,
    0
  }
};

static Language::Segmentor::SegmentorInterface_var segmentor;

void
test() throw (eh::Exception)
{
  for (int i = 0; tests[i][0]; i++)
  {
#ifdef DP
    std::cout << i << ": >" << tests[i][0] << "<" << std::endl;
    std::cout << i << ": >" << (tests[i][1] ? tests[i][1] : "ERROR") <<
      "<" << std::endl;
#endif

    try
    {
      Language::Trigger::Trigger result;

      Language::Trigger::normalize(String::SubString(tests[i][0]), result,
        segmentor.in());

#ifdef DP
      for (size_t j = 0; j < result.parts.size(); j++)
      {
        const char* q = result.parts[j].quotes ? "\"" : "";
        std::cout << ">" << q << result.parts[j].part << q << "<\n";
      }
#endif

      if (!tests[i][1])
      {
        std::cerr << i << ">>" << tests[i][0] <<
          "<<: No exception but " << result.trigger.size() << ">" <<
          result.trigger << "<" << std::endl;
      }
      else
      {
        if (result.trigger != tests[i][1])
        {
          std::cerr << i << ": Got " << result.trigger.size() <<
            ">" << result.trigger << "< but not " << strlen(tests[i][1]) <<
            ">" << tests[i][1] << "<" << std::endl;
        }
      }
    }
    catch (const Language::Trigger::Exception& ex)
    {
      if (tests[i][1])
      {
        std::cerr << i << ": Got exception " << ex.what() << "but not >" <<
          tests[i][1] << "<" << std::endl;
      }
    }

#ifdef DP
    std::cout << std::endl;
#endif
  }
}

int main(int argc, char** argv)
{
  try
  {
    segmentor = new Language::Segmentor::NormalizePolyglotSegmentor(
      "/opt/oix/polyglot/dict/");

    test();

    for (int i = 1; i < argc; i++)
    {
      String::SubString src;
      std::string str;

      if (!strcmp(argv[i], "-"))
      {
        if (std::cin.eof())
        {
          continue;
        }

        i--;

        std::getline(std::cin, str);
        if (str.empty())
        {
          continue;
        }
        src = str;
      }
      else
      {
        src = String::SubString(argv[i]);
      }

      std::string result;

      try
      {
        Language::Trigger::normalize(src, result, segmentor.in());
        std::cout << "Trigger >>" << src << "<< is normalized into >>" <<
          result << "<<" << std::endl;
      }
      catch (const eh::Exception& ex)
      {
        std::cerr << "Error normalizing trigger >>" << src << "<<: " <<
          ex.what() << std::endl;
      }

      Language::Trigger::Trigger trigger;

      try
      {
        Language::Trigger::normalize(src, trigger, segmentor.in());
        std::cout << "Trigger >>" << src << "<< is normalized into >>" <<
          trigger.trigger << "<<" << std::endl;
        std::cout << (trigger.exact ? "Exact" : "Not exact") << std::endl;
        for (Language::Trigger::Trigger::Parts::const_iterator itor(
          trigger.parts.begin()); itor != trigger.parts.end(); ++itor)
        {
          std::cout << ">>>" << (itor->quotes ? "\"" : "") << itor->part <<
            (itor->quotes ? "\"" : "") << "<<<" << std::endl;
        }
      }
      catch (const eh::Exception& ex)
      {
        std::cerr << "Error normalizing trigger >>" << src << "<<: " <<
          ex.what() << std::endl;
      }
    }

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception caught: " << e.what() << std::endl;
  }

  return -1;
}
