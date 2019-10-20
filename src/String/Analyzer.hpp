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





#ifndef STRING_ANALIZER_HPP
#define STRING_ANALIZER_HPP

#include <list>
#include <set>

#include <ReferenceCounting/ReferenceCounting.hpp>
#include <ReferenceCounting/Deque.hpp>

#include <String/AnalyzerParams.hpp>

#include <Generics/ActiveObject.hpp>


namespace String
{
  /**
   * Routines of universal translator for brief descriptions of lexemes
   */
  namespace SequenceAnalyzer
  {
    /**
     * Allow translate short descriptions to lexemes sets.
     */
    class Analyzer
    {
    public:
      /**
       * Base exception for Analyzer errors
       */
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
      /**
       * Raise to indicate non-critical and warning parsing cases
       */
      DECLARE_EXCEPTION(NoncriticalException, Exception);

      /**
       * Construct universal translator
       * @param params description and rules for translator
       * @param callback Tool to obtain information about errors
       */
      Analyzer(const AnalyzerParams& params,
        Generics::ActiveObjectCallback* callback)
        throw (Exception, eh::Exception);

      /**
       * Simple (empty) virtual destructor
       */
      virtual
      ~Analyzer() throw ();

      /**
       * Do translation
       * @param istr Input stream with data that should be translated into
       * lexemes set
       * @param result_arg Here will put the result of translation - set of
       * lexemes
       */
      template <typename ResultType>
      void
      process_char_sequence(std::istream& istr, ResultType& result_arg)
        throw (Exception, eh::Exception);

    private:
      typedef std::list<char> ChSeq;

      struct TreeNode;
      typedef ReferenceCounting::QualPtr<TreeNode> TreeNode_var;
      typedef ReferenceCounting::QualPtr<const TreeNode> ConstTreeNode_var;
      struct TreeNode : public ReferenceCounting::DefaultImpl<>
      {
      public:
        TreeNode() throw (eh::Exception);

        typedef ReferenceCounting::Deque<TreeNode_var> Children;

        ChSeq node_val;
        unsigned short int repeat_amount;
        Children child_list;

      protected:
        virtual
        ~TreeNode() throw ();
      };


      TreeNode_var
      process_char_subsequence(std::istream& istr,
        TreeNode_var start_general_node)
        throw (Exception, NoncriticalException, eh::Exception);

      void
      set_node_repeat_amount(std::istream& istr, TreeNode_var repeat_node)
        throw (Exception, NoncriticalException, eh::Exception);

      template <typename ResultType>
      void
      process_range_sequence(std::istream& istr, ResultType& result_arg)
        throw (Exception, eh::Exception);

      TreeNode_var
      process_range_subsequence(std::istream& istr,
        char range_seq_close_symbol, TreeNode_var start_general_node)
        throw (Exception, NoncriticalException, eh::Exception);

      static
      bool
      check_symbol_in_set(const CharSet& chars, char ch)
        throw (eh::Exception);

      void
      recognize_symbol(std::istream& istr)
        throw (Exception, NoncriticalException, eh::Exception);

      void
      recognize_range_symbol(std::istream& istr)
        throw (Exception, NoncriticalException, eh::Exception);

      TreeNode_var
      process_range_list(std::istream& istr, bool use_range_seq_close_symbol,
        char range_seq_close_symbol, TreeNode_var node)
        throw (Exception, NoncriticalException, eh::Exception);


      bool
      not_padding_symb(char ch) const
        throw (eh::Exception);

      static
      bool
      not_digit(char ch)
        throw ();

      bool
      not_allowed_char_range(char ch) const
        throw (eh::Exception);

      bool
      not_allowed_str_range(char ch) const
        throw (eh::Exception);

      bool
      check_allowed_int_range(unsigned int number) const
        throw (eh::Exception);

      static
      unsigned int
      extract_number(const ChSeq& chars, bool& num_range)
        throw (Exception, NoncriticalException, eh::Exception);

      TreeNode_var
      unroll_num_range(unsigned int start_int, unsigned int final_int,
        bool use_padding, unsigned int range_part2_length_with_padding,
        TreeNode_var parent_node)
        throw (Exception, NoncriticalException, eh::Exception);

      TreeNode_var
      unroll_str_range(const ChSeq& start_list, const ChSeq& final_list,
        bool use_padding, unsigned int range_part2_length_with_padding,
        TreeNode_var parent_node)
        throw (Exception, NoncriticalException, eh::Exception);

      bool
      check_param2_after_param1(const ChSeq& ch_list1,
        const ChSeq& ch_list2) const throw (eh::Exception);

      template <typename ResultType>
      void
      interprete_and_flush_tree(ResultType& result_arg) const
        throw (Exception, eh::Exception);

      template <typename ResultType>
      void
      flush_node(ConstTreeNode_var cur_node, ResultType& result_arg,
        Stream::Dynamic& ostr) const throw (Exception, eh::Exception);

      void
      write_result_arg(std::ostream& main_ostr,
        Stream::Dynamic& my_str) const throw (Exception, eh::Exception);

