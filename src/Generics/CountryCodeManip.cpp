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



// Generics/CountryCodeManip.cpp
#include <String/AsciiStringManip.hpp>

#include <Generics/CountryCodeManip.hpp>


namespace
{
  const char ISO3166[][3] =
  {
    "AF", "AX", "AL", "DZ", "AS", "AD", "AO", "AI", "AQ", "AG", "AR", "AM",
    "AW", "AU", "AT", "AZ", "BS", "BH", "BD", "BB", "BY", "BE", "BZ", "BJ",
    "BM", "BT", "BO", "BA", "BW", "BV", "BR", "IO", "BN", "BG", "BF", "BI",
    "KH", "CM", "CA", "CV", "KY", "CF", "TD", "CL", "CN", "CX", "CC", "CO",
    "KM", "CG", "CD", "CK", "CR", "CI", "HR", "CU", "CY", "CZ", "DK", "DJ",
    "DM", "DO", "EC", "EG", "SV", "GQ", "ER", "EE", "ET", "FK", "FO", "FJ",
    "FI", "FR", "GF", "PF", "TF", "GA", "GM", "GE", "DE", "GH", "GI", "GR",
    "GL", "GD", "GP", "GU", "GT", "GG", "GN", "GW", "GY", "HT", "HM", "VA",
    "HN", "HK", "HU", "IS", "IN", "ID", "IR", "IQ", "IE", "IM", "IL", "IT",
    "JM", "JP", "JE", "JO", "KZ", "KE", "KI", "KP", "KR", "KW", "KG", "LA",
    "LV", "LB", "LS", "LR", "LY", "LI", "LT", "LU", "MO", "MK", "MG", "MW",
    "MY", "MV", "ML", "MT", "MH", "MQ", "MR", "MU", "YT", "MX", "FM", "MD",
    "MC", "MN", "ME", "MS", "MA", "MZ", "MM", "NA", "NR", "NP", "NL", "AN",
    "NC", "NZ", "NI", "NE", "NG", "NU", "NF", "MP", "NO", "OM", "PK", "PW",
    "PS", "PA", "PG", "PY", "PE", "PH", "PN", "PL", "PT", "PR", "QA", "RE",
    "RO", "RU", "RW", "BL", "SH", "KN", "LC", "MF", "PM", "VC", "WS", "SM",
    "ST", "SA", "SN", "RS", "SC", "SL", "SG", "SK", "SI", "SB", "SO", "ZA",
    "GS", "ES", "LK", "SD", "SR", "SJ", "SZ", "SE", "CH", "SY", "TW", "TJ",
    "TZ", "TH", "TL", "TG", "TK", "TO", "TT", "TN", "TR", "TM", "TC", "TV",
    "UG", "UA", "AE", "GB", "US", "UM", "UY", "UZ", "VU", "VE", "VN", "VG",
    "VI", "WF", "EH", "YE", "ZM", "ZW"
  };

  // Fields "GG", "IM", "JE" duplicate ISO3166
  const char ISO3166_EX[][3] =
  {
    "AC", "CP", "DG", "EA", "EU", "FX", "IC", "TA", "UK", "GG", "IM", "JE",
    "CS", "NT", "SF", "SU", "TP", "YU", "ZR"
  };

