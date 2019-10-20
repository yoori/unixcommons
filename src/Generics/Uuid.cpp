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





#include <Generics/ISAAC.hpp>
#include <Generics/Time.hpp>
#include <Generics/Uuid.hpp>


namespace
{
  Sync::PosixMutex global_mutex;
  uint8_t
  random_byte() throw () // Called under mutex
  {
    static Generics::ISAAC generator;
    return static_cast<uint8_t>(generator.rand() >> 24);
  }
}

namespace Generics
{
  const Uuid::size_type Uuid::DATA_SIZE;

  Uuid::Uuid() throw ()
  {
    std::fill(data_, data_ + sizeof(data_), 0);
  }

  template <typename Iterator>
  Iterator
  Uuid::construct_(Iterator begin, Iterator end, bool padding)
    throw (eh::Exception, Exception, InvalidArgument)
  {
    size_type size = encoded_size(padding);
    std::string src;
    src.reserve(size);
    while (size--)
    {
      if (begin == end)
      {
        Stream::Error ostr;
        ostr << FNS << "Uuid string is too short";
        throw Generics::Uuid::InvalidArgument(ostr);
      }
      src.push_back(*begin);
      ++begin;
    }
    std::string result;
    String::StringManip::base64mod_decode(result, src, padding);
    *this = Uuid(result.data(), result.data() + result.size());

    return begin;
  }

  void
  Uuid::construct_(const String::SubString& str, bool padding)
    throw (eh::Exception, Exception, InvalidArgument)
  {
    if (construct_(str.begin(), str.end(), padding) != str.end())
    {
      Stream::Error ostr;
      ostr << FNS << "Uuid string contains extra symbols";
      throw InvalidArgument(ostr);
    }
  }

  Uuid::Uuid(const char* str, bool padding)
    throw (eh::Exception, Exception, InvalidArgument)
  {
    construct_(String::SubString(str), padding);
  }

  Uuid::Uuid(const String::SubString& str, bool padding)
    throw (eh::Exception, Exception, InvalidArgument)
  {
    construct_(str, padding);
  }

  Uuid::Uuid(std::istream& istr)
    throw (eh::Exception, Exception, InvalidArgument)
  {
    construct_(std::istreambuf_iterator<char>(istr),
      std::istreambuf_iterator<char>(0), true);
  }

  std::string
  Uuid::to_string(bool padding) const throw (eh::Exception)
  {
    std::string str;
    String::StringManip::base64mod_encode(str, data_, DATA_SIZE, padding);
    return str;
  }

  std::ostream&
  operator <<(std::ostream& ostr, const Uuid& uuid) throw ()
  {
    std::ostream::sentry ok(ostr);
    if (ok)
    {
      try
      {
        ostr << uuid.to_string(true);
      }
      catch (const eh::Exception&)
      {
        ostr.setstate(std::ios_base::failbit);
      }
    }
    return ostr;
  }

  std::istream&
  operator >>(std::istream& istr, Uuid& uuid) throw ()
  {
    std::istream::sentry ok(istr);
    if (ok)
    {
      try
      {
        uuid = Uuid(istr);
      }
      catch (const eh::Exception&)
      {
        istr.setstate(std::ios_base::failbit);
      }
    }
    return istr;
  }

  //random number based
  Uuid
  Uuid::create_random_based() throw ()
  {
    Uuid result;

    {
      Sync::PosixGuard lock(global_mutex);
      for (size_type i = 0; i < size(); ++i)
      {
        result.data_[i] = random_byte();
      }
    }

    // This code need for RFC 4122 compliance... see 4.4. paragraph.
    // set variant
    // should be 0b10xxxxxx
    result.data_[8] &= 0xBF;
    result.data_[8] |= 0x80;

    // set version
    // should be 0b0100xxxx
    result.data_[6] &= 0x4F; //0b01001111
    result.data_[6] |= 0x40; //0b01000000

    return result;
  }


