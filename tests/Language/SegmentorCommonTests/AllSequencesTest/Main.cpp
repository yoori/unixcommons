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
#include <fstream>
#include <sstream>
#include <Generics/Function.hpp>
#include <Stream/MemoryStream.hpp>
#include <Generics/AppUtils.hpp>
#include <TestCommons/MTTester.hpp>

#include <Language/ChineeseSegmentor/NLPIR.hpp>
#include <Language/JapaneseSegmentor/Mecab.hpp>
#include <Language/KoreanSegmentor/Klt.hpp>
#ifdef MORAN_TEST
 #include <Language/KoreanSegmentor/Moran.hpp>
#endif
#include <Language/SegmentorManager/SegmentorManager.hpp>
#include <Language/GenericSegmentor/Polyglot.hpp>

namespace
{
  const char USAGE_MSG[] =
    "Usage:\n"
    "  AllSequencesTest [-hasnrepoiq] "
    "[--segmentor=\"name\"] [--threads-num=<positive number>]"
    " [--lower-border=<positive number>] [--upper-border=<positive number>]"
    " [--if=\"input fule\"]\n"
    "  h: this message\n"
    "  a: all sequences of selected length; with \"o\" option, "
    "correct borders: 1-1; "
    "the default behaviour\n"
    "  s: standard utf8 symbols only; with \"o\" option, "
    "correct borders: 1-4; "
    "default: off\n"
    "  n: non-standard + standard utf8 symbols only; "
    "with \"o\" option, correct "
    "borders: 1-6; default: off\n"
    "  r: test parsing occurs for standard utf8 symbols\n"
    "  e: test parsing by template file with strings [word_from words_to]\n"
    "  es: test parsing (segmentation method) "
    "by template file with strings [word_from words_to]\n"
    "  p: print transforms of standard utf8 symbols (default: off)\n"
    "  q: quick test (turning off the polyglot segmentor); default: off\n"
    "  segmentor: run only segmentor \"name\" "
    "(can be the one of \"klt\", \"mecab\""
#ifdef MORAN_TEST
    ", \"moran\""
#endif
    ", \"nlpir\", \"polyglot\",\n"
    "   \"composite\" [members of complex segmentors: \"klt\", \"mecab\"]);\n"
    "   default: all segmentors, besides complex are used.\n"
    "  threads-num: number of threads run identical test case; default: 1\n"
    "  lower-border: initial length of sequences; default: 1\n"
    "  upper-border: final length of sequences; default: 3\n"
    "  if: input file;\n"
    "  ef: error file;\n";

  /** Klt segmentor constructor*/
  /*
  Language::Segmentor::SegmentorInterface_var
  get_klt_segmentor()
    throw (eh::Exception)
  {
    static const char* cfg1 = "/opt/KLT/hdic/KLT2000.ini";
    static const char* cfg2 = "-p";
    Language::Segmentor::SegmentorInterface_var klt(
      new Language::Segmentor::Korean::KltSegmentor(cfg1, cfg2));
    return klt;
  }
  */
#ifdef MORAN_TEST
  /** Moran segmentor constructor*/
  /*
  Language::Segmentor::SegmentorInterface_var
  get_moran_segmentor()
    throw (eh::Exception)
  {
    static const char* cfg = "/opt/Moran/dic/moran.dbs";
    Language::Segmentor::SegmentorInterface_var moran(
      new Language::Segmentor::Korean::MoranSegmentor(cfg));
    return moran;
  }
  */
#endif

  /** MeCab segmentor constructor*/
  /*
  Language::Segmentor::SegmentorInterface_var
  get_mecab_segmentor()
    throw (eh::Exception)
  {
    static const char* cfg = "/usr/etc/mecabrc";
    Language::Segmentor::SegmentorInterface_var mecab(
      new Language::Segmentor::Japanese::MecabSegmentor(cfg));
    return mecab;
  }
  */
  /** NLPIR segmentor constructor*/
  /*
  Language::Segmentor::SegmentorInterface_var
  get_nlpir_segmentor()
    throw (eh::Exception)
  {
    Language::Segmentor::SegmentorInterface_var nlpir(
      new Language::Segmentor::Chineese::NlpirSegmentor());
    return nlpir;
  }
  */
  /** Polyglot segmentor constructor*/
  Language::Segmentor::SegmentorInterface_var
  get_polyglot_segmentor()
    throw (eh::Exception)
  {
    static const char* cfg = "/opt/oix/polyglot/dict/";
    Language::Segmentor::SegmentorInterface_var polyglot(
      new Language::Segmentor::PolyglotSegmentor(cfg));
    return polyglot;
  }

