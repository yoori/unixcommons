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



#ifndef _GENERAL_SEGMETOR_TEST_TEST_CLASSES_HPP_
#define _GENERAL_SEGMETOR_TEST_TEST_CLASSES_HPP_

#include <sstream>
#include <Language/SegmentorCommons/SegmentorInterface.hpp>
#include <vector>
#include <Generics/Time.hpp>
#include <Sync/PosixLock.hpp>


typedef std::vector<Language::Segmentor::SegmentorInterface_var> Segmentors;

//
// class CheckSegmentResult
//

class CheckSegmentResult
{
public:

  struct Transform
  {
    std::string from;
    std::string to;

    Transform(const char* from_val, const char* to_val):
      from(from_val),
      to(to_val)
    {};
  };
  typedef std::list<Transform> Transforms;

  struct SegmentorStats
  {
    size_t segmented_count;
    size_t segmented_dropped_count;
    size_t dropped_count;
    size_t partially_dropped_count;
    size_t exceptions_count;
    Generics::Time processing_time;
    Transforms segment_transforms;
    Transforms segment_drop_transforms;
    Transforms drop_transforms;
    Transforms partially_drop_transforms;

    SegmentorStats(size_t new_segmented_count = 0,
      size_t new_segmented_dropped_count = 0, size_t new_dropped_count = 0,
      size_t new_partially_dropped_count = 0, size_t new_exceptions_count = 0,
      const Generics::Time& new_processing_time = Generics::Time(0));

    SegmentorStats& operator +=(SegmentorStats& src) throw(eh::Exception);
  };

  enum Operation
  {
    COPY,
    ADD
  };

  CheckSegmentResult(const Segmentors& segms_map, 
    bool check_transforms);

  void check(const char* phrase, size_t phrase_len);

  void dump(std::ostream& out);

  const SegmentorStats* find_segmentor_stats(
    const Language::Segmentor::SegmentorInterface* id) const throw();

  void flush_segmentor_stats(SegmentorStats& stats,
    const Language::Segmentor::SegmentorInterface* id, Operation op)
    throw(eh::Exception);

  size_t get_segmentations_count() const throw();
  double get_average_seqs_length() const throw();

private:

  static bool is_space_(const char ch) throw();

  typedef std::vector<SegmentorStats> SegmentorsStats_;

  SegmentorsStats_ segmentors_stats_vect_;
  Segmentors segmentors_vect_;
  size_t segmentations_count_;
  double average_seqs_length_;
  size_t checks_count_;
  bool check_transforms_;
};


//
// class CommonFunctor
//

class CommonFunctor
{
  public:

  CommonFunctor(const Segmentors& segms_map);

  virtual void operator()() const = 0;

  const CheckSegmentResult::SegmentorStats* find_segmentor_stats(
    const Language::Segmentor::SegmentorInterface* id) const throw();

  void dump(std::ostream& out);

  size_t get_segmentations_count() const throw();
  double get_average_seqs_length() const throw();

  virtual ~CommonFunctor() throw();

protected:

  void fix_results_(CheckSegmentResult& checker) const;

  Segmentors segms_map_;
  mutable Sync::PosixMutex lock_;
  mutable std::vector<CheckSegmentResult::SegmentorStats> stats_;
  mutable size_t segmentations_count_;
  mutable double average_seqs_length_;
  mutable size_t checks_count_;
};


//
// class RandomUtf8SegmentFunctor
//

class RandomUtf8SegmentFunctor: public CommonFunctor
{
public:

  RandomUtf8SegmentFunctor(const Segmentors& segms_map, int it_count,
    int sequences_len, bool check_transforms,
    bool in_standart_utf8 = true);

  virtual void operator()() const;

  virtual ~RandomUtf8SegmentFunctor() throw();

protected:

  int it_count_;
  int sequences_len_;
  bool check_transforms_;
  bool in_standart_utf8_;
};

//
// class ParseStdIn
//

class ParseStdIn: public CommonFunctor
{
public:

  ParseStdIn(const Segmentors& segms_map, bool check_transforms);

  virtual void operator()() const;

  virtual ~ParseStdIn() throw();

private:

  bool check_transforms_;
};


//
// class RandomAsciiSegmentFunctor
//

class RandomAsciiSegmentFunctor: public RandomUtf8SegmentFunctor
{
public:

  RandomAsciiSegmentFunctor(const Segmentors& segms_map, int it_count,
    int sequences_len, bool check_transforms);

  virtual void operator()() const;

  virtual ~RandomAsciiSegmentFunctor() throw();
};

//
// class ParseFile
//

class ParseFile: public CommonFunctor
{
public:

  ParseFile(const char* file_name, const Segmentors& segms_map,
    int max_iteration_number, bool check_transforms);

  virtual void operator()() const;

  virtual ~ParseFile() throw();

private:

  const std::string src_;
  int max_iteration_number_;
  bool check_transforms_;
};

#endif
