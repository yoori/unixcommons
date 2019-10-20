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



// @file String/Analyzer.cpp
#include <climits>

#include <String/SubString.hpp>
#include <String/Analyzer.hpp>


namespace
{
  void
  reserved_for_future_implementations(String::SubString function,
    const char* str)
    throw (String::SequenceAnalyzer::Analyzer::Exception, eh::Exception)
  {
    Stream::Error ostr;
    ostr << function << " " << str << " reserved for future implementations.";
    throw String::SequenceAnalyzer::Analyzer::Exception(ostr);
  }

  template <typename T>
  void
  debug(T /*str*/) throw (eh::Exception)
  {
    // std::cerr << x << std::endl
  }
}

namespace String
{
  namespace SequenceAnalyzer
  {
    //
    // class AnalyzerParams
    //

    AnalyzerParams::AnalyzerParams() throw ()
      : shield_symbol('\0'),
        ignore_successive_separators(false),
        allow_ignored_symbs(false),
        allow_recursion(false),
        recursion_max_depth(0),
        allow_repeat(false),
        allow_range(false),
        range_symbol('\0'),
        allow_padding(false),
        padding_symb(0),
        immediate_range_mode(false),
        use_int_range(false),
        default_int_range_start(0),
        use_char_range(false),
        default_char_range_start('\0'),
        use_str_range(false),
        default_str_char_range_start('\0')
    {
    }

    //
    // Analyzer::TreeNode class
    //

    inline
    Analyzer::TreeNode::TreeNode() throw (eh::Exception)
      : repeat_amount(1)
    {
      debug(FNB);
    }

    inline
    Analyzer::TreeNode::~TreeNode() throw ()
    {
      debug(FNB);
    }


    //
    // Analyzer class
    //

    Analyzer::Analyzer(const AnalyzerParams& init_params,
      Generics::ActiveObjectCallback* callback)
      throw (Exception, eh::Exception)
      : init_params_(init_params),
        callback_(ReferenceCounting::add_ref(callback))
    {
      // Check that initial analyzer parameters are set correctly
      if (init_params_.main_separators.empty())
      {
        Stream::Error ostr;
        ostr << FNS << "Empty list of lexemes separators.";
        throw Exception(ostr);
      }

      if (init_params_.regular_symbs.empty())
      {
        Stream::Error ostr;
        ostr << FNS << "Empty list of ranges of allowed symbols.";
        throw Exception(ostr);
      }

      if (init_params_.allow_ignored_symbs)
      {
        if (init_params_.ignored_symbs.empty())
        {
          init_params_.allow_ignored_symbs = false;
        }
      }

      if (init_params_.allow_repeat)
      {
        if (!init_params_.num_retries_symb.initialized())
        {
          Stream::Error ostr;
          ostr << FNS <<
            "Not defined symbols for marking number of lexeme retries.";
          throw Exception(ostr);
        }
        if (!init_params_.retry_part_symb.initialized())
        {
          Stream::Error ostr;
          ostr << FNS <<
            "Not defined symbols to quote that a lexeme or a group of "
            "lexemes should be repeated.";
          throw Exception(ostr);
        }
      }

      if (init_params_.allow_range)
      {
        if (!init_params_.range_part_symb.initialized())
        {
          Stream::Error ostr;
          ostr << FNS << "unset symbols for marking ranges.";
          throw Exception(ostr);
        }

        if (init_params_.range_separators.empty())
        {
          init_params_.range_separators = init_params_.main_separators;
        }

        if (init_params_.regular_range_symbs.empty())
        {
          init_params_.regular_range_symbs = init_params_.regular_symbs;
        }
      }
      else
      {
        if (init_params_.immediate_range_mode)
        {
          Stream::Error ostr;
          ostr << FNS <<
            "Immediate range mode cannot be used with disallowed ranges.";
          throw Exception(ostr);
        }
      }

      if (!init_params_.allow_recursion)
      {
        init_params_.recursion_max_depth = 1;
      }
      else
      {
        init_params_.recursion_max_depth =
          Generics::safe_next(init_params_.recursion_max_depth);
      }

      if (init_params_.use_int_range)
      {
        if (init_params_.int_range_bounds.empty())
        {
          Stream::Error ostr;
          ostr << FNS <<
            "Empty list of bounds of allowed unsigned int ranges.";
          throw Exception(ostr);
        }

        if (!check_allowed_int_range(init_params_.default_int_range_start))
        {
          Stream::Error ostr;
          ostr << FNS <<
            "default_int_range_start is not a number within one of "
            "int_range_bounds ranges.";
          throw Exception(ostr);
        }
      }

      if (init_params_.use_char_range)
      {
        reserved_for_future_implementations(FNB,
          "Currently use_char_range should be set to false.");

        if (init_params_.char_range_bounds.empty())
        {
          Stream::Error ostr;
          ostr << FNS << "Empty list of bounds of allowed char ranges.";
          throw Exception(ostr);
        }

        if (!check_symbol_in_set(init_params_.char_range_bounds,
          init_params_.default_char_range_start))
        {
          Stream::Error ostr;
          ostr << FNS << 
            "default_char_range_start is not a char within one of "
            "char_range_bounds ranges.";
          throw Exception(ostr);
        }
      }

      if (init_params_.use_str_range)
      {
        reserved_for_future_implementations(FNB,
          "Currently use_str_range should be set to false.");

        if (init_params_.str_char_range_bounds.empty())
        {
          Stream::Error ostr;
          ostr << FNS <<
            "Empty list of bounds of allowed chars for str ranges.";
          throw Exception(ostr);
        }

        if (!check_symbol_in_set(init_params_.str_char_range_bounds,
          init_params_.default_str_char_range_start))
        {
          Stream::Error ostr;
          ostr << FNS << "default_str_char_range_start is not a char "
            "within one of str_char_range_bounds ranges.";
          throw Exception(ostr);
        }
      }
      current_node_.reset();
      recursion_depth_ = 0;
    }

