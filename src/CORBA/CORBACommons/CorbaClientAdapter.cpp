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



#include "CorbaAdaptersInternal.hpp"


namespace
{
  const char ORB_CLIENT_NON_SECURE_NAME[] = "ClientNonSecureORB";
  const char ORB_CLIENT_SECURE_NAME[] = "ClientSecureORB";

  const String::SubString IOR1(
    "IOR:0100000001000000000000000100000000000000");
  const String::SubString IOR2("0101");
}

namespace CORBACommons
{
  //
  // CorbaObjectRef class
  //

  CorbaObjectRef::CorbaObjectRef() throw ()
  {
  }

  void
  CorbaObjectRef::save(CorbaObjectRefDef& out_corba_object_ref) const
    throw (eh::Exception, Exception)
  {
    try
    {
      out_corba_object_ref.object_ref = object_ref.c_str();

      CORBACommons::ConnectionDef& out_connection =
        out_corba_object_ref.connection;

      out_connection.connection_type =
        type == CorbaObjectConnection::CT_SECURE ?
          CORBACommons::CT_SECURE : CORBACommons::CT_NON_SECURE;

      if (type == CorbaObjectConnection::CT_SECURE)
      {
        out_connection.secure_connection.private_key =
          secure_connection_config.private_key;
        out_connection.secure_connection.pass_phrase =
          secure_connection_config.pass_phrase.c_str();
        out_connection.secure_connection.own_certificate =
          secure_connection_config.own_certificate;
        out_connection.secure_connection.peer_certificate_authority =
          secure_connection_config.peer_certificate_authority;
      }
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Caught CORBA Exception: " << ex;
      throw Exception(ostr);
    }
  }

  void
  CorbaObjectRef::load(const CorbaObjectRefDef& in_corba_object_ref)
    throw (eh::Exception, Exception)
  {
    try
    {
      object_ref = in_corba_object_ref.object_ref;

      type = in_corba_object_ref.connection.connection_type ==
        CORBACommons::CT_SECURE ? CT_SECURE : CT_NON_SECURE;

      if (type == CorbaObjectConnection::CT_SECURE)
      {
        const CORBACommons::SecureConnectionDef& in_secure_connection =
          in_corba_object_ref.connection.secure_connection;

        secure_connection_config.private_key =
          in_secure_connection.private_key;
        secure_connection_config.pass_phrase =
          in_secure_connection.pass_phrase;
        secure_connection_config.own_certificate =
          in_secure_connection.own_certificate;
        secure_connection_config.peer_certificate_authority =
          in_secure_connection.peer_certificate_authority;
      }
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Caught CORBA Exception: " << ex;
      throw Exception(ostr);
    }
  }


  //
  // CorbaClientAdapter::Orbs::OrbDesignator class
  //

  CorbaClientAdapter::Orbs::OrbDesignator::OrbDesignator(
    const CorbaClientConfig& corba_config,
    const SecureConnectionConfig& config) throw (eh::Exception)
    : timeout_(corba_config.timeout), config_(config)
  {
  }

  size_t
  CorbaClientAdapter::Orbs::OrbDesignator::hash() const throw ()
  {
    return config_.hash();
  }

  bool
  CorbaClientAdapter::Orbs::OrbDesignator::operator ==(
    const OrbDesignator& designator) const throw ()
  {
    return timeout_ == designator.timeout_ && config_ == designator.config_;
  }

  const Generics::Time&
  CorbaClientAdapter::Orbs::OrbDesignator::timeout() const throw ()
  {
    return timeout_;
  }

  const SecureConnectionConfig&
  CorbaClientAdapter::Orbs::OrbDesignator::config() const throw ()
  {
    return *config_;
  }


  //
  // CorbaClientAdapter::Orbs class
  //

  CorbaClientAdapter::Orbs::~Orbs() throw ()
  {
    for (OrbsHolder::iterator itor(orbs_.begin()); itor != orbs_.end();
      ++itor)
    {
      itor->second->destroy();
    }
  }

  CORBA::ORB_ptr
  CorbaClientAdapter::Orbs::get_orb(const OrbDesignator& designator)
    throw (eh::Exception, Exception)
  {
    {
      Sync::PosixRGuard guard(lock_);
      OrbsHolder::iterator orb(orbs_.find(designator));
      if (orb != orbs_.end())
      {
        return CORBA::ORB::_duplicate(orb->second);
      }
    }

    {
      Sync::PosixWGuard guard(lock_);

      {
        OrbsHolder::iterator orb(orbs_.find(designator));
        if (orb != orbs_.end())
        {
          return CORBA::ORB::_duplicate(orb->second);
        }
      }

      CORBA::ORB_var orb(create_orb_(designator));

      /* register factories */
      for (ValueFactoryDescriptions::iterator it =
        value_factories_.begin(); it != value_factories_.end(); ++it)
      {
        CORBA::ValueFactoryBase_var old_factory =
          orb->register_value_factory(it->type_name.c_str(),
            it->value_factory);
      }

      orbs_[designator] = orb;

      return orb._retn();
    }
  }

  void
  CorbaClientAdapter::Orbs::register_value_factory(const char* type_name,
    CORBA::ValueFactoryBase* factory) throw (eh::Exception, Exception)
  {
    Sync::PosixWGuard guard(lock_);

    try
    {
      for (OrbsHolder::iterator itor(orbs_.begin()); itor != orbs_.end();
        ++itor)
      {
        CORBA::ValueFactoryBase_var old_factory =
          itor->second->register_value_factory(type_name, factory);
      }

      ValueFactoryDescription factory_desc;
      factory_desc.type_name = type_name;
      factory->_add_ref();
      factory_desc.value_factory = factory;
      value_factories_.push_back(std::move(factory_desc));
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't register factory for type '" << type_name <<
        "'. Caught CORBA::SystemException:" << ex;
      throw Exception(ostr);
    }
  }

