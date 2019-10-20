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



// Application.cpp

#include <sys/wait.h>

#include <eh/Errno.hpp>
#include <TestCommons/MTTester.hpp>

#include <Generics/Listener.hpp>
#include <Generics/Rand.hpp>

#include <fcntl.h>  // fcntl

#include "Application.hpp"

using namespace Generics;

  /**
   * 1. Create N pipes, by pipe call.
   * 2. Put read pipes array into DescriptorListener.
   *    It will demultiplex messages in callbacks and check
   *    test correctness.
   *    Demultiplexor waits on even pipes.
   * 3. Simultaneously write into odd pipes (writeable pipes) from MTTester
   *    (array indexes are 0, 1, 3,..9) some message.
   */

namespace
{
  const std::size_t FULL_LINES_TEST_BUF_SIZE = 100;
  const std::size_t DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST = 10;
}

void
TestTasker::do_auto_test(bool buffering_mode) throw (eh::Exception)
{
  // Prepare file descriptors
  spawn_descriptors_(read_descriptors, write_descriptors, PIPES_COUNT_);
  if (buffering_mode)
  {
    callback_->set_full_lines_test(true);
  }

  ActiveDescriptorListener_var dl(
    new ActiveDescriptorListener(
      callback_.in(),
      read_descriptors.get(),
      PIPES_COUNT_,
      FULL_LINES_TEST_BUF_SIZE,
      buffering_mode));
  callback_->Generics::ActiveDescriptorListenerCallback::listener(dl);
  dl->activate_object();
  std::string buf;
  std::size_t length = Generics::safe_rand(1, 2048);
  for (std::size_t i = 0; i < length; ++i)
  {
    if ((i % Generics::safe_rand(1, 20)) == 0)
    {
      buf += '\n';
    }
    else
    {
      buf += 65 + (i % 26);
    }
  }
  buf += '\n';
  std::cout << "Random string length: " << buf.size()
    << std::endl;

  Writer writer(write_descriptors, buf.c_str());
  TestCommons::MTTester<Writer&> mt_tester(
    writer, 5);
  mt_tester.run(PIPES_COUNT_, 0, PIPES_COUNT_);

  for(std::size_t i = 0; i < PIPES_COUNT_; ++i)
  {
    close(write_descriptors[i]);
  }
  dl->wait_object();
  if (callback_->received_data() != buf)
  {
    std::cerr << "Test error: didn't got send message\n"
      << "Buffering is "
      << (buffering_mode ? "true" : "false") << std::endl
      << "ORIGINAL: " << buf << std::endl
      << "RESULT: " << callback_->received_data() << std::endl;
  }
  callback_->reset();
}

void
TestTasker::do_overflow_test(bool buffering_mode) throw (eh::Exception)
{
  // Prepare file descriptors
  spawn_descriptors_(read_descriptors, write_descriptors, PIPES_COUNT_);

  ActiveDescriptorListener_var dl(
    new ActiveDescriptorListener(
      callback_.in(),
      read_descriptors.get(),
      PIPES_COUNT_,
      10,
      buffering_mode));
  callback_->Generics::ActiveDescriptorListenerCallback::listener(dl);
  dl->activate_object();
  char buf[95];
  for (std::size_t i = 0; i < sizeof(buf); ++i)
  {
    buf[i] = 65 + (i % 26);
    if ((i % 10) == 0)
    {
      buf[i] = '\n';
    }
  }
  buf[sizeof(buf) - 1] = 0;
  buf[sizeof(buf) - 2] = '\n';
  Writer writer(write_descriptors, buf);
  TestCommons::MTTester<Writer&> mt_tester(
    writer, 5);
  mt_tester.run(PIPES_COUNT_, 0, PIPES_COUNT_);

  for(std::size_t i = 0; i < PIPES_COUNT_; ++i)
  {
    close(write_descriptors[i]);
  }
  dl->wait_object();

  std::string data = callback_->received_data();
  if (memcmp(buf, data.c_str(), sizeof(buf) - 2))
  {
    for (std::size_t i = 0; i < sizeof(buf) - 1; ++i)
    {
      if (buf[i] != data[i])
      {
        std::size_t code = buf[i];
        std::size_t code_r = data[i];
        std::cout << "Fail at " << i << std::endl;
        std::cout << "CODES: " << code << " - " << code_r << std::endl;
        break;
      }
    }
    std::cerr << "Test error: didn't got send message\n"
      << "Buffering is "
      << (buffering_mode ? "true" : "false") << std::endl
      << "ORIG: " << std::string(buf, sizeof(buf) - 1) << std::endl
      << "RESULT: " << callback_->received_data() << std::endl;

    throw Exception("Test error: didn't got send message");
  }
  callback_->reset();
}

