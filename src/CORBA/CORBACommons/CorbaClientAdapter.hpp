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



#ifndef CORBACOMMONS_CORBACLIENT_ADAPTER_HPP
#define CORBACOMMONS_CORBACLIENT_ADAPTER_HPP

#include <iostream>
#include <list>

#include <Generics/GnuHashTable.hpp>
#include <Generics/Singleton.hpp>

#include <Logger/Logger.hpp>

#include <CORBACommons/CorbaAdapters.hpp>


namespace CORBACommons
{
  /**
   * Configuration of CorbaClientAdaptor
   */
  struct CorbaClientConfig
  {
    Generics::Time timeout;
  };

  /**X CorbaObjectConnection */
  struct CorbaObjectConnection
  {
    CorbaObjectConnection() throw ();

    CorbaObjectConnection(
      const SecureConnectionConfig& secure_connection_config_) throw ();

    enum ConnectionType
    {
      CT_SECURE,
      CT_NON_SECURE
    };

    ConnectionType type;

    SecureConnectionConfig secure_connection_config;
  };


  /**X CorbaObjectRef */
  struct CorbaObjectRef : public CorbaObjectConnection
  {
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    CorbaObjectRef() throw ();

    /* initialize non secure object ref */
    CorbaObjectRef(const char* object_ref_) throw ();

    /* initialize secure object ref */
    CorbaObjectRef(const char* object_ref_,
      const SecureConnectionConfig& secure_connection_config)
      throw ();

    void
    load(const CORBACommons::CorbaObjectRefDef& in_corba_object_ref)
      throw (eh::Exception, Exception);

    void
    save(CORBACommons::CorbaObjectRefDef& out_corba_object_ref) const
      throw (eh::Exception, Exception);

    std::string object_ref;
  };

  typedef std::list<CorbaObjectRef> CorbaObjectRefList;
  
  /**X
   * CorbaClientAdapter
   */
  class CorbaClientAdapter :
    public virtual ::ReferenceCounting::AtomicImpl
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    explicit
    CorbaClientAdapter(Logging::Logger* logger = 0)
      throw ();

    explicit
    CorbaClientAdapter(const CorbaClientConfig& corba_config,
      Logging::Logger* logger = 0)
      throw (eh::Exception);

    std::string
    object_to_string(CORBA::Object* obj) const
      throw (eh::Exception, Exception);

    CORBA::Object_ptr
    resolve_object(const CorbaObjectRef& ref) const
      throw (eh::Exception, Exception);

    template <typename T>
    T*
    resolve_object(const CorbaObjectRef& ref) const
      throw (eh::Exception, Exception);


    void
    register_value_factory(const char* type_name,
      CORBA::ValueFactoryBase* factory) const
      throw (eh::Exception);


    CORBA::ORB_var
    designate_orb(const SecureConnectionConfig& config) const
      throw (eh::Exception);


    struct ObjectInfo
    {
      bool secure;
      std::string host;
      int port;
      std::string name;
    };

    static
    ObjectInfo
    get_object_info(CORBA::Object* obj)
      throw (eh::Exception, Exception);

  protected:
    virtual
    ~CorbaClientAdapter() throw ();


    /**
     * Holder of client orbs each of them is associated with
     * unique secure_connection_config
     */
    class Orbs
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

      struct OrbDesignator
      {
        OrbDesignator(const CorbaClientConfig& corba_config,
          const SecureConnectionConfig& config) throw (eh::Exception);

        size_t
        hash() const throw ();

        bool
        operator ==(const OrbDesignator& designator) const throw ();

        const Generics::Time&
        timeout() const throw ();

        const SecureConnectionConfig&
        config() const throw ();

      private:
        Generics::Time timeout_;
        SecureConnectionConfigAdaptor config_;
      };

      /**
       * Destructor
       */
      ~Orbs() throw ();

      /**
       * Returns existing or creates a new orb
       * @param designator a unique key for associated orb
       * @return orb uniquely associated with the key
       */
      CORBA::ORB_ptr
      get_orb(const OrbDesignator& designator)
        throw (eh::Exception, Exception);

      /**
       * Registers the value factory for all existing and created in
       * future orbs
       * @param type_name ValueType name
       * @param factory factory to register
       */
      void
      register_value_factory(const char* type_name,
        CORBA::ValueFactoryBase* factory)
        throw (eh::Exception, Exception);

      typedef Generics::GnuHashTable<OrbDesignator,
        CORBA::ORB_var> OrbsHolder;

      const OrbsHolder&
      get_orbs() const throw ();

    private:
      /**
       * Creates client orb for provided secure connection config
       * @param secure_connection_config secure connection config
       * @return created orb
       */
      CORBA::ORB_ptr
      create_orb_(const OrbDesignator& designator)
        throw (eh::Exception);


      struct ValueFactoryDescription
      {
        std::string type_name;
        CORBA::ValueFactoryBase_var value_factory;
      };

      typedef std::list<ValueFactoryDescription>
        ValueFactoryDescriptions;


      Sync::PosixRWLock lock_;
      OrbsHolder orbs_;
      ValueFactoryDescriptions value_factories_;
    };
    typedef Generics::Singleton<Orbs, Generics::Helper::AutoPtr<Orbs>,
      Generics::AtExitDestroying::DP_CLIENT_ORBS> OrbsSingleton;

  protected:
    CorbaClientConfig corba_config_;
    Logging::FLogger_var logger_;
  };
  typedef ::ReferenceCounting::ConstPtr<CorbaClientAdapter>
    CorbaClientAdapter_var;
  typedef ::ReferenceCounting::FixedPtr<CorbaClientAdapter>
    FixedCorbaClientAdapter_var;
}

namespace CORBACommons
{
  //
  // CorbaObjectConnection class
  //

  inline
  CorbaObjectConnection::CorbaObjectConnection() throw ()
    : type(CT_NON_SECURE)
  {
  }

  inline
  CorbaObjectConnection::CorbaObjectConnection(
    const SecureConnectionConfig& secure_connection_config_) throw ()
    : type(CT_SECURE),
      secure_connection_config(secure_connection_config_)
  {
  }


  //
  // CorbaObjectRef class
  //

  inline
  CorbaObjectRef::CorbaObjectRef(const char* object_ref_) throw ()
    : object_ref(object_ref_)
  {
  }

  inline
  CorbaObjectRef::CorbaObjectRef(const char* object_ref_,
    const SecureConnectionConfig& secure_connection_config_) throw ()
    : CorbaObjectConnection(secure_connection_config_),
      object_ref(object_ref_)
  {
  }


  //
  // CORBAClientAdapter class
  //

  template <typename T>
  T*
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
      typename T::_var_type obj_var = T::_narrow(obj);

      if (CORBA::is_nil(obj) || CORBA::is_nil(obj_var))
      {
        Stream::Error ostr;
        ostr << FNS << "Can't " <<
          (CORBA::is_nil(obj) ? "resolve" : "narrow") << " object '" <<
          corba_object_ref.object_ref << "' on " <<
          (corba_object_ref.secure_connection_config.is_secure() ?
            "secure" : "insecure") << " connection";
        throw Exception(ostr);
      }

      return obj_var._retn();
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "CORBA Exception: " << ex;
      throw Exception(ostr);
    }
  }

}

inline
std::ostream&
operator <<(std::ostream& ostr, const CORBACommons::CorbaObjectRef& ref)
  throw (eh::Exception)
{
  ostr << "'" << ref.object_ref << "'";
  return ostr;
}

#endif
