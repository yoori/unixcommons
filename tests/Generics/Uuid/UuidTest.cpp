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



// @file uuid_test.cpp
// Generics::uuid test
// First stage check uuids 
// 1. start some threads
// 2. generate in every thread N=20 uuids.
// 3. dump uuids into general place - all_uids
// 4. sort all_uids
// 5. compare neighbours, if equal throw exception.
// second stage: check base64 method
// check 00000000-0000-0000-0000-000000000000 encoding.
//

#include <algorithm>
#include <fstream>
#include <vector>
#include <tr1/array>
#include <Generics/Uuid.hpp>
#include <TestCommons/MTTester.hpp>

DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

struct UuidGenerator
{
  void 
  operator()() throw (eh::Exception);

  void
  check() throw (eh::Exception);
private:
  typedef Sync::PosixMutex Mutex_;
  typedef Sync::PosixGuard Guard_;
  typedef std::vector<Generics::Uuid> AllUidsType;

  AllUidsType  all_uids;
  Mutex_ mutex_;
};

void
uuid_test() throw (eh::Exception)
{
  try
  {
    std::cout << "Uuid generation test started.." << std::endl;

    UuidGenerator uuids;
    TestCommons::MTTester<UuidGenerator&> mt_tester(
      uuids, 10);

    mt_tester.run(1000, 0, 1000);
    uuids.check();

    const uint8_t data[] =
    {
      0x40, 0x40, 0x40, 0x40,
      0x50, 0x50, 0x60, 0x60,
      0x60, 0x60, 0x60, 0x60,
      0x60, 0x60, 0x60, 0x60
    };
    std::cout << "test base64 encoding: ";
    Generics::Uuid u(data, data + 16);

    std::string s = u.to_string();
    // .. at end is company specific, by RFC properly ==
    const std::string ETHALON("QEBAQFBQYGBgYGBgYGBgYA..");
    if (s == ETHALON)
    {
      std::cout << "succeeded." << std::endl;
    }
    else
    {
      throw Exception((s + " base64 encoding failed").c_str());
    }

#if 0
    std::cout << "Testing output formats:\n"
    << "default output " << u << std::endl
    << Generics::Uuid::ascii
    << "default ascii output " << u << std::endl
    << "With braces = "
    << Generics::Uuid::showbraces << u << std::endl
    << "and dashes " << Generics::Uuid::showdashes << u
    << std::endl
    << "without braces = " << Generics::Uuid::noshowbraces << u
    << std::endl
    << "and dashes " << Generics::Uuid::noshowdashes << u << std::endl
    << "standard base64 encoding put into stream "
    << Generics::Uuid::base64
    << u << std::endl
    << "right string " << ETHALON << std::endl
    << "Testing input formats:\n";

    char* ev = getenv("TEST_SRC_DIR");
    std::string test_file(ev ? ev : "tests/Generics/Uuid");
    test_file += "/Data/uuids.dat";

    std::ifstream ufs(test_file.c_str(), std::ios_base::in);
    Generics::Uuid uf;
    bool all_done = false;
    if (ufs.good())
    {
      ufs >> Generics::Uuid::ascii;
      ufs >> uf;
      std::cout << "Read uuid1 = " << Generics::Uuid::ascii << uf
        << std::endl;
      ufs >> uf;
      std::cout << "Read uuid2 = " << uf << std::endl;
      ufs >> Generics::Uuid::base64;
      Generics::Uuid uf_nill, uff;
      ufs >> uff;
      if (ufs)
      {
        std::cout << "Read uuid3 = " << uff << std::endl;
        if (!(uf_nill == uff))
        {
          all_done = true;
        }
      }
    }
    if (!all_done)
    {
      throw Exception("Weren't complete all input stream tests");
    }
#endif

    std::cout << "SUCCESS" << std::endl;
  }
  catch (eh::Exception& e)
  {
    std::cerr << "\nFAIL: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "\nFAIL: unknown exception" << std::endl;
    throw;
  }
}

class SignedUuidTest
{
public:
  SignedUuidTest(const char* pr, const char* pu)
    throw (eh::Exception);

  void
  operator ()() const throw (eh::Exception);

private:
  Generics::SignedUuidGenerator gen_;
  Generics::SignedUuidVerifier ver_;
};

SignedUuidTest::SignedUuidTest(const char* pr, const char* pu)
  throw (eh::Exception)
  : gen_(pr), ver_(pu)
{
}

