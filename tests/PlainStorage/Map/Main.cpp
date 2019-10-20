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



// @file Map/Main.cpp

#include <iostream>
#include <sstream>

#include <Generics/Time.hpp>

#include <PlainStorage/BlockFileAdapter.hpp>
#include <PlainStorage/Map.hpp>

#include <TestCommons/MTTester.hpp>

DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

struct StringIndexAccessor
{
  unsigned int
  size(const std::string& key)
    throw(eh::Exception)
  {
    return key.length();
  }

  void
  load(const void* buf, unsigned long size, std::string& key)
    throw(eh::Exception)
  {
    key = std::string((const char*)buf, size);
  }

  void
  save(const std::string& key, void* buf, unsigned long size)
    throw(eh::Exception)
  {
    unsigned int sz = key.length();
    if (sz > size)
    {
      throw Exception("error");
    }
    strncpy(static_cast<char*>(buf), key.c_str(), size);
  }
};

namespace
{
  typedef PlainStorage::Map<std::string, StringIndexAccessor> Map;

  const char* KEYS[] =
  {
    "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8",
    "KEY9", 
  };
  const std::size_t TEST_BUF_SIZE = 20000;
  const std::size_t RECORDS_COUNT = 1000;

}

bool
find_and_test(
  const char* test_name,
  Map& test_map,
  const char* key,
  const char* etalone_buf,
  unsigned long etalone_buf_size)
  throw(eh::Exception)
{
  Map::iterator it = test_map.find(std::string(key));

  if (it != test_map.end())
  {
    unsigned long sz = it->second->size();

    Generics::ArrayChar buf(sz);
    it->second->read(buf.get(), sz);

    if (sz != etalone_buf_size)
    {
      std::cerr << "ERROR(" << test_name << "): "
        << "saved and read data has diff size "
        << "(" << sz << "!=" << etalone_buf_size << ")."
        << std::endl;
      return false;
    }

    int first_diff_pos = -1;
    for (unsigned int i = 0; i < etalone_buf_size; ++i)
    {
      if (buf[i] != etalone_buf[i])
      {
        first_diff_pos = i;
        break;
      }
    }

    if (first_diff_pos != -1)
    {
      std::cerr << "ERROR(" << test_name << "): "
        << "saved and read data has diff in pos "
        << first_diff_pos
        << "(" << (int)(buf[first_diff_pos])
        << "!=" << (int)(etalone_buf[first_diff_pos]) << ")."
        << std::endl;

      return false;
    }
  }
  else
  {
    std::cerr << "ERROR(" << test_name << "): "
      << "inserted key not exist." << std::endl;

    return false;
  }

  return true;
}

class Reader
{
public:
  typedef PlainStorage::PlainWriter PlainWriter;
  Reader(PlainWriter* plain_writer) throw ();

  void
  operator ()() const throw (eh::Exception);
private:
  PlainStorage::PlainWriter_var plain_writer_;
};

class Writer
{
public:
  typedef PlainStorage::PlainWriter PlainWriter;
  Writer(PlainWriter* plain_writer) throw ();

  void
  operator ()() const throw (eh::Exception);
private:
  PlainStorage::PlainWriter_var plain_writer_;
};


//////////////////////////////////////////////////////////////////////////

Writer::Writer(PlainWriter* plain_writer) throw ()
  : plain_writer_(ReferenceCounting::add_ref(plain_writer))
{
}

void
Writer::operator ()() const throw (eh::Exception)
{
  PlainStorage::PlainReadWriteTransaction_var transac_(
    plain_writer_->create_readwrite_transaction());
  std::size_t transac_size_ = transac_->size();
  Generics::ArrayChar test_buf(transac_size_);
  Generics::ArrayChar read_buf(transac_size_);

  transac_->read(test_buf.get(), transac_size_);
//  plain_writer_->write(test_buf.get(), transac_size_); // DEAD LOCK
//  plain_writer_->read(test_buf.get(), transac_size_); // DEAD LOCK

  for (unsigned long i = 0; i < transac_size_; ++i)
  {
    test_buf[i] = 'B';
    read_buf[i] = 'R';
  }

  transac_->write(test_buf.get(), transac_size_);
  transac_->read(read_buf.get(), transac_size_);
  for (unsigned long i = 0; i < transac_size_; ++i)
  {
    if (test_buf[i] != read_buf[i])
    {
      std::cerr << "fail: " << test_buf[i] << " != " << read_buf[i]
      << ", i = " << i << std::endl;
    }
  }
}


