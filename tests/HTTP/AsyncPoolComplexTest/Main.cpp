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



#include <vector>
#include "Tests.hpp"

const unsigned int TEST_DURATION = 35;
const unsigned int MAKING_REQUESTS_DURATION = 30;
const unsigned int TASK_RUNNER_THR_COUNT = 20;
const unsigned int TASKS_PER_TEST = 1;
const unsigned int FUNCTORS_PER_TASK = 4;
const char NOTIFICATION_MSG[] = "///////////////////////////////////////////////\n"
                                " TO KNOW MORE ABOUT SCENARIOUS RUN WITH \"help\""
                                "\n///////////////////////////////////////////////";

void usage()
{
  std::cout << "General words about EchoTest, NonExistanceTest,\n"
               "BadAddressTest, BadRespTest and InterruptTest:\n"
               "These tests must work properly, when constants\n"
               "are set as printed below:\n"
               "TEST_DURATION = 35 (Aggregate duration)\n"
               "MAKING_REQUESTS_DURATION = 30 (Duration of sending requests "
               "(for each test))\n"
               "TASK_RUNNER_THR_COUNT = 20\n"
               "TASKS_PER_TEST = 1 (Num of threads responsible for requests's "
               "sending (for each test))\n"
               "FUNCTORS_PER_TASK = 4 (Num of functors responsible for requests's "
               "sending in each thread (for each test))\n"
               "Current values are:\n"
            << "TEST_DURATION = " << TEST_DURATION
            << "\nMAKING_REQUESTS_DURATION = " << TEST_DURATION
            << "\nTASK_RUNNER_THR_COUNT = " << TEST_DURATION
            << "\nTASKS_PER_TEST = " << TEST_DURATION
            << "\nFUNCTORS_PER_TASK = " << FUNCTORS_PER_TASK
            << "\n\n"
            << EchoTest::usage() << '\n'
            << NonExistanceTest::usage() << '\n'
            << BadAddressTest::usage() << '\n'
            << BadRespTest::usage() << '\n'
            << InterruptTest::usage() << std::endl;
}

int
main(int argc, char* argv[])
{
  HTTP::HttpActiveInterface_var pool;
  Generics::TaskRunner_var tests_runner;

  try
  {
    if (argc != 1 && std::string(argv[1]) == "help")
    {
      usage();
      return 0;
    }

    typedef ReferenceCounting::List<CTTestInterface_var> Tests;

    SimplePolicy* policy_ptr = new SimplePolicy;
    HTTP::PoolPolicy_var policy(policy_ptr);
    tests_runner = new Generics::TaskRunner(policy_ptr, TASK_RUNNER_THR_COUNT);
    tests_runner->activate_object();
    pool = CreatePool(policy.in(), tests_runner);
    pool->activate_object();
    HTTP::HttpInterface_var spool(HTTP::CreateSyncHttp());

    Tests tests;

    Sync::Semaphore finish_semaphore(0);

    tests.push_back(CTTestInterface_var(
      new EchoTest(finish_semaphore, pool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK, true)));
    tests.push_back(CTTestInterface_var(
      new EchoTest(finish_semaphore, spool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK, true)));

    tests.push_back(CTTestInterface_var(
      new NonExistanceTest(finish_semaphore, pool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK, true)));
    tests.push_back(CTTestInterface_var(
      new NonExistanceTest(finish_semaphore, spool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK, true)));

    tests.push_back(CTTestInterface_var(
      new BadAddressTest(finish_semaphore, pool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK)));
    tests.push_back(CTTestInterface_var(
      new BadAddressTest(finish_semaphore, spool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK)));

    tests.push_back(CTTestInterface_var(
      new BadRespTest(finish_semaphore, pool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK, true)));
    tests.push_back(CTTestInterface_var(
      new BadRespTest(finish_semaphore, spool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK, true)));

    tests.push_back(CTTestInterface_var(
      new InterruptTest(finish_semaphore, pool.in(),
        TEST_DURATION, MAKING_REQUESTS_DURATION, TASKS_PER_TEST,
        FUNCTORS_PER_TASK)));
    /*tests.push_back(CTTestInterface_var(
      new InterruptTest(spool.in(), TEST_DURATION,
        MAKING_REQUESTS_DURATION, TASKS_PER_TEST, FUNCTORS_PER_TASK)));*/

    for (Tests::iterator itor(tests.begin()); itor != tests.end(); ++itor)
    {
      tests_runner->enqueue_task(itor->in());
    }

    for (Tests::iterator itor(tests.begin()); itor != tests.end(); ++itor)
    {
      finish_semaphore.acquire();
    }

    pool->deactivate_object();
    pool->wait_object();
    tests_runner->deactivate_object();
    tests_runner->wait_object();

    std::cout << NOTIFICATION_MSG << "\n\n";
    for (Tests::iterator itor(tests.begin()); itor != tests.end(); ++itor)
    {
      std::cout << (*itor)->checkup_and_print_stat() << std::endl;
    }
  }
  catch (const eh::Exception& e)
  {
    if (pool.in())
    {
      pool->deactivate_object();
      pool->wait_object();
    }
    if (tests_runner.in())
    {
      tests_runner->deactivate_object();
      tests_runner->wait_object();
    }

    std::cerr << "[ERROR]: main(2). eh::Exception caught: " <<
      e.what() << std::endl;

    return 1;
  }

  return 0;
}
