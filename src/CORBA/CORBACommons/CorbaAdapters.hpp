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



#ifndef CORBACOMMONS_CORBAADAPTERS_HPP
#define CORBACOMMONS_CORBAADAPTERS_HPP

#include <list>

#include <tao/ORB.h>
#include <tao/Valuetype/ValueFactory.h>
#include <orbsvcs/Security/SL2_SecurityManager.h>

#include <String/SubString.hpp>

#include <Generics/Hash.hpp>

#include <CORBACommons/CorbaObjectRef.hpp>


namespace CORBACommons
{
  /**
   * Security information for SSL connection
   */
  struct SecureConnectionConfig
  {
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Constructor
     * Creates "insecure" information
     */
    SecureConnectionConfig() throw ();

    /**
     * Constructor
     * Creates "secure" information. Loads information from files.
     * @param key_file file with key in PEM format
     * @param pass_phrase password for the key file
     * @param certificate_file file with the certificate in PEM format for
     * the key
     * @param certificate_authority_file file with CA in PEM format for
     * peer key/certificate pair
     */
    SecureConnectionConfig(const char* key_file, const char* pass_phrase,
      const char* certificate_file,
      const char* certificate_authority_file)
      throw (Exception);

    /**
     * Fills with "secure" information. Loads information from files.
     * @param key_file file with key in PEM format
     * @param pass_phrase password for the key file
     * @param certificate_file file with the certificate in PEM format for
     * the key
     * @param certificate_authority_file file with CA in PEM format for
     * peer key/certificate pair
     */
    void
    parse(const char* key_file, const char* pass_phrase,
      const char* certificate_file,
      const char* certificate_authority_file)
      throw (Exception);

    /**
     * Compares on strict equality
     * @param right another information to compare with
     * @return if contents are binary equal or not
     */
    bool
    operator ==(const SecureConnectionConfig& right) const throw ();

    /**
     * Gives "secure" status
     * @return if it was initialized with security information or not
     */
    bool
    is_secure() const throw ();

    std::string pass_phrase;
    PrivateKey_var private_key;
    Certificate_var own_certificate;
    CertificateAuthority_var peer_certificate_authority;
  };


  /**
   * Hash adaptor for SecureConnectionConfig
   */
  class SecureConnectionConfigAdaptor
  {
  public:
    /**
     * Constructor
     * @param config configuration to adapt (and hold)
     */
    SecureConnectionConfigAdaptor(const SecureConnectionConfig& config)
      throw (eh::Exception);

    /**
     * Reference to stored configuration
     * @return reference to stored configuration
     */
    const SecureConnectionConfig&
    operator *() const throw ();

    /**
     * Pointer to stored configuration
     * @return pointer to stored configuration
     */
    const SecureConnectionConfig*
    operator ->() const throw ();

    /**
     * Calculated hash value for configuration
     * @return calculated hash value
     */
    size_t
    hash() const throw ();

    /**
     * Compares with another adapter on equality
     * @param other another adapter to compare with
     * @return if two adapters hold equal information or not
     */
    bool
    operator ==(const SecureConnectionConfigAdaptor& other) const throw ();

  private:
    SecureConnectionConfig config_;
    size_t hash_;
  };


  /**
   * Orb properties - list of arguments to pass to ORB_init
   */
  typedef std::list<std::string> ORBProperties;

  /**
   * Throws an object of Exception type, add termination zero
   * to output stream, which had early wrote a description of the error
   * @param substr The substring containing description of error
   */
  template <typename Exception>
  void
  throw_desc(const String::SubString& substr)
    throw (eh::Exception, Exception);
}

/**
 * Assign substring to CORBA string
 * @param str The destination to write a substring
 * @param substr The substring to be assigned to string
 */
void
operator <<(CORBA::String_var& str, const String::SubString& substr)
  throw (eh::Exception);

/**
 * Assign substring to TAO string
 * @param str The destination to write a substring
 * @param substr The substring to be assigned to string
 */
void
operator <<(TAO::String_Manager& str, const String::SubString& substr)
  throw (eh::Exception);


#include <CORBACommons/CorbaClientAdapter.hpp>
#include <CORBACommons/CorbaServerAdapter.hpp>


namespace CORBACommons
{
  //
  // SecureConnectionConfig class
  //

  inline
  SecureConnectionConfig::SecureConnectionConfig() throw ()
  {
  }

  inline
  bool
  SecureConnectionConfig::is_secure() const throw ()
  {
    const char* key = private_key;
    return key && *key;
  }

  inline
  bool
  SecureConnectionConfig::operator ==(const SecureConnectionConfig& right)
    const throw ()
  {
    return !is_secure() ? !right.is_secure() :
      right.is_secure() &&
      !strcmp(private_key, right.private_key) &&
      !strcmp(pass_phrase.c_str(), right.pass_phrase.c_str()) &&
      !strcmp(own_certificate, right.own_certificate) &&
      !strcmp(peer_certificate_authority,
        right.peer_certificate_authority);
  }


  //
  // SecureConnectionConfigAdaptor class
  //

  inline
  SecureConnectionConfigAdaptor::SecureConnectionConfigAdaptor(
    const SecureConnectionConfig& config) throw (eh::Exception)
    : config_(config), hash_(0)
  {
    if (config.is_secure())
    {
      Generics::Murmur64Hash hash(hash_);
      hash_add(hash, String::SubString(config.private_key));
    }
  }

  inline
  const SecureConnectionConfig&
  SecureConnectionConfigAdaptor::operator *() const throw ()
  {
    return config_;
  }

  inline
  const SecureConnectionConfig*
  SecureConnectionConfigAdaptor::operator ->() const throw ()
  {
    return &config_;
  }

  inline
  size_t
  SecureConnectionConfigAdaptor::hash() const throw ()
  {
    return hash_;
  }

  inline
  bool
  SecureConnectionConfigAdaptor::operator ==(
    const SecureConnectionConfigAdaptor& other) const throw ()
  {
    return config_ == other.config_;
  }

  //
  // auxiliary functions
  //

  template <typename Exception>
  void
  throw_desc(const String::SubString& substr) throw (eh::Exception, Exception)
  {
    Exception ex;
    ex.description << substr;
    throw ex;
  }
}

inline
void
operator <<(CORBA::String_var& str, const String::SubString& substr)
  throw (eh::Exception)
{
  char* ptr = CORBA::String_var::s_traits::allocate(substr.size() + 1);
  *std::copy(substr.begin(), substr.end(), ptr) = '\0';
  str = ptr;
}

inline
void
operator <<(CORBA::String_out& str, const String::SubString& substr)
  throw (eh::Exception)
{
  char* ptr = CORBA::String_out::s_traits::allocate(substr.size() + 1);
  *std::copy(substr.begin(), substr.end(), ptr) = '\0';
  str = ptr;
}

inline
void
operator <<(TAO::String_Manager& str, const String::SubString& substr)
  throw (eh::Exception)
{
  char* ptr = TAO::String_Manager::s_traits::allocate(substr.size() + 1);
  *std::copy(substr.begin(), substr.end(), ptr) = '\0';
  str = ptr;
}

inline
void
operator <<(TAO::unbounded_basic_string_sequence<char>::element_type&& str,
  const String::SubString& substr)
  throw (eh::Exception)
{
  char* ptr = TAO::unbounded_basic_string_sequence<char>::
    element_traits::allocate(substr.size() + 1);
  *std::copy(substr.begin(), substr.end(), ptr) = '\0';
  str = ptr;
}

#endif
