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
#include <map>
#include <string>

#include <eh/Exception.hpp>
#include <Generics/HashTableAdapters.hpp>
#include <Generics/GnuHashTable.hpp>

#include <TestCommons/ActiveObjectCallback.hpp>

#include "Application.hpp"

namespace
{
  const Generics::Time TEST_EXECUTION_TIME(20);
  const unsigned long  TEST_METERINGS_DUMP = 1000000;
//  const unsigned long  TEST_METERINGS_DUMP = 1000;

  const unsigned long TEST_SET_SIZE = 100000;
//  const unsigned long TEST_SET_SIZE = 1000;
  const unsigned long TEST_HASH_TABLE_SIZE = TEST_SET_SIZE / 2;

  const unsigned long STRING_KEY_SIZE = 20;

  const char STAT_STRING_HASH_TABLE_INSERTION[] =
  "String Hash Table Insertion";

  const char STAT_STRING_MAP_TABLE_INSERTION[] =
  "String Map Insertion";

  const char STAT_STRING_HASH_TABLE_FIND[] =
  "String Hash Table Find";

  const char STAT_STRING_MAP_TABLE_FIND[] =
  "String Map Find";

  const char STAT_STRING_HASH_TABLE_ERASE[] =
  "String Hash Table Erase";

  const char STAT_STRING_MAP_TABLE_ERASE[] =
  "String Map Erase";

  const char STAT_LONG_HASH_TABLE_INSERTION[] =
  "Long Hash Table Insertion";

  const char STAT_LONG_MAP_TABLE_INSERTION[] =
  "Long Map Insertion";

  const char STAT_LONG_HASH_TABLE_FIND[] =
  "Long Hash Table Find";

  const char STAT_LONG_MAP_TABLE_FIND[] =
  "Long Map Find";

  const char STAT_LONG_HASH_TABLE_ERASE[] =
  "Long Hash Table Erase";

  const char STAT_LONG_MAP_TABLE_ERASE[] =
  "Long Map Erase";
};

namespace Generics
{
  Application::Application() throw (eh::Exception)
    : active_(false),
      execution_time_(TEST_EXECUTION_TIME),
      callback_(new TestCommons::ActiveObjectCallbackStreamImpl(
        std::cerr, "HashTable"))
  {
    srand(time(0));
  }

  Application::~Application() throw ()
  {
  }

  void
  Application::test_iteration() throw (Exception, eh::Exception)
  {
    test_string_table();
    test_long_table();
    test_inserter_table();
    test_inserter_set();
  }

  void
  Application::test_long_table() throw (Exception, eh::Exception)
  {
    typedef GnuHashTable<NumericHashAdapter<unsigned long>, unsigned long>
      LongHashTable;

    typedef std::map<unsigned long, unsigned long> LongMap;
    typedef std::list<unsigned long> LongList;

    LongHashTable long_hash_table(TEST_HASH_TABLE_SIZE);
    LongMap       long_map;
    LongList      key_list;

    for(unsigned long i = 0; i < TEST_SET_SIZE; i++)
    {
      Generics::Timer timer;
      timer.start();

      long_hash_table[i] = i;

      timer.stop();

      Generics::Statistics::StatSink_var stat(
        statistics_->get(STAT_LONG_HASH_TABLE_INSERTION));

      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      timer.start();

      long_map[i] = i;

      timer.stop();

      stat = statistics_->get(STAT_LONG_MAP_TABLE_INSERTION);
      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      key_list.push_front(i);
    }

    for(unsigned long i = 0; i < TEST_SET_SIZE; i++)
    {
      Generics::Timer timer;
      timer.start();

      LongHashTable::iterator res = long_hash_table.find(i);

      timer.stop();

      if(res == long_hash_table.end() || res->second != i)
      {
        throw Exception("test_long_table: Bug in HashTable");
      }

      Generics::Statistics::StatSink_var stat(
        statistics_->get(STAT_LONG_HASH_TABLE_FIND));

      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      timer.start();

      LongMap::iterator rs = long_map.find(i);

      timer.stop();
      rs++;

      stat = statistics_->get(STAT_LONG_MAP_TABLE_FIND);
      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));
    }

