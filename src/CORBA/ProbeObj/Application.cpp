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

#include <Generics/Time.hpp>

#include <Logger/StreamLogger.hpp>

#include <CORBACommons/ProcessControl.hpp>
#include <CORBAConfigParser/ParameterConfig.hpp>

#include "Application.hpp"

//#define TRACE

namespace
{
  const char USAGE[] = "Usage:\n"
  "  ProbeObj [-timeout <value_sec>] -shutdown [-wait-for-completion] <url>\n"
  "  ProbeObj [-timeout <value_sec>] -comment <url>\n"
  "  ProbeObj [-timeout <value_sec>] -control <param_name> <param_value> <url>\n"
  "  ProbeObj [-timeout <value_sec>] [-is-a-mode] "
    "[-retry <value_msec> [-count <value>]] "
    "[-message <text|'FAILURE_DESC'|'LOG_DESC'>] "
    "[-status <not_alive|alive|ready>] <url>";

  enum MESSAGE_TYPE
  {
    MT_NONE,
    MT_FAILURE_DESC,
    MT_LOG_DESC,
    MT_USER
  };

  enum PROCESS_RESULT
  {
    PR_ALL_CORRECT = 0,
    PR_UNEXPECTED_STATUS,
    PR_UNRECOVERABLE_EXCEPTION,
    PR_INVALID_ARGUMENT,
    PR_INVALID_REFERENCE,
  };

  std::string prefix;
}

int
Application::run(int& argc, char** argv)
  throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
    CORBA::Exception)
{
  int index = 1;

  if (index < argc && !strcmp(argv[index], "-tao-log"))
  {
    TAO_debug_level = 100;
    index++;
  }

  CORBACommons::CorbaClientConfig config;

  if (index < argc && !strcmp(argv[index], "-timeout"))
  {
    if (++index >= argc)
    {
      throw InvalidArgument("-timeout value not specified");
    }
    long timeout = atol(argv[index]);
    if (timeout <= 0)
    {
      Stream::Error ostr;
      ostr << "invalid -timeout value '" << argv[index] << "'";
      throw InvalidArgument(ostr);
    }
    config.timeout = Generics::Time(timeout);
    index++;
  }

  {
    logger_ =
      new Logging::OStream::Logger(Logging::OStream::Config(std::cout));
    logger_->log_level(100);
    adapter_ = new CORBACommons::CorbaClientAdapter(config, logger_);
  }

  if (argc <= index)
  {
    throw InvalidArgument("Too few arguments");
  }

  const char* arg = argv[index++];

  if (!strcmp(arg, "-shutdown"))
  {
    return shutdown_(argc - index, argv + index);
  }
  else if (!strcmp(arg, "-comment"))
  {
    return status_(argc - index, argv + index);
  }
  else if (!strcmp(arg, "-control"))
  {
    return control_(argc - index, argv + index);
  }
  else
  {
    index--;
    return probe_(argc - index, argv + index);
  }
}

int
Application::shutdown_(int argc, char** argv)
  throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
    CORBA::Exception)
{
  bool wait_for_completion = false;
  std::string url;

  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i], "-wait-for-completion"))
    {
      wait_for_completion = true;
    }
    else
    {
      if (url.empty())
      {
        url = argv[i];
      }
      else
      {
        Stream::Error ostr;
        ostr << "unexpected argument '" << argv[i] << "' for shutdown mode";
        throw InvalidArgument(ostr);
      }
    }
  }

  if (url.empty())
  {
    throw InvalidArgument("CORBA object url undefined");
  }

  CORBAConfigParser::CorbaRefOption<CORBACommons::IProcessControl>
    process_control(adapter_);
  try
  {
    process_control.set(0, url.c_str());
  }
  catch (const eh::Exception& ex)
  {
    throw InvalidReference(ex.what());
  }

  (*process_control)->shutdown(wait_for_completion);

  return PR_ALL_CORRECT;
}

