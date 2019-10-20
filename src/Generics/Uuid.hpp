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





#ifndef GENERICS_UUID_HPP
#define GENERICS_UUID_HPP

#include <ios>

#include <Sync/PosixLock.hpp>

#include <String/StringManip.hpp>

#include <Generics/RSA.hpp>


namespace Generics
{
  class Uuid
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidArgument, Exception);

    typedef uint8_t value_type;
    typedef value_type& reference_type;
    typedef const value_type& const_reference_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef ssize_t difference_type;
    typedef size_t size_type;


    // random number based
    static
    Uuid
    create_random_based() throw ();

    Uuid() throw ();

    explicit
    Uuid(const char* str, bool padding = true)
      throw (eh::Exception, Exception, InvalidArgument);

    explicit
    Uuid(const String::SubString& str, bool padding = true)
      throw (eh::Exception, Exception, InvalidArgument);

    explicit
    Uuid(std::istream& istr)
      throw (eh::Exception, Exception, InvalidArgument);

    template <typename ByteInputIterator>
    Uuid(ByteInputIterator first, ByteInputIterator last)
      throw (eh::Exception, Exception, InvalidArgument);

    bool
    operator ==(const Uuid& rhs) const throw ();

    bool
    operator !=(const Uuid& rhs) const throw ();

    bool
    operator <(const Uuid& rhs) const throw ();

    bool
    operator >(const Uuid& rhs) const throw ();

    bool
    operator <=(const Uuid& rhs) const throw ();

    bool
    operator >=(const Uuid& rhs) const throw ();

    bool
    is_null() const throw ();

    std::string
    to_string(bool padding = true) const
      throw (eh::Exception);

    static
    size_type
    size() throw ();

    static
    size_type
    encoded_size(bool padding = true) throw ();

    iterator
    begin() throw ();

    const_iterator
    begin() const throw ();

    iterator
    end() throw ();

    const_iterator
    end() const throw ();

    void
    swap(Uuid& rhs) throw ();

    unsigned long
    hash() const throw ();

  private:
    static const size_type DATA_SIZE = 16;
    typedef value_type DataType[DATA_SIZE];

    template <typename Iterator>
    Iterator
    construct_(Iterator begin, Iterator end, bool padding)
      throw (eh::Exception, Exception, InvalidArgument);

    void
    construct_(const String::SubString& str, bool padding)
      throw (eh::Exception, Exception, InvalidArgument);

    union
    {
      DataType data_;
      uint64_t hash_[2];
    };
  }
# ifdef __GNUC__
  __attribute__ ((packed))
# endif
  ;

  std::ostream&
  operator <<(std::ostream& ostr, const Uuid& uuid) throw ();
  std::istream&
  operator >>(std::istream& istr, Uuid& uuid) throw ();

  template <typename Hash>
  void
  hash_add(Hash& hash, const Uuid& value) throw ();


  class SignedUuidGenerator;
  class SignedUuidVerifier;

  /**
   * Class containing Uuid, it's signature and four data bits
   * May be constructed only by SignedUuidGenerator or SignedUuidVerifier
   */
  class SignedUuid
  {
  public:
    /**
     * Returns contained uuid
     * @return contained uuid
     */
    const Uuid&
    uuid() const throw ();

    /**
     * Returns contained data bits
     * @return contained data bits
     */
    uint8_t
    data() const throw ();

    /**
     * String representation of Uuid and its signature. It can be parsed
     * by SignedUuidVerifier to create SignedUuid object.
     * @return string representation of the signed uuid
     */
    const std::string&
    str() const throw ();


  private:
    /**
     * Constructor
     * @param uuid uuid to hold
     * @param data data bits
     * @param sign its signature (base64-encoded)
     */
    SignedUuid(const Uuid& uuid, uint8_t data,
      const String::SubString& sign) throw (eh::Exception);

    Uuid uuid_;
    uint8_t data_;
    std::string str_;

    friend class SignedUuidGenerator;
    friend class SignedUuidVerifier;
    friend class SignedUuidProbe;
  };

  /**
   * Generator of SignedUuids
   * Requires private RSA key for signing
   */
  class SignedUuidGenerator
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Constructor
     * Reads the private RSA key
     * @param private_key name of ASN1 file containing private RSA key
     */
    SignedUuidGenerator(const char* private_key)
      throw (eh::Exception);

    /**
     * Generates random uuid and signs it.
     * @param data optional data bits
     * @return randomly generated SignedUuid
     */
    SignedUuid
    generate(uint8_t data = 0) const throw (eh::Exception, Exception);

    /**
     * Signs the supplied uuid.
     * @param uuid uuid to sign
     * @param data optional data bits
     * @return randomly generated SignedUuid
     */
    SignedUuid
    sign(const Uuid& uuid, uint8_t data = 0) const
      throw (eh::Exception, Exception);

  private:
    RSAKey<true> key_;
    const unsigned SIZE_;
  };

  /**
   * Verifies if a string represents SignedUuid
   * Requires public RSA key for signature verifying.
   */
  class SignedUuidVerifier
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Constructor
     * Reads the public RSA key
     * @param public_key name of ASN1 file containing public RSA key
     */
    SignedUuidVerifier(const char* public_key)
      throw (eh::Exception);

    /**
     * Verifies if a string represents SignedUuid and creates the object
     * @param uuid_str signed Uuid string
     * @param data_expected if data bits are expected or not
     * @return SignedUuid parsed from the string representation
     */
    SignedUuid
    verify(const String::SubString& uuid_str,
      bool data_expected = false) const throw (eh::Exception, Exception);

  private:
    RSAKey<false> key_;
    const unsigned SIZE_;
  };

  /**
   * Special SignedUuid creator returning the same _not signed_ Uuid.
   * Designed for special Probe Uuid.
   */
  class SignedUuidProbe
  {
  public:
    explicit
    SignedUuidProbe(const Uuid& probe) throw ();

    SignedUuid
    construct() const throw (eh::Exception);

  private:
    SignedUuid probe_;
  };
}

