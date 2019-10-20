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



// @file Client/ObjectPoolTestClient.cpp

#include <iostream>

#include <Generics/AppUtils.hpp>

#include <Logger/StreamLogger.hpp>

#include <CORBACommons/ObjectPool.hpp>
#include <CORBACommons/CorbaAdapters.hpp>
#include <CORBAConfigParser/ParameterConfig.hpp>

#include <TestCommons/MTTester.hpp>

#include "../Server/Simple.hpp"


using namespace CORBACommons;
typedef CORBAConfigParser::CorbaRefOption<
  CORBATest::TestObjectPool> RefOption;

typedef std::vector<std::string> Urls;

/**
 * class with tests
 */
template <typename PoolType>
class OPTester
{
public:
  typedef PoolType Pool;
  typedef typename PoolType::ConfigType::RefAndNumber RefAndNumber;

  /**
   * Constructor on Objects
   */
  OPTester(
    std::unique_ptr<RefOption>* obj_refs,
    std::size_t corba_objects_count,
    const typename PoolType::ConfigType& config_base =
      typename PoolType::ConfigType()) throw (eh::Exception);

  /**
   * Constructor on References
   */
  OPTester(
    std::unique_ptr<RefOption>* obj_refs,
    std::size_t corba_objects_count,
    const Urls& urls,
    const typename PoolType::ConfigType& config_base =
      typename PoolType::ConfigType()) throw (eh::Exception);


  /**
   * check round robin algorithm.
   */
  void
  test_demultiplex() throw (eh::Exception);

  /**
   * check random selection objects from pool
   */
  void
  test_random() throw (eh::Exception);

  /**
   * Check PT_BAD_SWITCH policy - object switch only if
   * it was bad at time of get_object call
   */
  void
  test_bad_switch() throw (eh::Exception);

  /**
   * check unfrozen after timeout and inaccessible.
   */
  void
  test_invalidate() throw (eh::Exception);

  /**
   * behavior in case of full refusal
   */
  void
  test_all_bad() throw (eh::Exception);

  void
  test() throw (eh::Exception);

  /**
   * Test round robin algorithm
   * @param pool pool to call objects
   * @param first_stage determine standard results for pool working
   */
  static void
  test_not_exist_yet(Pool& pool,
    typename Pool::ConfigType& config, bool first_stage);

  /**
   * Perform MT_TEST_REPETITIONS calls to pool from some threads, and
   * check distribution of results.
   * @param pool Pool that return objects for test actions
   * @param loop_policy Indicates that the pool uses a loop
   * strategy
   */
  static void
  multithread_test(typename Pool::ConfigType& config,
    ChoosePolicyType::POLICY_TYPE policy_type)
    throw (eh::Exception);

  static const std::size_t MT_TEST_REPETITIONS = 1000;

private:

  typename PoolType::ConfigType config_base_;
  typedef typename PoolType::ConfigType::References Refs;
  Refs ref_;
  typedef std::vector<CORBATest::TestObjectPool_var> Objects;
  Objects ob_;

  /**
   * elementary pool request
   */
  static bool
  pool_iteration_(Pool& pool, bool ignore_ir = false,
    unsigned key = Pool::SPECIAL_KEY) throw (eh::Exception);

  struct PoolIterator
  {
    PoolIterator(Pool& pool_ref) throw ();

    void
    operator ()() throw ();
  private:
    Pool& pool_;
  };

};

template <typename PoolType>
OPTester<PoolType>::OPTester(
  std::unique_ptr<RefOption>* obj_refs,
  std::size_t corba_objects_count,
  const typename PoolType::ConfigType& config_base_val)
  throw (eh::Exception)
  : config_base_(config_base_val)
{
  for (std::size_t i = 0; i < corba_objects_count; ++i)
  {
    ref_.push_back(typename PoolType::ConfigType::RefAndNumber(
      **(obj_refs[i])));
    ob_.push_back(CORBATest::TestObjectPool::_duplicate(**obj_refs[i]));
  }
}

