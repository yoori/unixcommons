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
#include <set>

#include <Generics/CountryCodeManip.hpp>
#include <Generics/Time.hpp>

DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

const char* ALL_CODES[] =
{
  "AC", "CP", "DG", "EA", "EU", "FX", "IC", "TA", "UK",
  "CS", "NT", "SF", "SU", "TP", "YU", "ZR", "AF", "AX",
  "AL", "DZ", "AS", "AD", "AO", "AI", "AQ", "AG", "AR",
  "AM", "AW", "AU", "AT", "AZ", "BS", "BH", "BD", "BB",
  "BY", "BE", "BZ", "BJ", "BM", "BT", "BO", "BA", "BW",
  "BV", "BR", "IO", "BN", "BG", "BF", "BI", "KH", "CM",
  "CA", "CV", "KY", "CF", "TD", "CL", "CN", "CX", "CC",
  "CO", "KM", "CG", "CD", "CK", "CR", "CI", "HR", "CU",
  "CY", "CZ", "DK", "DJ", "DM", "DO", "EC", "EG", "SV",
  "GQ", "ER", "EE", "ET", "FK", "FO", "FJ", "FI", "FR",
  "GF", "PF", "TF", "GA", "GM", "GE", "DE", "GH", "GI",
  "GR", "GL", "GD", "GP", "GU", "GT", "GG", "GN", "GW",
  "GY", "HT", "HM", "VA", "HN", "HK", "HU", "IS", "IN",
  "ID", "IR", "IQ", "IE", "IM", "IL", "IT", "JM", "JP",
  "JE", "JO", "KZ", "KE", "KI", "KP", "KR", "KW", "KG",
  "LA", "LV", "LB", "LS", "LR", "LY", "LI", "LT", "LU",
  "MO", "MK", "MG", "MW", "MY", "MV", "ML", "MT", "MH",
  "MQ", "MR", "MU", "YT", "MX", "FM", "MD", "MC", "MN",
  "ME", "MS", "MA", "MZ", "MM", "NA", "NR", "NP", "NL",
  "AN", "NC", "NZ", "NI", "NE", "NG", "NU", "NF", "MP",
  "NO", "OM", "PK", "PW", "PS", "PA", "PG", "PY", "PE",
  "PH", "PN", "PL", "PT", "PR", "QA", "RE", "RO", "RU",
  "RW", "BL", "SH", "KN", "LC", "MF", "PM", "VC", "WS",
  "SM", "ST", "SA", "SN", "RS", "SC", "SL", "SG", "SK",
  "SI", "SB", "SO", "ZA", "GS", "ES", "LK", "SD", "SR",
  "SJ", "SZ", "SE", "CH", "SY", "TW", "TJ", "TZ", "TH",
  "TL", "TG", "TK", "TO", "TT", "TN", "TR", "TM", "TC",
  "TV", "UG", "UA", "AE", "GB", "US", "UM", "UY", "UZ",
  "VU", "VE", "VN", "VG", "VI", "WF", "EH", "YE", "ZM",
  "ZW",
  "AFG", "ALB", "DZA", "ASM", "AND", "AGO", "AIA", "ATA",
  "ATG", "ARG", "ARM", "ABW", "AUS", "AUT", "AZE", "BHS",
  "BHR", "BGD", "BRB", "BLR", "BEL", "BLZ", "BEN", "BMU",
  "BTN", "BOL", "BIH", "BWA", "BVT", "BRA", "IOT", "VGB",
  "BRN", "BGR", "BFA", "BDI", "KHM", "CMR", "CAN", "CPV",
  "CYM", "CAF", "TCD", "CHL", "CHN", "CXR", "CCK", "COL",
  "COM", "COD", "COG", "COK", "CRI", "CIV", "CUB", "CYP",
  "CZE", "DNK", "DJI", "DMA", "DOM", "ECU", "EGY", "SLV",
  "GNQ", "ERI", "EST", "ETH", "FRO", "FLK", "FJI", "FIN",
  "FRA", "GUF", "PYF", "ATF", "GAB", "GMB", "GEO", "DEU",
  "GHA", "GIB", "GRC", "GRL", "GRD", "GLP", "GUM", "GTM",
  "GIN", "GNB", "GUY", "HTI", "HMD", "VAT", "HND", "HKG",
  "HRV", "HUN", "ISL", "IND", "IDN", "IRN", "IRQ", "IRL",
  "ISR", "ITA", "JAM", "JPN", "JOR", "KAZ", "KEN", "KIR",
  "PRK", "KOR", "KWT", "KGZ", "LAO", "LVA", "LBN", "LSO",
  "LBR", "LBY", "LIE", "LTU", "LUX", "MAC", "MKD", "MDG",
  "MWI", "MYS", "MDV", "MLI", "MLT", "MHL", "MTQ", "MRT",
  "MUS", "MYT", "MEX", "FSM", "MDA", "MCO", "MNG", "MSR",
  "MAR", "MOZ", "MMR", "NAM", "NRU", "NPL", "ANT", "NLD",
  "NCL", "NZL", "NIC", "NER", "NGA", "NIU", "NFK", "MNP",
  "NOR", "OMN", "PAK", "PLW", "PSE", "PAN", "PNG", "PRY",
  "PER", "PHL", "PCN", "POL", "PRT", "PRI", "QAT", "REU",
  "ROU", "RUS", "RWA", "SHN", "KNA", "LCA", "SPM", "VCT",
  "WSM", "SMR", "STP", "SAU", "SEN", "SCG", "SYC", "SLE",
  "SGP", "SVK", "SVN", "SLB", "SOM", "ZAF", "SGS", "ESP",
  "LKA", "SDN", "SUR", "SJM", "SWZ", "SWE", "CHE", "SYR",
  "TWN", "TJK", "TZA", "THA", "TLS", "TGO", "TKL", "TON",
  "TTO", "TUN", "TUR", "TKM", "TCA", "TUV", "VIR", "UGA",
  "UKR", "ARE", "GBR", "UMI", "USA", "URY", "UZB", "VUT",
  "VEN", "VNM", "WLF", "ESH", "YEM", "ZMB", "ZWE"
};

