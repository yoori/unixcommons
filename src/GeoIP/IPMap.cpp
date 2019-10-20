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





#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <GeoIPCity.h>

#include <String/StringManip.hpp>
#include <String/Tokenizer.hpp>

#include <Generics/GnuHashTable.hpp>
#include <Generics/HashTableAdapters.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>
#include <Stream/MMapStream.hpp>

#include <GeoIP/IPMap.hpp>


namespace
{
  class Regions : private Generics::Uncopyable
  {
  public:
    Regions()
      throw (eh::Exception);

    ~Regions()
      throw ();

    void
    region(const char* country, const char* region_code,
      String::SubString& region) const
      throw ();

  private:
    uint32_t
    hash_(const char* country, const char* region_code) const
      throw ();

    typedef Generics::GnuHashTable<Generics::NumericHashAdapter<uint32_t>,
      std::string> AllRegions;

    AllRegions regions_;
  };

  uint32_t
  Regions::hash_(const char* country, const char* region_code) const
    throw ()
  {
    if (!country[0] || !country[1] || country[2] ||
      !region_code[0] || !region_code[1] || region_code[2])
    {
      return 0;
    }

    return (static_cast<uint32_t>(country[0]) << 24) |
      (static_cast<uint32_t>(country[1]) << 16) |
      (static_cast<uint32_t>(region_code[0]) << 8) |
      (static_cast<uint32_t>(region_code[1]) << 0);
  }

  Regions::Regions() throw (eh::Exception)
  {
    Stream::FileParser in("/usr/share/GeoIP/fips_include");
    std::string line;
    while (getline(in, line))
    {
      String::StringManip::trim(line, line);
      if (line.size() < 9)
      {
        continue;
      }
      if (line[2] != ',' || line[5] != ',' ||
        line[6] != '"' || line[line.size() - 1] != '"')
      {
        continue;
      }

      line[2] = '\0';
      line[5] = '\0';
      line[line.size() - 1] = '\0';

      uint32_t hash = hash_(&line[0], &line[3]);
      if (!hash)
      {
        continue;
      }

      regions_[hash] = &line[7];
    }
  }

  Regions::~Regions() throw ()
  {
    GeoIP_cleanup();
  }

  void
  Regions::region(const char* country, const char* region_code,
    String::SubString& region) const
    throw ()
  {
    uint32_t hash = hash_(country, region_code);
    if (!hash)
    {
      region.clear();
      return;
    }
    AllRegions::const_iterator itor(regions_.find(hash));
    if (itor == regions_.end())
    {
      region.clear();
      return;
    }
    region = itor->second;
  }

  const Regions regions;


  unsigned long
  ip_to_ipv4(const char* ip) throw ()
  {
    for (const char* p = ip; *p; p++)
    {
      if (*p == ':')
      {
        in6_addr addr;
        if (!inet_pton(AF_INET6, ip, &addr))
        {
          return 0;
        }
        if (!addr.s6_addr32[0])
        {
          if (!addr.s6_addr32[1] && addr.s6_addr32[2] == 0xFFFF0000ul)
          {
            return ntohl(addr.s6_addr32[3]);
          }
        }
        else
        {
          if (addr.s6_addr16[0] == 0x0220)
          {
            return (static_cast<uint32_t>(ntohs(addr.s6_addr16[1])) << 16) |
              ntohs(addr.s6_addr16[2]);
          }
        }
        return 0;
      }
    }

    in_addr addr;
    if (!inet_pton(AF_INET, ip, &addr))
    {
      return 0;
    }
    return ntohl(addr.s_addr);
  }

}

namespace GeoIPMapping
{
  //
  // IPMapBase class
  //

  IPMapBase::IPMapBase(int type, const char* file) throw (Exception)
    : geo_ip_(0)
  {
    if (file)
    {
      geo_ip_ = GeoIP_open(file, GEOIP_MEMORY_CACHE);

      if (!geo_ip_)
      {
        Stream::Error ostr;
        ostr << FNS << "file '" << file << "' not found";
        throw Exception(ostr);
      }
    }
    else
    {
      geo_ip_ = GeoIP_open_type(type, GEOIP_MEMORY_CACHE);

      if (!geo_ip_)
      {
        Stream::Error ostr;
        ostr << FNS << "type " << type << " not found";
        throw Exception(ostr);
      }
    }
  }

  IPMapBase::~IPMapBase() throw ()
  {
    if (geo_ip_)
    {
      GeoIP_delete(geo_ip_);
    }
  }


  //
  // IPMapClass
  //

  IPMap::IPMap(const char* filename)
    throw (Exception)
    : IPMapBase(GEOIP_COUNTRY_EDITION, filename)
  {
  }

