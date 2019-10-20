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



#ifndef INCHASHTABLE_HPP
#define INCHASHTABLE_HPP

#include <Generics/CRC.hpp>
#include <Generics/GnuHashTable.hpp>


namespace Generics
{
  typedef size_t IncHashValue;

  template <typename CharType>
  struct IncHash
  {
    IncHashValue
    operator()() const throw ();
    IncHashValue
    operator()(IncHashValue hash, CharType hash_inc) const throw ();
  };

  template <typename CharType, typename ElementType,
    typename IncHashType = IncHash<CharType> >
  class IncHashTable
  {
  private:
    typedef IncHashValue Hash;

  public:
    typedef IncHashTable<CharType, ElementType, IncHashType> Container;
    typedef std::basic_string<CharType> Word;

  public:
    class WordHashAdapter
    {
    public:
      WordHashAdapter() throw (eh::Exception);
      WordHashAdapter(const Word& word) throw (eh::Exception);
      WordHashAdapter(const Word& word, Hash hash_val) throw (eh::Exception);

      void
      append(const CharType& key_char) throw (eh::Exception);

      Word&
      value() throw ();
      const Word&
      value() const throw ();

      operator Word&() throw ();
      operator const Word&() const throw ();

      Hash
      hash() const throw ();
      bool
      operator ==(const WordHashAdapter& right) const throw ();

    private:
      IncHashType inc_hash_op_;
      Word word_;
      Hash hash_;
    };

  private:
    typedef Generics::GnuHashTable<WordHashAdapter, ElementType> MainTable;
    typedef Generics::GnuHashSet<Generics::NumericHashAdapter<Hash> > InterTable;

  public:
    typedef WordHashAdapter key_type;
    typedef ElementType mapped_type;
    typedef typename MainTable::value_type value_type;
    typedef typename MainTable::const_iterator const_iterator;
    typedef typename MainTable::iterator iterator;

    struct ConstFinder
    {
      friend class IncHashTable<CharType, ElementType, IncHashType>;

    protected:
      ConstFinder(const Container* cont) throw (eh::Exception);

    public:
      bool
      find(const CharType& key_char) throw (eh::Exception);
      const ElementType*
      element() const throw ();

    private:
      const Container* cont_;
      WordHashAdapter word_hash_adapter_;
      typename MainTable::const_iterator main_table_it_;
    };

    const_iterator
    begin() const;
    const_iterator
    end() const;
    iterator
    begin();
    iterator
    end();

    ConstFinder
    finder() const;
    const_iterator
    find(const Word& key) const;
    const_iterator
    find(const WordHashAdapter& key) const;
    iterator
    find(const Word& key);
    iterator
    find(const WordHashAdapter& key);

    std::pair<const_iterator, bool>
    insert(const value_type& val);

    template <typename IteratorType>
    const_iterator
    find(const IteratorType& val_begin, const IteratorType& val_end) const;

    template <typename IteratorType>
    iterator
    find(const IteratorType& val_begin, const IteratorType& val_end);

  private:
    MainTable main_table_;
    InterTable inter_table_;
  };
}

namespace Generics
{
  //
  // IncHash class
  //

  template <typename CharType>
  IncHashValue
  IncHash<CharType>::operator()() const throw ()
  {
    return 0;
  }

  template <typename CharType>
  IncHashValue
  IncHash<CharType>::operator()(IncHashValue hash, CharType hash_inc) const
    throw ()
  {
    return Generics::CRC::quick(hash, &hash_inc, sizeof(hash_inc));
  }


  //
  // IncHashTable::ConstFinder class
  //

