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

#include <ReferenceCounting/ReferenceCounting.hpp>
#include <ReferenceCounting/Map.hpp>

#include <Generics/BoundedMap.hpp>
#include <Generics/GnuHashTable.hpp>

#include <TestCommons/MTTester.hpp>


DECLARE_EXCEPTION(Exception, eh::DescriptiveException);


void
show_stats(const Generics::BoundedMapStat& stat)
{
  std::cout << "Usage statistics:" << std::endl
    << "Inserted new:     " << stat.inserted_new << std::endl
    << "Insert existing:  " << stat.insert_existing << std::endl
    << "Removed outdated: " << stat.removed_outdated << std::endl
    << "Removed updated:  " << stat.removed_updated << std::endl
    << "Not inserted:     " << stat.not_inserted << std::endl
    << "Replaced:         " << stat.replaced << std::endl;
}

class DeleteNotifier : public virtual ReferenceCounting::AtomicImpl
{
public:
  DeleteNotifier(int& notify) throw ()
    : notify_(notify)
  {
  }

  ~DeleteNotifier() throw ()
  {
    notify_ = true;
  }

private:
  int& notify_;
};
typedef ReferenceCounting::QualPtr<DeleteNotifier> DeleteNotifierPtr;

void
check(const char* when, const char* what, bool test, bool expected)
{
  if (test != expected)
  {
    Stream::Error ostr;
    ostr << "After execution of '" << when << "' '" << what
      << "' is not " << (expected ? "true" : "false");
    throw Exception(ostr);
  }
}

class Checker
{
public:
  Checker(int size) throw (eh::Exception)
    : data_(size)
  {
  }

  int&
  operator [](int index) throw (eh::Exception)
  {
    return data_[index];
  }

  void
  operator ()(const char* when, ...) throw (eh::Exception, Exception)
  {
    char buf[64];
    va_list va;

    va_start(va, when);
    try
    {
      for (size_t index = 0; index != data_.size(); index++)
      {
        snprintf(buf, sizeof(buf), "data_%u", static_cast<unsigned>(index));
        check(when, buf, data_[index], va_arg(va, int));
        data_[index] = false;
      }
    }
    catch (...)
    {
      va_end(va);
      throw;
    }
    va_end(va);
  }

private:
  std::vector<int> data_;
};

#define CHECK(x) when = #x; x;
#define CHECK_(x) when = #x; check(when, "result", x, true);