  const char ISO3166_3[][4] =
  {
    "AFG", "ALB", "DZA", "ASM", "AND", "AGO", "AIA", "ATA", "ATG", "ARG",
    "ARM", "ABW", "AUS", "AUT", "AZE", "BHS", "BHR", "BGD", "BRB", "BLR",
    "BEL", "BLZ", "BEN", "BMU", "BTN", "BOL", "BIH", "BWA", "BVT", "BRA",
    "IOT", "VGB", "BRN", "BGR", "BFA", "BDI", "KHM", "CMR", "CAN", "CPV",
    "CYM", "CAF", "TCD", "CHL", "CHN", "CXR", "CCK", "COL", "COM", "COD",
    "COG", "COK", "CRI", "CIV", "CUB", "CYP", "CZE", "DNK", "DJI", "DMA",
    "DOM", "ECU", "EGY", "SLV", "GNQ", "ERI", "EST", "ETH", "FRO", "FLK",
    "FJI", "FIN", "FRA", "GUF", "PYF", "ATF", "GAB", "GMB", "GEO", "DEU",
    "GHA", "GIB", "GRC", "GRL", "GRD", "GLP", "GUM", "GTM", "GIN", "GNB",
    "GUY", "HTI", "HMD", "VAT", "HND", "HKG", "HRV", "HUN", "ISL", "IND",
    "IDN", "IRN", "IRQ", "IRL", "ISR", "ITA", "JAM", "JPN", "JOR", "KAZ",
    "KEN", "KIR", "PRK", "KOR", "KWT", "KGZ", "LAO", "LVA", "LBN", "LSO",
    "LBR", "LBY", "LIE", "LTU", "LUX", "MAC", "MKD", "MDG", "MWI", "MYS",
    "MDV", "MLI", "MLT", "MHL", "MTQ", "MRT", "MUS", "MYT", "MEX", "FSM",
    "MDA", "MCO", "MNG", "MSR", "MAR", "MOZ", "MMR", "NAM", "NRU", "NPL",
    "ANT", "NLD", "NCL", "NZL", "NIC", "NER", "NGA", "NIU", "NFK", "MNP",
    "NOR", "OMN", "PAK", "PLW", "PSE", "PAN", "PNG", "PRY", "PER", "PHL",
    "PCN", "POL", "PRT", "PRI", "QAT", "REU", "ROU", "RUS", "RWA", "SHN",
    "KNA", "LCA", "SPM", "VCT", "WSM", "SMR", "STP", "SAU", "SEN", "SCG",
    "SYC", "SLE", "SGP", "SVK", "SVN", "SLB", "SOM", "ZAF", "SGS", "ESP",
    "LKA", "SDN", "SUR", "SJM", "SWZ", "SWE", "CHE", "SYR", "TWN", "TJK",
    "TZA", "THA", "TLS", "TGO", "TKL", "TON", "TTO", "TUN", "TUR", "TKM",
    "TCA", "TUV", "VIR", "UGA", "UKR", "ARE", "GBR", "UMI", "USA", "URY",
    "UZB", "VUT", "VEN", "VNM", "WLF", "ESH", "YEM", "ZMB", "ZWE"
  };
}

namespace Generics
{
  inline
  uint32_t
  CountryCodeMap::get_country_code_(const String::SubString& str) throw ()
  {
    uint32_t code = 0;
    const std::size_t LEN = std::min(str.size(), static_cast<size_t>(4));
    for (std::size_t i = 0; i < LEN; i++)
    {
      code |= static_cast<uint32_t>(static_cast<uint8_t>(
        String::AsciiStringManip::Tables::ASCII_TOUPPER_TABLE[
          static_cast<uint8_t>(str[i])])) << (i * 8);
    }
    return code;
  }

  CountryCodeMap::CountryCodeMap() throw (eh::Exception)
  {
    for (std::size_t i = 0; i < sizeof(ISO3166) / sizeof(ISO3166[0]); i++)
    {
      country_map_.insert(get_country_code_(
        String::SubString(ISO3166[i], sizeof(ISO3166[i]) - 1)));
    }

    for (std::size_t i = 0;
      i < sizeof(ISO3166_EX) / sizeof(ISO3166_EX[0]); i++)
    {
      country_map_.insert(get_country_code_(
        String::SubString(ISO3166_EX[i], sizeof(ISO3166_EX[i]) - 1)));
    }

    for (std::size_t i = 0; i < sizeof(ISO3166_3) / sizeof(ISO3166_3[0]); i++)
    {
      country_map_.insert(get_country_code_(
        String::SubString(ISO3166_3[i], sizeof(ISO3166_3[i]) - 1)));
    }
  }

  bool
  CountryCodeMap::is_country_code(const String::SubString& code) const
    throw ()
  {
    return code.empty() ? false :
      country_map_.find(get_country_code_(code)) != country_map_.end();
  }
}