template<class KEY>
struct HashFun
{
  // hash
  inline size_t
  operator()(const KEY& value) const
  {
    return static_cast<size_t>(value);
  }
};

namespace
{
  inline uint32_t
  get_country_code(const char* str) throw ()
  {
    union
    {
      char str_code[sizeof(uint32_t)];
      uint32_t numeric;
    };
    numeric = 0;
    for (std::size_t i = 0; str[i] && i < 4; ++i)
    {
      str_code[i] = 
        String::AsciiStringManip::Tables::
        ASCII_TOUPPER_TABLE[static_cast<unsigned char>(str[i])];
    }
    return numeric;
  }

#if 0
  void
  check()
    throw (eh::Exception)
  {
    std::cout << "Hash tables performance test started" << std::endl;
    std::cout << "Meters GNU hash_set versus tr1::unordered_set,"
      " lower value is better" << std::endl;
    typedef std::set<uint32_t> Ranger;
    Ranger ranger;

    for(std::size_t i = 0;
        i < sizeof(ALL_CODES) / sizeof(ALL_CODES[0]);
        ++i)
    {
      ranger.insert(get_country_code(ALL_CODES[i]));
    }

//    typedef Generics::GnuHashSet<Generics::NumericHashAdapter<uint32_t> >
//    CountryMap_;

    typedef uint32_t KEY;
    typedef __gnu_cxx::hash_set<KEY, HashFun<KEY> > 
      CountryMap_;

    CountryMap_ country_map_(2029ul);

    typedef std::tr1::unordered_set<uint32_t> StdCountryMap_;
    StdCountryMap_ std_country_map_;

    std::size_t i = 0;
    for (Ranger::const_iterator it = ranger.begin();
      it != ranger.end();
      ++it)
    {
      ++i;
      country_map_.insert(*it);
      std_country_map_.insert(*it);
//      std::cout << *it << std::endl;
    }

    std::cout << "Unique elements: " << i << std::endl;
    std::cout << "GNU hashed: " << country_map_.size() << std::endl;
    std::cout << "STL hashed: " << std_country_map_.size() << std::endl;

    Generics::CPUTimer timer;
    timer.start();
    for (std::size_t j = 0; j < 1000; ++j)
    {
      for (Ranger::const_iterator it = ranger.begin();
           it != ranger.end();
           ++it)
      {
      }
    }
    timer.stop();
    std::cout << "Empty cycle: " << timer.elapsed_time() << std::endl;

    timer.start();
    for (std::size_t j = 0; j < 1000; ++j)
    {
      for (Ranger::const_iterator it = ranger.begin();
           it != ranger.end();
           ++it)
      {
        if (std_country_map_.find(*it) == std_country_map_.end())
        {
          throw 1;
        }
      }
    }
    timer.stop();
    std::cout << "STL::TR1 hash table: " << timer.elapsed_time()
      << std::endl;

    timer.start();
    for (std::size_t j = 0; j < 1000; ++j)
    {
      for (Ranger::const_iterator it = ranger.begin();
           it != ranger.end();
           ++it)
      {
        if (country_map_.find(*it) == country_map_.end())
        {
          throw 1;
        }
      }
    }
    timer.stop();
    std::cout << "GNU hash table: " << timer.elapsed_time() << std::endl;

  }
#endif

  void
  check_generics()
    throw (eh::Exception)
  {
    Generics::CountryCodeMap cmap;

    std::cout << "Generics::CountryCodeMap test started" << std::endl;
    Generics::CPUTimer timer;
    timer.start();
    for (std::size_t j = 0; j < 1000; ++j)
    {
      for(std::size_t i = 0;
          i < sizeof(ALL_CODES) / sizeof(ALL_CODES[0]);
          ++i)
      {
        if (!cmap.is_country_code(String::SubString(ALL_CODES[i])))
        {
          Stream::Error ostr;
          ostr << "Country code " << ALL_CODES[i] << 
            " numeric " << get_country_code(ALL_CODES[i]) << " not found";
          throw Exception(ostr);
        }
      }
    }
    timer.stop();
    std::cout << "Generics::CountryCodeMap performance: "
      << timer.elapsed_time() << std::endl;

    // negative part
    if (cmap.is_country_code(String::SubString(""))
        || cmap.is_country_code(String::SubString("AAA"))
        || cmap.is_country_code(String::SubString("123"))
        || cmap.is_country_code(String::SubString("@#$"))
        || cmap.is_country_code(String::SubString("afgn"))
        )
    {
      throw Exception("Mean garbage is country code");
    }
  }

}

int
main()
{
  try
  {
    check_generics();
    //check();

    std::cout << "Test complete" << std::endl;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception: " << std::endl;
  }

  return 0;
}
