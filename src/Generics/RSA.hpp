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



#ifndef GENERICS_RSA_HPP
#define GENERICS_RSA_HPP

#include <cstdio>

#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <Generics/Function.hpp>
#include <Generics/Uncopyable.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  /**
   * Reads and hold RSA key (via crypto library)
   * @param PRIVATE_KEY whether the key is private or public
   */
  template <const bool PRIVATE_KEY>
  class RSAKey : private Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Constructor
     * Tries to read the key from the file
     * @param filename name of ASN1 file containing the key
     */
    RSAKey(const char* filename)
      throw (eh::Exception, Exception);

    /**
     * Destructor
     * Frees the key
     */
    ~RSAKey() throw ();

    /**
     * Returns pointer to the key
     * @return pointer to the key
     */
    RSA*
    key() const throw ();

  private:
    RSA* key_;
  };
}

namespace Generics
{
  template <const bool PRIVATE_KEY>
  RSAKey<PRIVATE_KEY>::RSAKey(const char* filename)
    throw (eh::Exception, Exception)
    : key_(0)
  {
    std::FILE* file = std::fopen(filename, "rb");
    if (!file)
    {
      Stream::Error ostr;
      ostr << FNS << "Failed to open key file '" << filename << "'";
      throw Exception(ostr);
    }

    (PRIVATE_KEY ? d2i_RSAPrivateKey_fp : d2i_RSA_PUBKEY_fp)(file, &key_);

    std::fclose(file);

    if (!key_)
    {
      Stream::Error ostr;
      ostr << FNS << "Failed to load key from file '" << filename << "'";
      char buf[1024];
      while (unsigned long error = ERR_get_error())
      {
        ERR_error_string_n(error, buf, sizeof(buf));
        ostr << "\n" << buf;
      }
      throw Exception(ostr);
    }
  }

  template <const bool PRIVATE_KEY>
  RSAKey<PRIVATE_KEY>::~RSAKey() throw ()
  {
    RSA_free(key_);
  }

  template <const bool PRIVATE_KEY>
  RSA*
  RSAKey<PRIVATE_KEY>::key() const throw ()
  {
    return key_;
  }
}

#endif