void
SignedUuidTest::operator ()() const throw (eh::Exception)
{
  for (int i = 0; i < 10000; i++)
  {
    Generics::SignedUuid u1(gen_.generate());
    Generics::SignedUuid u2(ver_.verify(u1.str()));
    if (!(u1.uuid() == u2.uuid()))
    {
      std::cerr << "FAIL: generated '" << u1.str() << "' and verified '" <<
        u2.str() << "' uuids are not the same\n";
    }
    Generics::SignedUuidProbe p(u1.uuid());
    Generics::SignedUuid u3(p.construct());
    if (u3.uuid() != u2.uuid())
    {
      std::cerr << "FAIL: probe '" << u3.str() << "' is not '" <<
        u2.str() << "'\n";
    }
  }
}

void
signed_uuid_test() throw (eh::Exception)
{
  ERR_load_crypto_strings();
  const char* root = getenv("TEST_TOP_SRC_DIR");
  std::string rootstr(root ? root : root = ".");
  std::string pr(rootstr + "/tests/Data/pr.der");
  std::string pu(rootstr + "/tests/Data/pu.der");

  try
  {
    Generics::SignedUuidGenerator gen(pr.c_str());
    Generics::SignedUuidVerifier ver(pu.c_str());

    Generics::SignedUuid u1 = gen.generate();
    std::string sign = u1.str();
    std::cout << "Generated signed uuid '" << sign << "'\n";
    Generics::SignedUuid u2 = ver.verify(sign);
    std::cout << "Verified  signed uuid '" << u2.str() << "'\n";
    if (!(u1.uuid() == u2.uuid()))
    {
      std::cerr << "Verified uuid is not the same\n";
    }
    if (u1.str() != u2.str())
    {
      std::cerr << "Strings for uuids are not the same\n";
    }
    char& signch = sign[Generics::Uuid::encoded_size() + 3];
    signch = signch == 'A' ? 'B' : 'A';
    try
    {
      ver.verify(sign);
      std::cerr << "FAIL: Verified changed signature\n";
    }
    catch (const Generics::SignedUuidVerifier::Exception& ex)
    {
    }
    signch = '?';
    try
    {
      ver.verify(sign);
      std::cerr << "FAIL: Verified unencodable signature\n";
    }
    catch (const Generics::SignedUuidVerifier::Exception& ex)
    {
    }
    Generics::SignedUuidProbe prb(u1.uuid());
    Generics::SignedUuid u3 = prb.construct();
    std::cout << "Unsigned  signed uuid '" << u3.str() << "'\n";
    if (u1.uuid() != u2.uuid())
    {
      std::cerr << "Strings for uuids are not the same\n";
    }
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "FAIL: " << ex.what() << std::endl;
  }

  SignedUuidTest test(pr.c_str(), pu.c_str());
  TestCommons::MTTester<SignedUuidTest&> tester(test, 10);
  tester.run(10, 0, 10);
}

//
// Test body below
//

int
main()
{
  try
  {
    uuid_test();
    signed_uuid_test();
    return 0;
  }
  catch (...)
  {
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////
// Implementations

void 
UuidGenerator::operator()() throw (eh::Exception)
{
  typedef std::tr1::array<Generics::Uuid, 25> container;
  std::unique_ptr<container> armada(new container);
  for (container::iterator it = armada->begin(); it != armada->end(); ++it)
    *it = Generics::Uuid::create_random_based();
  Guard_ lock(mutex_);
  all_uids.insert(all_uids.begin(), armada->begin(), armada->end());
}

void
UuidGenerator::check() throw (eh::Exception)
{
  std::sort(all_uids.begin(), all_uids.end());
  std::cout << "Unique check: ";
  AllUidsType::const_iterator it = all_uids.begin();
  if (it != all_uids.end())
  {
    ++it;
  }
  for (AllUidsType::const_iterator pit = all_uids.begin();
    it != all_uids.end(); ++it, ++pit )
  {
    if (*it == *pit)
    {
      Stream::Error ostr;
      ostr << "uuid duplication. We must improve generation algorithm."
        << std::endl
        << "N1=" << pit-all_uids.begin() << " uuid1=" << *pit
        << std::endl
        << "N2=" << it-all_uids.begin() << " uuid1=" << *it;
      throw Exception(ostr);
    }
  }
  std::cout << "succeeded." << std::endl;
}