template <typename PoolType>
OPTester<PoolType>::OPTester(
  std::unique_ptr<RefOption>* obj_refs,
  std::size_t corba_objects_count,
  const Urls& urls,
  const typename PoolType::ConfigType& config_base_val)
  throw (eh::Exception)
  : config_base_(config_base_val)
{
  for (std::size_t i = 0; i < corba_objects_count; ++i)
  {
    ref_.push_back(CORBACommons::CorbaObjectRef(urls[i].c_str()));
    ob_.push_back(CORBATest::TestObjectPool::_duplicate(**obj_refs[i]));
  }
}

template <typename PoolType>
OPTester<PoolType>::PoolIterator::PoolIterator(Pool& pool_ref)
  throw ()
  : pool_(pool_ref)
{
}

template <typename PoolType>
void
OPTester<PoolType>::PoolIterator::operator ()() throw ()
{
  unsigned key = Generics::safe_rand(30);
  pool_iteration_(pool_, false, key > 20 ? Pool::SPECIAL_KEY : key);
}


template <typename PoolType>
bool
OPTester<PoolType>::pool_iteration_(Pool& pool, bool ignore_ir,
  unsigned key) throw (eh::Exception)
{
  try
  {
    typename Pool::ObjectHandlerType cs = pool.get_object(key);

    try
    {
      int sq = (*cs).square(11);
      if (sq != 121)
      {
        std::cerr << FNS << "11 * 11 != " << sq << std::endl;
        return false;
      }
      cs.release();
    }
    catch (const CORBA::SystemException& e)
    {
      cs.release_bad();
      std::cerr << FNS << e << std::endl;
    }
  }
  catch (const typename Pool::InvalidReference& ex)
  {
    if (!ignore_ir)
    {
      std::cerr << FNS << ex.what() << std::endl;
    }
  }
  catch (const eh::Exception& e)
  {
    std::cerr << FNS << e.what() << std::endl;
  }
  return true;
}

template <typename PoolType>
void
OPTester<PoolType>::test_demultiplex() throw (eh::Exception)
{
  typename PoolType::ConfigType configuration(config_base_);
  configuration.iors_list = ref_;

  Pool pool(configuration);

  for (std::size_t i = 0; i < 6 * ref_.size() && pool_iteration_(pool); ++i)
  {
  }
  typename Objects::iterator it(ob_.begin());
  for (; it != ob_.end(); ++it)
  {
    std::size_t ob_load = (*it)->get_calling_number();
    if (ob_load != 6)
    {
      std::cerr << FNS << "Fail: each object should be called 6 times at "
        "round robin selection, but actually number for some object " <<
        ob_load << std::endl;
      break;
    }
  }
  if (it == ob_.end())
  {
    std::cout << "Round robin selection is working" << std::endl;
  }
}

template <typename PoolType>
void
OPTester<PoolType>::test_random() throw (eh::Exception)
{
  typename PoolType::ConfigType configuration(config_base_);
  configuration.iors_list = ref_;

  Pool pool(configuration, ChoosePolicyType::PT_RAND);

  const std::size_t ITERATIONS = 100;
  for (std::size_t i = 0; i < ITERATIONS && pool_iteration_(pool); ++i)
  {
  }

  std::cout << "Perform " << ITERATIONS <<
    " calls to CORBA objects on server, with RANDOM object selection"
    "\nWork distribution by objects:" << std::endl;

  typename Objects::iterator it(ob_.begin());
  std::size_t i = 0;
  for (; i < ob_.size(); ++i)
  {
    std::size_t ob_load = (ob_[i])->get_calling_number();
    if (i < 10)
    {
      std::cout << i << "=" << ob_load << std::endl;
    }
    if (i == 10)
    {
      std::cout << "And so on..." << std::endl;
    }
    if (!ob_load)
    {
      std::cerr << FNS << "Load is not a random distribution, "
        "with high level of confidence" << std::endl;
      break;
    }
  }
  if (i == ob_.size())
  {
    std::cout << "Random selection is working" << std::endl;
  }
}

