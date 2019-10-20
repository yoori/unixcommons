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



#include <algorithm>
#include <iostream>

#include "TestClasses.hpp"
#include <Language/SegmentorCommonTests/Commons/TextGenerator.hpp>
#include <Generics/ArrayAutoPtr.hpp>
#include <Stream/BzlibStreams.hpp>


//
// class CheckSegmentResult
//

CheckSegmentResult::CheckSegmentResult(const Segmentors& segms_map,
    bool check_transforms)
  : segmentors_stats_vect_(segms_map.size()),
    segmentors_vect_(segms_map),
    segmentations_count_(0),
    average_seqs_length_(0),
    check_transforms_(check_transforms)
{
}

inline
bool
CheckSegmentResult::is_space_(const char ch) throw()
{
  return ch == ' ';
}

void
CheckSegmentResult::check(const char* phrase, size_t phrase_len)
{
  if (!phrase || *phrase == '\0' || !phrase_len)
  {
    return;
  }

  //std::string previous_res;
  for (size_t i = 0; i < segmentors_vect_.size(); ++i)
  {
    if (!segmentors_vect_[i].in())
    {
      continue;
    }

    Generics::CPUTimer timer;
    bool exception_caught = false;
    std::string result;
    timer.start();
    try
    {
      segmentors_vect_[i]->put_spaces(result, phrase, phrase_len);
    }
    catch(...)
    {
      exception_caught = true;
    }
    timer.stop();

    segmentors_stats_vect_[i].processing_time += timer.elapsed_time();

    //if (!previous_res.empty() && previous_res != result)
    //{
    //  std::cout << std::string(phrase, phrase_len) << " :: "
    //            << previous_res << " != "
    //            << result
    //            << (exception_caught? " (exception_caught)": "")
    //            << std::endl;
    //}
    //previous_res = result;

    if (exception_caught)
    {
      ++segmentors_stats_vect_[i].exceptions_count;
    }
    else
    {
      if (result.empty())
      {
        ++segmentors_stats_vect_[i].dropped_count;
        if (check_transforms_)
        {
          segmentors_stats_vect_[i].drop_transforms.push_back
            (Transform(std::string(phrase, phrase_len).c_str(), ""));
        }
      }
      else
      {
        std::string src(phrase, phrase_len);
        std::string::size_type src_spaces_count =
          std::count_if(src.begin(), src.end(), is_space_);
        std::string::size_type res_spaces_count =
          std::count_if(result.begin(), result.end(), is_space_);

        bool segmented = src_spaces_count < res_spaces_count;
        bool partially_dropped = phrase_len - src_spaces_count >
          result.length() - res_spaces_count;

        if (segmented && partially_dropped)
        {
          ++segmentors_stats_vect_[i].segmented_dropped_count;
          if (check_transforms_)
          {
            segmentors_stats_vect_[i].segment_drop_transforms.push_back(
              Transform(src.c_str(), result.c_str()));
          }
        }
        else
        {
          if (segmented)
          {
            ++segmentors_stats_vect_[i].segmented_count;
            if (check_transforms_)
            {
              segmentors_stats_vect_[i].segment_transforms.push_back(
                Transform(src.c_str(), result.c_str()));
            }
          }
          if (partially_dropped)
          {
            ++segmentors_stats_vect_[i].partially_dropped_count;
            if (check_transforms_)
            {
              segmentors_stats_vect_[i].partially_drop_transforms.push_back(
                Transform(src.c_str(), result.c_str()));
            }
          }
        }
      }
    }
  }

  average_seqs_length_ = static_cast<double>(average_seqs_length_ *
    segmentations_count_ + phrase_len) / (segmentations_count_ + 1);
  ++segmentations_count_;
}