//    long_hash_table.dump(std::cout);

    for(LongList::iterator it = key_list.begin(); it != key_list.end(); it++)
    {
      Generics::Timer timer;
      timer.start();

      long_hash_table.erase(*it);

      timer.stop();

      Generics::Statistics::StatSink_var stat(
        statistics_->get(STAT_LONG_HASH_TABLE_ERASE));

      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      timer.start();

      long_map.erase(*it);

      timer.stop();

      stat = statistics_->get(STAT_LONG_MAP_TABLE_ERASE);
      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));
    }
  }

  void
  Application::test_string_table() throw (Exception, eh::Exception)
  {
    typedef GnuHashTable<StringHashAdapter, unsigned long> StringHashTable;
    typedef std::map<std::string, unsigned long> StringMap;
    typedef std::list<std::string> StringList;

    StringHashTable string_hash_table(TEST_HASH_TABLE_SIZE);
    StringMap       string_map;
    StringList      key_list;

    for(unsigned long i = 0; i < TEST_SET_SIZE; i++)
    {
      char buff[STRING_KEY_SIZE + 1];

      snprintf(buff, sizeof(buff), "%0*lu",
        static_cast<int>(STRING_KEY_SIZE), i);

      Generics::Timer timer;
      timer.start();

      string_hash_table[buff] = i;

      timer.stop();

      Generics::Statistics::StatSink_var stat(
        statistics_->get(STAT_STRING_HASH_TABLE_INSERTION));

      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      timer.start();

      string_map[buff] = i;

      timer.stop();

      stat = statistics_->get(STAT_STRING_MAP_TABLE_INSERTION);
      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      key_list.push_front(buff);
    }

//    string_hash_table.dump(std::cout);
    for(unsigned long i = 0; i < TEST_SET_SIZE; i++)
    {
      char buff[STRING_KEY_SIZE + 1];

      snprintf(buff, sizeof(buff), "%0*lu",
        static_cast<int>(STRING_KEY_SIZE), i);

      Generics::Timer timer;
      timer.start();

      StringHashTable::iterator res = string_hash_table.find(buff);

      timer.stop();

      if(res == string_hash_table.end() || res->second != i)
      {
        throw Exception("Bug in HashTable");
      }

      Generics::Statistics::StatSink_var stat(
        statistics_->get(STAT_STRING_HASH_TABLE_FIND));

      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      timer.start();

      StringMap::iterator rs = string_map.find(buff);

      timer.stop();
      rs++;

      stat = statistics_->get(STAT_STRING_MAP_TABLE_FIND);
      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));
    }