template <typename PoolType>
void
OPTester<PoolType>::test_bad_switch() throw (eh::Exception)
{
  if (ref_.size() < 2)
  {
    std::cerr << FNS << "cannot test, not enough objects into pool" <<
      std::endl;
    return;
  }
  typename PoolType::ConfigType configuration(config_base_);
  configuration.iors_list = ref_;
  // timeout make object bad during test execution
  //configuration.timeout = 1;

  Pool pool(configuration, ChoosePolicyType::PT_BAD_SWITCH);

  const std::size_t ITERATIONS = ob_.size() * 3;
  for (std::size_t i = 0; i < ITERATIONS && pool_iteration_(pool); ++i)
  {
  }
  std::size_t ob_load = ob_[0]->get_calling_number();
  std::size_t ob_rest_load = 0;

  typename Objects::iterator it(ob_.begin());
  for (++it; it != ob_.end(); ++it)
  {
    ob_rest_load &= (*it)->get_calling_number();
  }
  if (ob_load != ITERATIONS || ob_rest_load)
  {
    std::cerr << FNS << "Switch performed on good object, but should only "
      "if bad" << std::endl;
  }

  for (std::size_t j = 0; j < 3; ++j)
  {
    for (std::size_t i = 0;
      i < ob_.size() && pool_iteration_(pool);
      ++i)
    {
    }
    typename Pool::ObjectHandlerType cs = pool.get_object();
    cs.release_bad();
  }

  std::cout << "Perform " << ITERATIONS <<
    " calls to CORBA objects on server, "
    "with PT_BAD_SWITCH object selection\n"
    "Calls distribution by objects (with 3 modeling switches):" <<
    std::endl;

  std::size_t i = 0;
  for (; i < ob_.size(); ++i)
  {
    std::size_t ob_load = ob_[i]->get_calling_number();
    if (i < 10)
    {
      std::cout << i << "=" << ob_load << std::endl;
    }
    if (i == 10)
    {
      std::cout << "And so on.." << std::endl;
    }
    if (ob_load != ITERATIONS / 3 && ob_load != 0)
    {
      break;
    }
  }
  if (i < ob_.size())
  {
    std::cerr << FNS << "Switch was not perform on bad object, "
      "but should" << std::endl;
  }
}

template <typename PoolType>
void
OPTester<PoolType>::test_invalidate() throw (eh::Exception)
{
  if (ref_.size() < 2)
  {
    std::cerr << FNS << "cannot test, not enough objects into pool" <<
      std::endl;
    return;
  }
  typename PoolType::ConfigType configuration(config_base_);
  configuration.timeout = Generics::Time::ONE_SECOND;
  configuration.iors_list = ref_;

  Pool pool(configuration);

  typename Pool::ObjectHandlerType cs = pool.get_object();
  cs.release_bad();

  for (std::size_t i = 0; i < ob_.size() && pool_iteration_(pool); ++i)
  {
  }

  bool success = false;
  for (typename Objects::iterator it(ob_.begin()); it != ob_.end(); ++it)
  {
    std::size_t ob_load = (*it)->get_calling_number();
    if (!success && !ob_load)
    {
      std::cout << "Negative unfrozen test is working" << std::endl;
      success = true;
    }
  }
  if (!success)
  {
    std::cerr << FNS << "Fail: object cannot frozen" << std::endl;
  }

  sleep(1);
  for (std::size_t i = 0; i < ob_.size() && pool_iteration_(pool); ++i)
  {
  }
  typename Objects::iterator it(ob_.begin());
  for (; it != ob_.end(); ++it)
  {
    std::size_t ob_load = (*it)->get_calling_number();
    if (ob_load != 1)
    {
      std::cerr << FNS << "Fail: object cannot unfrozen, current load=" <<
        ob_load << std::endl;
      break;
    }
  }
  if (it == ob_.end())
  {
    std::cout << "Positive unfrozen test is working" << std::endl;
  }
}

