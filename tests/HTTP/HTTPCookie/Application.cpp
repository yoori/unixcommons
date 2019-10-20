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
#include <HTTP/HTTPCookie.hpp>
#include <String/StringManip.hpp>

void
test_cookie_list() throw (eh::Exception)
{
  std::cout << "test_cookie_list()\n";

  HTTP::ClientCookieFacility cookie_facility;
/*
  cookie_holder.set_cookies(
    "LE1=V1; LE3=V2; expires=Wednesday 3-Aug-2005 13:51:59 GMT;"
    " path=\\; domain=.",
    "http://acc.adintelligence.net/hserver/requestid="
    "13F9ED00E45511D89A0800304852BBCE/site=WB.POP/channel=.Shopping+"
    "CUSTOM.Auto./uid=%7BH4e3896f-bdb5-5347-f1fd-42c7b11d65df%7D/v="
    "1.0.106/aamsz=/need=?keywords=suvs&amp;search=&amp;search-words=");
*/
  HTTP::SubHeaderList hl;

  hl.push_back(HTTP::SubHeader("Set-Cookie",
    "LE1=V1; LE3=V3; expires=Wed 03-Aug-2005 13:01:59 GMT;"
    " path=\\; domain=."));

  hl.push_back(HTTP::SubHeader("Set-Cookie",
    "le1=v1; le3=v3; expires=Wed 03-Aug-2015 13:01:59 GMT;"
    " path=\\; domain=."));

  hl.push_back(HTTP::SubHeader("Set-Cookie",
    "LE1=V11; LE3=V33; expires=Mon 28-Feb-2015 23:50:59 GMT;"
    " path=\\; domain=.adintelligence.net"));

  HTTP::HTTPAddress addr(
    String::SubString("http://acc.adintelligence.net/hserver/requestid="
    "13F9ED00E45511D89A0800304852BBCE/site=WB.POP/channel=.Shopping+"
    "CUSTOM.Auto./uid=%7BH4e3896f-bdb5-5347-f1fd-42c7b11d65df%7D/v="
    "1.0.106/aamsz=/need=?keywords=suvs&amp;search=&amp;search-words="));
  cookie_facility.load_from_headers(hl, addr);

  HTTP::HeaderList headers;
  cookie_facility.set_cookie_header(headers);

  std::cout << "set_cookie_header:\n";

  for (HTTP::HeaderList::const_iterator it = headers.begin();
    it != headers.end(); ++it)
  {
    std::cout << "  " << it->name << " : " << it->value << std::endl;
  }

  std::cout << "cookie_header:\n";
  std::string cookies = cookie_facility.cookie_header(
    HTTP::HTTPAddress(String::SubString(
      "http://acc.adintelligence.net/hserver/")));

  std::cout << "   Cookie : " << cookies << "\n----------------\n";

  HTTP::CookieList cookie_list;

  hl.clear();

  hl.push_back(HTTP::SubHeader("Cookie", "LE1=V1; LE3=V3"));

  hl.push_back(HTTP::SubHeader("Cookie", "LE1=V1; LE3=V3, LE2=V2; LE3=V33"));
  hl.push_back(HTTP::SubHeader("Cookie", "a=b,; c=d;, e=f;"));

  cookie_list.load_from_headers(hl);

  std::cout << "Cookie : " << cookie_list.cookie_header() << std::endl;
}

void
test_cookie_def_list() throw (eh::Exception)
{
  std::cout << "test_cookie_def_list()\n";

  HTTP::SubHeaderList hl1;
  hl1.push_back(HTTP::SubHeader("Set-Cookie", "sc=0/GCSdEeDAA|; expires=Sat, 30-Jan-2020 12:25:55 GMT; path=/services/"));
  hl1.push_back(HTTP::SubHeader("Set-Cookie", "uid=PPPPPPPPPPPPPPPPPPPPPP||; expires=Sat, 30-Jan-2020 12:25:55 GMT; path=/services/"));

  HTTP::CookieDefList cookie_list;
  HTTP::HTTPAddress addr(String::SubString(
    "http://prof1.ocslab.com:28080/services/nslookup"
    "?testrequest=0&setuid=1&prck=0&glbfcap=0&format=unit-test&xinfopsid=0"
    "&rnd=388334&v=1.3.0-3.ssv1&app=PS&require-debug-info=header"));
  cookie_list.load_from_headers(hl1, addr);

  for (HTTP::CookieDefList::const_iterator it = cookie_list.begin();
    it != cookie_list.end(); ++it)
  {
    std::cout << "Cookie: domain '" << it->domain << "' path '" <<
      it->path << "' expires " << it->expires.get_gm_time() << " secure " <<
      it->secure << " name '" << it->name << "' value '" << it->value <<
      "'\n";
  }
  std::cout << "Cookie : " << cookie_list.cookie_header(HTTP::HTTPAddress(
    String::SubString("http://prof1.ocslab.com/services/la-la-la"))) <<
    std::endl;
}

int
main(int /*argc*/, char** /*argv*/)
{
  try
  {
    test_cookie_list();
    test_cookie_def_list();
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "main: exception caught. Description:\n" <<
      e.what() << std::endl;
    return -1;
  }

  return 0;
}
