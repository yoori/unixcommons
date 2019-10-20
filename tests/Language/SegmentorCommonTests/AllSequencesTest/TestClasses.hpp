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




#ifndef LANGUAGE_SEGMENTOR_COMMON_TESTS_ALL_SEQUENCE_TESTS_TEST_CLASSES_HPP
#define LANGUAGE_SEGMENTOR_COMMON_TESTS_ALL_SEQUENCE_TESTS_TEST_CLASSES_HPP

#include <iostream>
#include <Language/SegmentorCommons/SegmentorInterface.hpp>

/**
 * @class Segment
 * Testing task unit for segment
 */
class Segment : public ReferenceCounting::AtomicImpl
{
public:

  /**
   * Segment task exception
   */
  DECLARE_EXCEPTION(SegmentError, eh::DescriptiveException);

  /**
   * Enumerate possible scenarios to check
   */
  enum TestScenarios
  {
    TS_ALL,
    TS_STANDARD_UTF8,
    TS_NON_STANDARD_UTF8,
    TS_SEPARATORS,
    TS_PHRASES,
    TS_PHRASES_SEQ
  };

  /**
   * Default constructor
   * @param segmentor is segmentor to check
   * @param start_border is start size of utf8 chars
   * @param finish_border is last size of utf8 chars
   * @param scenario is scenario to check
   * @param print_utf8_transforms print or not transform
   * @param symbols_only symbols only
   */
  Segment(Language::Segmentor::SegmentorInterface_var segmentor,
          unsigned long start_border,
          unsigned long finish_border,
          TestScenarios scenario,
          bool print_utf8_transforms,
          bool symbols_only)
    throw (SegmentError);

  virtual ~Segment() throw ();

  /**
   * Main entry point for task execution
   * @param istrm is input stream if need
   * @param estrm is error stream for errors
   */
  void
  execute(std::istream& istrm, std::ostream& estrm) const
    throw (SegmentError);

private:

  /**
   * Type of execution part
   */
  typedef void (Segment::*Scenario)(std::istream&, std::ostream&) const;

  /**
   * Check all symbols
   */
  void
  check_all_(std::istream& istrm, std::ostream& estrm) const
    throw (SegmentError);

  /**
   * Check utf8 symbols set from specific Walker
   */
  template <class Walker>
  void
  check_with_walker_(std::istream&, std::ostream& estrm) const
    throw (SegmentError);

  /**
   * Check standard utf8 symbols to be eaten or converted to space
   */
  void
  check_separators_(std::istream& istrm, std::ostream& estrm) const
    throw (SegmentError);

  /**
   * Check parsing by dictionary
   * @param istrm is input stream of this dictionary
   */
  void
  check_phrases_(std::istream& istrm, std::ostream& estrm) const
    throw (SegmentError);

  /**
   * Check parsing by dictionary
   * but use segmentation parsing method
   * @param istrm is input stream of this dictionary
   */
  void
  check_phrases_seq_(std::istream& istrm, std::ostream& estrm) const
    throw (SegmentError);

  /**
   * Compare strings ignore spaces
   * @param orig is original string
   * @param orig_len is length of original string
   * @param with_spaces is string with spaces
   * @param with_spaces_len is length of string with spaces
   * @return true if equal else false
   */
  static
  bool
  equal_ignore_spaces(const char *orig, size_t orig_len,
                      const char *with_spaces, size_t with_spaces_len)
    throw ();

  Language::Segmentor::SegmentorInterface_var segmentor_;
  unsigned long start_border_;
  unsigned long finish_border_;
  bool print_utf8_transforms_;
  bool symbols_only_;
  Scenario scenario_;
};

typedef ReferenceCounting::SmartPtr<Segment> Segment_var;

inline
Segment::~Segment()
  throw ()
{
}

inline
void
Segment::execute (std::istream& istrm, std::ostream& estrm) const
  throw (SegmentError)
{
  (this->*scenario_)(istrm, estrm);
}

template <class Walker>
inline
void
Segment::check_with_walker_(std::istream&, std::ostream& estrm) const
  throw (SegmentError)
{
  Walker test_str(start_border_);

  size_t octets = 0;
  while ((octets = test_str.octets()) <= finish_border_)
  {
    std::cout << "Start processing of sequences of " 
              << octets << " byte(s) length." << std::endl;
    do
    {
      try
      {
        std::string result;
        segmentor_->put_spaces(result, test_str, octets);

        if (print_utf8_transforms_ &&
            !Segment::equal_ignore_spaces(test_str, test_str.octets(), 
                                          result.c_str(), result.size()))
        {
          // print transformation where input was changed, ignore spaces
          // for general purposes
          std::cout << "transformation: '"
                    << test_str << "' => '" << result << "\' ";
          test_str.dump(std::cout);
          std::cout << " (U+" << std::hex << test_str.code() << ')'
                    << std::endl;
        }
      }
      catch (const eh::Exception& e)
      {
        estrm << "exception: '" << test_str
              << "' => \"" << e.what() << '\"'
              << std::endl;
      }
    }
    while (test_str.next());
  }
}

#endif //LANGUAGE_SEGMENTOR_COMMON_TESTS_ALL_SEQUENCE_TESTS_TEST_CLASSES_HPP
