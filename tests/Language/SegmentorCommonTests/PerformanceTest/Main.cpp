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



#include "TestClasses.hpp"

/*
#ifdef NLPIR_TEST
#include <Language/ChineeseSegmentor/NLPIR.hpp>
#endif
#include <Language/JapaneseSegmentor/Mecab.hpp>
#include <Language/KoreanSegmentor/Klt.hpp>

#ifdef MORAN_TEST
#include <Language/KoreanSegmentor/Moran.hpp>
#endif
*/

#include <Language/SegmentorManager/SegmentorManager.hpp>

#include <Language/GenericSegmentor/Polyglot.hpp>
#include <iostream>
#include <TestCommons/MTTester.hpp>

enum SegmentorIds
{
  COMPOSITE = 0,
  KLT,
  MECAB,
#ifdef MORAN_TEST
  MORAN,
#endif
#ifdef NLPIR_TEST
  NLPIR,
#endif
  POLYGLOT,
  SEGMENTORS_COUNT,
  ALL_SEGMENTORS
};

const char* SegmentorsNames[] =
{
  "Composite",
  "KLT",
  "MeCab",
#ifdef MORAN_TEST
  "Moran",
#endif
#ifdef NLPIR_TEST
  "NLPIR",
#endif
  "Polyglot",
  0,
  0
};

struct Config
{
  std::string data_dir;
  unsigned int rand_iteration_number;
  size_t sequence_len;
  bool quick;
  SegmentorIds segms;
  size_t threads_count;
  bool check_all_transforms;
  bool read_from_cin;
  bool test_compound;
  bool no_errors_in_cmd;

  Config();
};

const char* RealPhrasesFileNames[] =
{
  "korean_phrases_01.bz2",
  "japanese_phrases_01.bz2",
  "de_book_01.bz2",
#if 0
  "de_book_02.bz2",
  "de_book_03.bz2",
  "de_book_04.bz2",
  "de_book_05.bz2",
  "de_book_06.bz2",
  "de_book_07.bz2",
  "de_book_08.bz2",
  "de_book_09.bz2",
  "de_book_10.bz2",
  "de_book_11.bz2",
#endif
  "en_book_01.bz2",
  "rus_book_01.bz2",
#if 0
  "rus_book_03.bz2",
  "rus_book_04.bz2",
  "rus_book_05.bz2",
  "rus_book_06.bz2",
#endif
  "chineese_book_01.bz2",
};

const size_t ThreadsCount[] =
{
  1,
  2,
  5
};


const char USAGE[] =
  "Usage: <path>/SegmentorPerformanceTest [-i <integer>] [-t <integer>] "
    "[-qac[m | l]] [-s <name>] [data_dir_name]\n"
  "  -i: specify max iterations number (max number of segmentation/put_spaces "
    "calls for each segmentor) (default: 1 000 000)\n"
  "  -t: specify number of threads (max number of concurrent segmentation/put_spaces "
    "calls for each segmentor) (default: 1, 2, 5)\n"
  "  -q: quick test (iterations number=1000, threads number<=2)\n"
  "  -a: print all transforms (segmented, segmented and partially dropped, "
    "partially dropped, fully dropped)\n"
  "  -c: read from cin\n"
  "  -m: test complex segmentors: polyglot, composite (members: "
    "mecab, klt)\n"
  "  -s: run only \"name\" segmentor, \"name\" can be the one of \"klt\", \"mecab\", "
#ifdef MORAN_TEST
    "\"moran\", "
#endif
#ifdef NLPIR_TEST
    "\"nlpir\", "
#endif
    "\"polyglot\", \"composite\" "
    "(default: all segmentors are run)\n"
  "  data_dir_name: specify data directory name (with trailing \"/\")\n"
  "Example: ./SegmentorPerformanceTest "
    "~/projects/unixcommons/trunk/tests/Language/Data/\n";


typedef Language::Segmentor::SegmentorInterface_var SegmentorInterface_var;
typedef Language::Segmentor::SegmentorInterface SegmentorInterface;


void init_segmentors(Segmentors& segms, const Config& conf);

void print_test_stats(const Segmentors& segms,
  const CommonFunctor& fun, const Config& conf);

void print_stats(std::ostream& out, const CommonFunctor& fun,
  const SegmentorInterface* id, const Config& conf);

const Config parse_cmd(int argc, char **argv);

void parse_input_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num);
void parse_files_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num);
void random_ascii_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num);
void random_nonstandart_utf8_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num);
void random_utf8_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num);