    Analyzer::~Analyzer() throw ()
    {
    }

    bool
    Analyzer::pass_separator_symbols_(std::istream& istr)
      throw (eh::Exception)
    {
      if (init_params_.ignore_successive_separators)
      {
        while (istr.good() &&
          (check_symbol_in_set(init_params_.main_separators,
            current_symbol_) ||
          (init_params_.allow_ignored_symbs &&
            check_symbol_in_set(init_params_.ignored_symbs,
              current_symbol_))))
        {
          istr.get(current_symbol_);
        }
        return true;
      }
      // ignore_successive_separators=false ,, -> ""
      if (istr.good() && check_symbol_in_set(
        init_params_.main_separators, current_symbol_))
      {
        istr.get(current_symbol_);
        if (istr.good() && check_symbol_in_set(
          init_params_.main_separators, current_symbol_))
        {
          create_tree_();
          return false;
        }
      }
      return true;
    }

    void
    Analyzer::create_tree_() throw (eh::Exception)
    {
      try
      {
        cur_lexeme_tree_ = new TreeNode;
      }
      catch (const eh::Exception& e)
      {
        cur_lexeme_tree_.reset();
        current_node_.reset();
        Stream::Error ostr;
        ostr << FNS << "raise exception while trying to construct "
          "new TreeNode. Description: " << e.what();
        throw Exception(ostr);
      }
      current_node_ = cur_lexeme_tree_;
    }

    void
    Analyzer::create_node_(TreeNode_var& start_general_node)
      throw (Exception)
    {
      try
      {
        TreeNode_var node(new TreeNode);
        start_general_node->child_list.push_back(node);
        current_node_ = std::move(node);
      }
      catch (const eh::Exception& e)
      {
        Stream::Error ostr;
        ostr << FNS << "Got eh::Exception while trying to allocate "
          "new TreeNode. Description: " << e.what();
        throw Exception(ostr);
      }
    }