void
TestTasker::do_closed_descriptors_test(bool buffering_mode)
  throw (eh::Exception)
{
  // File descriptors
  spawn_descriptors_(read_descriptors, write_descriptors, PIPES_COUNT_);

  ActiveDescriptorListener_var dl(
    new ActiveDescriptorListener(
      callback_.in(),
      read_descriptors.get(),
      PIPES_COUNT_,
      10,
      buffering_mode));
  callback_->Generics::ActiveDescriptorListenerCallback::listener(dl);
  dl->activate_object();

  Writer writer(write_descriptors, "Hi!! there..");
  TestCommons::MTTester<Writer&> mt_tester(
    writer, 5);

  for(std::size_t i = 0; i < PIPES_COUNT_ / 2; ++i)
  {
    close(write_descriptors[i]);
  }
  mt_tester.run(PIPES_COUNT_, 0, PIPES_COUNT_);

  for (std::size_t i = PIPES_COUNT_ / 2; i < PIPES_COUNT_; ++i)
  {
    close(write_descriptors[i]);
  }
  dl->wait_object();
  std::cout << callback_->received_data() << std::endl;
  std::size_t failures = callback_->get_and_reset_closed();
  if (failures != PIPES_COUNT_)
  {
    std::cerr << "Test error: don't hook exactly on_close events"
    << "Await: " << PIPES_COUNT_ << " happened failures: "
    << failures << std::endl;
  }
  callback_->reset();
}

void
TestTasker::do_execute_and_listen_test(const char* program_name)
  throw (eh::Exception)
{
  std::cout << "try perform execute_and_listen test" << std::endl;

  typedef std::vector<int> Descriptors;
  Descriptors descriptors;
  // you must push back DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST
  // at minimum.
//  descriptors.push_back(STDOUT_FILENO);
//  descriptors.push_back(STDERR_FILENO);
  for (std::size_t i = 0; i < DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST;
    ++i)
  {
    descriptors.push_back(i);
  }

  std::string descriptors_string;
  String::StringManip::base64mod_encode(descriptors_string,
    &descriptors[0], DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST *
      sizeof (Descriptors::value_type));

  std::string buf;
  std::size_t length = Generics::safe_rand(1, 2048);
  for (std::size_t i = 0; i < length; ++i)
  {
    if ((i % Generics::safe_rand(1, 20)) == 0)
    {
      buf += '\n';
    }
    else
    {
      buf += 65 + (i % 26);
    }
  }
  buf += '\n';
//  buf = "TMP TEST STRING\n";
  std::cout << "Random string length: " << buf.size()
    << std::endl;

  std::vector<char*> args;
  args.reserve(4);   // name + descriptors + standard + zero ptr
  args.push_back(const_cast<char*>(program_name));
  args.push_back(const_cast<char*>(descriptors_string.c_str()));
  args.push_back(const_cast<char*>(buf.c_str()));
  args.push_back(0);

  callback_->set_full_lines_test(true);
  int result = Generics::execute_and_listen(callback_.in(),
    program_name, &args[0],
    descriptors.size(), &descriptors[0], 0, 0,
    4096, true);
  std::cout << "Result execute and listen " << result << std::endl;

  if (callback_->received_data() != buf)
  {
    std::cerr << "Test error: didn't got send message\n"
      << "ORIGINAL: " << buf << std::endl
      << "RESULT: " << callback_->received_data() << std::endl;
  }
  callback_->reset();
}

void
do_execute_and_listen_test_child_code(char* argv[])
  throw (eh::Exception)
{
  Descriptors descriptors(DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST);
  std::string descriptors_string;
  String::StringManip::base64mod_decode(descriptors_string,
    String::SubString(argv[1]));
  memcpy(descriptors.get(), descriptors_string.data(),
    descriptors_string.size());

//  std::cout << argv[2] << std::flush;
  Writer writer(descriptors, argv[2]);
  TestCommons::MTTester<Writer&> mt_tester(
    writer, 1);

  for(std::size_t i = 0; i < DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST / 2; ++i)
  {
    close(descriptors[i]);
  }
  mt_tester.run(DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST, 0,
    DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST);

  for(std::size_t i = DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST / 2;
      i < DESCRIPTORS_AMOUNT_EXECUTE_LISTEN_TEST; ++i)
  {
    close(descriptors[i]);
  }
}

