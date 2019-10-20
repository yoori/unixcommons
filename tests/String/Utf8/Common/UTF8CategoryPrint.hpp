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



// @file Common/UTF8CategoryPrint.hpp
#ifndef _STRING_UTF8CATEGORY_PRINT_HPP_
#define _STRING_UTF8CATEGORY_PRINT_HPP_

#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <string.h>
#include <String/UTF8Category.hpp>
#include <Stream/FlagsSaver.hpp>

namespace String
{

  class Utf8CategoryPrintable : public Utf8Category
  {
  public:
    /**
     * Default constructor. Make empty category.
     */
    Utf8CategoryPrintable() throw (eh::Exception);

    /**
     * empty
     */
    virtual
    ~Utf8CategoryPrintable() throw ();

    /**
     * Print internal N-arc tree of Utf8Category to C++ structs.
     * @param name - C++ identifier for main tree naming.
     */
    void
    print_to_cpp(const char* name) throw (eh::Exception);

    virtual void
    print_finish_leaf(UnicodeProperty::TreeLeaf leaf,
      char* current_prefix, unsigned current_depth)
      throw (eh::Exception);

    virtual const UnicodeProperty::Node*
    modificator() throw ();

    class FinishObjectsPool;

    virtual FinishObjectsPool*
    get_pool() const throw ();

    /**
     * Don't necessary use duplication objects as tree endings.
     * Single object is enough for all trees.
     */
    class FinishObjectsPool
    {
    public:
      /**
       * @param try put into pool, if already exist there - then
       * nothing.
       */
      virtual bool
      insert(const UnicodeProperty::TreeLeaf* value,
        char* current_prefix, unsigned current_depth)
        throw (eh::Exception);

      /**
       * Print pool content to C++ declarations.
       */
      virtual void
      print_to_cpp() const throw (eh::Exception);

      virtual
      ~FinishObjectsPool() throw () {}
    };

  private:
    /**
     * In most cases we want unique finish objects declarations
     * and so singleton ObjectsPool.
     */
    static std::unique_ptr<FinishObjectsPool> pool_;

  protected:
    /**
     * Need for right carry management.
     * Store current position and push carry at the right moment.
     * Not thread-safe.
     */
    class FormatGuard_
    {
    public:
      /**
       * @param len - forecast length of output.
       * if output exceed 70 positions, we carry to new line.
       */
      FormatGuard_(size_t len) throw ();
      /**
       * Renew stored number of filled positions.
       */
      ~FormatGuard_() throw ();
      /**
       * Clearance internal state to zero out position.
       */
      static void
      reset() throw ();
    private:
      static std::size_t
      current_length_(std::size_t add, bool reset = false) throw ();

      std::size_t additional_length_;
    };

    /**
     * Guard that print namespace String { namespace UnicodeProperty {
     * at construction and } } at destruction.
     */
    struct NamespaceDecorator_
    {
      NamespaceDecorator_() throw (eh::Exception);
      ~NamespaceDecorator_() throw ();
    };

    unsigned
    get_size_() throw ();

    /**
     * Recursively round the
     * @param current N-arc tree
     * @param name C++ id for entrance struct
     * @param arr_len count of elements in current tree node. May be 256, 64
     */
    bool
    print_to_cpp_(const UnicodeProperty::Node* current,
                  const char* name,
                  std::size_t arr_len = 256,
                  unsigned rec_depth = 0)
      throw (eh::Exception);

    // save information about processing UTF8-byte sequence when down by the tree
    static char current_symbol_[5];

    static std::size_t memory_used_;
  };

  /**
   * Get additional information about each symbols into underlying
   * UTF8Category and print specialized tree C structures
   */
  class Utf8CategoryExtendedPrintable : public Utf8CategoryPrintable
  {
  public:
    typedef std::map<UnicodeSymbol, uint8_t> SymbolProperties;

    /**
     * Get additional info linked with UnicodeSymbol
     */
    Utf8CategoryExtendedPrintable(SymbolProperties& add_info)
      throw (eh::Exception);

    virtual
    ~Utf8CategoryExtendedPrintable() throw ();

    virtual void
    print_finish_leaf(UnicodeProperty::TreeLeaf leaf,
      char* current_prefix, unsigned current_depth)
      throw (eh::Exception);

