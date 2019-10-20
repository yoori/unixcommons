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

#include <Generics/Rand.hpp>

#include <CORBACommons/StatsImpl.hpp>

#include <TestCommons/MTTester.hpp>

class Test
{
public:
  Test() throw (eh::Exception, CORBA::Exception);

  void
  test() throw (eh::Exception, CORBA::Exception);


private:
  enum Prefix
  {
    P_LONG,
    P_ULONG,
    P_DOUBLE,
    P_STRING,
    P_LAST
  };

  static const char LETTERS_[P_LAST];

  class PublicAdaptor
  {
  public:
    DECLARE_EXCEPTION(CORBAException, eh::DescriptiveException);

    PublicAdaptor(Test& test) throw ();
    void
    operator ()() throw (eh::Exception, CORBAException);

  private:
    Test& test_;
  };

  std::string
  name_(Prefix prefix, sig_atomic_t index)
    throw (eh::Exception);

  template <typename T>
  void
  set_simple_(Prefix prefix, sig_atomic_t index, T value)
    throw (eh::Exception);

  template <typename T>
  void
  set_(Prefix prefix, T value)
    throw (eh::Exception);

  template <typename T>
  void
  add_(Prefix prefix, sig_atomic_t index, T value)
    throw (eh::Exception);

  template <typename T>
  void
  set_random_(Prefix prefix, T value)
    throw (eh::Exception);

  template <typename T>
  void
  add_random_(Prefix prefix, T value)
    throw (eh::Exception);

  template <typename T>
  void
  add_or_set_random_(Prefix prefix, T value) throw (eh::Exception);

  void
  get_(Prefix prefix)
    throw (eh::Exception);

  template <typename T>
  void
  add_fail_(Prefix prefix, sig_atomic_t index, T value, const char* type)
    throw (eh::Exception);

  template <typename T>
  void
  add_test_(Prefix prefix, sig_atomic_t index, T value, const char* type,
    Prefix value_type) throw (eh::Exception);

  template <typename T>
  void
  test_num_(Prefix prefix, sig_atomic_t index, T value, const char* type)
    throw (eh::Exception);

  void
  func_test_() throw (eh::Exception, CORBA::Exception);

  void
  operator ()() throw (eh::Exception, CORBA::Exception);

  void
  mt_test_() throw (eh::Exception, CORBA::Exception);


  Generics::Values_var stat_, stat2_;
  volatile sig_atomic_t counters_[P_LAST];
};


Test::PublicAdaptor::PublicAdaptor(Test& test) throw ()
  : test_(test)
{
}

void
Test::PublicAdaptor::operator ()() throw (eh::Exception, CORBAException)
{
  try
  {
    test_();
  }
  catch (const CORBA::Exception& ex)
  {
    Stream::Error ostr;
    ostr << "Test::PublicAdaptor::operator ()(): CORBA Exception " << ex;
    throw CORBAException(ostr);
  }
}

const char Test::LETTERS_[P_LAST] = { 'L', 'U', 'D', 'S' };


Test::Test() throw (eh::Exception, CORBA::Exception)
  : stat_(new Generics::Values),
    stat2_(new Generics::Values)
{
  for (int prefix = 0; prefix < P_LAST; prefix++)
  {
    counters_[prefix] = 0;
  }
}

template <typename T>
void
Test::set_simple_(Prefix prefix, sig_atomic_t index, T value)
  throw (eh::Exception)
{
  const std::string& id = name_(prefix, index);
  stat_->set(id.c_str(), value);
}

template <typename T>
void
Test::set_(Prefix prefix, T value)
  throw (eh::Exception)
{
  sig_atomic_t saved = counters_[prefix];
  set_simple_(prefix, saved, value);
  counters_[prefix] = saved + 1;
}

template <typename T>
void
Test::add_(Prefix prefix, sig_atomic_t index, T value)
  throw (eh::Exception)
{
  const std::string& id = name_(prefix, index);
  stat_->add(id.c_str(), value);
}

template <typename T>
void
Test::set_random_(Prefix prefix, T value)
  throw (eh::Exception)
{
  if (!Generics::safe_rand(10))
  {
    set_(prefix, value);
  }
}

template <typename T>
void
Test::add_random_(Prefix prefix, T value)
  throw (eh::Exception)
{
  add_(prefix, Generics::safe_rand(counters_[prefix]), value);
}

template <typename T>
void
Test::add_or_set_random_(Prefix prefix, T value) throw (eh::Exception)
{
  if (Generics::safe_rand(10))
  {
    return;
  }

  stat2_->add_or_set(name_(prefix, Generics::safe_rand(6)), value);
}

template <typename T>
void
Test::add_fail_(Prefix prefix, sig_atomic_t index, T value, const char* type)
  throw (eh::Exception)
{
  try
  {
    add_(prefix, index, value);
    std::cerr << "Failed to fail on adding " << type <<
      " to inexistent key" << std::endl;
  }
  catch (const Generics::Values::KeyNotFound&)
  {
    // Expecting it
  }
}

