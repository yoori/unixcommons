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
#include <sstream>

#include <sys/file.h>

#include <Generics/Rand.hpp>
#include <Generics/AppUtils.hpp>
#include <Generics/Statistics.hpp>
#include <Generics/MMap.hpp>

#include <Logger/StreamLogger.hpp>

#include <CORBACommons/CorbaAdapters.hpp>
#include <CORBAConfigParser/ParameterConfig.hpp>

#include <TestCommons/MTTester.hpp>

#include "../Server/TestInt.hpp"
#include "Application.hpp"

#define USE_EXTENDED_ADAPTER

namespace
{
  unsigned long DEFAULT_NORMAL_TEST_TIME = 10; // 10 sec
  unsigned long DEFAULT_SECURE_TEST_TIME = 10; // 10 sec
  unsigned long DEFAULT_THREADS_AMOUNT = 15;
  unsigned long DEFAULT_SIMUL_TASK = 100;
  int DEFAULT_TASK_LIMIT = -1;
  unsigned long DEFAULT_LOW_BOUND = 0;
  unsigned long DEFAULT_RANDOM_SIZE = 10000;
}

struct Stat
{
  Stat() throw ();

  _Atomic_word usage;
  _Atomic_word timeouts;
  _Atomic_word cf_54410306, cfo;
};

Stat::Stat() throw ()
  : usage(0), timeouts(0), cf_54410306(0), cfo(0)
{
}

struct TestContext
{
  TestContext() throw ();

  Generics::AppUtils::Option<unsigned long> threads_amount;
  Generics::AppUtils::Option<unsigned long> sim_task_amount;
  Generics::AppUtils::Option<int> task_limit;
  Generics::AppUtils::Option<unsigned long> low_bound;
  Generics::AppUtils::Option<unsigned long> random_size;
  Generics::AppUtils::StringOption lock_file;
  Generics::AppUtils::CheckOption only_twoway;

  std::unique_ptr<Generics::MMapFile> mf;
  volatile Stat stat;
  volatile Stat* pstat;
};

TestContext::TestContext() throw ()
  : threads_amount(DEFAULT_THREADS_AMOUNT),
    sim_task_amount(DEFAULT_SIMUL_TASK),
    task_limit(DEFAULT_TASK_LIMIT),
    low_bound(DEFAULT_LOW_BOUND),
    random_size(DEFAULT_RANDOM_SIZE)
{
}

class ClientFunctor
{
public:
  static void
  test(CORBATest::TestInt_ptr test_int, unsigned long time,
    const TestContext& ctx, const char* type,
    bool make_oneway_test = true)
  {
    std::string prefix = "Call delay ";
    test(test_int, time, ctx, &CORBATest::TestInt::test,
      (prefix + type + " twoway").c_str());
    if (make_oneway_test && !ctx.only_twoway.enabled())
    {
      test(test_int, time, ctx, &CORBATest::TestInt::oneway_test,
        (prefix + type + " oneway").c_str());
    }
  }

private:
  typedef void (CORBATest::TestInt::* Func)(const CORBATest::OctetSeq&);

  ClientFunctor(CORBATest::TestInt_ptr test_int, Func func, const char* name,
    const TestContext& ctx)
    : test_int_(CORBATest::TestInt::_duplicate(test_int)), func_(func),
      context_(ctx)
  {
    Generics::Statistics::DumpRunner_var stat_runner(
      new Generics::Statistics::NullDumpRunner);
    Generics::Statistics::DumpPolicy_var stat_policy(
      new Generics::Statistics::NullDumpPolicy);
    statistics_ = new Generics::Statistics::Collection(stat_runner.in());
    statistics_->add(name,
      new Generics::Statistics::TimedStatSink(), stat_policy.in());
    stat_ = statistics_->get(name);
  }