    void
    print_finishers_to_cpp() throw (eh::Exception);

    virtual const UnicodeProperty::Node*
    modificator() throw ();

    virtual FinishObjectsPool*
    get_pool() const throw ();

    class FinishDataObjectsPool : public FinishObjectsPool
    {
    public:
      FinishDataObjectsPool(SymbolProperties& add_info)
        throw (eh::Exception);

      virtual
      ~FinishDataObjectsPool() throw ();

      /**
       * @param try put into pool, if already exist there - then
       * nothing.
       */
      virtual bool
      insert(const UnicodeProperty::TreeLeaf* value,
        char* current_prefix, unsigned current_depth)
        throw (eh::Exception);

      /**
       * Print pool content to C++ declarations.
       */
      virtual void
      print_to_cpp() const throw (eh::Exception);

      struct TreeLeaf
      {
        bool
        operator <(const TreeLeaf& rhs) const throw ();

        static bool
        compare(const TreeLeaf& left, const TreeLeaf& right) throw ();

        unsigned char raw_data[64];
        std::size_t name_val;
      };

      const TreeLeaf&
      get_leaf(char* current_prefix,
        unsigned current_depth) const
        throw (eh::Exception);

    private:

      bool
      create_leaf_(TreeLeaf& leaf, char* current_prefix,
        unsigned current_depth) const throw (eh::Exception);

      SymbolProperties& add_info_;

      typedef std::set<TreeLeaf> ObjectsContainer;
      ObjectsContainer ranges_;
      // using to naming objects
      static std::size_t object_counter_;
    };

  private:

    static std::unique_ptr<FinishDataObjectsPool> data_objects_pool_;

    static const UnicodeProperty::TreeNode ALL_TREE_STOP;
  };
} // namespace String

//////////////////////////////////////////////////////////////////////////
//    Implementations
//////////////////////////////////////////////////////////////////////////

namespace String
{
  char Utf8CategoryPrintable::current_symbol_[5];

  std::unique_ptr<Utf8CategoryExtendedPrintable::FinishDataObjectsPool>
    Utf8CategoryExtendedPrintable::data_objects_pool_;

  std::size_t Utf8CategoryExtendedPrintable::FinishDataObjectsPool::
    object_counter_ = 0;

  const UnicodeProperty::TreeNode
    Utf8CategoryExtendedPrintable::ALL_TREE_STOP =
  {
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
    &UnicodeProperty::TREE_STOP, &UnicodeProperty::TREE_STOP,
  };

  //
  // class Utf8CategoryPrintable
  //

  inline
  Utf8CategoryPrintable::Utf8CategoryPrintable() throw (eh::Exception)
    : Utf8Category(Utf8Set::Utf8Chars())
  {
  }

  inline void
  Utf8CategoryPrintable::print_to_cpp(const char* name)
    throw (eh::Exception)
  {
    std::cout << "// @file String/UTF8Is";
    {
      const char* p = strchr(name, '_');
      if (p)
      {
        std::cout << String::AsciiStringManip::to_upper(p[1]) << p + 2 <<
          String::AsciiStringManip::to_upper(*name);
        std::cout.write(name + 1, p - name - 1);
      }
      else
      {
        std::cout << String::AsciiStringManip::to_upper(*name) << name + 1;
      }
    }
    std::cout << ".cpp\n" <<
      "#include <String/UTF8IsProperty.hpp>\n" << std::endl;
    NamespaceDecorator_ guard;
    memory_used_ = 0;
    memset(current_symbol_, 0, sizeof(current_symbol_));
    std::string upname(name);
    String::AsciiStringManip::to_upper(upname);
    upname += "_TREE";
    print_to_cpp_(&*get_container_(), upname.c_str());
#if 0
    std::cout << "    // This tree occupies " << memory_used_
      << " bytes in memory." << std::endl <<
      "    // + some memory for tree finishers objects,"
      " if used it." << std::endl;
#endif
  }

  //
  // Utf8CategoryPrintable class
  //

  Utf8CategoryPrintable::~Utf8CategoryPrintable() throw ()
  {
  }