  /** Composite segmentor constructor*/
  /*
  Language::Segmentor::SegmentorInterface_var
  get_composite_segmentor()
    throw (eh::Exception)
  {
    Language::Segmentor::CompositeSegmentor_var composite(
      new Language::Segmentor::CompositeSegmentor);
    {
      Language::Segmentor::SegmentorInterface_var klt =
        get_klt_segmentor();
      composite->add_segmentor(klt);
    }
    {
      Language::Segmentor::SegmentorInterface_var mecab =
        get_mecab_segmentor();
      composite->add_segmentor(mecab);
    }
    return Language::Segmentor::SegmentorInterface_var(composite);
  }
  */

  /**
   * Adapter with name for segmentor's tasks
   */
  class TaskFunctor
  {
  public:
    
    /** Default constructor */
    TaskFunctor(const char* name, 
                const Segment_var& task,
                const std::string& input_file,
                bool  has_error_file,
                const std::string& error_file)
      throw ();

    /** Copy constructor */
    TaskFunctor(const TaskFunctor& c) throw ();

    /** Get segmentor name of this task */
    const char*
    name() const throw ();

    /** Execute task */
    void
    operator ()() throw (eh::Exception);

    /** Get errors of execution of this task */
    const std::string&
    errors() const throw ();

  protected:
    /** must be initialized with name and task */
    TaskFunctor ();

    /** Execute task with input and error streams*/
    void
    execute_(std::istream& istrm, std::ostream& estrm)
      throw (eh::Exception);

    /** Execute task with error stream*/
    void
    execute_(std::ostream& estrm)
       throw (eh::Exception);

    std::string name_;
    Segment_var task_;
    std::string input_file_;
    bool has_error_file_;
    std::string error_file_;
    std::string errors_;
  };

  inline
  TaskFunctor::TaskFunctor(const char* name, 
                           const Segment_var& task,
                           const std::string& input_file,
                           bool  has_error_file,
                           const std::string& error_file)
    throw ()
    : name_(name),
      task_(task),
      input_file_(input_file),
      has_error_file_(has_error_file),
      error_file_(error_file)
  {
  }

  inline
  TaskFunctor::TaskFunctor(const TaskFunctor& c)
    throw ()
    : name_(c.name_), 
      task_(c.task_),
      input_file_(c.input_file_),
      has_error_file_(c.has_error_file_),
      error_file_(c.error_file_)
  {
  }

  inline
  const char*
  TaskFunctor::name() const
    throw ()
  {
    return name_.c_str();
  }

  inline
  const std::string&
  TaskFunctor::errors() const
    throw ()
  {
    return errors_;
  }

  inline
  void
  TaskFunctor::execute_(std::istream& istrm, std::ostream& estrm)
    throw (eh::Exception)
  {
    if (has_error_file_)
    {
      estrm << name_ << " segmentor:" << std::endl;
    }
    task_->execute(istrm, estrm);
  }

  inline
  void
  TaskFunctor::execute_(std::ostream& estrm)
    throw (eh::Exception)
  {
    if (input_file_.empty())
    {
      execute_(std::cin, estrm);
    }
    else
    {
      std::ifstream input(input_file_.c_str());
      if (!input.is_open())
      {
        estrm << "Can't open file: " << input_file_;
        return;
      }
      execute_(input, estrm);
    }
  }

  inline
  void
  TaskFunctor::operator ()()
    throw (eh::Exception)
  {
    if (has_error_file_)
    {
      if (error_file_.empty())
      {
        execute_(std::cerr);
      }
      else
      {
        std::ofstream errors(error_file_.c_str(), 
                             std::ios_base::app);
        if (!errors.is_open())
        {
          errors_ = "Can't open file: " + error_file_;
          return;
        }
        execute_(errors);
      }
    }
    else
    {
      std::ostringstream estrm;
      execute_(estrm);
      errors_ = estrm.str();
    }
  }

  typedef std::vector<TaskFunctor> TaskSeq;
  typedef TaskSeq::iterator TaskSeqIter;
}

/**
 * Holds config and provide tasks factory
 */
class Config : private Generics::Uncopyable
{
public:
  /**
   * Exception raised when params are invalid
   */
  DECLARE_EXCEPTION(ParamsException, eh::DescriptiveException);

