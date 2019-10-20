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



// Application.hpp
#ifndef TEST_APPLICATION_TASK_RUNNER_HPP_INCLUDED
#define TEST_APPLICATION_TASK_RUNNER_HPP_INCLUDED

  /**
   * Special callback with checks and correctness control abilities.
   */
class DescriptorListenerCallbackTester :
  public virtual Generics::ActiveDescriptorListenerCallback,
  public virtual Generics::DescriptorListenerCallback,
  public virtual Generics::ExecuteAndListenCallback,
  public ReferenceCounting::AtomicImpl
{
public:
  DescriptorListenerCallbackTester() throw ();

  virtual void
  on_data_ready(int fd, std::size_t fd_index,
    const char* str, std::size_t size) throw ();

  virtual void
  on_closed(int fd, std::size_t fd_index, int error) throw ();

  virtual void
  on_all_closed() throw ();

  std::size_t
  get_and_reset_closed() throw ();

  std::string
  received_data() const throw ();

  void
  reset() throw ();

  void
  set_full_lines_test(bool new_value) throw ();

  virtual void
  report_error(Severity severity, const String::SubString& description,
    const char* error_code = 0) throw ();
protected:
  virtual
  ~DescriptorListenerCallbackTester() throw ();
private:
  volatile _Atomic_word close_counter_;
  std::string ready_data_;
  Sync::PosixMutex mutex_;
  int checking_descriptor_;
  bool full_lines_test_;
};

  typedef ReferenceCounting::QualPtr<DescriptorListenerCallbackTester>
    DescriptorListenerCallbackTester_var;

class TestTasker
{
public:
  typedef Generics::ArrayAutoPtr<int> Descriptors;

  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  TestTasker() throw (eh::Exception);

  virtual
  ~TestTasker() throw ();

  /**
   * Check all data delivery.
   */
  void
  do_overflow_test(bool buffering_mode) throw (eh::Exception);

  /**
   * Close half of descriptors amount. Check correct of quantity.
   */
  void
  do_closed_descriptors_test(bool buffering_mode) throw (eh::Exception);

  /**
   * Check all data delivery + full lines mode only.
   */
  void
  do_auto_test(bool buffering_mode) throw (eh::Exception);

  /**
   * Do execute_and_listen function test
   */
  void
  do_execute_and_listen_test(const char* program_name)
    throw (eh::Exception);

private:

  void
  spawn_descriptors_(Descriptors& read_descriptors,
    Descriptors& write_descriptors,
    std::size_t count)
    throw (eh::Exception);

  static const std::size_t PIPES_COUNT_ = 10;

  Descriptors read_descriptors;
  Descriptors write_descriptors;
  DescriptorListenerCallbackTester_var callback_;
};

typedef TestTasker::Descriptors Descriptors;

/**
 * Child part for execute_and_listen function test
 */
void
do_execute_and_listen_test_child_code(char* argv[])
  throw (eh::Exception);

class Writer
{
public:
  Writer(Descriptors& dscs, const char* msg) throw ();

  void
  operator()() throw (eh::Exception);

  /**
   * Should reset multiplexer before new test cycle.
   */
  void
  reset() throw ();

private:

  Descriptors& write_pipes_;
  const std::string MSG_;

  // use it for choosing pipe by some thread.
  volatile _Atomic_word multiplexor_;
};

class MTAdapter
{
public:
  MTAdapter(const char* progname) throw ();
  void
  operator ()() throw (eh::Exception);

private:
  const char* progname;
};

class MPAdapter
{
public:
  MPAdapter(const char* progname, int threads, time_t interval,
    int limit = -1) throw ();
  void
  operator ()() throw (eh::Exception);

private:
  const char* progname;
  int threads;
  time_t interval;
  int limit;
};


#endif  // _TEST_APPLICATION_TASK_RUNNER_HPP_INCLUDED_