Reader::Reader(PlainStorage::PlainWriter* plain_writer) throw ()
  : plain_writer_(ReferenceCounting::add_ref(plain_writer))
{
}

void
Reader::operator ()() const throw (eh::Exception)
{
  PlainStorage::PlainTransaction_var transac_(
    plain_writer_->create_readonly_transaction());
  std::size_t transac_size_ = transac_->size();
  Generics::ArrayChar test_buf(transac_size_);

  transac_->read(test_buf.get(), transac_size_);
  plain_writer_->read(test_buf.get(), transac_size_);
}
//////////////////////////////////////////////////////////////////////////

void
transaction_creating_test(Map& test_map) throw (eh::Exception)
{
  bool completed = true;
  const char* key1 = KEYS[0];
  const char* test_name = "transaction_creating_test";

  Generics::ArrayChar test_buf(TEST_BUF_SIZE);
  for (std::size_t i = 0; i < TEST_BUF_SIZE; ++i)
  {
    test_buf[i] = 'B';
  }

  try
  {
    PlainStorage::PlainWriter_var plain_writer = test_map[key1];
    plain_writer->write(test_buf.get(), TEST_BUF_SIZE);

    {
      PlainStorage::PlainTransaction_var read_transaction =
        plain_writer->create_readonly_transaction();

      Reader reader(plain_writer);
      TestCommons::MTTester<Reader&> mt_tester(reader, 10);
      mt_tester.run(1000, 0, 1000);
    }

    {
/*      PlainStorage::PlainTransaction_var transaction =
        plain_writer->create_transaction();*/

      Writer writer(plain_writer);
      TestCommons::MTTester<Writer&> mt_tester(writer, 10);
      mt_tester.run(1000, 0, 1000);
    }
  }
  catch (const eh::Exception& ex)
  {
    completed = false;

    std::cerr << "ERROR(" << test_name << "): "
      << "Caught exception: " << ex.what() << std::endl;
  }

  if (completed)
  {
    std::cout << "Test with name '" << test_name
      << "' successfully completed." << std::endl;
  }
}

void
insert_find_test(
  Map& test_map)
  throw (eh::Exception)
{
  const char FUN[] = "insert_find_test(): ";
  bool completed = true;

  Generics::ArrayChar buffs[10];
  char ch = 'A';
  for (int i = 0; i < 10; ++i, ++ch)
  {
    buffs[i].reset(TEST_BUF_SIZE);
    memset(buffs[i].get(), ch, TEST_BUF_SIZE);
  }

  try
  {
    test_map.insert(KEYS[0]);
    test_map.insert(KEYS[1]);
    test_map.insert(KEYS[2]);
    test_map.insert(KEYS[3]);

/*
    PlainWriter_var plain_writer;

    {
      WriteGuard_ lock(map_lock_);
      user_profile = test_map[user_id];
    }

    PlainTransaction_var user_profile->create_transaction();
*/

    test_map[KEYS[5]]->write(buffs[0].get(), TEST_BUF_SIZE);
    if (!find_and_test(FUN,
      test_map, KEYS[5], buffs[0].get(), TEST_BUF_SIZE))
    {
      completed = false;
    }

    test_map[KEYS[5]]->write(buffs[1].get(), TEST_BUF_SIZE);
    if (!find_and_test(FUN,
      test_map, KEYS[5], buffs[1].get(), TEST_BUF_SIZE))
    {
      completed = false;
    }

    test_map[KEYS[5]]->write(buffs[5].get(), TEST_BUF_SIZE);
    test_map[KEYS[1]]->write(buffs[1].get(), TEST_BUF_SIZE);

    if (!find_and_test(FUN,
      test_map, KEYS[1], buffs[1].get(), TEST_BUF_SIZE))
    {
      completed = false;
    }

    if (!find_and_test(FUN, test_map, KEYS[5], buffs[5].get(), TEST_BUF_SIZE))
    {
      completed = false;
    }
  }
  catch (const eh::Exception& ex)
  {
    completed = false;

    std::cerr
      << "ERROR(" << FUN << "): "
      << "Caught exception: " << ex.what() << std::endl;
  }

  if (completed)
  {
    std::cout << "Test '" << FUN << "' completed successfully."
      << std::endl;
  }
  else
  {
    std::cerr << FUN << " failed" << std::endl;
  }
}