  template <typename CharType, typename ElementType, typename IncHashType>
  IncHashTable<CharType, ElementType, IncHashType>::
  ConstFinder::ConstFinder(const Container* cont)
    throw (eh::Exception)
    : cont_(cont)
  {
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  bool
  IncHashTable<CharType, ElementType, IncHashType>::
    ConstFinder::find(const CharType& key_char) throw (eh::Exception)
  {
    word_hash_adapter_.append(key_char);
    main_table_it_ = cont_->main_table_.find(word_hash_adapter_);

    return cont_->inter_table_.find(word_hash_adapter_.hash()) !=
      cont_->inter_table_.end();
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  const ElementType*
  IncHashTable<CharType, ElementType, IncHashType>::
    ConstFinder::element() const throw ()
  {
    return main_table_it_ != cont_->main_table_.end() ?
      &(main_table_it_->second) : 0;
  }


  //
  // IncHashTable::WordHashAdapter class
  //

  template <typename CharType, typename ElementType, typename IncHashType>
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::WordHashAdapter() throw (eh::Exception)
  {
    hash_ = inc_hash_op_();
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::WordHashAdapter(const Word& word) throw (eh::Exception)
    : word_(word)
  {
    hash_ = inc_hash_op_();
    for (typename Word::const_iterator it = word.begin();
      it != word.end(); ++it)
    {
      hash_ = inc_hash_op_(hash_, *it);
    }
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  IncHashTable<CharType, ElementType, IncHashType>::
  WordHashAdapter::WordHashAdapter(const Word& word, Hash hash_val)
    throw (eh::Exception)
    : word_(word), hash_(hash_val)
  {
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  void
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::append(const CharType& key_char) throw (eh::Exception)
  {
    word_.push_back(key_char);
    hash_ = inc_hash_op_(hash_, key_char);
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::Hash
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::hash() const throw ()
  {
    return hash_;
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::Word&
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::value() throw ()
  {
    return word_;
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  const typename IncHashTable<CharType, ElementType, IncHashType>::Word&
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::value() const throw ()
  {
    return word_;
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::operator Word&() throw ()
  {
    return word_;
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::operator const Word&() const throw ()
  {
    return word_;
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  bool
  IncHashTable<CharType, ElementType, IncHashType>::
    WordHashAdapter::operator ==(const WordHashAdapter& right) const throw ()
  {
    return word_ == right.word_;
  }


  //
  // IncHashTable class
  //

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::const_iterator
  IncHashTable<CharType, ElementType, IncHashType>::begin() const
  {
    return main_table_.begin();
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::const_iterator
  IncHashTable<CharType, ElementType, IncHashType>::end() const
  {
    return main_table_.end();
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::iterator
  IncHashTable<CharType, ElementType, IncHashType>::begin()
  {
    return main_table_.begin();
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::iterator
  IncHashTable<CharType, ElementType, IncHashType>::end()
  {
    return main_table_.end();
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::ConstFinder
  IncHashTable<CharType, ElementType, IncHashType>::finder() const
  {
    return ConstFinder(this);
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::const_iterator
  IncHashTable<CharType, ElementType, IncHashType>::find(
    const Word& key) const
  {
    return main_table_.find(WordHashAdapter(key));
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::const_iterator
  IncHashTable<CharType, ElementType, IncHashType>::find(
    const WordHashAdapter& key_adapter) const
  {
    return main_table_.find(key_adapter);
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::iterator
  IncHashTable<CharType, ElementType, IncHashType>::find(
    const Word& key)
  {
    return main_table_.find(WordHashAdapter(key));
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  typename IncHashTable<CharType, ElementType, IncHashType>::iterator
  IncHashTable<CharType, ElementType, IncHashType>::find(
    const WordHashAdapter& key_adapter)
  {
    return main_table_.find(key_adapter);
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  std::pair<
    typename IncHashTable<CharType, ElementType, IncHashType>::const_iterator,
    bool>
  IncHashTable<CharType, ElementType, IncHashType>::insert(
    const value_type& val)
  {
    IncHashType hash_op;
    Hash hash_cur = hash_op();

    if (!val.first.value().empty())
    {
      const Word& word = val.first.value();
      typename Word::const_iterator pre_end_it = --word.end();
      for (typename Word::const_iterator it = word.begin();
        it != pre_end_it; ++it)
      {
        hash_cur = hash_op(hash_cur, *it);
        inter_table_.insert(hash_cur);
      }

      hash_cur = hash_op(hash_cur, *pre_end_it);
    }

    return main_table_.insert(
      value_type(WordHashAdapter(val.first.value(), hash_cur), val.second));
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  template <typename IteratorType>
  typename IncHashTable<CharType, ElementType, IncHashType>::const_iterator
  IncHashTable<CharType, ElementType, IncHashType>::find(
    const IteratorType& val_begin, const IteratorType& val_end) const
  {
    Word word;
    word.assign(val_begin, val_end);
    return find(word);
  }

  template <typename CharType, typename ElementType, typename IncHashType>
  template <typename IteratorType>
  typename IncHashTable<CharType, ElementType, IncHashType>::iterator
  IncHashTable<CharType, ElementType, IncHashType>::find(
    const IteratorType& val_begin, const IteratorType& val_end)
  {
    Word word;
    word.assign(val_begin, val_end);
    return find(word);
  }
}

#endif