class Aggregator :
  public Generics::ExecuteAndListenCallback,
  public ReferenceCounting::AtomicImpl
{
public:
  virtual void
  on_data_ready(int fd, std::size_t fd_index,
    const char* str, std::size_t size) throw ();

  virtual void
  report_error(Severity severity, const String::SubString& description,
    const char* error_code = 0) throw ();

  std::string
  buffer(int index) throw (eh::Exception);

protected:
  virtual
  ~Aggregator() throw ();

private:
  std::map<int, std::string> buffers_;
};
typedef ReferenceCounting::QualPtr<Aggregator> Aggregator_var;

Aggregator::~Aggregator() throw ()
{
}

void
Aggregator::on_data_ready(int /*fd*/, std::size_t fd_index,
  const char* str, std::size_t size) throw ()
{
  buffers_[fd_index].append(str, size);
}

void
Aggregator::report_error(Severity /*severity*/,
  const String::SubString& description,
  const char* /*error_code*/) throw ()
{
  std::cerr << description << std::endl;
}

std::string
Aggregator::buffer(int index) throw (eh::Exception)
{
  return buffers_[index];
}

void
pipe_test(const char* command, const char* argv1, const char* argv2,
  const char* out, const char* err, const char* pipe)
{
  const char* const ARGV[] = { command, argv1, argv2, 0 };
  const int DESCRIPTORS[] = { STDOUT_FILENO, STDERR_FILENO };
  const char* EXPECTED[] = { out, err, pipe };
  const int REDIRECT_DESCRIPTORS[] = { STDIN_FILENO };

  Aggregator_var aggregator(new Aggregator);
  execute_and_listen(aggregator, command, const_cast<char* const*>(ARGV),
    sizeof(DESCRIPTORS) / sizeof(*DESCRIPTORS), DESCRIPTORS,
    sizeof(REDIRECT_DESCRIPTORS) / sizeof(*REDIRECT_DESCRIPTORS),
    REDIRECT_DESCRIPTORS, 4096, false, true);
  for (size_t i = 0; i < sizeof(EXPECTED) / sizeof(*EXPECTED); i++)
  {
    const std::string& str = aggregator->buffer(i);
    //std::cout << i << ": '" << str << "'" << std::endl;
    if (str.find(EXPECTED[i]) == std::string::npos)
    {
      std::cerr << "While executing '" << command << "' expected " << i <<
        " to contain '" << EXPECTED[i] << "' but it is '" << str << "'" <<
        std::endl;
    }
  }
}

void
pipes_test() throw (eh::Exception)
{
  pipe_test("/bin/echo", "-n", "Yes", "Yes", "", "");
  pipe_test("sh", "-c", "/bin/echo -n No >&2", "", "No", "");
  pipe_test("/bin/no_such_file", 0, 0, "", "", "execvp failed for");
}

MTAdapter::MTAdapter(const char* progname) throw ()
  : progname(progname)
{
}

void
MTAdapter::operator ()() throw (eh::Exception)
{
  TestTasker tasker;
  tasker.do_execute_and_listen_test(progname);
}


MPAdapter::MPAdapter(const char* progname, int threads, time_t interval,
  int limit) throw ()
  : progname(progname), threads(threads), interval(interval), limit(limit)
{
}

void
MPAdapter::operator ()() throw (eh::Exception)
{
  MTAdapter adapter(progname);
  TestCommons::MTTester<MTAdapter&> tester(adapter, threads);
  tester.run(threads, interval, limit);
}

