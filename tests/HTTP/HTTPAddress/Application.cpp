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
#include <string.h>
#include <getopt.h>

#include <eh/Exception.hpp>
#include <Generics/Function.hpp>
#include <HTTP/UrlAddress.hpp>

using namespace HTTP;

struct HTTPAddressInfo
{
  bool strict;
  const char* url;
  const char* normal;
  bool secure;
  const char* user_info;
  const char* host_name;
  unsigned short port;
  const char* path;
  const char* query;
  const char* fragment;
};

class HTTPAddressTest
{
public:
  HTTPAddressTest() throw ();
  int
  main(int argc, char** argv) throw ();

private:
  void
  print_url(const HTTPAddress &print_address, std::ostream& stream)
    throw (eh::Exception);
  bool
  compare_url(const HTTPAddress &url1, const HTTPAddress &url2)
    throw (eh::Exception);

  int
  test_url(const HTTPAddress& url) throw (eh::Exception);

  void
  print_test_url(const HTTPAddress& url, int test_case = -1)
    throw (eh::Exception);

  template <typename Checker, typename Address>
  bool
  interactive_test2008(int argc, char** argv) throw (eh::Exception);

  template <typename Checker, typename Address>
  int
  run_tests(bool strct) throw (eh::Exception);

private:
  bool strict_;
  static const HTTPAddressInfo test_cases[];
};

const HTTPAddressInfo HTTPAddressTest::test_cases[]=
{
  { false, "test12:90",
    "",
    false, "", "test12", 90, "", "", "" },
  { false, "https://test12?que%ry",
    "",
    true, "", "test12", 0, "/", "que%25ry", "" },
  { false, "http://us\x07""er@T112/p\ta%25TH%%?qUe%r%59#fra\xFFg",
    "http://t112/p%09a%25th%25%25?que%25ry",
    false, "us\%07er", "T112", 0, "/p\%09a%25TH%25%25", "qUe%25r%59",
    "fra\%FFg" },
  { true, "https://test12?query",
    "",
    true, "", "test12", 0, "/", "query", "" },
  { true, "hTTp://www.linux.org.ru:80?lor",
    "http://www.linux.org.ru/?lor",
    false, "", "www.linux.org.ru", 0, "/", "lor", "" },
  { true, "http://dev.ocslab.com:28180/services/nslookup",
    "",
    false, "", "dev.ocslab.com", 28180, "/services/nslookup", "", "" },
  { false, "http://cs.ocslab.com/cgi-bin/doo/index.cgi?"
    "orig-url=%%mime-url:ORIG_URL%%&request-id=%%REQUEST_ID%%"
    "&srv=dns.rubylan.net&oi_prompt=%%OI_PROMPT%%",
    "http://cs.ocslab.com/cgi-bin/doo/index.cgi?"
    "orig-url=%25%25mime-url:orig_url%25%25&"
    "request-id=%25%25request_id%25%25"
    "&srv=dns.rubylan.net&oi_prompt=%25%25oi_prompt%25%25",
    false, "", "cs.ocslab.com", 0, "/cgi-bin/doo/index.cgi",
    "orig-url=%25%25mime-url:ORIG_URL%25%25&"
    "request-id=%25%25REQUEST_ID%25%25"
    "&srv=dns.rubylan.net&oi_prompt=%25%25OI_PROMPT%25%25", "" },
  { false, "http://www.Alliancefran\xc3\x83\xc2\xa7" "aise.nu",
    "http://www.xn--alliancefranaise-kta39h.nu/",
    false, "", "www.xn--alliancefranaise-kta39h.nu", 0, "/", "", "" },
  { false, "http://www.\xd1\x82\xd0\xb5\xd1\x81\xd1\x82.ru",
    "http://www.xn--e1aybc.ru/",
    false, "", "www.xn--e1aybc.ru", 0, "/", "", "" },
  { false, "http://\xd0\xbf\xd1\x80\xd0\xb8\xd0\xbc\xd0\xb5\xd1\x80."
      "\xd0\xb8\xd1\x81\xd0\xbf\xd1\x8b\xd1\x82\xd0\xb0"
      "\xd0\xbd\xd0\xb8\xd0\xb5",
    "http://xn--e1afmkfd.xn--80akhbyknj4f/",
    false, "", "xn--e1afmkfd.xn--80akhbyknj4f", 0, "/", "", "" },
  { false, "\xd9\x85\xd8\xab\xd8\xa7\xd9\x84."
      "\xd8\xa5\xd8\xae\xd8\xaa\xd8\xa8\xd8\xa7\xd8\xb1",
    "http://xn--mgbh0fb.xn--kgbechtv/",
    false, "", "xn--mgbh0fb.xn--kgbechtv", 0, "/", "", "" },
  { false, "//a.com", "http://a.com/", false, "", "a.com", 0, "/", "", "" },
  { false, "//@a.com?#", "http://a.com/",
    false, "", "a.com", 0, "/", "", "" },
};

HTTPAddressTest::HTTPAddressTest() throw ()
  : strict_(false)
{
}

void
HTTPAddressTest::print_url(const HTTPAddress &print_address,
  std::ostream& stream) throw (eh::Exception)
{
  stream << std::endl << "url:" << print_address.url();
  stream << " scheme:" << print_address.scheme();
  stream << " secure:" << print_address.secure();
  stream << " authority:" << print_address.authority();
  stream << " host name:" << print_address.host();
  stream << " port:" << print_address.port_number();
  stream << " path:" << print_address.path();
  stream << " query:" << print_address.query() << std::endl;
}