template <typename T>
void
Test::add_test_(Prefix prefix, sig_atomic_t index, T value,
  const char* type, Prefix value_type) throw (eh::Exception)
{
  try
  {
    add_(prefix, index, value);
    if (prefix != value_type)
    {
      std::cerr << "Failed to fail on adding " << type <<
        " to " << LETTERS_[value_type] << " key" << std::endl;
    }
  }
  catch (const Generics::Values::InvalidType&)
  {
    if (prefix == value_type)
    {
      throw;
    }
  }
}

template <typename T>
void
Test::test_num_(Prefix prefix, sig_atomic_t index, T value, const char* type)
  throw (eh::Exception)
{
  const std::string& id = name_(prefix, index);
  std::unique_ptr<CORBA::Any> any(
    CORBACommons::ValuesConverter::get_any(*stat_, id.c_str()));
  T result;
  (*any) >>= result;
  if (fabs(result - value) > 0.1)
  {
    std::cerr << "Invalid " << type << " result " << result <<
      " expected " << value << std::endl;
  }
}

void
Test::get_(Prefix prefix)
  throw (eh::Exception)
{
  for (sig_atomic_t index = 0; index < counters_[prefix]; index++)
  {
    const std::string& id = name_(prefix, index);
    CORBA::Any_var any(
      CORBACommons::ValuesConverter::get_any(*stat_, id.c_str()));
  }
}

void
Test::test() throw (eh::Exception, CORBA::Exception)
{
  func_test_();
  mt_test_();
}

void
Test::func_test_() throw (eh::Exception, CORBA::Exception)
{
  for (int prefix = 0; prefix < P_LAST; prefix++)
  {
    Prefix pref = static_cast<Prefix>(prefix);
    set_simple_(pref, 0, 1lu);
    set_simple_(pref, 0, 1l);
    set_simple_(pref, 0, 1.0);
    set_simple_(pref, 0, "1");
  }

  for (int prefix = 0; prefix < P_LAST; prefix++)
  {
    Prefix pref = static_cast<Prefix>(prefix);
    add_fail_(pref, 1, 1lu, "unsigned long");
    add_fail_(pref, 1, 1l, "long");
    add_fail_(pref, 1, 1.0, "double");
    add_fail_(pref, 1, "1", "string");
  }

  set_simple_(P_LONG, 0, 0l);
  set_simple_(P_ULONG, 0, 0lu);
  set_simple_(P_DOUBLE, 0, 0.0);
  set_simple_(P_STRING, 0, "0");

  for (int prefix = 0; prefix < P_LAST; prefix++)
  {
    Prefix pref = static_cast<Prefix>(prefix);
    add_test_(pref, 0, 1l, "long", P_LONG);
    add_test_(pref, 0, 2lu, "unsigned long", P_ULONG);
    add_test_(pref, 0, 4.0, "double", P_DOUBLE);
    add_test_(pref, 0, "8", "string", P_STRING);
  }

  test_num_(P_LONG, 0, 1l, "long");
  test_num_(P_ULONG, 0, 2ul, "unsigned long");
  test_num_(P_DOUBLE, 0, 4.0, "double");
  {
    CORBA::Any_var any(
      CORBACommons::ValuesConverter::get_any(*stat_,
        name_(P_STRING, 0).c_str()));
    const CORBA::Char* result = 0;
    any >>= result;
    if (!result || strcmp(result, "08"))
    {
      std::cerr << "Invalid string result " << result <<
        " expected 01" << std::endl;
    }
  }
}

void
Test::mt_test_() throw (eh::Exception, CORBA::Exception)
{
  set_(P_LONG, 0l);
  set_(P_ULONG, 0lu);
  set_(P_DOUBLE, 0.0);
  set_(P_STRING, "");

  PublicAdaptor adaptor(*this);
  TestCommons::MTTester<PublicAdaptor&> test(adaptor, 10);
  test.run(20, 5);

  for (int prefix = 0; prefix < P_LAST; prefix++)
  {
    get_(static_cast<Prefix>(prefix));
  }

  CORBACommons::StatsValueSeq_var stats(
    CORBACommons::ValuesConverter::get_stats(*stat_));
}

std::string
Test::name_(Prefix prefix, sig_atomic_t index)
  throw (eh::Exception)
{
  char buf[32];
  snprintf(buf, sizeof(buf), "%c%lu", LETTERS_[prefix],
    static_cast<unsigned long>(index));
  return buf;
}

void
Test::operator ()() throw (eh::Exception, CORBA::Exception)
{
  set_random_(P_LONG, 0l);
  set_random_(P_ULONG, 0lu);
  set_random_(P_DOUBLE, 0.0);
  set_random_(P_STRING, "");

  add_random_(P_LONG, 1l);
  add_random_(P_ULONG, 1lu);
  add_random_(P_DOUBLE, 1.0);
  add_random_(P_STRING, "a");

  add_or_set_random_(P_LONG, 1l);
  add_or_set_random_(P_ULONG, 1lu);
  add_or_set_random_(P_DOUBLE, 1.0);
  add_or_set_random_(P_STRING, "a");
}

int
main()
{
  try
  {
    Test test;
    test.test();
    return 0;
  }
  catch (const CORBA::Exception& ex)
  {
    std::cerr << "CORBA::Exception caught: " << ex << std::endl;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "eh::Exception caught: " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception caught" << std::endl;
  }

  return -1;
}