void
test_work() throw (eh::Exception)
{
  typedef Generics::NumericHashAdapter<int> Key;
#if 1
  typedef Generics::BoundedMap<Key, DeleteNotifierPtr,
    Generics::DefaultSizePolicy<Key, DeleteNotifierPtr>,
    Sync::Policy::PosixThread,
    ReferenceCounting::Map<Key,
      Generics::BoundedMapTypes<Key, DeleteNotifierPtr>::Item> >
    Map;
#else
  typedef Generics::BoundedMap<Key, DeleteNotifierPtr> Map;
#endif

  const char* when = 0;
  Checker ch(4);

  // Map creation: max of 3 items, timeout 3 seconds
  CHECK(Map map(3, Generics::Time(3)));
  ch(when, 0, 0, 0, 0);

  // Insertion, searching for and erasing of one item
  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[0]));
    map.insert(Map::value_type(Key(0), d)); });
  ch(when, 0, 0, 0, 0);

  CHECK_(map.find(Key(0)) != map.end());
  ch(when, 0, 0, 0, 0);

  CHECK(map.erase(map.find(Key(0))));
  ch(when, 1, 0, 0, 0);

  // Insertion of two items with the same index
  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[0]));
    map.insert(Map::value_type(Key(0), d)); });
  ch(when, 0, 0, 0, 0);

  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[1]));
    map.insert(Map::value_type(Key(0), d)); });
  ch(when, 0, 1, 0, 0);

  // Removing of the item leaving it in the iterator
  {
    CHECK(Map::iterator itor(map.find(Key(0))));
    ch(when, 0, 0, 0, 0);

    CHECK(itor != map.end());
    ch(when, 0, 0, 0, 0);

    CHECK(map.erase(itor));
    ch(when, 0, 0, 0, 0);
  }
  when = "iterator destruction";
  ch(when, 1, 0, 0, 0);

  // Insertion of three items with different indexes
  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[0]));
    map.insert(Map::value_type(Key(0), d)); });
  ch(when, 0, 0, 0, 0);

  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[1]));
    map.insert(Map::value_type(Key(1), d)); });
  ch(when, 0, 0, 0, 0);

  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[2]));
    map.insert(Map::value_type(Key(2), d)); });
  ch(when, 0, 0, 0, 0);

  // Insertion of the fourth item within timeout
  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[3]));
    map.insert(Map::value_type(Key(3), d)); });
  ch(when, 0, 0, 0, 1);

  // Finding of the first item making it the most recent
  CHECK_(map.find(Key(0)) != map.end());
  ch(when, 0, 0, 0, 0);

  // "Insertion" of the item with an existing index makes it the most recent
  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[3]));
    map.insert(Map::value_type(Key(1), d)); });
  ch(when, 0, 0, 0, 1);

  // Insertion of the fourth item outside timeout
  sleep(4);
  CHECK({ DeleteNotifierPtr d(new DeleteNotifier(ch[3]));
    map.insert(Map::value_type(Key(3), d)); });
  ch(when, 0, 0, 1, 0);

  // Replacement of the second item with the third
  CHECK({DeleteNotifierPtr d(new DeleteNotifier(ch[2]));
    map[Key(1)] = d; });
  ch(when, 0, 1, 0, 0);

  // Clearing of the entire map
  CHECK(map.clear());
  ch(when, 1, 0, 1, 1);

  show_stats(map.statistics());
}

class Size : public virtual ReferenceCounting::AtomicImpl
{
public:
  Size(size_t size) throw ()
    : size_(size)
  {
  }
  void
  resize(size_t size) throw ()
  {
    size_ = size;
  }
  size_t
  size() const throw ()
  {
    return size_;
  }
private:
  size_t size_;
};
typedef ReferenceCounting::QualPtr<Size> Size_var;

class Sizer : public DeleteNotifier, public Size
{
public:
  Sizer(int& notify, size_t size) throw ()
    : DeleteNotifier(notify), Size(size)
  {
  }
};
typedef ReferenceCounting::QualPtr<Sizer> Sizer_var;

size_t
get_size(const Generics::NumericHashAdapter<int>&, const Size* size)
  throw ()
{
  assert(size);
  return size->size();
}

void
test_size() throw (eh::Exception)
{
  typedef Generics::NumericHashAdapter<int> Key;
  typedef Generics::BoundedMap<Key, Sizer_var,
    size_t (*)(const Key&, const Size* sizer)> Map;

  const char* when = 0;
  Checker ch(3);

  // Map creation: max of 3 items, timeout 3 seconds
  CHECK(Map map(3, Generics::Time(3), get_size));
  ch(when, 0, 0, 0);

  // Insertion of three items with different indexes
  CHECK({ Sizer_var s(new Sizer(ch[0], 1));
    map.insert(Map::value_type(Key(0), s)); });
  ch(when, 0, 0, 0);

  CHECK({ Sizer_var s(new Sizer(ch[1], 1));
    map.insert(Map::value_type(Key(1), s)); });
  ch(when, 0, 0, 0);

  CHECK({ Sizer_var s(new Sizer(ch[2], 1));
    map.insert(Map::value_type(Key(2), s)); });
  ch(when, 0, 0, 0);

  // Resizing of the first item to zero
  {
    Map::iterator itor = map.find(Key(0));
    itor->second->resize(0);
    CHECK(map.update(itor));
  }
  ch(when, 0, 0, 0);

  // Resizing of the third item to two
  {
    Map::iterator itor = map.find(Key(2));
    itor->second->resize(2);
    CHECK(map.update(itor));
  }
  ch(when, 0, 0, 0);

  sleep(4);

  // Resizing of the second item to three
  {
    Map::iterator itor = map.find(Key(1));
    itor->second->resize(3);
    CHECK(map.update(itor));
  }
  ch(when, 1, 0, 1);

  // Resizing of the second item to four
  {
    Map::iterator itor = map.find(Key(1));
    itor->second->resize(4);
    CHECK(map.update(itor));
  }
  ch(when, 0, 1, 0);

  show_stats(map.statistics());
}