  //
  // SignedUuid class
  //

  SignedUuid::SignedUuid(const Uuid& uuid, uint8_t data,
    const String::SubString& sign)
    throw (eh::Exception)
    : uuid_(uuid), data_(data)
  {
    String::StringManip::base64mod_encode(str_, uuid.begin(),
      uuid.size(), sign.empty(), data_);
    if (!sign.empty())
    {
      sign.append_to(str_);
    }
  }

  //
  // SignedUuidGenerator class
  //

  SignedUuidGenerator::SignedUuidGenerator(const char* private_key)
    throw (eh::Exception)
    : key_(private_key), SIZE_(RSA_size(key_.key()))
  {
  }

  SignedUuid
  SignedUuidGenerator::sign(const Uuid& uuid, uint8_t data) const
    throw (eh::Exception, Exception)
  {
    unsigned char sign[SIZE_];
    unsigned size;

    if (!RSA_sign_ASN1_OCTET_STRING(0,
      reinterpret_cast<const unsigned char*>(uuid.begin()),
      uuid.size(), sign, &size, key_.key()))
    {
      Stream::Error ostr;
      ostr << FNS << "Failed to sign generated Uuid";
      throw Exception(ostr);
    }

    std::string sign_str;
    String::StringManip::base64mod_encode(sign_str, sign, size, false);
    return SignedUuid(uuid, data, sign_str);
  }

  SignedUuid
  SignedUuidGenerator::generate(uint8_t data) const
    throw (eh::Exception, Exception)
  {
    return sign(Uuid::create_random_based(), data);
  }


  //
  // SignedUuidVerifier class
  //

  SignedUuidVerifier::SignedUuidVerifier(const char* public_key)
    throw (eh::Exception)
    : key_(public_key), SIZE_(RSA_size(key_.key()))
  {
  }

  SignedUuid
  SignedUuidVerifier::verify(const String::SubString& uuid_str,
    bool data_expected) const
    throw (eh::Exception, Exception)
  {
    if (uuid_str.size() != Uuid::encoded_size(false) +
      String::StringManip::base64mod_encoded_size(SIZE_, false))
    {
      Stream::Error ostr;
      ostr << FNS << "Incorrect size of string '" << uuid_str <<
        "' to be SignedUuid";
      throw Exception(ostr);
    }

    String::SubString encoded_sign(uuid_str.data() +
      Uuid::encoded_size(false),
      uuid_str.size() - Uuid::encoded_size(false));
    std::string sign;
    Uuid uuid;
    uint8_t data = 0;

    try
    {
      std::string dec;
      String::StringManip::base64mod_decode(dec,
        String::SubString(uuid_str.data(), Uuid::encoded_size(false)),
        false, data_expected ? &data : nullptr);

      uuid = Uuid(dec.begin(), dec.end());

      String::StringManip::base64mod_decode(sign, encoded_sign, false);
    }
    catch (const String::StringManip::InvalidFormatException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Failed to decode sign from '" << uuid_str << "': " <<
        ex.what();
      throw Exception(ostr);
    }

    if (!RSA_verify_ASN1_OCTET_STRING(0,
      reinterpret_cast<const unsigned char*>(uuid.begin()), uuid.size(),
      reinterpret_cast<unsigned char*>(&sign[0]), sign.size(), key_.key()))
    {
      Stream::Error ostr;
      ostr << FNS << "Signature does not suit Uuid in '" << uuid_str << "'";
      throw Exception(ostr);
    }
    return SignedUuid(uuid, data, encoded_sign);
  }


  //
  // SignedUuidProbe class
  //

  SignedUuidProbe::SignedUuidProbe(const Uuid& probe) throw ()
    : probe_(probe, 0, String::SubString())
  {
  }

  SignedUuid
  SignedUuidProbe::construct() const throw (eh::Exception)
  {
    return probe_;
  }
} // namespace Generics