int
Application::status_(int argc, char** argv)
  throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
    CORBA::Exception)
{
  std::string url;

  for (int i = 0; i < argc; i++)
  {
    if (url.empty())
    {
      url = argv[i];
    }
    else
    {
      Stream::Error ostr;
      ostr << "unexpected argument '" << argv[i] << "' for comment mode";
      throw InvalidArgument(ostr);
    }
  }
  if (url.empty())
  {
    throw InvalidArgument("CORBA object url undefined");
  }

  CORBAConfigParser::CorbaRefOption<CORBACommons::IProcessControl>
    process_control(adapter_);
  try
  {
    process_control.set(0, url.c_str());
  }
  catch (const eh::Exception& ex)
  {
    throw InvalidReference(ex.what());
  }

  try
  {
    CORBA::String_var comment((*process_control)->comment());
    std::cout << comment << std::endl;
  }
  catch (const CORBACommons::OutOfMemory& ex)
  {
    Stream::Error ostr;
    ostr << "Received OutOfMemory exception as a result: " << ex;
    throw Exception(ostr);
  }

  return PR_ALL_CORRECT;
}

int
Application::control_(int argc, char** argv)
  throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
    CORBA::Exception)
{
  if (argc != 3)
  {
    Stream::Error ostr;
    ostr << "invalid number of arguments (" << argc << ") for control mode";
    throw InvalidArgument(ostr);
  }

  std::string url(argv[2]);

  CORBAConfigParser::CorbaRefOption<CORBACommons::IProcessControl>
    process_control(adapter_);
  try
  {
    process_control.set(0, url.c_str());
  }
  catch (const eh::Exception& ex)
  {
    throw InvalidReference(ex.what());
  }

  try
  {
    CORBA::String_var result((*process_control)->control(argv[0], argv[1]));
    std::cout << result << std::endl;
  }
  catch (const CORBACommons::ImplementationError& ex)
  {
    Stream::Error ostr;
    ostr << "Received ImplementationError exception as a result: " <<
      ex.error;
    throw Exception(ostr);
  }
  catch (const CORBACommons::OutOfMemory& ex)
  {
    Stream::Error ostr;
    ostr << "Received OutOfMemory exception as a result: " << ex;
    throw Exception(ostr);
  }

  return PR_ALL_CORRECT;
}