      template <typename Element, typename Allocator>
      void
      write_result_arg(std::list<Element, Allocator>& main_list,
        Stream::Dynamic& my_str) const throw (Exception, eh::Exception);

      template <typename Element, typename Compare, typename Allocator>
      void
      write_result_arg(std::set<Element, Compare, Allocator>& main_set,
        Stream::Dynamic& my_str) const
        throw (Exception, eh::Exception);

    private:
      /**
       * Try get symbol from stream, return when we should create
       * lexeme tree node.
       * ,lexeme - ignore
       * ,,lexeme - return after second ,
       * @param istr Input stream that contain description of lexemes.
       * @return true - flush is not need, stream may be in fail state.
       * false - and you should create tree node and call flush
       * to put empty lexeme
       */
      bool
      pass_separator_symbols_(std::istream& istr) throw (eh::Exception);

      void
      create_tree_() throw (eh::Exception);

      /**
       * Simple create node and add it to tree.
       * @param start_general_node tree to stick node.
       */
      void
      create_node_(TreeNode_var& start_general_node)
        throw (Exception);

      /**
       * Construct child node and stick it to start_general_node,
       * set current_node to it. Check recursion depth.
       * @param start_general_node tree node to stick child.
       * @return final_general_node, mark for the end of resulting range.
       */
      TreeNode_var
      create_tree_node_(TreeNode_var& start_general_node)
        throw (Exception, NoncriticalException);

      /**
       * raise NoncriticalException if
       * @param istr has bad state, and
       * @param stop_symbol isn't current symbol.
       */
      void
      check_readability_(std::istream& istr, char stop_symbol) const
        throw (NoncriticalException);

      void
      recognize_shielded_symbol_(std::istream& istr)
        throw (Exception, NoncriticalException, eh::Exception);

      AnalyzerParams init_params_;
      Generics::ActiveObjectCallback_var callback_;

      char current_symbol_;
      TreeNode_var cur_lexeme_tree_;
      TreeNode_var current_node_;
      unsigned short int recursion_depth_;
    };
  } // namespace SequenceAnalyzer
}

namespace String
{
  namespace SequenceAnalyzer
  {
    inline
    bool
    Analyzer::check_symbol_in_set(const CharSet& ch_set, char ch)
      throw (eh::Exception)
    {
      return ch_set(ch);
    }

    inline
    bool
    Analyzer::not_padding_symb(char ch) const
      throw (eh::Exception)
    {
      return ch != init_params_.padding_symb;
    }

    inline
    bool
    Analyzer::not_digit(char ch) throw ()
    {
      return !String::AsciiStringManip::NUMBER(ch);
    }

    inline
    bool
    Analyzer::not_allowed_char_range(char ch) const
      throw (eh::Exception)
    {
      return !check_symbol_in_set(init_params_.char_range_bounds, ch);
    }

    inline
    bool
    Analyzer::not_allowed_str_range(char ch) const
      throw (eh::Exception)
    {
      return !check_symbol_in_set(init_params_.str_char_range_bounds, ch);
    }

    inline
    bool
    Analyzer::check_allowed_int_range(unsigned int number) const
      throw (eh::Exception)
    {
      return init_params_.int_range_bounds.belongs(number);
    }

    inline
    void
    Analyzer::write_result_arg(std::ostream& main_ostr,
      Stream::Dynamic& my_str) const throw (Exception, eh::Exception)
    {
      main_ostr << init_params_.before_lexeme_out_str << my_str.str() <<
        init_params_.after_lexeme_out_str;
    }

    template <typename Element, typename Allocator>
    void
    Analyzer::write_result_arg(std::list<Element, Allocator>& main_list,
      Stream::Dynamic& my_str) const
      throw (Exception, eh::Exception)
    {
      main_list.push_back(my_str.str().str());
    }

    template <typename Element, typename Compare, typename Allocator>
    void
    Analyzer::write_result_arg(std::set<Element, Compare, Allocator>&
      main_set, Stream::Dynamic& my_str) const
      throw (Exception, eh::Exception)
    {
      main_set.insert(my_str.str());
    }

    template <typename ResultType>
    void
    Analyzer::flush_node(ConstTreeNode_var cur_node, ResultType& result_arg,
      Stream::Dynamic& ostr) const
      throw (Exception, eh::Exception)
    {
      for (unsigned short int i = 0; i < cur_node->repeat_amount; i++)
      {
        Stream::Dynamic my_str(4096);
        my_str << ostr.str();
        if (!cur_node->node_val.empty())
        {
          for (ChSeq::const_iterator it = cur_node->node_val.begin();
            it != cur_node->node_val.end(); ++it)
          {
            my_str << *it;
          }
        }

        if (!cur_node->child_list.empty())
        {
          for (TreeNode::Children::const_iterator iter =
            cur_node->child_list.begin();
            iter != cur_node->child_list.end(); ++iter)
          {
            flush_node(ConstTreeNode_var(*iter), result_arg, my_str);
          }
        }
        else
        {
          write_result_arg(result_arg, my_str);
        }
      }
    }