void
CheckSegmentResult::dump(std::ostream& out)
{
  for (size_t i = 0; i < segmentors_vect_.size(); ++i)
  {
    if (!segmentors_vect_[i].in())
    {
      continue;
    }

    out << "\nSegmentor id: " << segmentors_vect_[i].in()
        << ". Total processings: " << segmentations_count_
        << " (ave sequences len: " << average_seqs_length_
        << ")\nsegmented w/o partially dropping: "
          << segmentors_stats_vect_[i].segmented_count
        << "\nsegmented with partially dropping: "
          << segmentors_stats_vect_[i].segmented_dropped_count
        << "\npartially dropped w/o segmenting: "
          << segmentors_stats_vect_[i].partially_dropped_count
        << "\nfully dropped: " << segmentors_stats_vect_[i].dropped_count
        << "\nexceptions thrown: " << segmentors_stats_vect_[i].exceptions_count
        << "\nTotal processing time: " << segmentors_stats_vect_[i].processing_time
        << ", average: " << (!segmentations_count_ ? Generics::Time::ZERO :
          segmentors_stats_vect_[i].processing_time / segmentations_count_)
        << '\n';
  }
}

const CheckSegmentResult::SegmentorStats*
CheckSegmentResult::find_segmentor_stats(
  const Language::Segmentor::SegmentorInterface* id) const throw()
{
  for (size_t i = 0; i < segmentors_vect_.size(); ++i)
  {
    if (segmentors_vect_[i].in() == id)
    {
      return &(segmentors_stats_vect_[i]);
    }
  }

  return 0;
}

void
CheckSegmentResult::flush_segmentor_stats(
  CheckSegmentResult::SegmentorStats& stats,
  const Language::Segmentor::SegmentorInterface* id,
  CheckSegmentResult::Operation op)
  throw(eh::Exception)
{
  for (size_t i = 0; i < segmentors_vect_.size(); ++i)
  {
    if (segmentors_vect_[i].in() == id)
    {
      segmentors_vect_[i].reset();

      if (op == COPY)
      {
        stats = segmentors_stats_vect_[i];
      }
      else if (op == ADD)
      {
        stats += segmentors_stats_vect_[i];
      }
      else
      {
        std::cerr << "CheckSegmentResult::flush_segmentor_stats: invalid "
                     "operation (COPY or ADD are expected.)" << std::endl;
      }

      return;
    }
  }

  std::cerr << "CheckSegmentResult::flush_segmentor_stats(): segmentor (id: "
            << id <<  ") was not found."
            << std::endl;
}

size_t
CheckSegmentResult::get_segmentations_count() const throw()
{
  return segmentations_count_;
}

double
CheckSegmentResult::get_average_seqs_length() const throw()
{
  return average_seqs_length_;
}

//
// struct CheckSegmentResult::SegmentorStats_
//

CheckSegmentResult::SegmentorStats::SegmentorStats(size_t new_segmented_count,
    size_t new_segmented_dropped_count, size_t new_dropped_count,
    size_t new_partially_dropped_count, size_t new_exceptions_count,
    const Generics::Time& new_processing_time):
  segmented_count(new_segmented_count),
  segmented_dropped_count(new_segmented_dropped_count),
  dropped_count(new_dropped_count),
  partially_dropped_count(new_partially_dropped_count),
  exceptions_count(new_exceptions_count),
  processing_time(new_processing_time)
{
}

CheckSegmentResult::SegmentorStats&
CheckSegmentResult::SegmentorStats::operator +=(
  CheckSegmentResult::SegmentorStats& src) throw(eh::Exception)
{
  segmented_count += src.segmented_count;
  segmented_dropped_count += src.segmented_dropped_count;
  dropped_count += src.dropped_count;
  partially_dropped_count += src.partially_dropped_count;
  exceptions_count += src.exceptions_count;
  processing_time += src.processing_time;

  segment_transforms.splice(segment_transforms.end(), src.segment_transforms,
    src.segment_transforms.begin(), src.segment_transforms.end());
  segment_drop_transforms.splice(segment_drop_transforms.end(), src.segment_drop_transforms,
    src.segment_drop_transforms.begin(), src.segment_drop_transforms.end());
  drop_transforms.splice(drop_transforms.end(), src.drop_transforms,
    src.drop_transforms.begin(), src.drop_transforms.end());
  segment_transforms.splice(partially_drop_transforms.end(), src.partially_drop_transforms,
    src.partially_drop_transforms.begin(), src.partially_drop_transforms.end());

  return *this;
}


