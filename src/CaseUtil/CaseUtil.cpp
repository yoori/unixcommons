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
#include <fstream>
#include <iterator>

#include <String/UTF8Case.hpp>

#include <Generics/AppUtils.hpp>


class Application
{
public:
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  int
  run(int argc, char* argv[]) throw (eh::Exception);

private:
  void
  usage_() throw (eh::Exception);

  typedef bool (*CaseChange)(const String::SubString& src, std::string& dst,
    std::size_t* counter);

  template <typename Convert>
  CaseChange
  get_case_change() throw ();

  void
  convert_(std::istream& istr, CaseChange case_change)
    throw (eh::Exception);

  Generics::AppUtils::CheckOption help_;
  Generics::AppUtils::CheckOption uniform_;
  Generics::AppUtils::CheckOption upper_;
  Generics::AppUtils::CheckOption lower_;
  Generics::AppUtils::CheckOption simplify_;
};


//
// Application class
//

void
Application::usage_() throw (eh::Exception)
{
  std::cout << "Usage:\n"
    "CaseUtil [--help | -h] [--uniform | --upper | --lower | --simplify] "
      "[<filename>]\n"
    "Utility changes the case of each line from stdin or the file\n\n"
    "\t<filename>      - use the file instead of stdin for lines\n"
    "\t--uniform       - use Full Case Folding (default)\n"
    "\t--upper         - use ToUpper conversion\n"
    "\t--lower         - use ToLower conversion\n"
    "\t--simplify      - use Simplify conversion\n"
    "" << std::endl;
}

template <typename Convert>
Application::CaseChange
Application::get_case_change() throw ()
{
  return String::case_change<Convert, std::char_traits<char>,
    std::allocator<char> >;
}

int
Application::run(int argc, char* argv[]) throw (eh::Exception)
{
  try
  {
    Generics::AppUtils::Args args(1);

    args.add(Generics::AppUtils::equal_name("help") ||
      Generics::AppUtils::short_name("h"), help_);
    args.add(Generics::AppUtils::equal_name("uniform"), uniform_);
    args.add(Generics::AppUtils::equal_name("upper"), upper_);
    args.add(Generics::AppUtils::equal_name("lower"), lower_);
    args.add(Generics::AppUtils::equal_name("simplify"), simplify_);
    args.parse(argc, argv);

    if (help_.enabled())
    {
      usage_();
      return 0;
    }

    if ((uniform_.enabled() && (upper_.enabled() || lower_.enabled())) ||
      (upper_.enabled() && lower_.enabled()))
    {
      throw Exception("Invalid arguments");
    }

    CaseChange case_change = get_case_change<String::Uniform>();
    if (upper_.enabled())
    {
      case_change = get_case_change<String::Upper>();
    }
    else if (lower_.enabled())
    {
      case_change = get_case_change<String::Lower>();
    }
    else if (simplify_.enabled())
    {
      case_change = get_case_change<String::Simplify>();
    }

    const Generics::AppUtils::Args::CommandList& commands = args.commands();
    if (commands.empty())
    {
      convert_(std::cin, case_change);
    }
    else
    {
      std::ifstream in(commands.front().c_str());
      if (!in)
      {
        Stream::Error ostr;
        ostr << "Failed to open '" << commands.front() << "'";
        throw Exception(ostr);
      }
      convert_(in, case_change);
    }

    return 0;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl << std::endl;
  }

  usage_();
  return -1;
}

void
Application::convert_(std::istream& istr, CaseChange case_change)
  throw (eh::Exception)
{
  std::string src;
  while (getline(istr, src))
  {
    std::string dst;
    std::size_t counter;
    if (case_change(src, dst, &counter))
    {
      std::cout << dst << std::endl;
    }
    else
    {
      std::cerr << "Non-UTF-8 line: '" << src << "'" << std::endl;
      if (&istr != &std::cin)
      {
        break;
      }
    }
  }
}

int
main(int argc, char* argv[])
{
  try
  {
    Application app;
    return app.run(argc - 1, argv + 1);
  }
  catch (...)
  {
  }

  return -1;
}
