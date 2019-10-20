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

#include <GeoIP/IPMap.hpp>


using namespace GeoIPMapping;

int
main(void)
{
#if 0
#if 0
  IPMap ipm(0);
#else
  IPMap ipm("GeoIP.dat");
#endif

  try
  {
    std::cout << "Test IP: '0.0.0.0'\n";
    std::cout << "Code: "
              << ipm.country_code_by_addr("0.0.0.0").c_str()
              << std::endl;

    std::cerr << "Unexpected behaviour" << std::endl;
  }
  catch(const IPMap::Exception& e)
  {
    std::cout << "Expected exception: " <<  e.what() << std::endl;
  }

  try
  {
    std::cout << "\nTest IP: '193.124.163.144'\n";
    std::cout << "Code3: "
              << ipm.country_code3_by_addr("193.124.163.144").c_str()
              << std::endl;
  }
  catch(const IPMap::Exception& e)
  {
    std::cerr << "Unexpected exception: " << e.what() << std::endl;
  }

  try
  {
    std::cout << "\nTest IP: '193.124.163.144'\n";
    std::cout << "Name: "
              << ipm.country_name_by_addr("193.124.163.144").c_str()
              << std::endl;
  }
  catch(const IPMap::Exception& e)
  {
    std::cerr << "Unexpected exception: " << e.what() << std::endl;
  }
#endif

#if 1
  IPMapCity city_map(0);
#else
  IPMapCity city_map(0, "GeoLiteCityv6.dat");
#endif

  try
  {
    const char* addrs[] =
    {
      "193.124.163.144",
      "::FFFF:193.124.163.144",
      "00::FFFF:193.124.163.144",
      "2002:C17C:A390::",
      "2002:C17C:A390:AB:CD:EF:01:23",
    };
    IPMapCity::CityLocation loc;
    for (size_t i = 0; i < sizeof(addrs) / sizeof(*addrs); i++)
    {
      std::cout << "\nTest IP: '" << addrs[i] << "'\n";
      city_map.city_location_by_addr(addrs[i], loc);
      std::cout << "Country code: " << loc.country_code << " Region: " <<
        loc.region << " City: " << loc.city << std::endl;
    }
  }
  catch (const IPMapCity::Exception& e)
  {
    std::cerr << "Unexpected exception: " << e.what() << std::endl;
  }

  try
  {
    IPMapCity::CityLocation loc;
    std::cout << "\nTest IP: '2001:0db8:11a3:09d7:1f34:8a2e:07a0:765d'\n";
    city_map.city_location_by_addr(
      "2001:0db8:11a3:09d7:1f34:8a2e:07a0:765d", loc);
    std::cout << "Country code: " << loc.country_code << " Region: " <<
      loc.region << " City: " << loc.city << std::endl;

    std::cerr << "Unexpected behaviour" << std::endl;
  }
  catch(const IPMap::Exception& e)
  {
    std::cout << "Expected exception: " <<  e.what() << std::endl;
  }

  return 0;
}