void
erase_test(Map& test_map) throw (eh::Exception)
{
  const char FUN[] = "erase_test";
  bool completed = true;
  const char* key1 = KEYS[0];
  const char* key2 = KEYS[1];

  Generics::ArrayChar test_buf(TEST_BUF_SIZE);
  for (std::size_t i = 0; i < TEST_BUF_SIZE; ++i)
  {
    test_buf[i] = 'A';
  }

  try
  {
    test_map.insert(key1);

    test_map[key1]->write(test_buf.get(), TEST_BUF_SIZE);
    test_map[key2]->write(test_buf.get(), TEST_BUF_SIZE);

    Map::iterator erase_it = test_map.find(key1);
    test_map.erase(erase_it);

    Map::iterator it = test_map.find(key1);

    if (it != test_map.end())
    {
      completed = false;
      std::cerr << "ERROR(" << FUN << "): "
        << "find key '" << key1 << "' after erasing."
        << std::endl;
    };
  }
  catch (const eh::Exception& ex)
  {
    completed = false;

    std::cerr << "ERROR(" << FUN << "): "
      << "Caught exception: " << ex.what() << std::endl;
  }

  if (completed)
  {
    std::cout << "Test with name '" << FUN << "' successfully completed."
      << std::endl;
  }
}

void
full_fetching_test(Map& test_map) throw (eh::Exception)
{
  std::cout << "FULL FETCHING, READED KEYS: " << std::endl;

  for (Map::iterator it = test_map.begin();
    it != test_map.end(); ++it)
  {
    std::cout << "  '" << it->first << "'" << std::endl;
  }

  std::cout << "FULL FETCHING FINISHED" << std::endl;
}

void
performance_test(
  Map& test_map,
  unsigned int record_size,
  bool content_test = true)
  throw (eh::Exception)
{
  const char* test_name = "performance_test";
  std::string key_str = "KEY_";

  Generics::ArrayChar test_read_buf(record_size);
  unsigned int test_read_buf_size = record_size;

  Generics::ArrayChar test_buf(record_size);

  try
  {
    std::cout << "PERFORMANCE TESTING for record size = "
      << record_size << ": " << std::endl;
    Generics::Timer timer;

    timer.start();

    for (std::size_t i = 0; i < RECORDS_COUNT; ++i)
    {
      std::ostringstream ostr;
      ostr << key_str << i;
      PlainStorage::PlainWriter_var plain_writer =
        test_map[ostr.str()];

      PlainStorage::PlainReadWriteTransaction_var trans =
        plain_writer->create_readwrite_transaction();

      if (trans->size() > 0)
      {
        if (trans->size() > test_read_buf_size)
        {
          test_read_buf.reset(trans->size());
          test_read_buf_size = trans->size();
        }

        trans->read(test_read_buf.get(), trans->size());
      }

      if (content_test)
      {
        memset(test_buf.get(), 'X', record_size);
      }

      trans->write(test_buf.get(), record_size);
    }

    timer.stop();

    unsigned int msec = (timer.elapsed_time() * 1000).tv_sec / RECORDS_COUNT;

    std::cout << "read count: " << RECORDS_COUNT << std::endl;
    std::cout << "average time: ";
    std::cout << msec / 1000 % 10 << "."
      << msec / 100 % 10
      << msec / 10 % 10
      << msec % 10 << std::endl;

    if (content_test)
    {
      for (std::size_t i = 0; i < RECORDS_COUNT; ++i)
      {
        std::ostringstream ostr;
        ostr << key_str << i;
        PlainStorage::PlainWriter_var plain_writer =
          test_map[ostr.str()];

        PlainStorage::PlainTransaction_var trans =
          plain_writer->create_readonly_transaction();

        if (trans->size() != record_size)
        {
          Stream::Error ostr;
          ostr << "read test record has non correct size: "
            << trans->size() << " != " << record_size;
          throw Exception(ostr);
        }

        trans->read(test_buf.get(), trans->size());

        if (test_buf[0] != 'X')
        {
          throw Exception(
            "read test record has non correct content");
        }
      }
    }

    std::cout << "PERFORMANCE TESTING FINISHED" << std::endl;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr
      << "ERROR(" << test_name << "): "
      << "Caught exception: " << ex.what() << std::endl;
  }
}