template <typename PoolType>
void
OPTester<PoolType>::test_all_bad() throw (eh::Exception)
{
  typename PoolType::ConfigType configuration(config_base_);
  configuration.timeout = Generics::Time::ONE_SECOND;
  configuration.iors_list = ref_;

  Pool pool(configuration);

  for (std::size_t i = 0; i < ref_.size(); ++i)
  {
    typename Pool::ObjectHandlerType cs(pool.get_object());
    cs.release_bad();
  }

  try
  {
    typename Pool::ObjectHandlerType cs(pool.get_object());
    std::cerr << FNS << "Fail: Exception \"All references are bad\""
      " is NOT raised." << std::endl;
  }
  catch (const typename Pool::NoGoodReference& e)
  {
    std::cout << "Exception NoGoodReference caught" << std::endl;
  }

  sleep(1);
  for (std::size_t i = 0; i < ob_.size() && pool_iteration_(pool); ++i)
  {
  }

  typename Objects::iterator it(ob_.begin());
  for (; it != ob_.end(); ++it)
  {
    std::size_t ob_load = (*it)->get_calling_number();
    if (ob_load != 1)
    {
      std::cerr << FNS << "Fail: object cannot unfrozen" << std::endl;
      break;
    }
  }
  if (it == ob_.end())
  {
    std::cout << "All references are bad - is working" << std::endl;
  }
}

template <typename PoolType>
void
OPTester<PoolType>::test() throw (eh::Exception)
{
  test_demultiplex();
  test_random();
  test_bad_switch();
  test_invalidate();
  test_all_bad();
}

template <typename PoolType>
void
OPTester<PoolType>::test_not_exist_yet(Pool& pool,
  typename Pool::ConfigType& config, bool first_stage)
{
  for (std::size_t i = 0;
    i < (config.iors_list.size() - 1) * config.iors_list.size() &&
      pool_iteration_(pool, first_stage);
    ++i)
  {
  }

  // Iterate all objects in pool, it must give us N non-zero numbers.
  // It will mean that pool operate 4 unique objects on server side
  ::CORBA::Long await = config.iors_list.size() - 1;
  bool error = false;
  for (std::size_t i = 0; i < config.iors_list.size(); ++i)
  {
    try
    {
      typename Pool::ObjectHandlerType cs = pool.get_object();
      ::CORBA::Long use_count = (*cs).get_calling_number();
      if (use_count != await)
      {
        std::cerr << FNS << (first_stage ? "First" : "Second") <<
          " stage of reference on non-existing object test failed: "
          "awaiting " << await << " use pooled objects, but " <<
          use_count << " use in fact." << std::endl;
        error = true;
      }
    }
    catch (const typename Pool::InvalidReference& ex)
    {
      if (!first_stage)
      {
        std::cerr << FNS << ex.what() << std::endl;
      }
    }
  }
  if (error)
  {
    return;
  }
  std::cout << (first_stage ? "First" : "Second") <<
    " stage of reference on non-existing object test is working" <<
    std::endl;
  if (!first_stage)
  {
    std::cout << "UpOnline object successfully used into pool" << std::endl;
  }
}

template <typename PoolType>
void
OPTester<PoolType>::multithread_test(typename Pool::ConfigType& config,
  ChoosePolicyType::POLICY_TYPE policy_type)
  throw (eh::Exception)
{
  Pool pool(config, policy_type);
  PoolIterator pool_iterator(pool);
  TestCommons::MTTester<PoolIterator&> mt_tester(
    pool_iterator, 10);

  mt_tester.run(MT_TEST_REPETITIONS, 0, MT_TEST_REPETITIONS);
}

/**
 * Check compatibility of Base Object Configuration with
 * Derived Objects Pool
 */
