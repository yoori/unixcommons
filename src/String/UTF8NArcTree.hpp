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





#ifndef STRING_UTF8_NARCTREE_HPP
#define STRING_UTF8_NARCTREE_HPP

#include <String/UTF8Handler.hpp>


namespace String
{
  namespace UnicodeProperty
  {
    typedef uint64_t TreeLeaf;
    union Node
    {
      constexpr
      Node() throw ();
      constexpr
      Node(int) throw ();
      constexpr
      Node(long long unsigned int leaf) throw ();
      constexpr
      Node(const Node* node) throw ();

      TreeLeaf leaf;
      const Node* node;
    };
    typedef const Node TreeStartNode[256];
    typedef const Node TreeNode[64];
    extern const Node TREE_STOP;

    /**
     * function compute value exist or not into tree
     * @param tree Tree for function evaluation
     * @param str input data with checking character UTF-8 encoded.
     */
    bool
    belong(const TreeStartNode& tree, const char* str) throw ();
  } // namespace UnicodeProperty
} // namespace String

//
// INLINES
//

namespace String
{
  namespace UnicodeProperty
  {
    //
    // Node union
    //

    inline
    constexpr
    Node::Node() throw ()
      : node(0)
    {
    }

    inline
    constexpr
    Node::Node(int) throw ()
      : node(0)
    {
    }

    inline
    constexpr
    Node::Node(long long unsigned int leaf) throw ()
      : leaf(leaf)
    {
    }

    inline
    constexpr
    Node::Node(const Node* node) throw ()
      : node(node)
    {
    }


    inline
    bool
    belong(const TreeStartNode& tree, const char* str) throw ()
    {
      const Node* current_tree = &tree[static_cast<uint8_t>(*str)];
      for (unsigned long depth = UTF8Handler::get_octet_count(*str);
        depth != 2; depth--)
      {
        if (!current_tree->node)
        {
          return false;
        }
        if (current_tree->node == &TREE_STOP)
        {
          return true;
        }
        current_tree = &current_tree->node[
            static_cast<uint8_t>(*++str) & 0x3F];
      }
      return (current_tree->leaf & (static_cast<TreeLeaf>(1) <<
        (static_cast<uint8_t>(*(++str)) & 0x3F))) != 0;
    }
  }
}

#endif
