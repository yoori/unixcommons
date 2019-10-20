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
#include <Generics/AppUtils.hpp>
#include <CORBACommons/ProcessControlImpl.hpp>
#include <CORBACommons/CorbaAdapters.hpp>
#include <CORBAConfigParser/ParameterConfig.hpp>

#include "Application.hpp"
#include "TestIntImpl.hpp"


namespace
{
  const char PROCESS_CONTROL_SERVANT[] = "ProcessControl";
  const char TEST_INT_SERVANT[] = "TestInt";
  const char EXT_TEST_INT_SERVANT[] = "TestInt";
  const char EXT_TEST_INT_SECURE_SERVANT[] = "SecureTestInt";

  const unsigned long PORT = 10000;
  const unsigned long SECURE_PORT = 10001;
}

Application::Application() throw (eh::Exception)
  : error_state_(false)
{
}

Application::~Application() throw ()
{
}

void
Application::error(const char* message) throw ()
{
  error_state_ = true;
  std::cerr << message << std::endl;
};

char*
Application::control(const char* param_name, const char* param_value) throw ()
{
  std::cout << "Param '" << param_name << "' value '" << param_value << "'" <<
    std::endl;

  std::string result(param_name);
  result.push_back('=');
  result.append(param_value);

  return CORBA::String_var(result.c_str())._retn();
}

void
Application::run(int argc, char* argv[]) throw (Exception, eh::Exception)
{
  try
  {
    typedef std::vector<unsigned long> Ports;
    Generics::AppUtils::OptionsSet<Ports> opt_port;
    Generics::AppUtils::OptionsSet<Ports> opt_secure_port;
    Generics::AppUtils::Option<std::string> opt_host("*");
    CORBAConfigParser::SecureParamsOption opt_secure_params;
    Generics::AppUtils::Option<unsigned long> opt_threads(15);
    Generics::AppUtils::Option<unsigned long> opt_normal_threads(3);
    Generics::AppUtils::Option<unsigned long> opt_min_threads(2);
    Generics::AppUtils::CheckOption opt_orb_per_endpoint;
    typedef std::vector<std::string> Names;
    Generics::AppUtils::OptionsSet<Names> opt_name{Names()};
    Generics::AppUtils::Args args;

    args.add(
      Generics::AppUtils::equal_name("port") ||
      Generics::AppUtils::short_name("p"),
      opt_port, "IIOP endpoint port to listen to", "PORT NUMBER");
    args.add(
      Generics::AppUtils::equal_name("host") ||
      Generics::AppUtils::short_name("h"),
      opt_host, "interface to use for endpoints", "IP or HOSTNAME");
    args.add(
      Generics::AppUtils::equal_name("secure-port"),
      opt_secure_port, "SSLIOP endpoint port to listen to", "PORT NUMBER");
    args.add(
      Generics::AppUtils::equal_name("secure-params") ||
      Generics::AppUtils::short_name("sp"),
      opt_secure_params, "SSLIOP parameters");
    args.add(
      Generics::AppUtils::equal_name("threads") ||
      Generics::AppUtils::short_name("thr"),
      opt_threads, "Thread pool size");
    args.add(
      Generics::AppUtils::equal_name("norm-threads") ||
      Generics::AppUtils::short_name("nt"),
      opt_normal_threads, "Normal threads");
    args.add(
      Generics::AppUtils::equal_name("min-threads") ||
      Generics::AppUtils::short_name("mt"),
      opt_min_threads, "Minimum threads");
    args.add(
      Generics::AppUtils::short_name("ope"),
      opt_orb_per_endpoint, "Orb per endpoint");
    args.add(
      Generics::AppUtils::short_name("name"),
      opt_name, "Insecure name");

    try
    {
      args.parse(argc - 1, argv + 1);
    }
    catch (const Generics::AppUtils::Exception&)
    {
      std::cerr << "Usage:\n";
      args.usage(std::cerr);
      throw;
    }

    CORBACommons::CorbaConfig corba_config;
    corba_config.thread_pool = *opt_threads;
    corba_config.normal_threads = *opt_normal_threads;
    corba_config.min_threads = *opt_min_threads;
    corba_config.orb_per_endpoint = opt_orb_per_endpoint.enabled();

    if (opt_port.installed())
    {
      for (Ports::const_iterator itor((*opt_port).begin());
        itor != ((*opt_port).end()); ++itor)
      {
        CORBACommons::EndpointConfig endpoint_config;
        endpoint_config.host = *opt_host;
        endpoint_config.port = *itor;
        if (opt_name->empty())
        {
          endpoint_config.objects[TEST_INT_SERVANT].insert(
            EXT_TEST_INT_SERVANT);
        }
        else
        {
          for (Names::const_iterator itor(opt_name->begin());
            itor != opt_name->end(); ++itor)
          {
            endpoint_config.objects[TEST_INT_SERVANT].insert(
              *itor);
          }
        }
        endpoint_config.objects[PROCESS_CONTROL_SERVANT].insert(
          PROCESS_CONTROL_SERVANT);
        corba_config.endpoints.push_back(endpoint_config);
      }
    }

    if (opt_secure_port.installed() && opt_secure_params.installed())
    {
      for (Ports::const_iterator itor((*opt_secure_port).begin());
        itor != ((*opt_secure_port).end()); ++itor)
      {
        CORBACommons::EndpointConfig endpoint_config;
        endpoint_config.host = *opt_host;
        endpoint_config.port = *itor;
        endpoint_config.objects[TEST_INT_SERVANT].insert(
          EXT_TEST_INT_SECURE_SERVANT);
        endpoint_config.secure_connection_config = *opt_secure_params;
        corba_config.endpoints.push_back(endpoint_config);
      }
    }

    Logging::FLogger_var logger(
      new Logging::OStream::Logger(Logging::OStream::Config(
        std::cout, 1000)));

    CORBACommons::CorbaServerAdapter_var corba_server_adapter(
      new CORBACommons::CorbaServerAdapter(corba_config, logger));

    CORBATest::TestIntImpl_var test_int_impl(new CORBATest::TestIntImpl());
    corba_server_adapter->add_binding(TEST_INT_SERVANT, test_int_impl);
    corba_server_adapter->add_binding(PROCESS_CONTROL_SERVANT, this);
    ProcessControlImpl::shutdowner_ = corba_server_adapter->shutdowner();
    corba_server_adapter->run();
    shutdowner_.reset();

    std::cout << "Received requests: " <<
      test_int_impl->received_requests << std::endl;
  }
  catch (const CORBA::Exception& e)
  {
    Stream::Error ostr;
    ostr << "Application::run: CORBA::Exception caught. Description:\n" << e;
    throw Exception(ostr);
  }

  if (error_state_)
  {
    throw Exception("Application::run: servant found errors.");
  }
}

int
main(int argc, char** argv)
{
  const char* td = getenv("TAO_DEBUG");
  TAO_debug_level = td ? atoi(td) : 0;

  try
  {
    ReferenceCounting::QualPtr<Application> app(new Application);

    app->run(argc, argv);

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception exception caught. Description:" <<
      std::endl << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "main: unknown exception caught" << std::endl;
  }

  return 1;
}