int main(int argc, char **argv)
{
  try
  {
    const Config CONF = parse_cmd(argc, argv);
    if (!CONF.no_errors_in_cmd)
    {
      return -1;
    }

    Segmentors segms(SEGMENTORS_COUNT,
      Language::Segmentor::SegmentorInterface_var());
    init_segmentors(segms, CONF);

    size_t i_end = sizeof(ThreadsCount) / sizeof(ThreadsCount[0]);
    for (size_t i = 0; i < i_end; ++i)
    {
      const size_t ACTUAL_THREADS_COUNT =
        CONF.threads_count ? CONF.threads_count : ThreadsCount[i];
      if (CONF.quick && ACTUAL_THREADS_COUNT > 2)
      {
        continue;
      }

      const int CUR_RANDOM_ITERATION_NUMBER =
        CONF.rand_iteration_number / ACTUAL_THREADS_COUNT;

      if (CONF.read_from_cin)
      {
        parse_input_test(segms, CONF, ACTUAL_THREADS_COUNT,
          CUR_RANDOM_ITERATION_NUMBER);
        break;
      }

      if (!CONF.data_dir.empty())
      {
        parse_files_test(segms, CONF, ACTUAL_THREADS_COUNT,
          CUR_RANDOM_ITERATION_NUMBER);
      }

      random_ascii_test(segms, CONF, ACTUAL_THREADS_COUNT,
        CUR_RANDOM_ITERATION_NUMBER);

      random_nonstandart_utf8_test(segms, CONF, ACTUAL_THREADS_COUNT,
        CUR_RANDOM_ITERATION_NUMBER);

      random_utf8_test(segms, CONF, ACTUAL_THREADS_COUNT,
        CUR_RANDOM_ITERATION_NUMBER);

      if (CONF.threads_count)
      {
        break;
      }
    }
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "eh::Exception caught: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}

void
print_stats(std::ostream& out, const CommonFunctor& fun,
  const SegmentorInterface* id, const Config& conf)
{
  if (!id)
  {
    return;
  }

  const CheckSegmentResult::SegmentorStats* const stats =
    fun.find_segmentor_stats(id);

  out << "  Total processings: " << fun.get_segmentations_count()
      << " (ave sequences len: " << fun.get_average_seqs_length()
      << ")\n  segmented w/o partially dropping: " 
        << stats->segmented_count
      << "\n  segmented with partially dropping: " 
        << stats->segmented_dropped_count
      << "\n  partially dropped w/o segmenting: " 
        << stats->partially_dropped_count
      << "\n  fully dropped: " << stats->dropped_count
      << "\n  exceptions thrown: " << stats->exceptions_count
      << "\n  Total processing time: " << stats->processing_time
      << ", average: " << (!fun.get_segmentations_count() ?
        Generics::Time::ZERO :
        stats->processing_time / fun.get_segmentations_count())
      << '\n';

  if (conf.check_all_transforms)
  {
    {
      out << "\n\n  ======= SEGMENTATIONS =======\n\n";
      CheckSegmentResult::Transforms::const_iterator it =
        stats->segment_transforms.begin();
      for (; it != stats->segment_transforms.end(); ++it)
      {
        out << it->from << " => " << it->to << '\n';
      }
      out << "\n\n\n";
    }
  
    {
      out << "  ======= SEGMENTATIONS + DROPS =======\n\n";
      CheckSegmentResult::Transforms::const_iterator it =
        stats->segment_drop_transforms.begin();
      for (; it != stats->segment_drop_transforms.end(); ++it)
      {
        out << it->from << " => " << it->to << '\n';
      }
      out << "\n\n\n";
    }
  
    {
      out << "  ======= DROPS =======\n\n";
      CheckSegmentResult::Transforms::const_iterator it =
        stats->drop_transforms.begin();
      for (; it != stats->drop_transforms.end(); ++it)
      {
        out << it->from << " => " << it->to << '\n';
      }
      out << "\n\n\n";
    }
  
    {
      out << "  ======= PARTIALLY DROPS =======\n\n";
      CheckSegmentResult::Transforms::const_iterator it =
        stats->partially_drop_transforms.begin();
      for (; it != stats->partially_drop_transforms.end(); ++it)
      {
        out << it->from << " => " << it->to << '\n';
      }
      out << "\n\n\n";
    }
  }
}

void
init_segmentors(Segmentors& segms, const Config& conf)
{
  /*
  if (conf.segms == ALL_SEGMENTORS || conf.segms == KLT ||
      conf.test_compound)
  {
    segms[KLT] = new Language::Segmentor::Korean::KltSegmentor(
      "/opt/KLT/hdic/KLT2000.ini", "-p");
  }

  if (conf.segms == ALL_SEGMENTORS || conf.segms == MECAB ||
      conf.test_compound)
  {
    segms[MECAB] = new Language::Segmentor::Japanese::MecabSegmentor(
      "/usr/etc/mecabrc");
  }

#ifdef MORAN_TEST
  if (conf.segms == ALL_SEGMENTORS || conf.segms == MORAN ||
      conf.test_compound)
  {
    segms[MORAN] = new Language::Segmentor::Korean::MoranSegmentor(
      "/opt/Moran/dic/moran.dbs");
  }
#endif

#ifdef NLPIR_TEST
  if (conf.segms == ALL_SEGMENTORS || conf.segms == NLPIR ||
      conf.test_compound)
  {
    segms[NLPIR] = new Language::Segmentor::Chineese::NlpirSegmentor();
  }
#endif
  */

  if (!conf.quick && (conf.segms == ALL_SEGMENTORS ||
      conf.segms == POLYGLOT))
  {
    segms[POLYGLOT] = new Language::Segmentor::PolyglotSegmentor(
      "/opt/oix/polyglot/dict/");
  }

  /*
  if (conf.test_compound)
  {
    if (conf.segms == ALL_SEGMENTORS || conf.segms == COMPOSITE)
    {
      Language::Segmentor::CompositeSegmentor_var composite(
        new Language::Segmentor::CompositeSegmentor);
      composite->add_segmentor(segms[KLT]);
      composite->add_segmentor(segms[MECAB]);
      segms[COMPOSITE] = composite;
    }

    segms[KLT].reset();
    segms[MECAB].reset();
#ifdef MORAN_TEST
    segms[MORAN].reset();
#endif
  }
  */
}

void
print_test_stats(const Segmentors& segms,
  const CommonFunctor& fun, const Config& conf)
{
  std::cout << "\n Test results:\n";

  for (int i = 0; i < SEGMENTORS_COUNT; i++)
  {
    if (conf.segms != ALL_SEGMENTORS && conf.segms != i)
    {
      continue;
    }
    switch (i)
    {
    case COMPOSITE:
      if (!conf.test_compound)
      {
        continue;
      }
      break;
    case KLT:
    case MECAB:
#ifdef MORAN_TEST
    case MORAN:
#endif
      if (conf.test_compound)
      {
        continue;
      }
      break;
    case POLYGLOT:
      if (conf.quick)
      {
        continue;
      }
      break;
    }
    std::cout << "\n  " << SegmentorsNames[i] << " results:\n";
    print_stats(std::cout, fun, segms[i], conf);
  }
}

const Config parse_cmd(int argc, char **argv)
{
  Config res;
  int opt = -1;
  while ((opt = ::getopt(argc, argv, "i:qs:t:acml")) != -1)
  {
    switch (opt)
    {
      case 'i':
        {
          Stream::Parser istr(optarg);
          istr >> res.rand_iteration_number;

          if (istr.bad() || istr.fail())
          {
            std::cerr << "Error: Incorrect value of \"i\" parameter.\n\n"
                      << USAGE
                      << std::endl;
            return res;
          }
        }
        break;
      case 't':
        {
          Stream::Parser istr(optarg);
          istr >> res.threads_count;

          if (istr.bad() || istr.fail())
          {
            std::cerr << "Error: Incorrect value of \"t\" parameter.\n\n"
                      << USAGE
                      << std::endl;
            return res;
          }
        }
        break;
      case 'q':
        res.rand_iteration_number = 1000;
        res.quick = true;
        break;
      case 'a':
        res.check_all_transforms = true;
        break;
      case 'c':
        res.read_from_cin = true;
        break;
      case 'm':
        res.test_compound = true;
        break;
      case 's':
        {
          std::string val(optarg);
          /*
          if (val == "klt")
          {
            res.segms = KLT;
          }
          else if (val == "mecab")
          {
            res.segms = MECAB;
          }
#ifdef MORAN_TEST
          else if (val == "moran")
          {
            res.segms = MORAN;
          }
#endif
#ifdef NLPIR_TEST
          else if (val == "nlpir")
          {
            res.segms = NLPIR;
          }
#endif
          else */
          if (val == "polyglot")
          {
            res.segms = POLYGLOT;
          }
          /*
          else if (val == "composite")
          {
            res.segms = COMPOSITE;
            res.test_compound = true;
          }
          */
          else
          {
            std::cerr << "Error: Incorrect value of \"s\" parameter: unknown "
                         "segmentor name: \"" << val << "\"\n\n"
                      << USAGE
                      << std::endl;
            return res;
          }
        }
        break;
      default:
        std::cerr << "Error: Unknown parameter \"" 
                  << static_cast<char>(opt) << "\".\n\n"
                  << USAGE << std::endl;
        return res;
    }
  }

  if (argc > optind)
  {
    res.data_dir = argv[optind];
  }

  res.no_errors_in_cmd = true;
  return res;
}

Config::Config():
  data_dir(),
  rand_iteration_number(1000000),
  sequence_len(20),
  quick(false),
  segms(ALL_SEGMENTORS),
  threads_count(0),
  check_all_transforms(false),
  read_from_cin(false),
  test_compound(false),
  no_errors_in_cmd(false)
{
}

void
parse_input_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t /*iteration_num*/)
{
  std::cout << "\nPhrases from input test ("
            << threads_num << " thread(s)) started " << std::endl;

  ParseStdIn fun(segms, conf.check_all_transforms);
  TestCommons::MTTester<ParseStdIn&> mt_tester(fun, threads_num);
  mt_tester.run(threads_num, 0, threads_num);

  print_test_stats(segms, fun, conf);

  std::cout << "\nPhrases from input test ("
            << threads_num << " thread(s)) finished " << std::endl;
}

void
parse_files_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num)
{
  std::cout << "\nReal phrases test ("
            << threads_num << " thread(s)) started " << std::endl;

  size_t files_count = sizeof(RealPhrasesFileNames) /
    sizeof(RealPhrasesFileNames[0]);
  for (size_t j = 0; j < files_count; ++j)
  {
    std::string full_name = conf.data_dir + RealPhrasesFileNames[j];
    std::cout << "\nProcessed file: \"" << full_name << " " <<
      threads_num << " thread(s)";
    ParseFile fun(full_name.c_str(), segms, (conf.quick? iteration_num: -1),
      conf.check_all_transforms);
    TestCommons::MTTester<ParseFile&> mt_tester(fun, threads_num);
    mt_tester.run(threads_num, 0, threads_num);

    print_test_stats(segms, fun, conf);
  }

  std::cout << "\nReal phrases test ("
            << threads_num << " thread(s)) finished " << std::endl;
}