#undef CHECK
#undef CHECK_

class MultiTest
{
public:
  MultiTest(int map_size, const Generics::Time& timeout,
    int diff, int size)
    throw (eh::Exception)
    : map_(map_size, timeout, get_size), diff_(diff), size_(size)
  {
  }

  ~MultiTest() throw ()
  {
    std::cout << "Size " << map_.size() << std::endl;
    show_stats(map_.statistics());
  }

  void
  operator ()() throw (eh::Exception)
  {
    Key key(rand() % diff_);
    {
      Size_var s(new Size(rand() % size_));
      std::pair<Map::iterator, bool> result =
        map_.insert(Map::value_type(key, s));
    }
    Map::iterator itor(map_.find(key));
    if (itor != map_.end())
    {
      itor->second->resize(rand() % size_);
      map_.update(itor);
    }

    {
      Size_var s(new Size(rand() % size_));
      map_[rand() % diff_] = s;
    }
  }

private:
  typedef Generics::NumericHashAdapter<int> Key;
  typedef Generics::BoundedMap<Key, Size_var,
    size_t (*)(const Key&, const Size*)> Map;

  Map map_;
  int diff_;
  int size_;
};

void
test_multi() throw (eh::Exception)
{
  MultiTest multi(500, Generics::Time(0, 10000), 30000, 100);
  TestCommons::MTTester<MultiTest&> test(multi, 8);
  test.run(20, 30);
}

class Sum
{
public:
  Sum() throw ()
    : sum_(0), sums_(0), num_(0)
  {
  }

  Sum&
  operator *() throw ()
  {
    return *this;
  }

  template <typename Pair>
  void
  operator =(const Pair& pair) throw (eh::Exception)
  {
    int key = pair.first.value();
    int value = pair.second;

    if (key + 1 != value)
    {
      std::cerr << "Got invalid pair (" << key << ", " << value << ")" <<
        std::endl;
    }

    sum_ += key;
    sums_ += key * key;
    num_++;
  }

  void
  operator ++() throw ()
  {
  }

  void
  check(size_t exp_sum, size_t exp_sums, size_t exp_num) throw (eh::Exception)
  {
    if (sum_ != exp_sum)
    {
      std::cerr << "Expected sum is " << exp_sum << " but got " << sum_ <<
        std::endl;
    }
    if (sums_ != exp_sums)
    {
      std::cerr << "Expected square sum is " << exp_sums << " but got " <<
        sums_ << std::endl;
    }
    if (num_ != exp_num)
    {
      std::cerr << "Expected count is " << exp_num << " but got " << num_ <<
        std::endl;
    }
  }

private:
  size_t sum_, sums_, num_;
};

void
test_copy() throw (eh::Exception)
{
  typedef Generics::NumericHashAdapter<int> Key;
#if 0
  typedef Generics::BoundedMap<Key, int,
    Generics::DefaultSizePolicy<Key, int>,
    Sync::Policy::PosixThread,
    Generics::GnuHashTable<Key,
      Generics::BoundedMapTypes<Key, int>::Item > > Map;
#else
  typedef Generics::BoundedMap<Key, int> Map;
#endif

  Map map(10, Generics::Time(100));

  int exp_sum = 0, exp_sums = 0;
  for (size_t i = 0; i < map.bound(); i++)
  {
    int j = i + 1;
    map.insert(Map::value_type(i, j));
    exp_sum += i;
    exp_sums += i * i;
  }

  Sum sum;
  sum = map.copy_to(sum);
  sum.check(exp_sum, exp_sums, map.bound());

  std::cout << "test_copy complete\n";
}

int
main()
{
  int result = 1;

  try
  {
    test_work();
    test_size();
    test_multi();
    test_copy();

    result = 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "Exception: " << std::endl << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
  }

  return result;
}