  /** Default constructor from main arguments */
  Config(int argc, char **argv) throw (ParamsException);
  
  /**
   * Is usage param setted
   * @return is usage param set
   */
  bool
  is_usage() const throw ();

  /**
   * Threads num param
   * @return threads num
   */
  unsigned long
  threads_num() const throw ();

  /**
   * Create tasks for this config
   * @return sequence of tasks
   */
  TaskSeq
  create_tasks() const throw (eh::Exception);

protected:

  typedef Generics::AppUtils::Option<unsigned long> ULongOption;
  typedef Generics::AppUtils::StringOption StringOption;
  typedef Generics::AppUtils::CheckOption CheckOption;

  // common
  CheckOption usage_;
  ULongOption threads_num_;
  // scenario
  Segment::TestScenarios scenario_;
  CheckOption std_utf8_;
  CheckOption non_std_utf8_;
  CheckOption separators_;
  CheckOption phrases_;
  CheckOption phrases_seq_;
  // borders
  ULongOption lower_border_;
  ULongOption upper_border_;
  // segmentors
  CheckOption all_;
  StringOption segmentor_name_;
  bool composite;
  CheckOption quick_;
  // options
  CheckOption print_;
  CheckOption symbols_only_;
  // input
  StringOption input_file_;
  // error
  StringOption error_file_;

  /** Construct scenario from params */
  Segment::TestScenarios
  get_scenario_() const throw ();

  /** Validate borders params*/
  void
  check_borders_() throw (ParamsException);

  /** Validate threads num param */
  void
  check_threads_num_() const throw (ParamsException);

  /**
   * Create segmentor task for this config
   * @param name is name of segmentor
   * @param segmentor is segmentor for this task
   * @return task for this config and segmentor
   */
  TaskFunctor
  get_task_(const char* name,
    Language::Segmentor::SegmentorInterface_var& segmentor) const
    throw (eh::Exception);
};

Segment::TestScenarios
Config::get_scenario_() const
  throw ()
{
  if (std_utf8_.enabled())
  {
    return Segment::TS_STANDARD_UTF8;
  }
  if (non_std_utf8_.enabled())
  {
    return Segment::TS_NON_STANDARD_UTF8;
  }
  if (separators_.enabled())
  {
    return Segment::TS_SEPARATORS;
  }
  if (phrases_.enabled())
  {
    return Segment::TS_PHRASES;
  }
  if (phrases_seq_.enabled())
  {
    return Segment::TS_PHRASES_SEQ;
  }
  return Segment::TS_ALL;
}

void 
Config::check_borders_()
  throw (ParamsException)
{
  if (lower_border_.installed() && *lower_border_ < 1)
  {
    Stream::Error err;
    err << FNS << " Invalid arguments: lower-border should be "
      "a positive number";
    throw ParamsException(err);
  }
  if (upper_border_.installed())
  {
    if (*upper_border_ < *lower_border_)
    {
      Stream::Error err;
      err << FNS << " Invalid arguments: upper-border should be "
        "greater than lower-border";
      throw ParamsException(err);
    }
  }
  if (*lower_border_ > *upper_border_)
  {
    upper_border_.set_value(*lower_border_);
  }
}

void
Config::check_threads_num_() const
  throw (ParamsException)
{
  if (threads_num_.installed() && *threads_num_ < 1)
  {
    Stream::Error err;
    err << FNS << " Invalid arguments: threads-num should be "
      "a positive number";
    throw ParamsException(err);
  }
}

TaskFunctor
Config::get_task_(const char* name, 
  Language::Segmentor::SegmentorInterface_var& iface) const
  throw (eh::Exception)
{
  Segment_var segm(new Segment(iface,
    *lower_border_,
    *upper_border_,
    scenario_,
    print_.enabled(),
    symbols_only_.enabled()));

  return TaskFunctor(name, segm, 
                     *input_file_,
                     error_file_.installed(),
                     *error_file_);
}