void
random_ascii_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num)
{
  std::cout << "\nRandom ASCII test ("
            << threads_num << " thread(s)) started " << std::endl;

  RandomAsciiSegmentFunctor fun(segms, iteration_num, conf.sequence_len,
    conf.check_all_transforms);
  TestCommons::MTTester<RandomAsciiSegmentFunctor&> mt_tester(
    fun, threads_num);
  mt_tester.run(threads_num, 0, threads_num);

  print_test_stats(segms, fun, conf);

  std::cout << "\nRandom ASCII test ("
            << threads_num << " thread(s)) finished " << std::endl;
}

void
random_nonstandart_utf8_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num)
{
  std::cout << "\nRandom OutOfStandartUtf8 test ("
            << threads_num << " thread(s)) started " << std::endl;

  RandomUtf8SegmentFunctor fun(segms, iteration_num, conf.sequence_len,
    conf.check_all_transforms, false);
  TestCommons::MTTester<RandomUtf8SegmentFunctor&> mt_tester(
    fun, threads_num);
  mt_tester.run(threads_num, 0, threads_num);

  print_test_stats(segms, fun, conf);

  std::cout << "\nRandom OutOfStandartUtf8 test ("
            << threads_num << " thread(s)) finished " << std::endl;
}

void
random_utf8_test(const Segmentors& segms, const Config& conf,
  size_t threads_num, size_t iteration_num)
{
  std::cout << "\nRandom Utf8 test ("
            << threads_num << " thread(s)) started " << std::endl;

  RandomUtf8SegmentFunctor fun(segms, iteration_num,
    conf.sequence_len, conf.check_all_transforms);
  TestCommons::MTTester<RandomUtf8SegmentFunctor&> mt_tester(
    fun, threads_num);
  mt_tester.run(threads_num, 0, threads_num);

  print_test_stats(segms, fun, conf);

  std::cout << "\nRandom Utf8 test ("
            << threads_num << " thread(s)) finished " << std::endl;
}