  std::string
  IPMap::country_code_by_addr(uint32_t ip, bool net_byte_order)
    throw (Exception, eh::Exception)
  {
    struct in_addr addr;

    if (!net_byte_order)
    {
      ip = htonl(ip);
    }

    addr.s_addr = ip;
    char host_ip[32];
    if (!inet_ntop(AF_INET, &addr, host_ip, sizeof(host_ip)))
    {
      Stream::Error ostr;
      ostr << FNS << "inet_ntop(" << ip << ") failed";
      throw Exception(ostr);
    }

    Sync::PosixGuard guard(lock_);

    const char* code = GeoIP_country_code_by_addr(geo_ip_, host_ip);
    if (!code)
    {
      Stream::Error ostr;
      ostr << FNS << "GeoIP_country_code_by_addr(" << host_ip << ") failed";
      throw Exception(ostr);
    }

    return code;
  }

  std::string
  IPMap::country_code_by_addr(const char* ip, bool no_throw)
    throw (Exception, eh::Exception)
  {
    if (!ip)
    {
      Stream::Error ostr;
      ostr << FNS << "ip is NULL";
      throw Exception(ostr);
    }

    Sync::PosixGuard guard(lock_);

    const char* code = GeoIP_country_code_by_addr(geo_ip_, ip);
    if (!code)
    {
      if (no_throw)
      {
        return std::string();
      }
      else
      {
        Stream::Error ostr;
        ostr << FNS << "GeoIP_country_code_by_addr(" << ip << ") failed";
        throw Exception(ostr);
      }
    }

    return code;
  }

  std::string
  IPMap::country_code3_by_addr(const char* ip)
    throw (Exception, eh::Exception)
  {
    if (!ip)
    {
      Stream::Error ostr;
      ostr << FNS << "ip is NULL";
      throw Exception(ostr);
    }

    Sync::PosixGuard guard(lock_);

    const char* code = GeoIP_country_code3_by_addr(geo_ip_, ip);
    if (!code)
    {
      Stream::Error ostr;
      ostr << FNS << "GeoIP_country_code3_by_addr(" << ip << ") failed";
      throw Exception(ostr);
    }

    return code;
  }

  std::string
  IPMap::country_name_by_addr(const char* ip)
    throw (Exception, eh::Exception)
  {
    if (!ip)
    {
      Stream::Error ostr;
      ostr << FNS << "ip is NULL";
      throw Exception(ostr);
    }

    Sync::PosixGuard guard(lock_);

    const char* name = GeoIP_country_name_by_addr(geo_ip_, ip);
    if (!name)
    {
      Stream::Error ostr;
      ostr << FNS << "GeoIP_country_name_by_addr(" << ip << ") failed";
      throw Exception(ostr);
    }

    return name;
  }


  //
  // IPMapCity class
  //

  IPMapCity::IPMapCity(const char* filename)
    throw (Exception)
    : IPMapBase(GEOIP_CITY_EDITION_REV1, filename)
  {
  }

  bool
  IPMapCity::city_location_by_addr(const char* ip, CityLocation& location,
    bool throw_if_absent)
    throw (Exception, eh::Exception)
  {
    if (!ip)
    {
      Stream::Error ostr;
      ostr << FNS << "ip is NULL";
      throw Exception(ostr);
    }

    unsigned long ipv4 = ip_to_ipv4(ip);
    if (!ipv4)
    {
      Stream::Error ostr;
      ostr << FNS << "unsupported IPv4 '" << ip << "'";
      throw Exception(ostr);
    }

    Sync::PosixGuard guard(lock_);

    GeoIPRecord* iprec = GeoIP_record_by_ipnum(geo_ip_, ipv4);
    if (!iprec || !iprec->country_code)
    {
      if (iprec)
      {
        GeoIPRecord_delete(iprec);
      }

      if (throw_if_absent)
      {
        Stream::Error ostr;
        ostr << FNS << "GeoIP_record_by_addr(" << ip << ") failed";
        throw Exception(ostr);
      }

      return false;
    }

    try
    {
      location.country_code = iprec->country_code;
      if (iprec->region)
      {
        regions.region(iprec->country_code, iprec->region, location.region);
      }
      else
      {
        location.region.clear();
      }
      if (iprec->city)
      {
        location.city = iprec->city;
      }
      else
      {
        location.city.clear();
      }
      location.latitude = iprec->latitude;
      location.longitude = iprec->longitude;
    }
    catch (...)
    {
      GeoIPRecord_delete(iprec);
      throw;
    }

    GeoIPRecord_delete(iprec);

    return true;
  }

  //
  // IPMapCity2 class
  //
  IPMapCity2::IPMapCity2(const char* filename)
    throw (FileNotExists, InvalidFormat)
    : max_check_bits_(0)
  {
    mask_to_locations_.resize(32);
    load_(filename ?
      String::SubString(filename) :
      String::SubString("/usr/share/GeoIP/ipv4.csv"));
  }

  IPMapCity2::~IPMapCity2() throw()
  {}