  const UnicodeProperty::Node*
  Utf8CategoryPrintable::modificator() throw ()
  {
    return &UnicodeProperty::TREE_STOP;
  }

  Utf8CategoryPrintable::FinishObjectsPool*
  Utf8CategoryPrintable::get_pool() const throw ()
  {
    return pool_.get();
  }

  //
  // class Utf8CategoryPrintable::FinishObjectsPool
  //

  std::unique_ptr<Utf8CategoryPrintable::FinishObjectsPool>
    Utf8CategoryPrintable::pool_(new
      Utf8CategoryPrintable::FinishObjectsPool);

  inline bool
  Utf8CategoryPrintable::FinishObjectsPool::insert(
    const UnicodeProperty::TreeLeaf* value, char*, unsigned)
    throw (eh::Exception)
  {
    return *value != 0;
  }

  inline void
  Utf8CategoryPrintable::FinishObjectsPool::print_to_cpp() const
    throw (eh::Exception)
  {
  }

  //
  // Utf8CategoryExtendedPrintable class
  //

  Utf8CategoryExtendedPrintable::Utf8CategoryExtendedPrintable(
    SymbolProperties& add_info)
    throw (eh::Exception)
  {
    data_objects_pool_.reset(new
      Utf8CategoryExtendedPrintable::FinishDataObjectsPool(add_info));
  }

  const UnicodeProperty::Node*
  Utf8CategoryExtendedPrintable::modificator() throw ()
  {
    return &*ALL_TREE_STOP;
  }

  Utf8CategoryPrintable::FinishObjectsPool*
  Utf8CategoryExtendedPrintable::get_pool() const throw ()
  {
    return data_objects_pool_.get();
  }

  Utf8CategoryExtendedPrintable::~Utf8CategoryExtendedPrintable() throw ()
  {
  }

  void
  Utf8CategoryExtendedPrintable::print_finish_leaf(
    UnicodeProperty::TreeLeaf leaf, char* current_prefix,
    unsigned current_depth)
    throw (eh::Exception)
  {
    if (!leaf)
    {
      FormatGuard_ fg(3);
      std::cout << "0, ";
    }
    else if (static_cast<unsigned char>(*current_prefix) <= 0x7F)
    {
      FormatGuard_ fg(6);
      std::cout << "0x" << std::setw(2) << std::hex << static_cast<int>(
        data_objects_pool_->get_leaf(current_prefix, current_depth).
          raw_data[0]) << ", ";
    }
    else
    {
      FormatGuard_ fg(10);
      std::cout << "LEAF_" << std::setw(2) << std::hex <<
        data_objects_pool_->
          get_leaf(current_prefix, current_depth).name_val << ", ";
    }
  }

  inline void
  Utf8CategoryExtendedPrintable::print_finishers_to_cpp() throw (eh::Exception)
  {
    Stream::FlagsSaver save_flags(std::cout);
    std::cout << std::uppercase;
    get_pool()->print_to_cpp();
  }


  //
  // Utf8CategoryExtendedPrintable::FinishDataObjectsPool class
  //

  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::
    FinishDataObjectsPool(SymbolProperties& add_info)
    throw (eh::Exception)
    : add_info_(add_info)
  {
  }

  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::
    ~FinishDataObjectsPool() throw ()
  {
  }

  inline bool
  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::create_leaf_(
    TreeLeaf& leaf, char* current_prefix, unsigned current_depth) const
    throw (eh::Exception)
  {
    bool empty = true;
    for (unsigned char ch = 0; ch < 64; ++ch)
    {
      current_prefix[current_depth] = 0x80 + ch;
      current_prefix[current_depth + 1] = 0;
      SymbolProperties::const_iterator it;
      try
      {
        UnicodeSymbol sym(current_prefix);
        it = add_info_.find(sym);
      }
      catch (const UnicodeSymbol::RangeException&)
      {
        it = add_info_.end();
      }
      leaf.raw_data[ch] = it == add_info_.end() ? 0 : it->second;
      empty = empty && leaf.raw_data[ch] == 0;
    }
    return empty;
  }