void
check_narrow(const CorbaClientAdapter_var& corba_client_adapter,
  const char* pool_obj_url)
  throw (eh::Exception)
{
  typedef CORBATest::PoolObject PoolObject;
  typedef CORBATest::Base ConfigObject;

  typedef CORBACommons::ObjectPool<PoolObject,
    ObjectPoolConfiguration<ConfigObject> > Pool;
  typedef CORBACommons::ObjectPool<ConfigObject,
    ObjectPoolConfiguration<ConfigObject> > BasePool;
  typedef Pool::ConfigType::RefAndNumber RefAndNumber;

  typedef CORBAConfigParser::CorbaRefOption<
    CORBATest::Base> BaseRefOption;

  Pool::ConfigType base_obj_config;

  BaseRefOption pool_obj_opt(corba_client_adapter.in());
  pool_obj_opt.set("", pool_obj_url);

  base_obj_config.iors_list.push_back(
    RefAndNumber(*pool_obj_opt, 5));
  Pool pool(base_obj_config);
  BasePool base_pool(base_obj_config);

  Pool::ObjectHandlerType cs = pool.get_object();
  BasePool::ObjectHandlerType obj = base_pool.get_object();

  try
  {
    ::CORBA::Long test_value = (*cs).is_base();
    if (test_value != 12345)
    {
      std::cerr << FNS << "Pool of Derived - Config on Base, "
        "cannot call member of Derived" << std::endl;
      return;
    }
    ::CORBA::Long base_test_value = (*obj).is_base();
    if (base_test_value != 12345)
    {
      std::cerr << FNS << "cannot call member of Derived" << std::endl;
      return;
    }
    cs.release();
    obj.release();
  }
  catch (const CORBA::SystemException& e)
  {
    cs.release_bad();
    obj.release_bad();
    std::cerr << FNS << e << std::endl;
  }
}

void
check_no_good_reference(
  const CorbaClientAdapter_var& corba_client_adapter,
  const char* url)
  throw ()
{
  typedef CORBACommons::ObjectPool<CORBATest::PoolObject,
    ObjectPoolRefConfiguration> Pool;
  Pool::ConfigType config(corba_client_adapter.in());
  typedef Pool::ConfigType::RefAndNumber RefAndNumber;
  const std::size_t OBJECTS_COUNT = 20;

  for (std::size_t i = 0; i < OBJECTS_COUNT; ++i)
  {
    config.iors_list.push_back(RefAndNumber(
      CORBACommons::CorbaObjectRef(url), 5));
  }
  config.timeout = Generics::Time(10);
  Pool pool(config);
  std::size_t i = 0;
  try
  {
    for (i = 0; i < OBJECTS_COUNT + 1; ++i)
    {
      Pool::ObjectHandlerType cs = pool.get_object();
      cs.release_bad(i < OBJECTS_COUNT / 2 ?
        String::SubString("Test bad release") : String::SubString(""));
    }
  }
  catch (const Pool::NoGoodReference& ex)
  {
    (i == OBJECTS_COUNT ? std::cout : std::cerr << "FAIL:") <<
      FNS << ex.what() << std::endl;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << FNS << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << FNS << "unknown" << std::endl;
  }
}

void
get_urls(Urls& urls) throw (eh::Exception)
{
  std::ifstream ifs("./urls.txt");
  while (ifs.good())
  {
    std::string line;
    std::getline(ifs, line);
    if (line.empty())
    {
      continue;
    }
    urls.push_back(line);
    std::cout << "Read: " << line << std::endl;
  }
}

