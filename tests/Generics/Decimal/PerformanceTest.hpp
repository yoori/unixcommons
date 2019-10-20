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
#include <string>
#include <vector>

#include <Generics/Time.hpp>
#include <Generics/ArrayAutoPtr.hpp>

template<typename DecimalType>
class PerformanceTestSuite
{
private:
  struct Empty
  {
    static
    void
    func(DecimalType& res, const DecimalType& /*arg1*/,
      const DecimalType& arg2) throw (eh::Exception)
      __attribute__((always_inline))
    {
      res = arg2;
    }
  };

  struct Add
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1, const DecimalType& arg2)
      throw (eh::Exception)
      __attribute__((always_inline))
    {
      DecimalType::add(arg1, arg2, res);
    }
  };

  struct Sub
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1, const DecimalType& arg2)
      throw (eh::Exception)
      __attribute__((always_inline))
    {
      DecimalType::sub(arg1, arg2, res);
    }
  };

  struct MulF
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1, const DecimalType& arg2)
      throw (eh::Exception)
      __attribute__((always_inline))
    {
      res = DecimalType::mul(arg1, arg2, DMR_FLOOR);
    }
  };

  struct MulR
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1, const DecimalType& arg2)
      throw (eh::Exception)
      __attribute__((always_inline))
    {
      res = DecimalType::mul(arg1, arg2, DMR_ROUND);
    }
  };

  struct MulC
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1, const DecimalType& arg2)
      throw (eh::Exception)
      __attribute__((always_inline))
    {
      res = DecimalType::mul(arg1, arg2, DMR_CEIL);
    }
  };

  struct DivF
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1, const DecimalType& arg2)
      throw (eh::Exception)
    {
      res = DecimalType::div(arg1, arg2, DDR_FLOOR);
    }
  };

  struct DivC
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1, const DecimalType& arg2)
      throw (eh::Exception)
      __attribute__((always_inline))
    {
      res = DecimalType::div(arg1, arg2, DDR_CEIL);
    }
  };

  struct DivR
  {
    static
    void
    func(DecimalType& res, const DecimalType& arg1,
      const DecimalType& arg2) throw (eh::Exception)
      __attribute__((always_inline))
    {
      DecimalType::div(arg1, arg2, res);
    }
  };

  struct Ceil
  {
    static
    void
    func(DecimalType& res, const DecimalType& /*arg1*/,
      const DecimalType& arg2) throw (eh::Exception)
      __attribute__((always_inline))
    {
      res = arg2;
      res.ceil(1);
    }
  };

public:
  explicit
  PerformanceTestSuite(const std::string& name) throw (eh::Exception)
    : name_(name), max_length_(0)
  {
    add_test_case_<Empty>("Empty1");
    add_test_case_<Empty>("Empty2");
    add_test_case_<Add>("Addition");
    add_test_case_<Sub>("Subtraction");
    add_test_case_<MulF>("Multiplication floor");
    add_test_case_<MulR>("Multiplication round");
    add_test_case_<MulC>("Multiplication ceil");
    add_test_case_<DivF>("Division floor");
    add_test_case_<DivC>("Division ceil");
    add_test_case_<DivR>("Division reminder");
    add_test_case_<Ceil>("Ceil");
  }

  void
  run() throw (eh::Exception)
  {
    set_up_();
    std::cout << "Run " << name_ << std::endl;

    for (typename Cases::iterator i = test_cases_.begin();
      i != test_cases_.end(); ++i)
    {
      run_(*i);
    }
  }

private:
  const std::string name_;

  typedef void (*test_func)(const DecimalType* data, DecimalType* sample);
  struct TestCase
  {
    test_func func;
    std::string name;
  };
  typedef std::vector<TestCase> Cases;

  Cases test_cases_;
  size_t max_length_;

  static const int DATA_SIZE = 5;
  static const int SAMPLE_SIZE = DATA_SIZE * DATA_SIZE;
  static const int SAMPLE_RUNS = 10000000;

  DecimalType test_data_[DATA_SIZE];
  DecimalType sample_[SAMPLE_SIZE];

private:

  template <typename Op>
  static
  void
  wrapper_(const DecimalType* data, DecimalType* sample)
    throw (eh::Exception)
  {
    for (int i = 0; i < SAMPLE_RUNS; ++i)
    {
      const DecimalType arg1 = data[i % DATA_SIZE];
      const DecimalType arg2 = data[(i / DATA_SIZE) % DATA_SIZE];
      Op::func(sample[i % SAMPLE_SIZE], arg1, arg2);
    }
  }

  template <typename Op>
  void
  add_test_case_(const char* case_name) throw (eh::Exception)
  {
    TestCase test_case = { wrapper_<Op>, case_name };
    max_length_ = std::max(test_case.name.size(), max_length_);
    test_cases_.push_back(test_case);
  }

  void
  run_(TestCase& test_case) throw (eh::Exception)
  {
    Generics::CPUTimer timer;
    timer.start();
    test_case.func(test_data_, sample_);
    timer.stop();
    std::cout << "\t" << test_case.name;
    for (size_t i = max_length_ + 1 - test_case.name.size(); i--;)
    {
      std::cout << ' ';
    }
    std::cout << timer.elapsed_time() << std::endl;
  }

  void
  set_up_() throw (eh::Exception)
  {
    test_data_[0] = DecimalType(false, 1001, 1);
    test_data_[1] = DecimalType(1.0001);
    test_data_[2] = DecimalType(false, 2, 7182818);
    test_data_[3] = DecimalType(true, 0, 1717);
    test_data_[4] = DecimalType(false, 3, 1415926);
  }
};


template <typename DecimalType>
void
perfomance_test(const char* name) throw (eh::Exception)
{
  PerformanceTestSuite<DecimalType> test(name);
  test.run();
}

void
perfomance_test() throw (eh::Exception)
{
  perfomance_test<Generics::Decimal<uint64_t, 36, 16> >(
    "Decimal<uint64_t,36,16>");
  perfomance_test<Generics::Decimal<uint64_t, 18, 8> >(
    "Decimal<uint64_t,18,8>");
  perfomance_test<Generics::SimpleDecimal<uint64_t, 18, 8> >(
    "SimpleDecimal<uint64_t,18,8>");
}