void
test_iterators() throw (eh::Exception)
{
  const char FUN[] = "test_iterators(): ";
  Map test_map("test.db");
//  Map test_map;
  Map::iterator begin = test_map.begin();
  Map::iterator end = test_map.end();
  if (begin == end)
  {
    std::cout << "empty.begin() == empty.end()" << std::endl;
  }
  Map::iterator it = test_map.find("key");
  Map::const_iterator cit = test_map.end();
  if (cit == it)
  {
    std::cout << "const_iterator == iterator" << std::endl;
  }

  cit = it;
  std::string key = "Key IT test";
  test_map.insert(key);
  test_map[key]->write(FUN, sizeof(FUN));

  // Code for instantiate const_iterator members
  for (Map::const_iterator it = test_map.begin();
    it != test_map.end(); ++it)
  {
  }

  for (Map::iterator it = test_map.begin();
    it != test_map.end(); ++it)
  {
    std::cout << " first '" << it->first << "'" << std::endl;
    std::cout << " first reference '" << (*it).first << "'" << std::endl;
    std::cout << " second '" << it->second << "'" << std::endl;
    std::cout << " second reference '" << (*it).second << "'" << std::endl;
    char read_buf[sizeof(FUN)] = "\0";
    it->second->read(read_buf, sizeof(FUN));
    if (strncmp(read_buf, FUN, sizeof(FUN)))
    {
      std::cerr << FUN << "failed" << std::endl;
    }
    it->second->write("WROTE", 5);
    (*it).second->read(read_buf, 5);
    if (strncmp(read_buf, "WROTE", 5))
    {
      std::cerr << FUN << "failed" << std::endl;
    }
  }
  test_map.erase(key);
  std::cout << "Test " << FUN << "completed" << std::endl;
}

void
test_default_parameters() throw (eh::Exception)
{
//  Map test_map0("empty0.db", 0); // core
  Map test_map1("empty1.db", 1);
  Map test_map8("empty8.db", 8);
  Map test_map16("empty16.db", 16);
  Map test_map32("empty32.db", 32);

  typedef PlainStorage::Map<std::string> MapDefault;
  MapDefault test_map("test.db");
}

/**
 * Remove all test artifacts on disk
 */
void
cleanup() throw ()
{
  unlink("./test.db");
  unlink("./empty1.db");
  unlink("./empty16.db");
  unlink("./empty32.db");
  unlink("./empty8.db");
}

int
main(int argc, char* argv[])
{
  cleanup();
  if (argc > 1)
  {
    if (strcmp(argv[1], "struct") == 0)
    {
      PlainStorage::ReadBlockFileAdapter read_block_file_adapter(
        "test.db", 64*1024);
      for (PlainStorage::BlockIndex i = 0;
        i < read_block_file_adapter.max_block_index();
        ++i)
      {
        PlainStorage::ReadBlockFileAdapter::ReadBlockStruct_var
          block = read_block_file_adapter.get_block(i);

        std::cout << i << "=>" << block->next_index() << std::endl;
      }
    }
    if (strcmp(argv[1], "keys") == 0)
    {
      Map test_map("test.db");

    }
    else if (strcmp(argv[1], "perf") == 0)
    {
      Map test_map("test.db");
      performance_test(test_map, 10*1024);
      performance_test(test_map, 20*1024);
      performance_test(test_map, 100*1024);
      performance_test(test_map, 1024*1024, false);
      performance_test(test_map, 1024*1024, true);
    }
  }
  else
  { // test default actions
    test_iterators();
    test_default_parameters();
    Map test_map("test.db");
    full_fetching_test(test_map);

    insert_find_test(test_map);
    erase_test(test_map);
    transaction_creating_test(test_map);
    performance_test(test_map, 10*1024);
  }
  cleanup();
  return 0;
}