template <typename PoolTester>
void
up_object_test(
  typename PoolTester::Pool::ConfigType& configuration,
  std::unique_ptr<RefOption> obj_refs[],
  const std::size_t CORBA_OBJECTS_COUNT,
  const Urls& urls)
{
  try
  {
    obj_refs[CORBA_OBJECTS_COUNT - 1]->set("",
      urls[CORBA_OBJECTS_COUNT - 1].c_str());
    (**obj_refs[CORBA_OBJECTS_COUNT - 1])->square(12);
    std::cerr << "FAIL: object exist on server, "
      "Non-exist test cannot be done."  << std::endl;
  }
  catch (const eh::Exception& e)
  {
  }

  Urls::const_iterator cit(urls.begin());
  configuration.iors_list.push_back(typename PoolTester::RefAndNumber(
    CORBACommons::CorbaObjectRef(cit->c_str())));
  for (++cit; cit != urls.end(); ++cit)
  {
    configuration.iors_list.push_back(typename PoolTester::RefAndNumber(
      CORBACommons::CorbaObjectRef(cit->c_str())));
  }

  typename PoolTester::Pool pool(configuration);
  std::cout << "Created pool with non-existing reference to UpOnline" <<
    std::endl;

  // Check first stage of not exist test
  PoolTester::test_not_exist_yet(pool, configuration, true);

  std::cout << "Upgrade server to support UpOnline object" << std::endl;
  (**obj_refs[0])->up();
  sleep(1);
  std::cout << "Sleep complete" << std::endl;
  std::cout << "Check pool properties" << std::endl;
  {
    // Remote orb is closed, all connections are invalid.
    try
    {
      (**obj_refs[0])->get_calling_number();
    }
    catch (const CORBA::COMM_FAILURE&)
    {
    }
  }
  PoolTester::test_not_exist_yet(pool, configuration, false);
}

template <typename PoolTester>
void
switch_policy_test(typename PoolTester::Pool::ConfigType& configuration)
{
  std::size_t general_count = 0;
  const std::size_t POOLED_OBJECTS = configuration.iors_list.size();
  Generics::ArrayAutoPtr<typename PoolTester::Pool::ObjectRef>
    objects(POOLED_OBJECTS);
  auto it = configuration.iors_list.begin();
  for (std::size_t i = 0; i < POOLED_OBJECTS; ++it, i++)
  {
    objects[i] = configuration.resolver.
      template resolve<typename PoolTester::Pool::Object>(it->ior);
    std::size_t use_count = objects[i]->get_calling_number();
    if (use_count)
    {
      std::cerr << "Object " << i << ": use count " << use_count <<
        ", but not zero" << std::endl;
    }
  }

  {
    std::cout << "PT_BAD_SWITCH policy:" << std::endl;
    PoolTester::multithread_test(configuration,
      ChoosePolicyType::PT_BAD_SWITCH);

    bool only_one = false;
    for (std::size_t i = 0; i < POOLED_OBJECTS; ++i)
    {
      std::size_t use_count = objects[i]->get_calling_number();
      general_count += use_count;
      std::cout << "Object " << i << " used " << use_count <<
        " times." << std::endl;
      if (only_one && use_count != 0)
      {
        std::cerr << "Switched from good object or object failure" <<
          std::endl;
      }
      if (use_count != 0)
      {
        if (use_count != PoolTester::MT_TEST_REPETITIONS)
        {
          std::cerr << "Not enough calls to object" << std::endl;
        }
        only_one = true;
      }
    }
  }

  {
    std::cout << "PT_LOOP policy:" << std::endl;
    PoolTester::multithread_test(configuration,
      ChoosePolicyType::PT_LOOP);

    for (std::size_t i = 0; i < POOLED_OBJECTS; ++i)
    {
      ::CORBA::Long use_count = objects[i]->get_calling_number();
      general_count += use_count;
      std::cout << "Object " << i << " used " << use_count <<
        " times." << std::endl;
      if (use_count == 0)
      {
        std::cerr << FNS << "Unused objects in pool" << std::endl;
      }
    }
  }

  {
    std::cout << "PT_RAND policy:" << std::endl;
    PoolTester::multithread_test(configuration,
      ChoosePolicyType::PT_RAND);

    for (std::size_t i = 0; i < POOLED_OBJECTS; ++i)
    {
      ::CORBA::Long use_count = objects[i]->get_calling_number();
      general_count += use_count;
      std::cout << "Object " << i << " used " << use_count <<
        " times." << std::endl;
    }
  }

  {
    std::cout << "PT_PERSISTENT policy:" << std::endl;
    PoolTester::multithread_test(configuration,
      ChoosePolicyType::PT_PERSISTENT);

    for (std::size_t i = 0; i < POOLED_OBJECTS; ++i)
    {
      ::CORBA::Long use_count = objects[i]->get_calling_number();
      general_count += use_count;
      std::cout << "Object " << i << " used " << use_count <<
        " times." << std::endl;
    }
  }

  {
    std::cout << "PT_PRECISE policy:" << std::endl;
    PoolTester::multithread_test(configuration,
      ChoosePolicyType::PT_PRECISE);

    for (std::size_t i = 0; i < POOLED_OBJECTS; ++i)
    {
      ::CORBA::Long use_count = objects[i]->get_calling_number();
      general_count += use_count;
      std::cout << "Object " << i << " used " << use_count <<
        " times." << std::endl;
    }
  }

  if (general_count != 5 * PoolTester::MT_TEST_REPETITIONS)
  {
    std::cerr << "Not " << 5 * PoolTester::MT_TEST_REPETITIONS <<
      " tasks calculated, but " << general_count << std::endl;
  }
}

