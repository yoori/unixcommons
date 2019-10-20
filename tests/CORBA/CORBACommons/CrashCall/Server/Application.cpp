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

#include <CORBACommons/ProcessControlImpl.hpp>
#include <CORBACommons/CorbaAdapters.hpp>
#include <CORBAConfigParser/ParameterConfig.hpp>
#include <Logger/StreamLogger.hpp>
#include <Generics/AppUtils.hpp>

#include "Application.hpp"

namespace
{
  const char PROCESS_CONTROL_SERVANT[] = "ProcessControl";
  const char TEST_INT_SERVANT[] = "TestCrash";
  const char EXT_TEST_INT_SERVANT[] = "TestCrash";
  const char EXT_TEST_INT_SECURE_SERVANT[] = "SecureTestCrash";
}

Application::Application() throw (eh::Exception)
{
}

Application::~Application() throw ()
{
}

void Application::run(int argc, char* argv[])
  throw (Exception, eh::Exception)
{
  try
  {
    Generics::AppUtils::Option<unsigned long> opt_port;
    Generics::AppUtils::Option<unsigned long> opt_secure_port;
    Generics::AppUtils::Option<std::string> opt_host("localhost");
    CORBAConfigParser::SecureParamsOption opt_secure_params;
    Generics::AppUtils::Args args;

    args.add(
      Generics::AppUtils::equal_name("port") ||
      Generics::AppUtils::short_name("p"),
      opt_port);
    args.add(
      Generics::AppUtils::equal_name("host") ||
      Generics::AppUtils::short_name("h"),
      opt_host);
    args.add(
      Generics::AppUtils::equal_name("secure-port"),
      opt_secure_port);
    args.add(
      Generics::AppUtils::equal_name("secure-params") ||
      Generics::AppUtils::short_name("sp"),
      opt_secure_params);

    args.parse(argc - 1, argv + 1);

    CORBACommons::CorbaConfig corba_config;
    corba_config.thread_pool = 1;

    if (opt_port.installed())
    {
      CORBACommons::EndpointConfig endpoint_config;
      endpoint_config.host = *opt_host;
      endpoint_config.port = *opt_port;
      endpoint_config.objects[TEST_INT_SERVANT].insert(
        EXT_TEST_INT_SERVANT);
      endpoint_config.objects[PROCESS_CONTROL_SERVANT].insert(
        PROCESS_CONTROL_SERVANT);
      corba_config.endpoints.push_back(endpoint_config);
    }

    if (opt_secure_port.installed() && opt_secure_params.installed())
    {
      CORBACommons::EndpointConfig endpoint_config;
      endpoint_config.host = *opt_host;
      endpoint_config.port = *opt_secure_port;
      endpoint_config.objects[TEST_INT_SERVANT].insert(
        EXT_TEST_INT_SECURE_SERVANT);
      endpoint_config.secure_connection_config = *opt_secure_params;
      corba_config.endpoints.push_back(endpoint_config);
    }

    Logging::FLogger_var logger(
      new Logging::OStream::Logger(Logging::OStream::Config(std::cout)));

    CORBACommons::CorbaServerAdapter_var corba_server_adapter(
      new CORBACommons::CorbaServerAdapter(corba_config, logger));

    CORBATest::TestCrashImpl_var test_impl(new CORBATest::TestCrashImpl());
    corba_server_adapter->add_binding(TEST_INT_SERVANT, test_impl);
    corba_server_adapter->add_binding(PROCESS_CONTROL_SERVANT, this);
    ProcessControlImpl::shutdowner_ = corba_server_adapter->shutdowner();
    corba_server_adapter->run();
    shutdowner_.reset();
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
    ReferenceCounting::QualPtr<Application> app(new Application);

    app->run(argc, argv);

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

  return 1;
}