int
main(int argc, char* argv[])
{
  try
  {
    if (argc >= 2) // child process
    {
      do_execute_and_listen_test_child_code(argv);
      return 0;
    }
    std::cout << "DescriptorListener tests started.." << std::endl;

#if 0
    {
      MPAdapter adapter(argv[0], 20, 5, 500);
      TestCommons::mp_test<MPAdapter&>(adapter, 2);
    }
#else
    TestTasker tasker;
    tasker.do_execute_and_listen_test(argv[0]);
    for (std::size_t bool_ = 0; bool_ < 2; ++bool_)
    {
      tasker.do_auto_test(bool_);
      tasker.do_overflow_test(bool_);
      tasker.do_closed_descriptors_test(bool_);
    }
    std::cout << "Test complete" << std::endl;

    pipes_test();
#endif
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "exception: " << ex.what() << std::endl;
    throw;
  }
  catch (...)
  {
    std::cerr << "FAIL: unknown exception raised" << std::endl;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Details implementations

//
// class TestTasker
//

TestTasker::TestTasker() throw (eh::Exception)
  : callback_(new DescriptorListenerCallbackTester)
{
}

TestTasker::~TestTasker() throw ()
{
}

void
TestTasker::spawn_descriptors_(Descriptors& read_descriptors,
  Descriptors& write_descriptors,
  std::size_t count)
  throw (eh::Exception)
{
  int pipe_[2];

  read_descriptors.reset(count);
  write_descriptors.reset(count);

  for (std::size_t i = 0; i < count; ++i)
  {
    if (pipe(pipe_) == -1)
    {
      eh::throw_errno_exception<Exception>("pipe");
    }

    read_descriptors[i] = pipe_[0];
    write_descriptors[i] = pipe_[1];
  }
}

//
// class Writer
//

Writer::Writer(Descriptors& dscs, const char* msg) throw ()
  : write_pipes_(dscs),
    MSG_(msg),
    multiplexor_(0)
{
}

void
Writer::operator()() throw (eh::Exception)
{
  std::size_t my_index = __gnu_cxx::__exchange_and_add(&multiplexor_, 1);

  const std::size_t PORTION = 3;
  const char *str = MSG_.c_str();
  std::size_t length = MSG_.size();

  for (std::size_t i = 0; i < length / PORTION; ++i, str+=PORTION)
  {
    write(write_pipes_[my_index], str, PORTION);
  }
  if (length % PORTION)
  {
    write(write_pipes_[my_index], str, length % PORTION);
  }
}

void
Writer::reset() throw ()
{
  multiplexor_ = 0;
}

//
// class DescriptorListenerCallbackTester
//

DescriptorListenerCallbackTester::DescriptorListenerCallbackTester()
  throw ()
  : close_counter_(0),
    checking_descriptor_(0),
    full_lines_test_(0)
{
}

void
DescriptorListenerCallbackTester::on_data_ready(
  int fd, std::size_t /*fd_index*/, const char* str,
  std::size_t size) throw ()
{
  {
    Sync::PosixGuard lock(mutex_);
    if (!checking_descriptor_)
    {
      checking_descriptor_ = fd;
      ready_data_.clear();
    }
    if (checking_descriptor_ == fd)
    {
      ready_data_.append(str, size);
    }
  }
  if (full_lines_test_)
  {
    if (size != FULL_LINES_TEST_BUF_SIZE && str[size - 1] != '\n')
    {
      std::cerr << "Test Error: divided without '\\n'" << std::endl;
    }
    else
    {
      const char* ptr = static_cast<const char*>(memchr(str, '\n', size - 1));
      if (ptr)
      {
        std::cerr << "Test Error: excess carrying '\\n'"
        << "DATA:" << str << std::endl;
        std::cerr << "Position=" << ptr - str << std::endl;
      }
    }
  }
}

void
DescriptorListenerCallbackTester::on_closed(int fd, std::size_t /*fd_index*/,
  int error) throw ()
{
  __gnu_cxx::__atomic_add(&close_counter_, 1);
  std::cout << "on_closed: " << error << " fd=" << fd << std::endl;
}

void
DescriptorListenerCallbackTester::on_all_closed() throw ()
{
  std::cout << "Deactivation by callback.." << std::endl;
  Generics::ActiveDescriptorListenerCallback::on_all_closed();
  Generics::DescriptorListenerCallback::on_all_closed();
}

void
DescriptorListenerCallbackTester::report_error(Severity /*severity*/,
  const String::SubString& /*description*/,
  const char* /*error_code*/) throw ()
{
  std::cerr << "on_error: " << std::endl;
}

std::size_t
DescriptorListenerCallbackTester::get_and_reset_closed() throw ()
{
  std::size_t old = close_counter_;
  close_counter_ = 0;
  return old;
}

std::string
DescriptorListenerCallbackTester::received_data() const throw ()
{
  return ready_data_;
}

void
DescriptorListenerCallbackTester::reset() throw ()
{
  ready_data_.clear();
  close_counter_ = 0;
  checking_descriptor_ = 0;
  full_lines_test_ = false;
}

void
DescriptorListenerCallbackTester::set_full_lines_test(bool new_value) throw ()
{
  full_lines_test_ = new_value;
}

DescriptorListenerCallbackTester::~DescriptorListenerCallbackTester() throw ()
{
}
