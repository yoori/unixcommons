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
#include <deque>

#include <Generics/AppUtils.hpp>
#include <Generics/DirSelector.hpp>
#include <Generics/Network.hpp>



class IsThis
{
public:
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  int
  run(int argc, char* argv[]) throw (eh::Exception, Exception);

private:
  void
  init_(int argc, char* argv[]) throw (eh::Exception, Exception);
  void
  create_names_from_list_(const String::SubString& list)
    throw (eh::Exception, Exception);
  void
  create_names_from_dir_(const char* dir) throw (eh::Exception, Exception);
  void
  determine_() throw (eh::Exception);
  int
  resolute_() throw (eh::Exception);


  typedef std::deque<std::string> Names;


  Generics::AppUtils::CheckOption single_;
  Generics::AppUtils::CheckOption single_check_;
  Generics::AppUtils::CheckOption ls_;

  Names names_;

  int hits_;
  Stream::Dynamic hosts_;
};

class Application
{
public:
  int
  run(int argc, char* argv[]) throw (eh::Exception);

private:
  void
  usage_() throw (eh::Exception);
};


//
// IsThis class
//

void
IsThis::create_names_from_list_(const String::SubString& list)
  throw (eh::Exception, Exception)
{
  String::StringManip::SplitComma tokenizer(list);
  String::SubString name;
  while (tokenizer.get_token(name))
  {
    names_.push_back(name.str());
  }
}

void
IsThis::create_names_from_dir_(const char* dir)
  throw (eh::Exception, Exception)
{
  Generics::DirSelect::directory_selector(dir,
    Generics::DirSelect::list_creator(std::back_inserter(names_)), "[^.]*",
    Generics::DirSelect::DSF_NON_RECURSIVE |
    Generics::DirSelect::DSF_ALL_FILES |
    Generics::DirSelect::DSF_DONT_RESOLVE_LINKS |
    Generics::DirSelect::DSF_EXCEPTION_ON_OPEN |
    Generics::DirSelect::DSF_NO_EXCEPTION_ON_STAT |
    Generics::DirSelect::DSF_FILE_NAME_ONLY);
}

void
IsThis::init_(int argc, char* argv[]) throw (eh::Exception, Exception)
{
  Generics::AppUtils::Args args(1);

  args.add(Generics::AppUtils::equal_name("single"), single_);
  args.add(Generics::AppUtils::equal_name("single-check"), single_check_);
  args.add(Generics::AppUtils::equal_name("ls"), ls_);
  args.parse(argc, argv);

  const Generics::AppUtils::Args::CommandList& commands = args.commands();

  if (commands.size() != 1 ||
    (single_.enabled() && single_check_.enabled()))
  {
    throw Exception("Invalid arguments");
  }

  if (ls_.enabled())
  {
    create_names_from_dir_(commands.front().c_str());
  }
  else
  {
    create_names_from_list_(commands.front());
  }

}

void
IsThis::determine_() throw (eh::Exception)
{
  Generics::Network::IsLocalInterface is_local;

  hits_ = 0;

  for (Names::const_iterator itor(names_.begin()); itor != names_.end();
    ++itor)
  {
    bool check = false;
    try
    {
      check = is_local.check_host_name(itor->c_str());
    }
    catch (...)
    {
    }

    if (!check)
    {
      continue;
    }

    if (hits_)
    {
      hosts_ << ", ";
    }
    hosts_ << *itor;

    hits_++;

    if (single_.enabled())
    {
      break;
    }
  }
}

int
IsThis::resolute_() throw (eh::Exception)
{
  if (!hits_)
  {
    return 1;
  }

  if (hits_ > 1 && single_check_.enabled())
  {
    std::cerr << "More than one name suits to current host: '" <<
      hosts_.str() << "'" << std::endl;
    return 2;
  }

  std::cout << hosts_.str() << std::endl;

  return 0;
}

int
IsThis::run(int argc, char* argv[]) throw (eh::Exception, Exception)
{
  init_(argc, argv);

  if (names_.empty())
  {
    return 3;
  }

  determine_();

  return resolute_();
}


//
// Application class
//

void
Application::usage_() throw (eh::Exception)
{
  std::cout << "Usage: \n"
    "HostnameUtil is-this [--single | --single-check] "
    "(<list of hosts> | --ls <directory>)\n"
    "Utility determines which of the specified host names relate \n"
    "to the current host (those names are printed)\n"
    "Utility's exit status is zero if any of the specified names relate,\n"
    "or non-zero otherwise\n\n"
    "\t<list of hosts> - comma-separated list of hosts\n"
    "\t<directory>     - holds entries treated as host names\n"
    "\t--single        - print only one related name\n"
    "\t--single-check  - the only name must relate\n"
    "" << std::endl;
}

int
Application::run(int argc, char* argv[]) throw (eh::Exception)
{
  try
  {
    if (argc >= 2)
    {
      if (String::SubString("is-this", 7) == argv[1])
      {
        IsThis is_this;
        return is_this.run(argc - 2, argv + 2);
      }
    }
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return -1;
  }

  usage_();
  return -1;
}

int
main(int argc, char* argv[])
{
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch (...)
  {
  }

  return -1;
}