//
// class CommonFunctor
//

CommonFunctor::CommonFunctor(const Segmentors& segms_map):
  segms_map_(segms_map),
  stats_(segms_map.size()),
  segmentations_count_(0),
  average_seqs_length_(0),
  checks_count_(0)
{
}

void
CommonFunctor::fix_results_(CheckSegmentResult& checker) const
{
  Sync::PosixGuard guard(lock_);

  size_t len = stats_.size();
  for (size_t i = 0; i < len; ++i)
  {
    if (!segms_map_[i].in())
    {
      continue;
    }

    checker.flush_segmentor_stats(
      stats_[i], segms_map_[i].in(), CheckSegmentResult::ADD);
  }

  segmentations_count_ += checker.get_segmentations_count();
  average_seqs_length_ = static_cast<double>(average_seqs_length_ *
    checks_count_ + checker.get_average_seqs_length()) / (checks_count_ + 1);
  ++checks_count_;
}

const CheckSegmentResult::SegmentorStats*
CommonFunctor::find_segmentor_stats(
  const Language::Segmentor::SegmentorInterface* id) const throw()
{
  for (size_t i = 0; i < segms_map_.size(); ++i)
  {
    if (segms_map_[i].in() == id)
    {
      return &(stats_[i]);
    }
  }

  return 0;
}

void
CommonFunctor::dump(std::ostream& out)
{
  for (size_t i = 0; i < segms_map_.size(); ++i)
  {
    if (!segms_map_[i].in())
    {
      continue;
    }

    out << "\nSegmentor id: " << segms_map_[i].in()
        << ". Total processings: " << segmentations_count_
        << " (ave sequences len: " << average_seqs_length_
        << ")\nsegmented w/o partially dropping: "
          << stats_[i].segmented_count
        << "\nsegmented with partially dropping: "
          << stats_[i].segmented_dropped_count
        << "\npartially dropped w/o segmenting: "
          << stats_[i].partially_dropped_count
        << "\nfully dropped: " << stats_[i].dropped_count
        << "\nexceptions thrown: " << stats_[i].exceptions_count
        << "\nTotal processing time: " << stats_[i].processing_time
        << ", average: " << (!segmentations_count_ ?
          Generics::Time::ZERO :
          stats_[i].processing_time / segmentations_count_)
        << '\n';
  }
}


size_t
CommonFunctor::get_segmentations_count() const throw()
{
  return segmentations_count_;
}

double
CommonFunctor::get_average_seqs_length() const throw()
{
  return average_seqs_length_;
}

CommonFunctor::~CommonFunctor() throw()
{
}


//
// class RandomUtf8SegmentFunctor
//

RandomUtf8SegmentFunctor::RandomUtf8SegmentFunctor(const Segmentors& segms_map,
    int it_count, int sequences_len, bool check_transforms,
    bool in_standart_utf8):
  CommonFunctor(segms_map),
  it_count_(it_count),
  sequences_len_(sequences_len + 4),
  check_transforms_(check_transforms),
  in_standart_utf8_(in_standart_utf8)
{
}

void
RandomUtf8SegmentFunctor::operator()() const
{
  try
  {
    std::unique_ptr<CheckSegmentResult> checker(
      new CheckSegmentResult(segms_map_, check_transforms_));
    Generics::ArrayChar buf(sequences_len_);
    char* ptr = buf.get();

    for (int i = 0; i < it_count_; ++i)
    {
      size_t real_len = SegmentorTestCommons::Utf8Generator::gen_rand_utf8_sequence(
        ptr, sequences_len_, in_standart_utf8_);
      try {
        checker->check(ptr, real_len);
      }
      catch (const eh::Exception& e)
      {
        std::cerr << "RandomUtf8SegmentFunctor::operator(): "
                  << std::string(ptr, real_len)
                  << " failed: " << e.what() << std::endl;
      }
    }

    fix_results_(*checker);
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "RandomUtf8SegmentFunctor::operator() failed: "
              << e.what() << std::endl;
  }
}