  inline bool
  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::insert(
    const UnicodeProperty::TreeLeaf*, char* current_prefix,
    unsigned current_depth)
    throw (eh::Exception)
  {
    TreeLeaf leaf;
    if (create_leaf_(leaf, current_prefix, current_depth))
    {
      return false;
    }
    std::pair<ObjectsContainer::iterator, bool> inserted =
      ranges_.insert(leaf);
    if (inserted.second)
    {
      std::ostringstream ost;
      ost << std::hex << std::uppercase << std::setw(2) << object_counter_;
      leaf.name_val = object_counter_++;
      ranges_.erase(leaf);
      ranges_.insert(leaf);
    }
    return true;
  }

  const Utf8CategoryExtendedPrintable::FinishDataObjectsPool::TreeLeaf&
  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::get_leaf(
    char* current_prefix, unsigned current_depth) const
    throw (eh::Exception)
  {
    TreeLeaf leaf;
    create_leaf_(leaf, current_prefix, current_depth);
    ObjectsContainer::const_iterator it = ranges_.find(leaf);
    assert(it != ranges_.end());
    return *it;
  }

  inline void
  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::print_to_cpp() const
    throw (eh::Exception)
  {
    typedef std::vector<TreeLeaf> NameSorter;
    NameSorter sorter(ranges_.begin(), ranges_.end());
    std::sort(sorter.begin(), sorter.end(), TreeLeaf::compare);

    NamespaceDecorator_ guard;
    for (NameSorter::const_iterator it(sorter.begin());
      it != sorter.end(); ++it)
    {
      std::cout << "    static AllTreeLeaf LEAF_" << std::setw(2)
        << std::hex << (*it).name_val << " =\n    {";
      std::size_t slicer = 11;
      for (std::size_t i = 0; i < 64; ++i)
      {
        if (slicer++ == 11)
        {
          std::cout << "\n     ";
          slicer = 0;
        }
        std::cout << " 0x" << std::setw(2) << (int)(*it).raw_data[i] << ",";
      }

      std::cout << "\n    };\n"  << std::endl;
    }
  }

  bool
  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::TreeLeaf::
    operator <(const TreeLeaf& rhs) const throw ()
  {
    return memcmp(raw_data, rhs.raw_data, sizeof(raw_data)) < 0;
  }

  bool
  Utf8CategoryExtendedPrintable::FinishDataObjectsPool::TreeLeaf::
    compare(const TreeLeaf& left, const TreeLeaf& right) throw ()
  {
    return left.name_val < right.name_val;
  }

  //
  // class Utf8CategoryPrintable::FormatGuard_
  //

  inline
  Utf8CategoryPrintable::FormatGuard_::FormatGuard_(size_t len) throw ()
    : additional_length_(len)
  {
    if (current_length_(0) + additional_length_ > 70)
    {
      std::cout << std::endl << "      ";
      current_length_(0, true);
    }
  }

  inline
  Utf8CategoryPrintable::FormatGuard_::~FormatGuard_() throw ()
  {
    current_length_(additional_length_);
  }

  inline void
  Utf8CategoryPrintable::FormatGuard_::reset() throw ()
  {
    current_length_(0, true);
  }

  inline std::size_t
  Utf8CategoryPrintable::FormatGuard_::current_length_(std::size_t add,
                                                       bool reset)// = false
    throw ()
  {
    static size_t len = 0;
    if (reset)
    {
      len = 0;
      return len;
    }
    len += add;
    return len;
  }

  //
  // class Utf8CategoryPrintable::NamespaceDecorator_
  //

  inline
  Utf8CategoryPrintable::NamespaceDecorator_::NamespaceDecorator_()
    throw (eh::Exception)
  {
    std::cout << std::endl
      << "namespace String\n{\n  namespace UnicodeProperty\n  {"
      << std::endl;
  }

  inline
  Utf8CategoryPrintable::NamespaceDecorator_::~NamespaceDecorator_()
    throw ()
  {
    std::cout
      << "\n  } // namespace UnicodeProperty\n} // namespace String"
      << std::endl;
    std::cout << std::endl;
  }

  //
  // class Utf8CategoryPrintable, private members
  //

  std::size_t Utf8CategoryPrintable::memory_used_ = 0;

  inline unsigned
  Utf8CategoryPrintable::get_size_() throw ()
  {
    return UTF8Handler::get_octet_count(*current_symbol_);
  }

