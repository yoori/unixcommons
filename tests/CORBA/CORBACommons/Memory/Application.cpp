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

#include <Logger/StreamLogger.hpp>
#include <Generics/Rand.hpp>
#include <Generics/AppUtils.hpp>
#include <Generics/Statistics.hpp>
#include <CORBACommons/CorbaAdapters.hpp>
#include <CORBAConfigParser/ParameterConfig.hpp>

#include "../Overload/Server/TestInt.hpp"

DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

void
test(CORBATest::TestInt_ptr test)
{
  test->print_memory(true);
  for (int i = 0; i < 5; i++)
  {
    CORBATest::Seq3_var(test->memory_test());
    test->print_memory(true);
  }
}

void
run(int argc, char* argv[]) throw (Exception, eh::Exception)
{
  try
  {
    Logging::FLogger_var logger(
      new Logging::OStream::Logger(Logging::OStream::Config(std::cout)));

    CORBACommons::CorbaClientConfig config;

    CORBACommons::CorbaClientAdapter_var corba_client_adapter(
      new CORBACommons::CorbaClientAdapter(config, logger));

    CORBAConfigParser::CorbaRefOption<CORBATest::TestInt> opt_url(
      corba_client_adapter.in());
    CORBAConfigParser::CorbaRefOption<CORBATest::TestInt> opt_secure_url(
      corba_client_adapter.in(),
      "server.key:adserver:server.der;ce.der");

    Generics::AppUtils::Args args;

    args.add(
      Generics::AppUtils::equal_name("url") ||
      Generics::AppUtils::short_name("u"),
      opt_url);
    args.add(
      Generics::AppUtils::equal_name("secure-url") ||
      Generics::AppUtils::short_name("su"),
      opt_secure_url);

    args.parse(argc - 1, argv + 1);

    if (opt_url.installed())
    {
      std::cout << "To test normal connection." << std::endl;

      CORBATest::TestInt_var test_int = *opt_url;

      test(test_int);

      std::cout << "Test normal connection finished." << std::endl;
    }

    if (opt_secure_url.installed())
    {
      std::cout << "To test secure connection." << std::endl;

      CORBATest::TestInt_var test_int = *opt_secure_url;

      test(test_int);

      std::cout << "Test secure connection finished." << std::endl;
    }
  }
  catch (const CORBA::Exception& e)
  {
    std::ostringstream ostr;
    ostr << "Application::run: CORBA::Exception caught. Description:\n"
         << e;

    throw Exception(ostr.str());
  }
}

int
main(int argc, char** argv)
{
  try
  {
    run(argc, argv);

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr
      << "main: eh::Exception exception caught. Description:" << std::endl
      << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "main: unknown exception caught" << std::endl;
  }

  return -1;
}