Config::Config(int argc, char **argv)
  throw (ParamsException)
  : threads_num_(1),
    lower_border_(1),
    upper_border_(3)
{
  Generics::AppUtils::Args params;
  params.add(Generics::AppUtils::equal_name("threads-num"), threads_num_);
  params.add(Generics::AppUtils::equal_name("lower-border"), lower_border_);
  params.add(Generics::AppUtils::equal_name("upper-border"), upper_border_);
  params.add(Generics::AppUtils::equal_name("segmentor"), segmentor_name_);
  params.add(Generics::AppUtils::equal_name("if"), input_file_);
  params.add(Generics::AppUtils::equal_name("ef"), error_file_);
  params.add(Generics::AppUtils::short_name("h"), usage_);
  params.add(Generics::AppUtils::short_name("a"), all_);
  params.add(Generics::AppUtils::short_name("s"), std_utf8_);
  params.add(Generics::AppUtils::short_name("n"), non_std_utf8_);
  params.add(Generics::AppUtils::short_name("r"), separators_);
  params.add(Generics::AppUtils::short_name("e"), phrases_);
  params.add(Generics::AppUtils::short_name("es"), phrases_seq_);
  params.add(Generics::AppUtils::short_name("p"), print_);
  params.add(Generics::AppUtils::short_name("o"), symbols_only_);
  params.add(Generics::AppUtils::short_name("q"), quick_);

  params.parse(argc - 1, argv + 1);

  scenario_ = get_scenario_();

  check_threads_num_();
  check_borders_();
}

inline
bool
Config::is_usage() const
  throw ()
{ 
  return usage_.enabled();
}

inline
unsigned long
Config::threads_num() const
  throw ()
{ 
  return *threads_num_;
}

TaskSeq
Config::create_tasks() const
  throw (eh::Exception)
{
  bool all = !segmentor_name_.installed();
  TaskSeq tasks;
  /*
  if (all || *segmentor_name_ == "klt")
  {
    Language::Segmentor::SegmentorInterface_var klt
      = get_klt_segmentor();
    TaskFunctor task = get_task_("Klt", klt);
    tasks.push_back(task);
  }
#ifdef MORAN_TEST
  if (all || *segmentor_name_ == "moran")
  {
    Language::Segmentor::SegmentorInterface_var moran
      = get_moran_segmentor();
    TaskFunctor task = get_task_("Moran", moran);
    tasks.push_back(task);
  }
#endif
  if (all || *segmentor_name_ == "mecab")
  {
    Language::Segmentor::SegmentorInterface_var mecab
      = get_mecab_segmentor();
    TaskFunctor task = get_task_("MeCab", mecab);
    tasks.push_back(task);
  }
  if (all || *segmentor_name_ == "nlpir")
  {
    Language::Segmentor::SegmentorInterface_var nlpir
      = get_nlpir_segmentor();
    TaskFunctor task = get_task_("NLPIR", nlpir);
    tasks.push_back(task);
  }
  */

  if ((all || *segmentor_name_ == "polyglot") && !quick_.enabled())
  {
    Language::Segmentor::SegmentorInterface_var polyglot
      = get_polyglot_segmentor();
    TaskFunctor task = get_task_("Polyglot", polyglot);
    tasks.push_back(task);
  }

  /*
  if (*segmentor_name_ == "composite")
  {
    Language::Segmentor::SegmentorInterface_var composite
      = get_composite_segmentor();
    TaskFunctor task = get_task_("Composite", composite);
    tasks.push_back(task);
  }
  */

  if (tasks.empty())
  {
    Stream::Error err;
    err << FNS << " Invalid arguments: unknown segmentor name: \""
        << *segmentor_name_ << "\"" << std::endl;
    throw ParamsException(err);
  }
  return tasks;
}

int
main(int argc, char **argv)
{
  try
  {
    int ret = 0;

    Config config(argc, argv);
    if (config.is_usage())
    {
      std::cout << USAGE_MSG << std::endl;
      return ret;
    }
    
    unsigned long threads_num = config.threads_num();
    TaskSeq tasks = config.create_tasks();

    for (TaskSeqIter i = tasks.begin(); i != tasks.end(); ++i)
    {
      TaskFunctor& task = *i;
      const char* name = task.name();

      std::cout << name << " segmentor checking started."
                << std::endl;
      
      TestCommons::MTTester<TaskFunctor&> tester(task, threads_num);
      tester.run(threads_num, 0, threads_num);
      
      bool with_errors = !task.errors().empty();
      std::cout << name << " segmentor checking finished"
                << (with_errors ? " with errors." : ".")
                << std::endl;

      if (with_errors)
      {
        std::cerr << name << " segmentor checking errors:\n"
                  << task.errors()
                  << std::endl;
        ret = -1;
      }
    }    
    return ret;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << std::endl
              << "  main(): eh::Exception caught: " << e.what()
              << std::endl << USAGE_MSG << std::endl;
    return -1;
  }
}