    Analyzer::TreeNode_var
    Analyzer::create_tree_node_(TreeNode_var& start_general_node)
      throw (Exception, NoncriticalException)
    {
      if (recursion_depth_ == init_params_.recursion_max_depth)
      {
        unsigned short int level = init_params_.recursion_max_depth - 1;
        Stream::Error ostr;
        ostr << FNS << "The number of nested ranges or repeated parts has "
          "exceeded it's critical allowed level: " << level;
        throw NoncriticalException(ostr);
      }
      create_node_(start_general_node);
      try
      {
        return TreeNode_var(new TreeNode);
      }
      catch (const eh::Exception& e)
      {
        Stream::Error ostr;
        ostr << FNS << "Got eh::Exception while trying to allocate "
          "new TreeNode. Description: " << e.what();
        throw Exception(ostr);
      }
    }

    inline
    void
    Analyzer::check_readability_(std::istream& istr,
      char stop_symbol) const
      throw (NoncriticalException)
    {
      if (!istr.good() && current_symbol_ != stop_symbol)
      {
        Stream::Error ostr;
        ostr << FNS << "bad expression on input, missing closing symbol=" <<
          stop_symbol;
        throw NoncriticalException(ostr);
      }
    }

    /**
     */
    void
    Analyzer::recognize_shielded_symbol_(std::istream& istr)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      istr.get(current_symbol_);
      if (!istr.good())
      {
        Stream::Error ostr;
        ostr << FNS << "Cannot read a symbol after the shield symbol.";
          throw Exception(ostr);
      }
      if (!init_params_.shield_map.empty())
      {
        ShieldMap::const_iterator it =
          init_params_.shield_map.find(current_symbol_);
        if (it != init_params_.shield_map.end())
        {
          const std::string& tmpstr = it->second;
          ChSeq& node_val = current_node_->node_val;
          node_val.insert(node_val.end(), tmpstr.begin(), tmpstr.end());
          return;
        }
      }
      if (!init_params_.allow_ignored_symbs ||
        !check_symbol_in_set(init_params_.ignored_symbs,
          current_symbol_))
      {
        short int code = current_symbol_;
        Stream::Error ostr;
        ostr << FNS << "illegal symbol '" << current_symbol_ <<
          "' with code " << code;
        throw NoncriticalException(ostr);
      }
    }

