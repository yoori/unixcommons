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
#include <iomanip>

#include <eh/Exception.hpp>
#include <Generics/Time.hpp>
#include <Generics/Rand.hpp>

#include <TestCommons/MTTester.hpp>


struct EmptyTimeFunctor
{
  EmptyTimeFunctor(): error(false) {};

  /* non thread safe functor */
  void operator ()() const
  {
    Generics::Timer timer;

    timer.start();
    timer.stop();

    Generics::Time el_time = timer.elapsed_time();

    if (el_time < Generics::Time(0))
    {
      error = true;
      std::cerr << "received negative time metering: " << el_time << std::endl;
    }
  }

  mutable bool error;
};

struct TimeFunctor
{
  /* non thread safe functor */
  void operator ()() const
  {
    Generics::Timer timer;

    timer.start();
    Generics::Time::get_time_of_day();
    timer.stop();

    Generics::Time el_time = timer.elapsed_time();

    if (el_time > max_time)
    {
      max_time = el_time;
    }
  }

  mutable Generics::Time max_time;
};

bool
check(Generics::Time& time, const char* expected_res, const char* operation)
  throw (eh::Exception);


void
check_manipulations() throw (eh::Exception)
{
  //
  // -1 < time < 1
  //

  Generics::Time time(0, 234567);
  check(time *= 1, "0:234567", "0:234567 * 1");
  check(time *= -1, "-0:234567", "0:234567 * -1");
  check(time *= 1, "-0:234567", "-0:234567 * 1");
  check(time *= -1, "0:234567", "-0:234567 * -1");

  check(time -= 1, "-0:765433", "0:234567 - 1");

  check(time /= 1, "-0:765433", "-0:765433 / 1");
  check(time /= -1, "0:765433", "-0:765433 / -1");
  check(time /= 1, "0:765433", "0:765433 / 1");
  check(time /= -1, "-0:765433", "0:765433 / -1");

  //
  // -1 > time || time > 1 (short numbers)
  //

  time.set(5, 234567);

  check(time *= 1, "5:234567", "5:234567 * 1");
  check(time *= -1, "-5:234567", "5:234567 * -1");
  check(time *= 1, "-5:234567", "-5:234567 * 1");
  check(time *= -1, "5:234567", "-5:234567 * -1");

  check(time /= 1, "5:234567", "5:234567 / 1");
  check(time /= -1, "-5:234567", "5:234567 / -1");
  check(time /= 1, "-5:234567", "-5:234567 / 1");
  check(time /= -1, "5:234567", "-5:234567 / -1");

  //
  // -1 > time || time > 1 (long numbers)
  //

  time.set(3000, 234567);
  check(time /= 9000, "0:333359", "3000:234567 / 9000");
  time.set(3000, 234567);
  check(time /= -9000, "-0:333359", "3000:234567 / -9000");
  time.set(3000, 234567);
  time *= -1;
  check(time /= 9000, "-0:333359", "-3000:234567 / 9000");
  time.set(3000, 234567);
  time *= -1;
  check(time /= -9000, "0:333359", "-3000:234567 / -9000");

  time.set(3000, 234567);
  check(time *= 3000, "9000703:701000", "3000:234567 * 3000");
  time.set(3000, 234567);
  check(time *= -3000, "-9000703:701000", "3000:234567 * -3000");
  time.set(3000, 234567);
  time *= -1;
  check(time *= 3000, "-9000703:701000", "-3000:234567 * 3000");
  time.set(3000, 234567);
  time *= -1;
  check(time *= -3000, "9000703:701000", "-3000:234567 * -3000");
}

bool
check(Generics::Time& time, const char* expected_res, const char* operation)
  throw (eh::Exception)
{
  std::string expected(expected_res);
  expected += " (sec:usec)";

  std::ostringstream ostr;
  ostr << time;
  if (ostr.str() != expected)
  {
    std::cerr << ostr.str()
              << ": wrong (expected: " << expected_res
              << ") //operation: " << operation
              << std::endl;
    return false;
  }
  else
  {
    std::cout << ostr.str() << ": right //operation: " << operation
              << std::endl;
    return true;
  }
}

void
check_format() throw (eh::Exception)
{
  Generics::ExtendedTime time(2345, 10, 12, 13, 24, 56, 89987);
  const char FORMAT[] =
    "%H:%M:%S.%q %d.%m.%Y %F %T %d.%B.%Y %H:%M:%S.%q %z";
  std::string formatted = time.format(FORMAT);
  const std::string EXPECTED1(
    "13:24:56.089987 12.10.2345 2345-10-12 13:24:56 "
      "12.October.2345 13:24:56.089987 +0000");
  const std::string EXPECTED2(
    "13:24:56.089987 12.10.2345 2345-10-12 13:24:56 "
      "12.October.2345 13:24:56.089987 +0300");
  if (formatted != EXPECTED1)
  {
    std::cerr << "Invalid Generics::ExtendedTime::format() behaviour: "
      "expected '" << EXPECTED1 << "' but got '" << formatted << "'" <<
      std::endl;
  }
  time.timezone = Generics::Time::TZ_LOCAL;
  formatted = time.format(FORMAT);
  if (formatted != EXPECTED2)
  {
    std::cerr << "Invalid Generics::ExtendedTime::format() behaviour: "
      "expected '" << EXPECTED2 << "' but got '" << formatted << "'" <<
      std::endl;
  }
}