  ~ClientFunctor() throw ()
  {
    statistics_->dump(std::cout);
  }

public:
  void
  operator()()
  {
    const unsigned int PARAM_LEN =
      Generics::safe_rand(*context_.low_bound,
        *context_.low_bound + *context_.random_size);
    CORBATest::OctetSeq param;
    param.length(PARAM_LEN);
    for (CORBA::ULong i = 0; i < PARAM_LEN; ++i)
    {
      param[i] = i % 256;
    }

    Generics::Timer timer;
    timer.start();
    try
    {
      (test_int_->*func_)(param);
    }
    catch (const CORBA::TIMEOUT& ex)
    {
      __gnu_cxx::__atomic_add(&context_.pstat->timeouts, 1);
    }
    catch (const CORBA::COMM_FAILURE& ex)
    {
      write(2, "!", 1);
      if (ex.minor() == 0x54410306)
      {
        __gnu_cxx::__atomic_add(&context_.pstat->cf_54410306, 1);
      }
      else
      {
        std::cerr << FNS << "CORBA::COMM_FAILURE: " << ex.minor() <<
          " " << ex << std::endl;
        __gnu_cxx::__atomic_add(&context_.pstat->cfo, 1);
      }
    }
    catch (const CORBA::SystemException& ex)
    {
      std::cerr << FNS << "Unexpected CORBA::SystemException: " <<
        ex << std::endl;
    }
    timer.stop();
 
    stat_->consider(Generics::Statistics::TimedSubject(
      timer.elapsed_time()));
  }

private:
  static void
  test(CORBATest::TestInt_ptr test_int, unsigned long time,
    const TestContext& ctx, Func func,
    const char* name) throw (eh::Exception)
  {
    ClientFunctor functor(test_int, func, name, ctx);
    TestCommons::MTTester<ClientFunctor&> mt_tester(functor,
      *ctx.threads_amount);
    mt_tester.run(*ctx.sim_task_amount, time, *ctx.task_limit);
  }

  CORBATest::TestInt_var test_int_;
  Func func_;
  const TestContext& context_;
  Generics::Statistics::Collection_var statistics_;
  Generics::Statistics::StatSink_var stat_;
};

class ExtendedCorbaClientAdapter : public CORBACommons::CorbaClientAdapter
{
public:
  ExtendedCorbaClientAdapter(const CORBACommons::CorbaClientConfig& config,
    Logging::Logger* logger) throw ();

  void
  orbs_run() throw (eh::Exception);

  void
  orbs_shutdown() throw (eh::Exception);

protected:
  virtual
  ~ExtendedCorbaClientAdapter() throw ();

private:
  static void*
  thread_func_(void* arg) throw ();

  typedef std::list<pthread_t> Threads;
  Threads threads_;
};
typedef ReferenceCounting::QualPtr<ExtendedCorbaClientAdapter>
  ExtendedCorbaClientAdapter_var;

ExtendedCorbaClientAdapter::ExtendedCorbaClientAdapter(
  const CORBACommons::CorbaClientConfig& config, Logging::Logger* logger)
  throw ()
  : CORBACommons::CorbaClientAdapter(config, logger)
{
}

ExtendedCorbaClientAdapter::~ExtendedCorbaClientAdapter() throw ()
{
}

void
ExtendedCorbaClientAdapter::orbs_run() throw (eh::Exception)
{
  const Orbs::OrbsHolder& orbs = OrbsSingleton::instance().get_orbs();
  for (Orbs::OrbsHolder::const_iterator itor(orbs.begin());
    itor != orbs.end(); ++itor)
  {
    pthread_t thread;
    pthread_create(&thread, 0, thread_func_, itor->second.in());
    threads_.push_back(thread);
  }
}

void
ExtendedCorbaClientAdapter::orbs_shutdown() throw (eh::Exception)
{
  const Orbs::OrbsHolder& orbs = OrbsSingleton::instance().get_orbs();
  for (Orbs::OrbsHolder::const_iterator itor(orbs.begin());
    itor != orbs.end(); ++itor)
  {
    itor->second->shutdown();
    pthread_join(threads_.front(), 0);
    threads_.pop_front();
  }
}

void*
ExtendedCorbaClientAdapter::thread_func_(void* arg) throw ()
{
  static_cast<CORBA::ORB_ptr>(arg)->run();
  return 0;
}