    void
    Analyzer::recognize_symbol(std::istream& istr)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      if (init_params_.allow_repeat &&
        current_symbol_ == init_params_.retry_part_symb.first())
      {
        if (!current_node_->node_val.empty())
        {
          Stream::Error ostr;
          ostr << FNS << "Symbol '" << current_symbol_ <<
            "' is not the first symbol in lexeme";
          throw NoncriticalException(ostr);
        }
        TreeNode_var general_node;
        general_node = current_node_;
        current_node_ = process_char_subsequence(istr, general_node);
        istr.get(current_symbol_);
        if (!istr.good())
        {
          Stream::Error ostr;
          ostr << FNS <<
            "failed to read the next symbol after the retry part";
          throw Exception(ostr);
        }
        if (current_symbol_ != init_params_.num_retries_symb.first())
        {
          Stream::Error ostr;
          ostr << FNS << "The retry part should be followed by the '" <<
            init_params_.num_retries_symb.first() <<
            "' symbol with repeat number";
          throw NoncriticalException(ostr);
        }
        set_node_repeat_amount(istr, general_node);
      }
      else if (init_params_.allow_range &&
        current_symbol_ == init_params_.range_part_symb.first())
      {
        TreeNode_var general_node;
        general_node = current_node_;
        current_node_ = process_range_subsequence(istr,
          init_params_.range_part_symb.second(), general_node);
      }
      else if (init_params_.allow_repeat &&
        current_symbol_ == init_params_.num_retries_symb.first())
      {
        set_node_repeat_amount(istr, current_node_);
      }
      else if (current_symbol_ == init_params_.shield_symbol)
      {
        recognize_shielded_symbol_(istr);
      }
      else if (check_symbol_in_set(init_params_.regular_symbs,
        current_symbol_))
      {
        current_node_->node_val.push_back(current_symbol_);
      }
      else if (init_params_.allow_ignored_symbs) // unknown symbol = irregular
      {
        if (!check_symbol_in_set(init_params_.ignored_symbs,
          current_symbol_))
        {
          short int code = current_symbol_;
          Stream::Error ostr;
          ostr << FNS << "illegal symbol '" << current_symbol_ <<
            "' with code " << code;
          throw NoncriticalException(ostr);
        }
      }
      else
      {
        short int code = current_symbol_;
        Stream::Error ostr;
        ostr << FNS << "illegal symbol '" << current_symbol_ <<
          "' with code " << code;
        throw NoncriticalException(ostr);
      }
    }

    Analyzer::TreeNode_var
    Analyzer::process_char_subsequence(std::istream& istr,
      Analyzer::TreeNode_var start_general_node)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      TreeNode_var final_general_node =
        create_tree_node_(start_general_node);
      recursion_depth_++;

      istr.get(current_symbol_);
      while (istr.good() &&
        current_symbol_ != init_params_.retry_part_symb.second())
      {
        if (check_symbol_in_set(init_params_.main_separators,
          current_symbol_))
        {
          current_node_->child_list.push_back(final_general_node);
          if (current_node_->child_list.size() != 1)
          {
            Stream::Error ostr;
            ostr << FNS << "analyzer internal error";
            throw NoncriticalException(ostr);
          }

          create_node_(start_general_node);

          istr.get(current_symbol_);
          if (init_params_.ignore_successive_separators)
          {
            while (istr.good() &&
              (check_symbol_in_set(init_params_.main_separators,
                current_symbol_) ||
                (init_params_.allow_ignored_symbs &&
                  check_symbol_in_set(init_params_.ignored_symbs,
                    current_symbol_))))
            {
              istr.get(current_symbol_);
            }
          }
        }
        else
        {
          recognize_symbol(istr);
          istr.get(current_symbol_);
        }
      }
      current_node_->child_list.push_back(final_general_node);

      recursion_depth_--;

      return final_general_node;
    }

    void
    Analyzer::set_node_repeat_amount(std::istream& istr,
      TreeNode_var repeat_node)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      unsigned short int repeat;
      istr >> repeat;

      if (istr.good())
      {
        istr.get(current_symbol_);
        if (!istr.good())
        {
          Stream::Error ostr;
          ostr << FNS << "set_node_repeat_amount(): "
            "failed to read the next symbol after reading "
            "lexeme repeat amount";
          throw Exception(ostr);
        }
        if (current_symbol_ != init_params_.num_retries_symb.second())
        {
          short int code  = current_symbol_;
          Stream::Error ostr;
          ostr << FNS <<
            "the next symbol after lexeme repeat amount should be '" <<
            init_params_.num_retries_symb.second() << "', instead the '" <<
            current_symbol_ << "' symbol with code " << code << " is read";
          throw NoncriticalException(ostr);
        }
      }
      else if (istr.fail())
      {
        Stream::Error ostr;
        ostr << FNS <<
          "failed to read lexeme repeat amount after the '" <<
          init_params_.num_retries_symb.first() << "' symbol";
        istr.clear();
        throw NoncriticalException(ostr);
      }
      else // istr.eof()
      {
        Stream::Error ostr;
        ostr << FNS << "failed to read lexeme repeat amount";
        throw Exception(ostr);
      }
      repeat_node->repeat_amount *= repeat;
    }

    void
    Analyzer::recognize_range_symbol(std::istream& istr)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      if (init_params_.allow_repeat &&
          (current_symbol_ == init_params_.retry_part_symb.first()))
      {
        if (!current_node_->node_val.empty())
        {
          Stream::Error ostr;
          ostr << FNS << "The retry part open symbol is not the first "
            "symbol the after nearest lexeme separator.";
          throw NoncriticalException(ostr);
        }
        TreeNode_var general_node;
        general_node = current_node_;
        current_node_ = process_range_subsequence(istr,
          init_params_.retry_part_symb.second(), general_node);
        istr.get(current_symbol_);
        if (!istr.good() ||
          current_symbol_ != init_params_.num_retries_symb.first())
        {
          Stream::Error ostr;
          ostr << FNS <<
            "The retry part close symbol is not followed with the "
            "num_retries_symb.first() symbol.";
          throw NoncriticalException(ostr);
        }
        set_node_repeat_amount(istr, general_node);
      }
      else if (current_symbol_ == init_params_.range_part_symb.first())
      {
        TreeNode_var general_node;
        general_node = current_node_;
        current_node_ = process_range_subsequence(istr,
          init_params_.range_part_symb.second(), general_node);
      }
      else if (init_params_.allow_repeat &&
        current_symbol_ == init_params_.num_retries_symb.first())
      {
        set_node_repeat_amount(istr, current_node_);
      }
      else if (current_symbol_ == init_params_.shield_symbol)
      {
        recognize_shielded_symbol_(istr);
      }
      else if (check_symbol_in_set(init_params_.regular_range_symbs,
        current_symbol_))
      {
        current_node_->node_val.push_back(current_symbol_);
      }
      else if (init_params_.allow_ignored_symbs)
      {
        if (check_symbol_in_set(init_params_.ignored_symbs,
          current_symbol_))
        {
          return;
        }

        short int code = current_symbol_;
        Stream::Error ostr;
        ostr << FNS << "The '" << current_symbol_ << "' symbol "
          "with code " << code << " is illegal.";
          throw NoncriticalException(ostr);
      }
      else
      {
        short int code = current_symbol_;
        Stream::Error ostr;
        ostr << FNS << "The '" << current_symbol_ << "' symbol "
          "with code " << code << " is illegal.";
        throw NoncriticalException(ostr);
      }
    }

    Analyzer::TreeNode_var
    Analyzer::process_range_subsequence(std::istream& istr,
      char range_seq_close_symbol, TreeNode_var start_general_node)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      istr.get(current_symbol_);
      check_readability_(istr, range_seq_close_symbol);
      debug("PROCESS_RANGE_SUBSEQUENCE");

      TreeNode_var final_general_node =
        create_tree_node_(start_general_node);
      recursion_depth_++;

      while (istr.good() && current_symbol_ != range_seq_close_symbol)
      {
        if (check_symbol_in_set(init_params_.range_separators,
          current_symbol_))
        {
          current_node_->child_list.push_back(final_general_node);
          if (current_node_->child_list.size() != 1)
          {
            Stream::Error ostr;
            ostr << FNS << "analyzer internal error.";
            throw NoncriticalException(ostr);
          }

          create_node_(start_general_node);

          istr.get(current_symbol_);
          if (init_params_.ignore_successive_separators)
          {
            while (istr.good() &&
              (check_symbol_in_set(init_params_.range_separators,
                current_symbol_) ||
                (init_params_.allow_ignored_symbs &&
                  check_symbol_in_set(init_params_.ignored_symbs,
                    current_symbol_))))
            {
              istr.get(current_symbol_);
            }
          }
        }
        else if (current_symbol_ == init_params_.range_symbol)
        {
          current_node_ = process_range_list(istr, true,
            range_seq_close_symbol, current_node_);
        }
        else
        {
          recognize_range_symbol(istr);
          istr.get(current_symbol_);
        }
      }
      check_readability_(istr, range_seq_close_symbol);
      current_node_->child_list.push_back(final_general_node);

      recursion_depth_--;

      return final_general_node;
    }

    unsigned int
    Analyzer::extract_number(const ChSeq& ch_list, bool& num_range)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      unsigned int extracted_int = 0;

      ChSeq::const_iterator it = std::find_if(ch_list.begin(),
        ch_list.end(), Analyzer::not_digit);
      if (it != ch_list.end())
      {
        num_range = false;
      }
      else
      {
        for (ChSeq::const_iterator iter = ch_list.begin();
          iter != ch_list.end(); ++iter)
        {
          unsigned digit = *iter - '0';
          if ((UINT_MAX - digit) / 10 >= extracted_int)
          {
            extracted_int = extracted_int * 10 + digit;
          }
          else
          {
            num_range = false;
            break;
          }
        }
      }

      return extracted_int;
    }

    Analyzer::TreeNode_var
    Analyzer::process_range_list(std::istream& istr,
      bool use_range_seq_close_symbol, char range_seq_close_symbol,
      Analyzer::TreeNode_var start_general_node)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      unsigned int range_part1_length_with_padding =
        start_general_node->node_val.size();
      unsigned int range_part1_length;
      unsigned int range_part2_length_with_padding;
      unsigned int range_part2_length;
      bool numb_range = false;
      bool char_range = false;
      bool str_range = false;
      unsigned int first_part_int = 0;
      unsigned int second_part_int = 0;
      ChSeq range_part1;
      ChSeq range_part2;
      TreeNode_var final_general_node;

      current_node_ = start_general_node;

      debug("PROCESS RANGE LIST");

      if (init_params_.use_int_range)
      {
        numb_range = true;
      }
      if (init_params_.use_char_range)
      {
        char_range = true;
      }
      if (init_params_.use_str_range)
      {
        str_range = true;
      }

      try
      {
        final_general_node = new TreeNode;
      }
      catch (const eh::Exception& e)
      {
        Stream::Error ostr;
        ostr << FNS << "Got eh::Exception while trying to allocate "
          "new TreeNode. Description: " << e.what();
        throw Exception(ostr);
      }

      start_general_node->node_val.swap(range_part1);

      if (range_part1_length_with_padding == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "first range part is empty";
        throw NoncriticalException(ostr);
      }

      if (init_params_.allow_padding)
      {
        ChSeq::iterator it = std::find_if(range_part1.begin(),
          range_part1.end(),
          std::bind1st(std::mem_fun(&Analyzer::not_padding_symb), this));
        range_part1.erase(range_part1.begin(), it);
      }

      range_part1_length = range_part1.size();
      if (!range_part1.empty())
      {
        if (init_params_.use_int_range)
        {
          first_part_int = extract_number(range_part1, numb_range);
          if (numb_range)
          {
            numb_range = check_allowed_int_range(first_part_int);
          }
        }

        if (init_params_.use_char_range)
        {
          ChSeq::iterator it = std::find_if(range_part1.begin(),
            range_part1.end(),
            std::bind1st(std::mem_fun(
              &Analyzer::not_allowed_char_range), this));
          if (it != range_part1.end())
          {
            char_range = false;
          }
        }

        if (init_params_.use_str_range)
        {
          ChSeq::iterator it = std::find_if(range_part1.begin(),
            range_part1.end(),
            std::bind1st(std::mem_fun(
              &Analyzer::not_allowed_str_range), this));
          if (it != range_part1.end())
          {
            str_range = false;
          }
        }
      }
      else
      {
        first_part_int = init_params_.default_int_range_start;
        str_range = false;
        char_range = false;
      }

      istr.get(current_symbol_);
      while (istr.good() &&
        !check_symbol_in_set(init_params_.range_separators,
          current_symbol_) &&
        !(use_range_seq_close_symbol &&
          current_symbol_ == range_seq_close_symbol) &&
        current_symbol_ != init_params_.range_part_symb.first())
      {
        range_part2.push_back(current_symbol_);
        istr.get(current_symbol_);
      }

      range_part2_length_with_padding = range_part2.size();

      if (range_part2_length_with_padding == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "second range part is empty";
        throw NoncriticalException(ostr);
      }
      if (range_part2_length_with_padding !=
        range_part1_length_with_padding)
      {
        str_range = false;
        char_range = false;
      }

      if (init_params_.allow_padding)
      {
        range_part2.erase(range_part2.begin(),
          std::find_if(range_part2.begin(), range_part2.end(),
            std::bind1st(std::mem_fun(&Analyzer::not_padding_symb), this)));
      }
      range_part2_length = range_part2.size();
      if (range_part2_length == 0 ||
        range_part2_length < range_part1_length)
      {
        Stream::Error ostr;
        ostr << FNS << "range part lengths are not compatible.";
        throw NoncriticalException(ostr);
      }
      if (range_part1_length != range_part2_length)
      {
        str_range = false;
        char_range = false;
      }
      if (range_part1_length_with_padding != range_part1_length &&
          numb_range && range_part1_length_with_padding !=
            range_part2_length_with_padding)
      {
        numb_range = false;
      }

      if (numb_range)
      {
        second_part_int = extract_number(range_part2, numb_range);
        if (numb_range)
        {
          numb_range = check_allowed_int_range(second_part_int);
          if (first_part_int > second_part_int)
          {
            numb_range = false;
          }
          if (numb_range)
          {
            str_range = false;
            char_range = false;
          }
        }
      }

      if (!numb_range && str_range)
      {
        ChSeq::iterator it = std::find_if(range_part2.begin(),
          range_part2.end(),
          std::bind1st(std::mem_fun(&Analyzer::not_allowed_str_range),
            this));
        if (it != range_part2.end())
        {
          str_range = false;
        }
        if (str_range)
        {
          str_range = check_param2_after_param1(range_part1, range_part2);
        }
        if (str_range)
        {
          reserved_for_future_implementations(FNB,
            "Unrolling of str range");
          char_range = false;
        }
      }

      if (!numb_range && !str_range && char_range)
      {
        reserved_for_future_implementations(FNB, "Unrolling of char range");
      }

      if (!(numb_range || str_range || char_range))
      {
        Stream::Error ostr;
        ostr << FNS << "range type cannot be defined";
        throw NoncriticalException(ostr);
      }

      bool use_padding = false;
      if (range_part1_length_with_padding != range_part1_length ||
        range_part2_length_with_padding != range_part2_length)
      {
        use_padding = true;
      }

      if (numb_range)
      {
        current_node_ = unroll_num_range(first_part_int, second_part_int,
          use_padding, range_part2_length_with_padding, start_general_node);
      }

      if (str_range)
      {
        current_node_ = unroll_str_range(range_part1, range_part2,
          use_padding, range_part2_length_with_padding, start_general_node);
      }

      if ((use_range_seq_close_symbol &&
          (current_symbol_ != range_seq_close_symbol &&
            current_symbol_ == init_params_.range_part_symb.first())) ||
       (!use_range_seq_close_symbol &&
        current_symbol_ == init_params_.range_part_symb.first()))
      {
        TreeNode_var general_node;
        general_node = current_node_;
        current_node_ = process_range_subsequence(istr,
          init_params_.range_part_symb.second(), general_node);
        istr.get(current_symbol_);
      }

      current_node_->child_list.push_back(final_general_node);

      return final_general_node;
    }

    Analyzer::TreeNode_var
    Analyzer::unroll_num_range(unsigned int start_int,
      unsigned int final_int, bool use_padding,
      unsigned int range_part2_length_with_padding, TreeNode_var parent_node)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      debug("UNROLL_NUM_RANGE");
      TreeNode_var final_general_node;
      try
      {
        final_general_node = new TreeNode;
      }
      catch (const eh::Exception& e)
      {
        Stream::Error ostr;
        ostr << FNS << "Got eh::Exception while trying to allocate "
          "new TreeNode. Description: " << e.what();
        throw Exception(ostr);
      }

      for (unsigned int i = start_int; i <= final_int; i++)
      {
        current_node_ = new TreeNode;
        parent_node->child_list.push_back(current_node_);

        Stream::Dynamic intstr(4096);

        if (use_padding)
        {
          intstr.fill(init_params_.padding_symb);
          intstr.width(range_part2_length_with_padding);
        }
        intstr << i;

        String::SubString tmpstr = intstr.str();
        current_node_->node_val.insert(current_node_->node_val.end(),
          tmpstr.begin(), tmpstr.end());

        current_node_->child_list.push_back(final_general_node);
      }

      return final_general_node;
    }

    Analyzer::TreeNode_var
    Analyzer::unroll_str_range(const ChSeq& /*start_list*/,
      const ChSeq& /*final_list*/, bool /*use_padding*/,
      unsigned int /*range_part2_length_with_padding*/,
      TreeNode_var parent_node)
      throw (Exception, NoncriticalException, eh::Exception)
    {
      reserved_for_future_implementations(FNB, "Unrolling of str range");

      TreeNode_var final_general_node(new TreeNode);

      parent_node->child_list.push_back(final_general_node);
      return final_general_node;
    }

    bool
    Analyzer::check_param2_after_param1(
      const ChSeq& /*ch_list1*/, const ChSeq& /*ch_list2*/) const
      throw (eh::Exception)
    {
      reserved_for_future_implementations(FNB, "check_param2_after_param1");
      return true;
    }
  } // namespace SequenceAnalyzer
}