int
Application::probe_(int argc, char** argv)
  throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
    CORBA::Exception)
{
  bool is_a_mode = false;
  std::string url;
  bool retry = false;
  int count = -1;
  Generics::Time sleep_interval;
  MESSAGE_TYPE message_type = MT_NONE;
  std::string retry_text;
  bool check_status = false;
  CORBACommons::IProcessControl::ALIVE_STATUS status =
    CORBACommons::IProcessControl::AS_ALIVE;
  bool server_unreachable = false;
  CORBACommons::SecureConnectionConfig security_params;

  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i], "-is-a-mode"))
    {
      is_a_mode = true;
    }
    else if (!strcmp(argv[i], "-retry"))
    {
      if (++i >= argc)
      {
        throw InvalidArgument("-retry value not specified");
      }

      long timeout = atol(argv[i]);
      if (timeout <= 0)
      {
        Stream::Error ostr;
        ostr << "invalid -retry value '" << argv[i] << "'";
        throw InvalidArgument(ostr);
      }

      sleep_interval = Generics::Time(timeout / 1000,
        (timeout % 1000) * 1000);
      retry = true;

      if (i + 1 < argc && !strcmp(argv[i + 1], "-count"))
      {
        i += 2;
        if (i >= argc)
        {
          throw InvalidArgument("-count value not specified");
        }

        count = atoi(argv[i]);
        if (count < 0)
        {
          Stream::Error ostr;
          ostr << "invalid -count value '" << argv[i] << "'";
          throw InvalidArgument(ostr);
        }
      }
    }
    else if (!strcmp(argv[i], "-message"))
    {
      if (++i >= argc)
      {
        throw InvalidArgument("-message value not specified");
      }

      if (!strcmp(argv[i], "FAILURE_DESC"))
      {
        message_type = MT_FAILURE_DESC;
      }
      else if (!strcmp(argv[i], "LOG_DESC"))
      {
        message_type = MT_LOG_DESC;
      }
      else
      {
        message_type = MT_USER;
        retry_text = argv[i];
      }
    }
    else if (!strcmp(argv[i], "-status"))
    {
      if (++i >= argc)
      {
        throw InvalidArgument("-status value not specified");
      }

      if (!strcmp(argv[i], "not_alive"))
      {
        status = CORBACommons::IProcessControl::AS_NOT_ALIVE;
      }
      else if (!strcmp(argv[i], "alive"))
      {
        status = CORBACommons::IProcessControl::AS_ALIVE;
      }
      else if (!strcmp(argv[i], "ready"))
      {
        status = CORBACommons::IProcessControl::AS_READY;
      }
      else
      {
        Stream::Error ostr;
        ostr << "Invalid -status value '" << argv[i] << "'";
        throw InvalidArgument(ostr);
      }

      check_status = true;
    }
    else
    {
      if (url.empty())
      {
        url = argv[i];
        if (const char* pos = strchr(url.c_str(), '@'))
        {
          CORBAConfigParser::parse_secure_params_arg(
            String::SubString(url.c_str(), pos), security_params);
          url = std::string(pos + 1);
        }
      }
      else
      {
        Stream::Error ostr;
        ostr << "unexpected argument '" << argv[i] << "' for probe mode";
        throw InvalidArgument(ostr);
      }
    }
  }

  if (url.empty())
  {
    throw InvalidArgument("CORBA object url undefined");
  }

  std::string failure_desc;
  int result = PR_UNEXPECTED_STATUS;

  CORBA::ORB_var orb(adapter_->designate_orb(security_params));

  for (;;)
  {
    failure_desc = "No object found.";

    try
    {
      CORBA::Object_var obj = orb->string_to_object(url.c_str());
      if (!CORBA::is_nil(obj))
      {
        if (is_a_mode)
        {
          const char* rep_id =
            "IDL:prbably_no_such_module/probably_no_such_interface:99.99";

          if (obj->_is_a(rep_id) == false)
          {
            result = PR_ALL_CORRECT;
            break;
          }
        }
        else
        {
          CORBACommons::IProcessControl_var process_control =
            CORBACommons::IProcessControl::_narrow(obj);
          if (!CORBA::is_nil(process_control))
          {
            CORBACommons::IProcessControl::ALIVE_STATUS actual_status =
              process_control->is_alive();
            if (check_status)
            {
              if (actual_status == status)
              {
                result = PR_ALL_CORRECT;
                break;
              }
              else
              {
                failure_desc = "Object's status differs.";
              }
            }
            else
            {
              if (actual_status !=
                CORBACommons::IProcessControl::AS_NOT_ALIVE)
              {
                result = PR_ALL_CORRECT;
                break;
              }
              else
              {
                failure_desc = "Object is dead.";
              }
            }
          }
          else
          {
            server_unreachable = true;
          }
        }
      }
    }
    catch (const CORBA::BAD_PARAM&)
    {
    }
    catch (const CORBA::TRANSIENT&)
    {
    }
    catch (const CORBA::OBJECT_NOT_EXIST&)
    {
    }
    catch (const CORBA::Exception& ex)
    {
      Stream::Error ostr;
      ostr << "CORBA::Exception: " << ex;
      ostr.str().assign_to(failure_desc);
      server_unreachable = true;
    }

    if (result == PR_ALL_CORRECT)
    {
      break;
    }

    switch (message_type)
    {
      case MT_USER:
        std::cerr << retry_text << std::endl;
        break;
      case MT_LOG_DESC:
      case MT_FAILURE_DESC:
        std::cerr << failure_desc << std::endl;
        break;
      default:
        break;
    }

    if (!retry)
    {
      break;
    }

    if (count > 0)
    {
      if (!--count)
      {
        break;
      }
    }
    else
    {
      if (!count && server_unreachable)
      {
        break;
      }
    }

    if (message_type == MT_LOG_DESC)
    {
      std::cerr << "Sleeping for retry..." << std::endl;
    }
    usleep(sleep_interval.microseconds());
  }

  return result;
}

int
main(int argc, char** argv)
{
  {
    std::ostringstream ostr;

    ostr << "ProbeObj";
    for (int i = 0; i < argc; i++)
    {
      ostr << " " << argv[i];
    }
    ostr << ": ";
    ostr.str().swap(prefix);
  }

  try
  {
    Application app;

    return app.run(argc, argv);
  }
  catch (const Application::InvalidArgument& e)
  {
    std::cerr << prefix << e.what() << std::endl << USAGE << std::endl;
    return PR_INVALID_ARGUMENT;
  }
  catch (const Application::InvalidReference& e)
  {
    std::cerr << prefix << "Invalid reference: " << e.what() << std::endl;
    return PR_INVALID_REFERENCE;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << prefix << "eh::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }
  catch (const CORBA::Exception& e)
  {
    std::cerr << prefix << "eh::Exception exception caught. "
      "Description:" << std::endl << e << std::endl;
  }
  catch (...)
  {
    std::cerr << prefix << "unknown exception caught" << std::endl;
  }

  return PR_UNRECOVERABLE_EXCEPTION;
}