int
main(int argc, char** argv) throw ()
{
  try
  {
    Logging::FLogger_var logger(
      new Logging::OStream::Logger(Logging::OStream::Config(std::cout)));

    CORBACommons::CorbaClientAdapter_var corba_client_adapter(
      new CORBACommons::CorbaClientAdapter(logger.in()));

    Generics::AppUtils::Option<std::string> pool_obj_url;

    Generics::AppUtils::Args args;

    args.add(
      Generics::AppUtils::equal_name("purl") ||
      Generics::AppUtils::short_name("pu"),
      pool_obj_url);

    Urls urls;
    get_urls(urls);
    args.parse(argc - 1, argv + 1);

    if (urls.empty() ||
      !pool_obj_url.installed())
    {
      std::cerr << "insecure urls are not supplied" << std::endl;
      return -1;
    }

    const std::size_t CORBA_OBJECTS_COUNT = urls.size();
    std::cout << "Got " << CORBA_OBJECTS_COUNT << " objects" << std::endl;
    std::unique_ptr<RefOption> obj_refs[CORBA_OBJECTS_COUNT];

    for (std::size_t i = 0; i < CORBA_OBJECTS_COUNT - 1; ++i)
    {
      obj_refs[i].reset(new RefOption(corba_client_adapter.in()));
      obj_refs[i]->set("", urls[i].c_str());
    }

    check_narrow(corba_client_adapter, pool_obj_url->c_str());
    check_no_good_reference(corba_client_adapter, pool_obj_url->c_str());

    {
      OPTester<
        CORBACommons::ObjectPool<
          CORBATest::TestObjectPool,
          ObjectPoolRefConfiguration> >
        test(&obj_refs[0], CORBA_OBJECTS_COUNT - 1, urls,
          ObjectPoolRefConfiguration(corba_client_adapter.in()));

      test.test();
    }

    {
      OPTester<
        CORBACommons::ObjectPool<
        CORBATest::TestObjectPool,
        ObjectPoolConfiguration<CORBATest::TestObjectPool> > >
        test(&obj_refs[0], CORBA_OBJECTS_COUNT - 1);

      test.test();
    }


    typedef OPTester<
      CORBACommons::ObjectPool<
      CORBATest::TestObjectPool,
      ObjectPoolRefConfiguration> >
      PoolTester;

    PoolTester::Pool::ConfigType configuration(corba_client_adapter.in());
    obj_refs[CORBA_OBJECTS_COUNT - 1].reset(
      new RefOption(corba_client_adapter.in()));
    up_object_test<PoolTester>(configuration, obj_refs,
      CORBA_OBJECTS_COUNT, urls);
    switch_policy_test<PoolTester>(configuration);

    return 0;
  }
  catch (const CORBA::Exception& e)
  {
    std::cerr << FNS << "CORBA::Exception: " << e;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << FNS << "eh::Exception: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
  }

  return 0;
}