bool
HTTPAddressTest::compare_url(const HTTPAddress &url1,
  const HTTPAddress &url2) throw (eh::Exception)
{
  return url1.secure() == url2.secure() &&
    url1.port_number() == url2.port_number() &&
    url1.host() == url2.host() && url1.path() == url2.path() &&
    url1.query() == url2.query();
}

int
HTTPAddressTest::test_url(const HTTPAddress& url)
  throw (eh::Exception)
{
  HTTPAddress url_assign1;
  HTTPAddress url_assign2;
  HTTPAddress url_construct1(url);
  HTTPAddress url_construct2(url.host(), url.path(), url.query(),
    url.fragment(), url.port_number(), url.secure(), url.userinfo());
  url_assign1 = url;
  {
    BrowserAddress url_br(url.url());
    url_assign2 = url_br;
  }
  if (!compare_url(url, url_assign1))
  {
    print_url(url, std::cerr);
    print_url(url_assign1, std::cerr);
    return 1;
  }
  if (!compare_url(url, url_assign2))
  {
    print_url(url, std::cerr);
    print_url(url_assign2, std::cerr);
    return 2;
  }
  if (!compare_url(url, url_construct1))
  {
    print_url(url, std::cerr);
    print_url(url_construct1, std::cerr);
    return 3;
  }
  if (!compare_url(url, url_construct2))
  {
    print_url(url, std::cerr);
    print_url(url_construct2, std::cerr);
    return 4;
  }
  HTTPAddress url_empty;
  url_construct1 = url_empty;
  if (!compare_url(url_construct1, url_empty))
  {
    print_url(url, std::cerr);
    print_url(url_empty, std::cerr);
    return 5;
  }
  return 0;
}

void
HTTPAddressTest::print_test_url(const HTTPAddress& url, int test_case)
  throw (eh::Exception)
{
  switch (test_url(url))
  {
  case 0:
    return;
  case 1:
    std::cerr << " operator = has errors.";
    break;
  case 2:
    std::cerr << " copy constructor has errors.";
    break;
  case 4:
    std::cerr << " second constructor has errors.";
    break;
  case 8:
    std::cerr << " operator = has errors.";
    break;
  }
  if (test_case >= 0)
  {
    std::cerr << " test case " << test_case << ".";
  }
  std::cerr << std::endl;
}

template <typename Checker, typename Address>
bool
HTTPAddressTest::interactive_test2008(int argc, char** argv)
 throw (eh::Exception)
{
  if (argc)
  {
    for (int i = 0; i < argc; i++)
    {
      try
      {
        String::SubString test_url(argv[i]);
        std::string error;
        Checker checker;
        if (!checker(String::SubString(test_url), &error))
        {
          std::cerr << "Url: '" << test_url << "': check_url failed: " <<
            error << std::endl;
        }

        Address url(test_url);
        std::cout << std::endl << i << " Url:" << test_url;
        print_url(url, std::cout);
        print_test_url(url);
      }
      catch (const eh::Exception& ex)
      {
        std::cerr << "eh::Exception while processing '" << argv[i] << 
          "': " << ex.what() << std::endl;
      }
    }
    return true;
  }
  return false;
}

template <typename Checker, typename Address>
int
HTTPAddressTest::run_tests(bool strct) throw (eh::Exception)
{
  int ret_value = 0;
  for (size_t i = 0; i < sizeof(test_cases) / sizeof(*test_cases); i++)
  {
    const HTTPAddressInfo& test = test_cases[i];
    for (int strict = strct; strict <= test.strict; strict++)
    {
      try
      {
        std::string error;
        Checker checker;
        if (!checker(String::SubString(test.url), &error))
        {
          std::cerr << " check_url failed: " << error <<
            " test case " << i << "." << std::endl;
        }

        const std::string& normal =
          normalize_http_address(String::SubString(test.url));
        if (strcmp(normal.c_str(), test.normal))
        {
          std::cerr << "Invalid normal form\n'" << normal <<
            "' expected\n'" << test.normal << "'" << std::endl;
        }

        Address url{String::SubString(test.url)};
        print_test_url(url, i);

        {
          Address test_url(
            String::SubString(test.host_name),
            String::SubString(test.path),
            String::SubString(test.query),
            String::SubString(),
            test.port, test.secure);
          if (!compare_url(url, test_url))
          {
            std::cerr << "test case " << i << " failed. Urls:";
            print_url(url, std::cerr);
            print_url(test_url, std::cerr);
            std::cerr << std::endl;
            ret_value++;
          }
        }
      }
      catch (const eh::Exception& e)
      {
        std::cerr << FNT << " eh::Exception caught on test case " << i <<
          ". Description:" << e.what() << std::endl;
        ret_value++;
      }
    }
  }

  return ret_value;
}


int
HTTPAddressTest::main(int argc, char** argv) throw ()
{
  int ret_value = 0;
  try
  {
    int opt;
    while ((opt = getopt(argc, argv, "s")) != -1)
    {
      switch (opt)
      {
        case 's':
        {
          strict_ = true;
          break;
        }
      }
    }

    if (strict_ ?
      !interactive_test2008<HTTP::HTTPChecker, HTTP::HTTPAddress>(
        argc - optind, argv + optind) :
      !interactive_test2008<HTTP::BrowserChecker,
        HTTP::BrowserAddress>(argc - optind, argv + optind))
    {
      ret_value = run_tests<HTTP::HTTPChecker, HTTP::HTTPAddress>(true) +
        run_tests<HTTP::BrowserChecker, HTTP::BrowserAddress>(strict_);
    }
  }
  catch (const eh::Exception& e)
  {
    std::cerr << " eh::Exception caught. Description:" << e.what();
    return 1;
  }
  return ret_value;
}

int
main(int argc, char** argv)
{
  HTTPAddressTest test;
  return test.main(argc, argv);
}