  void
  Utf8CategoryPrintable::print_finish_leaf(
    UnicodeProperty::TreeLeaf leaf, char*, unsigned)
    throw (eh::Exception)
  {
    FormatGuard_ fg(22);
    std::cout << "0x" << std::setw(16) << std::hex << leaf << "ull, ";
  }

  inline bool
  Utf8CategoryPrintable::print_to_cpp_(const UnicodeProperty::Node* current,
                                       const char* name,
                                       std::size_t arr_len,
                                       unsigned rec_depth)
    throw (eh::Exception)
  {
    bool notempty[256], total = false;
    memory_used_ += sizeof(current);
    Stream::FlagsSaver save_flags(std::cout);
    std::cout << std::uppercase << std::hex;
    std::cout.fill('0');
    // Print current content and go to downstairs nodes
    for (std::size_t i = 0; i < arr_len; ++i)
    {
      current_symbol_[rec_depth] = (arr_len != 64) ? i : 0x80 + i;
      current_symbol_[rec_depth + 1] = 0;
      unsigned size = get_size_();
      const UnicodeProperty::Node* current_ch = &current[i];
      if (!size)
      {
        notempty[i] = false;
      }
      else
      {
        if (size == rec_depth + 2)
        {
          notempty[i] = get_pool()->insert(&current_ch->leaf,
            current_symbol_, rec_depth + 1);
        }
        else
        {
          if (current_ch->node == &UnicodeProperty::TREE_STOP)
          {
            current_ch = modificator();
          }
          if (current_ch->node == 0)
          {
            notempty[i] = false;
          }
          else if (current_ch->node == &UnicodeProperty::TREE_STOP)
          {
            notempty[i] = true;
          }
          else
          {
            std::ostringstream ostr;
            ostr << "NODE";
            for (unsigned j = 0; j <= rec_depth; j++)
            {
              ostr << "_" << std::uppercase << std::setw(2) << std::hex <<
                static_cast<unsigned>(
                  static_cast<unsigned char>(current_symbol_[j]));
            }
            notempty[i] = print_to_cpp_(current_ch->node,
              ostr.str().c_str(), 64, rec_depth + 1);
          }
        }
      }
      total = total || notempty[i];
    }

    if (!total)
    {
      return false;
    }

    std::cout << "    " <<
      (rec_depth ? "static Tree" : "TreeStart") <<
      "Node " << name << " =" << std::endl << "    {" << std::endl <<
      "      ";
    for (std::size_t i = 0; i < arr_len; ++i)
    {
      current_symbol_[rec_depth] = (arr_len != 64) ? i : 0x80 + i;
      current_symbol_[rec_depth + 1] = 0;
      unsigned size = get_size_();
      const UnicodeProperty::Node* current_ch = &current[i];
      if (size != rec_depth + 2)
      {
        if (!notempty[i] || current_ch->node == 0)
        {
          FormatGuard_ fg(3);
          std::cout << "0, ";
        }
        else
        {
          if (current_ch->node == &UnicodeProperty::TREE_STOP)
          {
            current_ch = modificator();
          }
          if (current_ch->node == &UnicodeProperty::TREE_STOP)
          {
            FormatGuard_ fg(12);
            std::cout << "&TREE_STOP, ";
          }
          else
          {
            std::ostringstream ostr;
            ostr << "NODE";
            for (unsigned j = 0; j <= rec_depth; j++)
            {
              ostr << "_" << std::uppercase << std::setw(2) << std::hex <<
                static_cast<unsigned>(
                  static_cast<unsigned char>(current_symbol_[j]));
            }
            ostr << ", ";
            const std::string& str = ostr.str();
            FormatGuard_ fg(str.size());
            std::cout << str;
          }
        }
      }
      else
      {
        if (size)
        { // FinishNode
          print_finish_leaf(current_ch->leaf,
          current_symbol_, rec_depth + 1);
        }
      }
    }
    std::cout << std::endl << "    };";
    if (rec_depth)
    {
      std::cout << std::endl  << std::endl;
    }
    FormatGuard_::reset();

    return true;
  }
} // namespace String

#endif  // _STRING_UTF8CATEGORY_PRINT_HPP_
