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





#ifndef STRING_ANALYZER_PARAMS_HPP
#define STRING_ANALYZER_PARAMS_HPP

#include <String/AsciiStringManip.hpp>

#include <Generics/CompressedSet.hpp>


namespace String
{
  namespace SequenceAnalyzer
  {
    /**
     * A class that provides for the ability to treat two characters as
     * a single object
     */ 
    class CharPair
    {
    public:
      /**
       * Construct object in non-initialized state
       */
      CharPair() throw ();
      /**
       * Construct (single, single) pair
       * @param single Value for (single,single) pair
       */
      CharPair(char single) throw ();
      /**
       * Construct (first, second) pair
       * @param first first char value in pair of 
       * @param second second value in pair of characters
       */
      CharPair(char first, char second) throw ();

      /**
       * Because this method needs does not use std::pair
       * @return true if object was initialized
       */
      bool
      initialized() const throw ();

      /**
      * @return first character of pair
      */
      char
      first() const throw ();

      /**
       * @return second character of pair
       */
      char
      second() const throw ();

    private:
      bool initialized_;
      char first_;
      char second_;
    };

    struct CharSet : public AsciiStringManip::CharCategory
    {
      /**
       * Default constructor
       * Initializes with no symbols in set
       */
      CharSet() throw ();

      template <typename... T>
      CharSet(T... data) throw (eh::Exception);
    };

    typedef Generics::CompressedSet<unsigned> UIntRanges;
    typedef std::map<char, std::string> ShieldMap;

    /**
     * Structure store parameters that define behavior of
     * translator for short descriptions of lexemes.
     */
    struct AnalyzerParams
    {
    public:
      /**
       * Constructor initialize all integral types fields with zero and
       * default constructors for other fields.
       */
      AnalyzerParams() throw ();

      /**
       * Symbol which will be used for inserting such special
       * symbols as 'new line', 'quote', and so on
       */
      char shield_symbol;

      /**
       * List of symbols which will be used to separate lexemes
       */
      CharSet main_separators; 

      /**
       * If false, empty lexemes will be in the output between
       * two successive separators
       */
      bool ignore_successive_separators; 

      /**
       * List of allowed symbols
       */
      CharSet regular_symbs;

      /**
       * Allow or not analyzer to ignore some symbols as if they
       * were not present in the input char sequence
       */
      bool allow_ignored_symbs;

      /**
       * List of symbols, which will be ignored in the input and
       * will not be included to the output. For correct processing of
       * these symbols they should not be special or regular symbols
       */
      CharSet ignored_symbs;

      /**
       * Allow or not nested ranges or repeated parts
       */
      bool allow_recursion;

      /**
       * Max nesting number.
       */
      unsigned short int recursion_max_depth;

      /**
       * Allow or not setting the amount of times to repeat a lexeme or
       * a group of lexemes
       */
      bool allow_repeat;

      /**
       * Symbol or symbols which will be used to mark the number of how
       * many times to repeat the preceding lexeme or group of lexemes
       * quoted with the retry_part_symb symbols
       */
      CharPair num_retries_symb;

      /**
       * Symbol or symbols which will be used to quote the group of lexemes
       * for repeat
       */
      CharPair retry_part_symb;

      /**
       * Allow or not ranges setting in lexemes
       */
      bool allow_range;

      /**
       * Symbol or symbols which will be used to quote ranges
       */
      CharPair range_part_symb;

      /**
       * List of symbols which will be interpreted as separators in ranges.
       * If empty, main separators will be used
       */
      CharSet range_separators;

      /**
       * List of ranges or allowed symbols between range quotes except
       * range separators and range symbol. If empty regular_symbs will
       * be used
       */
      CharSet regular_range_symbs;

      /**
       * Symbol which will be used between a range start and a range end
       */
      char range_symbol;

      /**
       * Switch for padding in ranges functional
       */
      bool allow_padding;

      /**
       * A symbol which will be used as the padding symbol if padding is
       * allowed
       */
      char padding_symb;

      /**
       * Interprets or not initial char sequence as if it were in ranges
       * quotes
       */
      bool immediate_range_mode;

      /**
       *  Should be set to true if unsigned int ranges will be used
       */
      bool use_int_range;

      /**
       * List of bounds of ranges in which unsigned int ranges setting will
       * be allowed in lexemes
       */
      UIntRanges int_range_bounds;

      /**
       * Number which will be used if a range start will contain only
       * padding symbols
       */
      unsigned int default_int_range_start;

      // Reserved for future implementations.
      // Currently it should be set to false.
      /**
       * Should be set to true if char ranges will be used. For sequence
       * of chars in range bounds for each position variations of symbols
       * between chars in corresponding positions in bounds will be made in
       * the output lexemes sequence
       */
      bool use_char_range;

      /**
       * List of chars in which chars setting will be allowed in lexemes
       */
      CharSet char_range_bounds;

      /**
       * A char which will be used if char range start will contain only
       * padding symbols
       */
      char default_char_range_start;

      // Reserved for future implementations.
      // Currently it should be set to false.
      /**
       * Should be set to true if string ranges will be used. For strings
       * in range bounds strings variation in the output lexemes sequence
       * will be made in the lexicographical-like manner.
       */
      bool use_str_range;

      /**
       * List of chars which sets the order of symbols for 'lexicographical'
       * variation
       */
      CharSet str_char_range_bounds;

      /**
       * A char which will be used if str range start will contain only
       * padding symbols
       */
      char default_str_char_range_start;

      /**
       * This string will preceded each lexeme in the output if ostream
       * will be used for output. Useful for using as lexemes output
       * separator
       */
      std::string before_lexeme_out_str;

      /**
       * This string will follow each lexeme in the output if ostream will
       * be used for output. Useful for using as lexemes output separator
       */
      std::string after_lexeme_out_str;

      /**
       * This map will be used to replace a symbol after the shield symbol
       * with corresponding symbol or string. If a symbol is not present as
       * key in this map it will be leaved unchanged, and will not be
       * interpreted as a special symbol or checked if it is allowed.
       */
      ShieldMap shield_map; 
    };
  } // namespace SequenceAnalyzer
}

// Inlines implementations
namespace String
{
  namespace SequenceAnalyzer
  {
    //
    // CharPair class
    //

    inline
    CharPair::CharPair() throw ()
      : initialized_(false)
    {
    }

    inline
    CharPair::CharPair(char single) throw ()
      : initialized_(true), first_(single), second_(single)
    {
    }

    inline
    CharPair::CharPair(char first, char second) throw ()
      : initialized_(true), first_(first), second_(second)
    {
    }

    inline
    bool
    CharPair::initialized() const throw ()
    {
      return initialized_;
    }

    inline
    char
    CharPair::first() const throw ()
    {
      return first_;
    }

    inline
    char
    CharPair::second() const throw ()
    {
      return second_;
    }


    //
    // CharSet class
    //

    inline
    CharSet::CharSet() throw ()
      : AsciiStringManip::CharCategory(static_cast<const char*>(0))
    {
    }

    template <typename... T>
    CharSet::CharSet(T... data) throw (eh::Exception)
      : AsciiStringManip::CharCategory(std::forward<T>(data)...)
    {
    }
  } // namespace SequenceAnalyzer
}
#endif