void
check_set() throw (eh::Exception)
{
  Generics::ExtendedTime time(2345, 10, 12, 13, 24, 56, 89987);
  const char FORMAT[] =
    "%H:%M:%S.%q %d.%m.%Y %d.%B.%Y %H:%M:%S.%q";
  std::string formatted = time.format(FORMAT);
  Generics::Time t1(formatted, FORMAT, false);
  Generics::Time t2(formatted, FORMAT, true);
  if (time != t1 || time != t2)
  {
    std::cerr << "Invalid Generics::Time::set() behaviour: "
      "expected " << time << " but got " << t1 << " and " << t2 <<
      std::endl;
  }
}

void
check_input() throw (eh::Exception)
{
  for (int i = 0; i < 1000; i++)
  {
    Generics::Time any(Generics::safe_rand(1000000000) - 500000000,
      Generics::safe_rand(Generics::Time::USEC_MAX));
    std::stringstream str;
    str << any;
    Generics::Time got;
    str >> got;
    if (!str.eof() || any != got)
    {
      std::cerr << "check_input(): failed to input timestamp " << any <<
        std::endl;
    }
  }
}

void
check_output() throw (eh::Exception)
{
  const Generics::Time TEST_TIME(
    String::SubString("20110405141336"), "%Y%m%d%H");
  const char VALID_RESULT[] =
    "......................1302012000:000000 (sec:usec)TEXT";

  Stream::Error ostr;
  ostr << std::setw(50) << std::setfill('.') << TEST_TIME << "TEXT";
  if (ostr.str() != VALID_RESULT)
  {
    std::cerr << "FAIL: Generics::Time incorrectly formatted\n"
      << ostr.str() << std::endl;
  }
}

static const int DAYS[12] =
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const tm ZTM = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const Generics::ExtendedTime ZET(ZTM, 0, Generics::Time::TZ_GMT);

void
rand_time(Generics::ExtendedTime& et) throw ()
{
  et.tm_year = Generics::safe_rand(70, 199);
  et.tm_mon = Generics::safe_rand(12);
  et.tm_mday = Generics::safe_rand(DAYS[et.tm_mon] +
    !((et.tm_year & 3) || et.tm_mon != 2)) + 1;
  et.tm_hour = Generics::safe_rand(24);
  et.tm_min = Generics::safe_rand(60);
  et.tm_sec = Generics::safe_rand(60);
}

void
check_time_to_gm() throw (eh::Exception)
{
  for (int i = 0; i < 1000; i++)
  {
    Generics::ExtendedTime et(ZET);
    rand_time(et);
    time_t res, ref;
    res = Generics::gm_to_time(et);
    ref = timegm(&et);
    if (res != ref)
    {
      std::cerr << et << " produced " << res << " instead of " <<
        ref << "\n";
    }
  }
}

void
check_gm_to_time() throw (eh::Exception)
{
  for (int i = 0; i < 1000; i++)
  {
    Generics::ExtendedTime et(ZET);
    rand_time(et);
    time_t time = Generics::gm_to_time(et);
    Generics::ExtendedTime res(ZET), ref(ZET);
    Generics::time_to_gm(time, res);
    gmtime_r(&time, &ref);
    if (res.tm_sec != ref.tm_sec || res.tm_min != ref.tm_min ||
      res.tm_hour != ref.tm_hour || res.tm_mday != ref.tm_mday ||
      res.tm_mon != ref.tm_mon || res.tm_year != ref.tm_year ||
      res.tm_yday != ref.tm_yday || res.tm_wday != ref.tm_wday)
    {
      std::cerr << time << " (" << et << ") produced " << res <<
        " instead of " << ref << "\n";
    }
  }
}

int
main()
{
  try
  {
    std::cout << "TimeManips test started" << std::endl;
    check_manipulations();
    check_format();
    check_set();
    check_input();
    check_output();
    check_time_to_gm();
    check_gm_to_time();


    {
      /* meter time perf */
      TimeFunctor functor;
      TestCommons::MTTester<TimeFunctor&> mt_tester(functor, 1);
      mt_tester.run(100, 100);
      std::cout
        << "performance metering: max-time = "
        << functor.max_time << std::endl;
      if (functor.max_time >= Generics::Time(1) / 100)
      {
        std::cerr
          << "max time of 2*gettimeofday execution is big (more than 0.01): "
          << functor.max_time
          << std::endl;
      }
    }

    {
      /* check timer monotonic */
      EmptyTimeFunctor functor;
      TestCommons::MTTester<EmptyTimeFunctor&> mt_tester(functor, 1);
      mt_tester.run(100, 100);
      if (functor.error)
      {
        std::cerr
          << "found negative time metering (gettimeofday isn't monotonic)."
          << std::endl;
      }
    }

    std::cout << "All checks finished" << std::endl;
    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception caught: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "main: unknown exception caught: " << std::endl;
  }

  return -1;
}