/*
    unsigned long iteration = 0;

    std::cout << "begin: ";
    string_hash_table.begin().dump(std::cout);
    std::cout << std::endl;

    std::cout << "end: ";
    string_hash_table.end().dump(std::cout);
    std::cout << std::endl;
*/
    for(StringHashTable::iterator it = string_hash_table.begin();
        it != string_hash_table.end(); it++)
    {
/*
      std::cout << "i: " << iteration++ << std::endl;
      it.dump(std::cout);
      std::cout << std::endl;
      std::cout << "end: ";
      string_hash_table.end().dump(std::cout);
      std::cout << std::endl;

      const StringHashAdapter& key = it->first;
      std::cout << key.text() << std::endl;
*/
    }

    for(StringList::iterator it = key_list.begin(); it != key_list.end(); it++)
    {
      Generics::Timer timer;
      timer.start();

      string_hash_table.erase(*it);

      timer.stop();

      Generics::Statistics::StatSink_var stat(
        statistics_->get(STAT_STRING_HASH_TABLE_ERASE));

      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));

      timer.start();

      string_map.erase(*it);

      timer.stop();

      stat = statistics_->get(STAT_STRING_MAP_TABLE_ERASE);
      stat->consider(Generics::Statistics::TimedSubject(timer.elapsed_time()));
    }

  }

  void
  Application::test_inserter_table() throw (eh::Exception, Exception)
  {
    typedef Generics::GnuHashTable<Generics::NumericHashAdapter<int>, int>
      Table;
    typedef std::list<Table::value_type> Init;

    Init init;
    int exp_sum = 0, exp_sums = 0;
    for (int i = 1; i < 20; i++)
    {
      Table::value_type value(i, i * i);
      init.push_back(value);
      exp_sum += value.first.value();
      exp_sums += value.second;
    }

    Table hash;

    std::copy(init.begin(), init.end(), std::inserter(hash, hash.end()));

    int sum = 0, sums = 0;
    for (Table::const_iterator itor(hash.begin()); itor != hash.end(); ++itor)
    {
      sum += itor->first.value();
      sums += itor->second;
    }

    if (sum != exp_sum || sums != exp_sums)
    {
      throw Exception("test_inserter: invalid resulted sums");
    }
  }

  void
  Application::test_inserter_set() throw (eh::Exception, Exception)
  {
    typedef Generics::GnuHashSet<Generics::NumericHashAdapter<int> >
      Set;
    typedef std::list<Set::value_type> Init;

    Init init;
    int exp_sum = 0;
    for (int i = 1; i < 20; i++)
    {
      Set::value_type value(i);
      init.push_back(value);
      exp_sum += value.value();
    }

    Set hash;

    std::copy(init.begin(), init.end(), std::inserter(hash, hash.end()));

    int sum = 0;
    for (Set::const_iterator itor(hash.begin()); itor != hash.end(); ++itor)
    {
      sum += itor->value();
    }

    if (sum != exp_sum)
    {
      throw Exception("test_inserter: invalid resulted sums");
    }
  }

  void
  Application::init(int& /*argc*/, char** /*argv*/)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    Write_Guard_ guard(lock_);

    try
    {
      statistics_ = new Statistics::Collection(callback_);

      Statistics::DumpPolicy_var dump_policy(
        new Statistics::CountBasedDumpPolicy(std::cout,
          TEST_METERINGS_DUMP));

      statistics_->add(STAT_STRING_HASH_TABLE_INSERTION,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_STRING_MAP_TABLE_INSERTION,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_STRING_HASH_TABLE_FIND,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_STRING_MAP_TABLE_FIND,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_STRING_HASH_TABLE_ERASE,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_STRING_MAP_TABLE_ERASE,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_LONG_HASH_TABLE_INSERTION,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_LONG_MAP_TABLE_INSERTION,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_LONG_HASH_TABLE_FIND,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_LONG_MAP_TABLE_FIND,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_LONG_HASH_TABLE_ERASE,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add(STAT_LONG_MAP_TABLE_ERASE,
                       new Statistics::TimedStatSink(),
                       dump_policy.in());
    }
    catch(const Statistics::Collection::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::init: Statistics::Collection::Exception caught. "
        "Description:" << std::endl << e.what();
      throw Exception(ostr);
    }
  }

  void
  Application::run()
    throw (InvalidOperationOrder, Exception, eh::Exception)
  {
    if(statistics_.in() == 0)
    {
      throw InvalidOperationOrder("Application::run: call init() first");
    }

    std::cout << std::endl << "Running test ..." << std::endl;

    try
    {
      active_ = true;
      statistics_->activate_object();

      start_time_ = Generics::Time::get_time_of_day();

      test();

      if(!statistics_->active())
        statistics_->wait_object();

      stop_time_ = Generics::Time::get_time_of_day();
    }
    catch(const Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::run: Exception caught. "
        "Description:" << std::endl << e.what();
      throw Exception(ostr);
    }
    catch(const Statistics::Collection::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::run: Statistics::Collection::Exception caught. "
        "Description:" << std::endl << e.what();
      throw Exception(ostr);
    }

    print_results();
  }

  void
  Application::stop() throw (Exception, eh::Exception)
  {
    {
      Write_Guard_ guard(lock_);

      if(!active_) return;
      active_ = false;
    }

    try
    {
      statistics_->deactivate_object();
    }
    catch(const Statistics::Collection::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::stop: Statistics::Collection::Exception caught. "
        "Description:" << std::endl << e.what();
      throw Exception(ostr);
    }
  }

  void
  Application::print_results() throw (eh::Exception)
  {
    std::cout << "*** Test Results ***" << std::endl << std::endl;

    if(start_time_ == Generics::Time::ZERO || stop_time_ == Generics::Time::ZERO)
    {
      std::cerr << "Test failed" << std::endl;
      return;
    }

    Generics::Time real_execution_time = stop_time_ - start_time_;

    std::cout << "Execution time: " << real_execution_time << std::endl <<
      std::endl;

    statistics_->dump(std::cout);

  }

  void
  Application::test() throw (Exception, eh::Exception)
  {
    try
    {
      while(active())
      {
        test_iteration();

        if(Generics::Time::get_time_of_day() - start_time_ >= execution_time_)
        {
          stop();
        }
      }
    }
    catch(const eh::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::test: eh::Exception caught."
         "Description: " << std::endl << e.what();

      callback_->critical(ostr.str());

      stop();
    }
  }
}

int
main(int argc, char** argv)
{
  int result = 1;

  try
  {
    Generics::Application app;

    app.init(argc, argv);
    app.run();

    result = 0;
  }
  catch(const Generics::Application::Exception& e)
  {
    std::cerr << "main: Generics::Application::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }
  catch(const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }

  return result;
}