void
Application::run(int argc, char* argv[]) throw (Exception, eh::Exception)
{
  try
  {
    Logging::FLogger_var logger(
      new Logging::OStream::Logger(Logging::OStream::Config(
        std::cout, 1000)));

    CORBACommons::CorbaClientConfig config;
    {
      const char* timeout = getenv("ORB_TIMEOUT");
      if (timeout)
      {
        config.timeout = Generics::Time(atoi(timeout));
      }
    }

#ifdef USE_EXTENDED_ADAPTER
    ExtendedCorbaClientAdapter_var corba_client_adapter(
      new ExtendedCorbaClientAdapter(config, logger));
#else
    CORBACommons::CorbaClientAdapter_var corba_client_adapter(
      new CORBACommons::CorbaClientAdapter(config, logger));
#endif

    CORBAConfigParser::CorbaRefOption<CORBATest::TestInt> opt_url(
      corba_client_adapter);
    CORBAConfigParser::CorbaRefOption<CORBATest::TestInt> opt_secure_url(
      corba_client_adapter, "server.key:adserver:server.der;ce.der");
    Generics::AppUtils::Option<unsigned long> opt_time(
      DEFAULT_NORMAL_TEST_TIME);
    Generics::AppUtils::Option<unsigned long> opt_secure_time(
      DEFAULT_SECURE_TEST_TIME);

    TestContext context;
    Generics::AppUtils::Args args;

    args.add(
      Generics::AppUtils::equal_name("url") ||
      Generics::AppUtils::short_name("u"),
      opt_url);
    args.add(
      Generics::AppUtils::equal_name("secure-url") ||
      Generics::AppUtils::short_name("su"),
      opt_secure_url);
    args.add(
      Generics::AppUtils::equal_name("time") ||
      Generics::AppUtils::short_name("t"),
      opt_time);
    args.add(
      Generics::AppUtils::equal_name("secure-time") ||
      Generics::AppUtils::short_name("st"),
      opt_secure_time);
    args.add(
      Generics::AppUtils::equal_name("threads") ||
      Generics::AppUtils::short_name("thr"),
      context.threads_amount);
    args.add(
      Generics::AppUtils::equal_name("simul-task") ||
      Generics::AppUtils::short_name("s"),
      context.sim_task_amount);
    args.add(
      Generics::AppUtils::equal_name("limit-task") ||
      Generics::AppUtils::short_name("l"),
      context.task_limit);
    args.add(
      Generics::AppUtils::equal_name("low-bound") ||
      Generics::AppUtils::short_name("lb"),
      context.low_bound);
    args.add(
      Generics::AppUtils::equal_name("random-size") ||
      Generics::AppUtils::short_name("rs"),
      context.random_size);
    args.add(
      Generics::AppUtils::equal_name("only-twoway") ||
      Generics::AppUtils::short_name("ot"),
      context.only_twoway);
    args.add(
      Generics::AppUtils::equal_name("lock-file") ||
      Generics::AppUtils::short_name("lf"),
      context.lock_file);

    args.parse(argc - 1, argv + 1);

#ifdef USE_EXTENDED_ADAPTER
    corba_client_adapter->orbs_run();
#endif

    if (context.lock_file.installed())
    {
      context.mf.reset(new Generics::MMapFile(context.lock_file->c_str(),
        0, 0, O_RDWR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE));
      context.pstat = static_cast<Stat*>(context.mf->memory());
      if (flock(context.mf->file_descriptor(), LOCK_SH) < 0)
      {
        std::cerr << "Failed to lock " << *context.lock_file << std::endl;
      }
    }
    else
    {
      context.pstat = &context.stat;
    }

    __gnu_cxx::__atomic_add(&context.pstat->usage, 1);

    if (opt_url.installed())
    {
      std::cout << "To test normal connection." << std::endl;

      CORBATest::TestInt_var test_int = *opt_url;

      ClientFunctor::test(test_int, *opt_time, context, "insecure");

      std::cout << "Test normal connection finished." << std::endl;
    }

    if (opt_secure_url.installed())
    {
      std::cout << "To test secure connection." << std::endl;

      CORBATest::TestInt_var test_int = *opt_secure_url;

      ClientFunctor::test(test_int, *opt_time, context, "secure"
#ifdef USE_EXTENDED_ADAPTER
        , false // TAO bug
#endif
        );

      std::cout << "Test secure connection finished." << std::endl;
    }

#ifdef USE_EXTENDED_ADAPTER
    corba_client_adapter->orbs_shutdown();
#endif

    if (__gnu_cxx::__exchange_and_add(&context.pstat->usage, -1) == 1)
    {
      Stream::Error ostr;
      ostr << "Timeouts: " << context.pstat->timeouts << "\n" <<
        "Comm failures: " << context.pstat->cf_54410306 << " " <<
        context.pstat->cfo << "\n";
      write((context.lock_file.installed() ? 2 : 1),
        ostr.str().data(), ostr.str().size());
    }
  }
  catch (const CORBA::Exception& e)
  {
    std::ostringstream ostr;
    ostr << "Application::run: CORBA::Exception caught. Description:\n"
         << e;

    throw Exception(ostr.str());
  }
}

int
main(int argc, char** argv)
{
  const char* td = getenv("TAO_DEBUG");
  TAO_debug_level = td ? atoi(td) : 0;

  try
  {
    Application app;

    app.run(argc, argv);

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr
      << "main: eh::Exception exception caught. Description:" << std::endl
      << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "main: unknown exception caught" << std::endl;
  }

  return -1;
}
