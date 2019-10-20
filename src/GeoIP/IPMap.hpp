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





#ifndef GEOIP_IPMAP_HPP
#define GEOIP_IPMAP_HPP

#include <inttypes.h>
#include <memory>
#include <vector>
#include <unordered_map>

#include <GeoIP.h>

#include <Sync/PosixLock.hpp>

#include <String/SubString.hpp>


namespace GeoIPMapping
{
  /**
   * Base functionality for different databases
   */
  class IPMapBase : private Generics::Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    IPMapBase(int type, const char* file) throw (Exception);

    /**
     * Destructor. Removes GeoIP data from memory.
     */
    ~IPMapBase() throw ();

  protected:
    GeoIP* geo_ip_;
    Sync::PosixMutex lock_;
  };

  /**
   * Object oriented wrapper for GeoIP library routines.
   */
  class IPMap : protected IPMapBase
  {
  public:
    using IPMapBase::Exception;

    /**
     * Opens the GeoIP database (country information only)
     *
     * @param file The GeoIP database filename.
     */
    explicit
    IPMap(const char* file) throw (Exception);

    /**
     * Retrieves country code by IP address.
     *
     * @param ip IP address.
     * @param net_byte_order If <code>true</code>, the IP is expected to
     * have network byte order;
     * host byte order otherwise. By default, host byte order is used.
     *
     * @return Country code, if found.
     */
    std::string
    country_code_by_addr(uint32_t ip, bool net_byte_order = false)
      throw (Exception, eh::Exception);

    /**
     * Retrieves country code by IP address string.
     *
     * @param ip IP address as a null-terminated string.
     * @param no_throw Don't throw exception
     * if country code can't be determined,
     * return empty string instead
     *
     * @return Country code, if found.
     */
    std::string
    country_code_by_addr(const char* ip, bool no_throw = false)
      throw (Exception, eh::Exception);

    /**
     * Retrieves 3-letter country code by IP address.
     *
     * @param ip IP address as a null-terminated string.
     *
     * @return 3-letter country code, if found.
     */
    std::string
    country_code3_by_addr(const char* ip)
      throw (Exception, eh::Exception);

    /**
     * Retrieves country name by IP address.
     *
     * @param ip IP address as a null-terminated string.
     *
     * @return Country name, if found.
     */
    std::string
    country_name_by_addr(const char* ip)
      throw (Exception, eh::Exception);
  };

  class IPMapCity : protected IPMapBase
  {
  public:
    using IPMapBase::Exception;

    /**
     * City information database
     */
    struct CityLocation
    {
      std::string country_code;
      String::SubString region;
      std::string city;
      float latitude;
      float longitude;
    };

    /**
     * Opens the GeoIP database (city information only).
     *
     * @param file The GeoIP city database filename.
     */
    explicit
    IPMapCity(const char* file) throw (Exception);

    /**
     * Retrieves city location information by IP address.
     *
     * @param ip IP address as a null-terminated string.
     * @param location resulted city location information.
     * @param throw_if_absent throw exception if there is no corresponding
     * record in the database
     * @result if the call was successful or not
     */
    bool
    city_location_by_addr(const char* ip, CityLocation& location,
      bool throw_if_absent = true)
      throw (Exception, eh::Exception);
  };

  class IPMapCity2
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(FileNotExists, Exception);
    DECLARE_EXCEPTION(InvalidFormat, Exception);

    /**
     * City information database
     */
    struct CityLocation
    {
      String::SubString country_code;
      String::SubString region;
      String::SubString city;
    };

    /**
     * Opens the GeoIP database (city information only).
     *
     * @param file The GeoIP city database filename.
     */
    explicit
    IPMapCity2(const char* file) throw (FileNotExists, InvalidFormat);

    virtual
    ~IPMapCity2() throw();

    /**
     * Retrieves city location information by IP address.
     *
     * @param ip IP address as a null-terminated string.
     * @param location resulted city location information.
     * @param throw_if_absent throw exception if there is no corresponding
     * record in the database
     * @result if the call was successful or not
     */
    bool
    city_location_by_addr(const char* ip,
      CityLocation& location,
      bool throw_if_absent = true)
      throw (Exception, eh::Exception);

  protected:
    struct CityLocationHolder
    {
      std::string country_code;
      std::string region;
      std::string city;
    };

    typedef std::unordered_map<
      uint32_t,
      CityLocationHolder>
      MaskCityLocationMap;

    typedef std::vector<MaskCityLocationMap>
      MaskCityLocationMapArray;

  protected:
    bool
    city_location_by_addr_(
      CityLocation& location,
      uint32_t ip)
      const throw ();

    void
    load_(const String::SubString& file)
      throw(FileNotExists, InvalidFormat);

    bool
    parse_ip_mask_(
      unsigned char& bits,
      uint32_t& mask,
      const String::SubString& ip_mask_str);

    bool
    parse_city_location_(
      CityLocationHolder& city_location,
      const String::SubString& city_loc_str);

  protected:
    unsigned int max_check_bits_;
    // 0-16 masks (X.X.X.X/31 - X.X.X.X/16)
    MaskCityLocationMapArray mask_to_locations_;
  };
} // namespace GeoIPMapping

#endif