  const CorbaClientAdapter::Orbs::OrbsHolder&
  CorbaClientAdapter::Orbs::get_orbs() const throw ()
  {
    return orbs_;
  }

  CORBA::ORB_ptr
  CorbaClientAdapter::Orbs::create_orb_(
    const OrbDesignator& designator)
    throw (eh::Exception)
  {
    ORBProperties properties;

    properties.emplace_back("-ORBSvcConfDirective");
    properties.emplace_back("static Resource_Factory "
      "\"-ORBProtocolFactory IIOP_Factory\"");

    const char* name = ORB_CLIENT_NON_SECURE_NAME;
    if (designator.config().is_secure())
    {
      PropertiesHandling::create_secure_properties(properties,
        designator.config());
      name = ORB_CLIENT_SECURE_NAME;
    }

    PropertiesHandling::create_common_properties(properties, false);

    if (designator.timeout() != Generics::Time::ZERO)
    {
      properties.emplace_back("-ORBSvcConfDirective");
      properties.emplace_back("dynamic TAO_Messaging_Loader "
        "Service_Object * " TAO_LIB("TAO_Messaging")
        ":_make_TAO_Messaging_Loader() \"\"");
    }

    properties.emplace_front();

    return OrbCreator::create_orb(properties, name,
      &designator.config(), designator.timeout());
  }


  //
  // CorbaClientAdapter class
  //

  CorbaClientAdapter::CorbaClientAdapter(
    Logging::Logger* logger) throw ()
    : logger_(::ReferenceCounting::add_ref(logger))
  {
    if (logger_)
    {
      AceLogger::add_logger(logger_);
    }
  }

  CorbaClientAdapter::CorbaClientAdapter(
    const CorbaClientConfig& corba_config, Logging::Logger* logger)
    throw (eh::Exception)
    : corba_config_(corba_config),
      logger_(::ReferenceCounting::add_ref(logger))
  {
    if (logger_)
    {
      AceLogger::add_logger(logger_);
    }
  }

  CorbaClientAdapter::~CorbaClientAdapter() throw ()
  {
    AceLogger::remove_logger(logger_);
  }

  std::string
  CorbaClientAdapter::object_to_string(CORBA::Object* obj) const
    throw (eh::Exception, Exception)
  {
    CORBA::ORB_var orb(OrbsSingleton::instance().get_orb(
      Orbs::OrbDesignator(corba_config_,
        SecureConnectionConfig())));
    try
    {
      return orb->object_to_string(obj);
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "CORBA Exception: " << ex;
      throw Exception(ostr);
    }
  }

  CORBA::Object_ptr
  CorbaClientAdapter::resolve_object(
    const CorbaObjectRef& corba_object_ref) const
    throw (eh::Exception, Exception)
  {
    CORBA::ORB_var orb(OrbsSingleton::instance().get_orb(
      Orbs::OrbDesignator(corba_config_,
        corba_object_ref.secure_connection_config)));
    try
    {
      CORBA::Object_var obj =
        orb->string_to_object(corba_object_ref.object_ref.c_str());

      if (CORBA::is_nil(obj))
      {
        Stream::Error ostr;
        ostr << FNS << "Can't resolve object '" <<
          corba_object_ref.object_ref << "' on " <<
          (corba_object_ref.secure_connection_config.is_secure() ?
            "secure" : "insecure") << " connection";
        throw Exception(ostr);
      }

      return obj._retn();
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "CORBA Exception: " << ex;
      throw Exception(ostr);
    }
  }

  void
  CorbaClientAdapter::register_value_factory(const char* type_name,
    CORBA::ValueFactoryBase* factory) const throw (eh::Exception)
  {
    OrbsSingleton::instance().register_value_factory(type_name, factory);
  }

  CORBA::ORB_var
  CorbaClientAdapter::designate_orb(
    const SecureConnectionConfig& config) const
    throw (eh::Exception)
  {
    CORBA::ORB_var orb(OrbsSingleton::instance().get_orb(
      Orbs::OrbDesignator(corba_config_, config)));
    return orb;
  }

  CorbaClientAdapter::ObjectInfo
  CorbaClientAdapter::get_object_info(CORBA::Object* obj)
    throw (eh::Exception, Exception)
  {
    if (!obj)
    {
      Stream::Error ostr;
      ostr << FNS << "Null object";
      throw Exception(ostr);
    }
    try
    {
      ObjectInfo info;
      CORBA::String_var ior(obj->_get_orb()->object_to_string(obj));
      String::SubString str(ior);
      if (str.substr(0, IOR1.size()) != IOR1 ||
        str.substr(52, IOR2.size()) != IOR2)
      {
        Stream::Error ostr;
        ostr << FNS << "Unexpected IOR format";
        throw Exception(ostr);
      }
      info.secure = str[58] != '0';
      uint32_t len1, len2;
      Generics::ArrayChar buf;
      uint16_t port;
      String::AsciiStringManip::hex_to_integer(&str[60], len1);
      buf.reset(len1);
      len1 <<= 1;
      String::AsciiStringManip::hex_to_buf(str.substr(68, len1), buf.get());
      info.host = buf.get();
      String::AsciiStringManip::hex_to_integer(&str[68 + len1], port);
      info.port = port;
      String::AsciiStringManip::hex_to_integer(&str[72 + len1], len2);
      buf.reset(len2);
      String::AsciiStringManip::hex_to_buf(
        str.substr(80 + len1, len2 << 1), buf.get());
      info.name.assign(buf.get(), len2);
      return info;
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "CORBA exception: " << ex;
      throw Exception(ostr);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << ex.what();
      throw Exception(ostr);
    }
  }
}