  bool
  IPMapCity2::city_location_by_addr(
    const char* ip,
    CityLocation& location,
    bool /*throw_if_absent*/)
    throw (Exception, eh::Exception)
  {
    if (!ip)
    {
      Stream::Error ostr;
      ostr << FNS << "ip is NULL";
      throw Exception(ostr);
    }

    uint32_t ipv4 = ip_to_ipv4(ip);
    if (!ipv4)
    {
      Stream::Error ostr;
      ostr << FNS << "unsupported IPv4 '" << ip << "'";
      throw Exception(ostr);
    }

    return city_location_by_addr_(location, ipv4);
  }
  
  bool
  IPMapCity2::city_location_by_addr_(
    CityLocation& location,
    uint32_t ip)
    const throw ()
  {
    uint32_t ip_mask = 0xFFFFFFFF;

    unsigned int bits_i = 0;
    for(auto it = mask_to_locations_.begin(); bits_i < max_check_bits_; ++it, ++bits_i)
    {
      uint32_t find_mask = ip & ip_mask;
      auto mask_it = it->find(find_mask);
      if(mask_it != it->end())
      {
        location.country_code = mask_it->second.country_code;
        location.region = mask_it->second.region;
        location.city = mask_it->second.city;

        return true;
      }

      ip_mask = ip_mask << 1;
    }

    return false;
  }

  void
  IPMapCity2::load_(const String::SubString& file)
    throw(FileNotExists, InvalidFormat)
  {
    static const char* FUN = "IPMapCity2::load_()";

    std::ifstream istr(file.str().c_str());
    if(!istr.is_open())
    {
      throw FileNotExists("");
    }

    std::string line_holder;
    while(!istr.eof())
    {
      std::getline(istr, line_holder);

      if(!line_holder.empty())
      {
        String::SubString line(line_holder);
        String::SubString::SizeType ip_mask_end = line.find(',');

        if(ip_mask_end != String::SubString::NPOS)
        {
          String::SubString ip_mask_str = line.substr(0, ip_mask_end);
          String::SubString city_loc_str = line.substr(ip_mask_end + 1);

          /*
          if(!top_splitter.get_token(city_loc_str))
          {
            Stream::Error ostr;
            ostr << FUN << ": can't parse line '" << line << "'";
            throw InvalidFormat(ostr);
          }
          */

          unsigned char ip_bits;
          uint32_t ip_mask;

          if(!parse_ip_mask_(ip_bits, ip_mask, ip_mask_str) ||
            (ip_bits > 32))
          {
            Stream::Error ostr;
            ostr << FUN << ": can't parse ip mask '" << ip_mask_str << "'";
            throw InvalidFormat(ostr);
          }

          CityLocationHolder city_location;

          if(city_loc_str.size() > 1 && *city_loc_str.begin() == '"' && *city_loc_str.rbegin() == '"')
          {
            city_loc_str = city_loc_str.substr(1, city_loc_str.size() - 2);
          }

          if(!parse_city_location_(city_location, city_loc_str))
          {
            Stream::Error ostr;
            ostr << FUN << ": can't parse ip mask '" << ip_mask_str << "'";
            throw InvalidFormat(ostr);
          }

          mask_to_locations_[32 - ip_bits].insert(std::make_pair(ip_mask, city_location));
          max_check_bits_ = std::max(max_check_bits_, static_cast<unsigned int>(32 - ip_bits));
          line.clear();
        }
      }
    }
  }

  bool
  IPMapCity2::parse_ip_mask_(
    unsigned char& bits,
    uint32_t& mask,
    const String::SubString& ip_mask_str)
  {
    String::StringManip::SplitSlash splitter(ip_mask_str);

    String::SubString ip_str;
    splitter.get_token(ip_str);
    mask = 0;
    mask = ip_to_ipv4(ip_str.str().c_str());

    if(!mask)
    {
      return false;
    }

    String::SubString bits_str;
    splitter.get_token(bits_str);

    if(!String::StringManip::str_to_int(bits_str, bits))
    {
      return false;
    }

    return true;
  }

  bool
  IPMapCity2::parse_city_location_(
    CityLocationHolder& city_location,
    const String::SubString& city_loc_str)
  {
    String::SubString::SizeType country_end = city_loc_str.find('/');

    if(country_end != String::SubString::NPOS)
    {
      city_location.country_code.assign(city_loc_str.data(), country_end);

      String::SubString::SizeType region_end = city_loc_str.find('/', country_end + 1);

      if(region_end != String::SubString::NPOS)
      {
        city_location.region.assign(city_loc_str.data() + country_end + 1, region_end - (country_end + 1));
        city_location.city.assign(city_loc_str.data() + region_end + 1, city_loc_str.size() - (region_end + 1));
      }
      else
      {
        city_location.region.assign(city_loc_str.data() + country_end + 1, city_loc_str.size() - (country_end + 1));
      }
    }
    else
    {
      city_location.country_code = city_loc_str.str();
    }

    /*
    city_location.latitude = 0;
    city_location.longitude = 0;
    */

    return true;
  }
}