RandomUtf8SegmentFunctor::~RandomUtf8SegmentFunctor() throw()
{
}

//
// class ParseStdIn
//


ParseStdIn::ParseStdIn(const Segmentors& segms_map, bool check_transforms)
  : CommonFunctor(segms_map), check_transforms_(check_transforms)
{
}

void
ParseStdIn::operator()() const
{
  try
  {
    std::unique_ptr<CheckSegmentResult> checker(
      new CheckSegmentResult(segms_map_, check_transforms_));
    do
    {
      std::string new_word;
      std::cin >> new_word;

      try {
        checker->check(new_word.c_str(), new_word.length());
      }
      catch (const eh::Exception& e)
      {
        std::cerr << "ParseStdIn::operator(): "
                  << new_word
                  << " failed: " << e.what() << std::endl;
      }
    }
    while (!std::cin.bad() && !std::cin.fail());

    fix_results_(*checker);
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "ParseStdIn::operator() failed: "
              << e.what() << std::endl;
  }
}

ParseStdIn::~ParseStdIn() throw()
{
}


//
// class RandomAsciiSegmentFunctor
//

RandomAsciiSegmentFunctor::RandomAsciiSegmentFunctor(const Segmentors& segms_map,
    int it_count, int sequences_len, bool check_transforms):
  RandomUtf8SegmentFunctor(segms_map, it_count, sequences_len, check_transforms)
{
  sequences_len_ = sequences_len;
}

void
RandomAsciiSegmentFunctor::operator()() const
{
  try
  {
    std::unique_ptr<CheckSegmentResult> checker(
      new CheckSegmentResult(segms_map_, check_transforms_));
    Generics::ArrayChar buf(sequences_len_);
    char* ptr = buf.get();

    for (int i = 0; i < it_count_; ++i)
    {
      SegmentorTestCommons::AsciiGenerator::gen_rand_ascii_sequence(
        ptr, sequences_len_);

      try {
        checker->check(ptr, sequences_len_);
      }
      catch (const eh::Exception& e)
      {
        std::cerr << "RandomAsciiSegmentFunctor::operator(): "
                  << std::string(ptr, sequences_len_)
                  << " failed: " << e.what() << std::endl;
      }
    }

    fix_results_(*checker);
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "RandomAsciiSegmentFunctor::operator() failed: "
              << e.what() << std::endl;
  }
}

RandomAsciiSegmentFunctor::~RandomAsciiSegmentFunctor() throw()
{
}


//
// class ParseFile
//


ParseFile::ParseFile(const char* file_name, const Segmentors& segms_map,
    int max_iteration_number, bool check_transforms)
  : CommonFunctor(segms_map), src_(file_name),
    max_iteration_number_(max_iteration_number),
    check_transforms_(check_transforms)
{
}

void
ParseFile::operator()() const
{
  int cur_iteration = max_iteration_number_;
  try
  {
    Stream::BzlibInStream f_src_(src_.c_str());

    CheckSegmentResult checker(segms_map_, check_transforms_);
    std::string new_word;
    while (std::getline(f_src_, new_word))
    {
      if (!new_word.empty())
      {
        try
        {
          checker.check(new_word.c_str(), new_word.size());
        }
        catch (const eh::Exception& e)
        {
          std::cerr << "ParseFile::operator(): " << new_word <<
            " failed: " << e.what() << std::endl;
        }
      }

      if (cur_iteration > 0)
      {
        --cur_iteration;
      }
      else if (!cur_iteration)
      {
        break;
      }
    }

    fix_results_(checker);
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "ParseFile::operator() failed: "
              << e.what() << std::endl;
  }
}

ParseFile::~ParseFile() throw()
{
}