    template <typename ResultType>
    void
    Analyzer::interprete_and_flush_tree(ResultType& result_arg) const
      throw (Exception, eh::Exception)
    {
      Stream::Dynamic ostr(4096);
      flush_node(ConstTreeNode_var(cur_lexeme_tree_), result_arg, ostr);
    }

    template <typename ResultType>
    void
    Analyzer::process_range_sequence(std::istream& istr,
      ResultType& result_arg) throw (Exception, eh::Exception)
    {
      istr.get(current_symbol_);
      while (istr.good())
      {
        if (check_symbol_in_set(init_params_.range_separators,
          current_symbol_))
        {
          do
          {
            if (cur_lexeme_tree_)
            {
              interprete_and_flush_tree(result_arg);
              cur_lexeme_tree_.reset();
            }
          }
          while (!pass_separator_symbols_(istr));
        }
        else if (current_symbol_ == init_params_.range_symbol)
        {
          try
          {
            current_node_ = process_range_list(istr, false,
              init_params_.range_part_symb.second(), current_node_);
          }
          catch (const NoncriticalException& e)
          {
            Stream::Error ostr;
            ostr << FNS << "Got NoncriticalException while trying to "
              "unroll range. Description: " << e.what() << "\n"
              "Proceeding from the nearest separator";

            callback_->warning(ostr.str());

            if (cur_lexeme_tree_)
            {
              cur_lexeme_tree_.reset();
              current_node_.reset();
            }
            recursion_depth_ = 0;

            while (istr.good() && !check_symbol_in_set(
              init_params_.range_separators, current_symbol_))
            {
              istr.get(current_symbol_);
            }
          }
          catch (const Exception& e)
          {
            Stream::Error ostr;
            ostr << FNS << "Got Exception while trying to unroll "
              "range. Description: " << e.what();
            throw Exception(ostr);
          }
        }
        else
        {
          try
          {
            if (!cur_lexeme_tree_)
            {
              create_tree_();
            }
            recognize_range_symbol(istr);
            istr.get(current_symbol_);
          }
          catch (const NoncriticalException& e)
          {
            Stream::Error ostr;
            ostr << FNS << "Got NoncriticalException while trying to "
              "process symbol. Description: " << e.what() << "\n"
              "Proceeding from the nearest separator";

            callback_->warning(ostr.str());

            if (cur_lexeme_tree_)
            {
              cur_lexeme_tree_.reset();
              current_node_.reset();
            }
            recursion_depth_ = 0;

            while (istr.good() && !check_symbol_in_set(
              init_params_.range_separators, current_symbol_))
            {
              istr.get(current_symbol_);
            }
          }
          catch (const Exception& e)
          {
            Stream::Error ostr;
            ostr << FNS << "Got Exception while trying to process "
              "symbol. Description: " << e.what();
            throw Exception(ostr);
          }
        }
      }
      if (cur_lexeme_tree_)
      {
        interprete_and_flush_tree(result_arg);
        cur_lexeme_tree_.reset();
      }
    }

    template <typename ResultType>
    void
    Analyzer::process_char_sequence(std::istream& istr,
      ResultType& result_arg) throw (Analyzer::Exception, eh::Exception)
    {
      if (!init_params_.immediate_range_mode)
      {
        istr.get(current_symbol_);
        while (istr.good())
        {
          if (check_symbol_in_set(init_params_.main_separators,
            current_symbol_))
          {
            do
            {
              if (cur_lexeme_tree_)
              {
                interprete_and_flush_tree(result_arg);
                cur_lexeme_tree_.reset();
              }
            }
            while (!pass_separator_symbols_(istr));
          }
          else
          {
            try
            {
              if (!cur_lexeme_tree_)
              {
                create_tree_();
              }
              recognize_symbol(istr);
              istr.get(current_symbol_);
            }
            catch (const NoncriticalException& e)
            {
              Stream::Error ostr;
              ostr << FNS << 
                "Got NoncriticalException while trying to process "
                "symbol. Description: " << e.what() << "\n"
                "Proceeding from the nearest separator";

              callback_->warning(ostr.str());

              cur_lexeme_tree_.reset();
              current_node_.reset();
              recursion_depth_ = 0;

              while (istr.good() || check_symbol_in_set(
                init_params_.main_separators, current_symbol_))
              {
                istr.get(current_symbol_);
              }
            }
            catch (const Exception& e)
            {
              Stream::Error ostr;
              ostr << FNS << "Got Exception while trying to process "
                "symbol. Description: " << e.what();
              throw Exception(ostr);
            }
          }
        }
        if (cur_lexeme_tree_)
        {
          interprete_and_flush_tree(result_arg);
          cur_lexeme_tree_.reset();
        }
      }
      else
      {
        process_range_sequence(istr, result_arg);
      }
    }
  } // namespace SequenceAnalyzer
}

#endif