namespace Generics
{
  //
  // Uuid class
  //

  template <typename ByteInputIterator>
  Uuid::Uuid(ByteInputIterator first, ByteInputIterator last)
    throw (eh::Exception, Exception, InvalidArgument)
  {
    size_t i = 0;
    for (; i < DATA_SIZE && first != last; ++i)
    {
      data_[i] = static_cast<value_type>(*first++);
    }
    if (i != DATA_SIZE)
    {
      Stream::Error ostr;
      ostr << FNS << "invalid input Uuid iterator pair, must span 16 bytes";
      throw InvalidArgument(ostr);
    }
  }

  inline
  bool
  Uuid::operator ==(const Uuid& rhs) const throw ()
  {
    for (size_t i = 0; i < DATA_SIZE; i++)
    {
      if (data_[i] != rhs.data_[i])
      {
        return false;
      }
    }
    return true;
  }

  inline
  bool
  Uuid::operator !=(const Uuid& rhs) const throw ()
  {
    return !operator ==(rhs);
  }

  inline
  bool
  Uuid::operator <(const Uuid& rhs) const throw ()
  {
    for (size_t i = 0; i < DATA_SIZE; i++)
    {
      if (data_[i] < rhs.data_[i])
      {
        return true;
      }
      if (data_[i] > rhs.data_[i])
      {
        break;
      }
    }
    return false;
  }

  inline
  bool
  Uuid::operator >(const Uuid& rhs) const throw ()
  {
    return rhs < *this;
  }

  inline
  bool
  Uuid::operator <=(const Uuid& rhs) const throw ()
  {
    return !operator >(rhs);
  }

  inline
  bool
  Uuid::operator >=(const Uuid& rhs) const throw ()
  {
    return !operator <(rhs);
  }

  inline
  bool
  Uuid::is_null() const throw ()
  {
    for (size_t i = 0; i < DATA_SIZE; i++)
    {
      if (data_[i] != 0)
      {
        return false;
      }
    }

    return true;
  }

  inline
  Uuid::size_type
  Uuid::size() throw ()
  {
    return DATA_SIZE;
  }

  inline
  Uuid::size_type
  Uuid::encoded_size(bool padding) throw ()
  {
    return String::StringManip::base64mod_encoded_size(DATA_SIZE, padding);
  }

  inline
  Uuid::iterator
  Uuid::begin() throw ()
  {
    return data_;
  }

  inline
  Uuid::const_iterator
  Uuid::begin() const throw ()
  {
    return data_;
  }

  inline
  Uuid::iterator
  Uuid::end() throw ()
  {
    return data_ + DATA_SIZE;
  }

  inline
  Uuid::const_iterator
  Uuid::end() const throw ()
  {
    return data_ + DATA_SIZE;
  }

  inline
  void
  Uuid::swap(Uuid& rhs) throw ()
  {
    DataType data;
    std::copy(begin(), end(), data);
    std::copy(rhs.begin(), rhs.end(), data_);
    std::copy(data, data + sizeof(data), rhs.data_);
  }

  inline
  unsigned long
  Uuid::hash() const throw ()
  {
    return hash_[1];
  }


  template <typename Hash>
  void
  hash_add(Hash& hash, const Uuid& value) throw ()
  {
    hash.add(value.begin(), value.size());
  }


  //
  // SignedUuid class
  //

  inline
  const Uuid&
  SignedUuid::uuid() const throw ()
  {
    return uuid_;
  }

  inline
  uint8_t
  SignedUuid::data() const throw ()
  {
    return data_;
  }

  inline
  const std::string&
  SignedUuid::str() const throw ()
  {
    return str_;
  }
} // namespace Generics

#endif
