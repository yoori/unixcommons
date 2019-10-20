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



#ifndef CORBACONFIGPARSER_PARAMETERCONFIG_HPP
#define CORBACONFIGPARSER_PARAMETERCONFIG_HPP

#include <Generics/AppUtils.hpp>

#include <CORBACommons/CorbaAdapters.hpp>


namespace CORBAConfigParser
{
  void
  parse_secure_params_arg(const String::SubString& config,
    CORBACommons::SecureConnectionConfig& val)
    throw (eh::Exception, Generics::AppUtils::InvalidParam);

  template <typename CorbaObject>
  class CorbaRefOption :
    public Generics::AppUtils::Option<TAO_Objref_Var_T<CorbaObject> >
  {
  public:
    CorbaRefOption(const CORBACommons::CorbaClientAdapter* client_adapter,
      const char* default_secure_params = "")
      throw (eh::Exception);

    void
    set(const char*, const char* corba_url)
      throw (eh::Exception, Generics::AppUtils::InvalidParam);

  private:
    CORBACommons::CorbaClientAdapter_var client_adapter_;
    std::string default_secure_params_;
  };

  class SecureParamsOption :
    public Generics::AppUtils::Option<CORBACommons::SecureConnectionConfig>
  {
  public:
    void
    set(const char*, const char* strval)
      throw (eh::Exception, Generics::AppUtils::InvalidParam);
  };
}


namespace CORBAConfigParser
{
  template <typename CorbaObject>
  CorbaRefOption<CorbaObject>::CorbaRefOption(
    const CORBACommons::CorbaClientAdapter* client_adapter,
    const char* default_secure_params)
    throw (eh::Exception)
    : client_adapter_(ReferenceCounting::add_ref(client_adapter)),
      default_secure_params_(default_secure_params)
  {
  }

  template <typename CorbaObject>
  void
  CorbaRefOption<CorbaObject>::set(const char*, const char* corba_url)
    throw (eh::Exception, Generics::AppUtils::InvalidParam)
  {
    std::string ref;

    try
    {
      TAO_Objref_Var_T<CorbaObject> obj;
      const char* url_pos = ::strchr(corba_url, '@');

      if (!url_pos && default_secure_params_.empty())
      {
        ref = corba_url;
        obj = client_adapter_->resolve_object<CorbaObject>(
          CORBACommons::CorbaObjectRef(corba_url));
      }
      else
      {
        CORBACommons::SecureConnectionConfig secure_params;

        if (url_pos)
        {
          parse_secure_params_arg(
            String::SubString(corba_url, url_pos - corba_url), secure_params);
        }
        else
        {
          parse_secure_params_arg(default_secure_params_, secure_params);
        }

        ref = url_pos + 1;
        CORBACommons::CorbaObjectRef corba_object_ref(ref.c_str(),
          secure_params);

        obj = client_adapter_->resolve_object<CorbaObject>(corba_object_ref);
      }

      this->set_value(obj);
    }
    catch (const CORBACommons::CorbaClientAdapter::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't resolve corba reference '" << ref << "': " <<
        ex.what();
      throw Generics::AppUtils::InvalidParam(ostr);
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't resolve corba reference '" << ref << "': " << ex;
      throw Generics::AppUtils::InvalidParam(ostr);
    }
  }
}

#endif
