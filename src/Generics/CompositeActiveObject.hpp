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



// Generics/CompositeActiveObject.hpp
#ifndef GENERICS_COMPOSITE_ACTIVE_OBJECT_HPP
#define GENERICS_COMPOSITE_ACTIVE_OBJECT_HPP

#include <algorithm>
#include <functional>
#include <set>

#include <ReferenceCounting/Deque.hpp>

#include <Generics/ActiveObject.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  /**
   * class CompositeActiveObjectBase
   * This implements Active Object for control of several Active Objects.
   * Use this class to delegate active/inactive state control
   * over some set of Active Objects into one Active Object. In other words,
   * it usefully for central active/deactivate state management.
   * You SHOULD NOT change active/deactivate status of any added child
   * object into this CompositeActiveObject by yourself. Use it simply,
   * and activate or deactivate they through this holder. It will give
   * you guarantee of straight activation order, first added - first
   * activated, and guarantee for stopping object, last added, deactivate
   * and join first (reverse order). Optionally, you can switch off
   * stop ordering, it will be faster than consequently stopping and
   * safe for independent ActiveObjects (other words, use parallel stopping
   * if allow application logic). This behavior tune by the constructor
   * parameter sync_termination.
   * CompositeActiveObject is reference countable object.
   */
  template <typename Container, typename FrontIns, typename BackIns>
  class CompositeActiveObjectBase : public SimpleActiveObject
  {
  public:
    DECLARE_EXCEPTION(ChildException, ActiveObject::Exception);
    DECLARE_EXCEPTION(CompositeAlreadyActive, ActiveObject::AlreadyActive);

    /**
     * Construct empty not active container for
     * ActiveObjects.
     * @param sync_termination true means do immediately wait after each
     * child Active Object deactivation.
     * @param clear_on_exit whether to call clear() in destructor or not
     */
    explicit
    CompositeActiveObjectBase(bool sync_termination = false,
      bool clear_on_exit = true) throw ();

    /**
     * Calls clear() for all owned objects
     */
    virtual
    void
    clear() throw (eh::Exception);

    /**
     * Deactivate and wait for stop for all owned Active Objects.
     * Clears list of the objects.
     */
    void
    clear_children() throw (Exception, eh::Exception);

    /**
     * This method fills CompositeActiveObject with other Active
     * objects. Control for consistent state, any time all object
     * that it own must be active or not active.
     * You delegate active state control to this container. Use
     * residual object only for work flow, do not change active status
     * through it.
     * @param child object that should go under the management of container.
     * @param add_to_head whether the object should be added to the head
     * of the list of contained objects or to the tail.
     */
    void
    add_child_object(ActiveObject* child, bool add_to_head = false)
      throw (Exception, eh::Exception);

  protected:
    /**
     * Perform deactivating all owned objects, and waits for
     * its completion.
     */
    virtual
    ~CompositeActiveObjectBase() throw ();

    // SimpleActiveObject interface
    /**
     * Activate all owned active objects. For empty case, simply change
     * status to Active. Throw CompositeAlreadyActive,
     * if you try to activate twice. All object activated successfully
     * or stay deactivated, if we were not able to activate any in the set.
     */
    virtual
    void
    activate_object_()
      throw (CompositeAlreadyActive, ChildException, eh::Exception);

    /**
     * Deactivate (initiate stopping) all owned Active Objects.
     * Do nothing for empty and already deactivating object.
     * Perform deactivation as LIFO, last added Active Object
     * will start deactivation first.
     */
    virtual
    void
    deactivate_object_() throw (Exception, eh::Exception);

    /**
     * Waits for deactivation all owned completion.
     * Perform waits as LIFO, last added Active Object will wait first.
     * That logic correspond deactivate_object method.
     */
    virtual
    void
    wait_object_() throw (Exception, eh::Exception);

    /**
     * Simply calls wait_object for the given interval of objects
     */
    template <typename ReverseIterator>
    void
    wait_for_some_objects_(ReverseIterator rbegin, ReverseIterator rend)
      throw (Exception, eh::Exception);

    /**
     * Thread-unsafe deactivation logic
     */
    void
    deactivate_object_(typename Container::reverse_iterator rit)
      throw (Exception, eh::Exception);

  protected:
    const bool SYNCHRONOUS_;
    const bool CLEAR_ON_EXIT_;

    Container child_objects_;
  };

  /**
   * Default CompositeActiveObject containing a deque of ActiveObject_var.
   */
  typedef CompositeActiveObjectBase<
    ReferenceCounting::Deque<ActiveObject_var>,
    std::front_insert_iterator<ReferenceCounting::Deque<ActiveObject_var>>,
    std::back_insert_iterator<ReferenceCounting::Deque<ActiveObject_var>>>
    CompositeActiveObject;
  typedef ReferenceCounting::QualPtr<CompositeActiveObject>
    CompositeActiveObject_var;


  class RemovableActiveObject;

  class ActiveObjectChildRemover :
    public virtual ReferenceCounting::Interface
  {
  protected:
    virtual
    void
    remove_child_(ActiveObject* child) throw () = 0;

    virtual
    ~ActiveObjectChildRemover() throw () = default;

    friend class RemovableActiveObject;
  };
  typedef ReferenceCounting::QualPtr<ActiveObjectChildRemover>
    ActiveObjectChildRemover_var;


  /**
   * Special CompositeActiveObject containing a set of ActiveObject* and
   * allowing erasing items via ActiveObjectChildRemover and
   * RemovableActiveObjects interfaces
   */
  class CompositeSetActiveObject :
    public virtual ReferenceCounting::AtomicImpl,
    public CompositeActiveObjectBase<std::set<ActiveObject*>,
      Inserter<std::set<ActiveObject*>>,
      Inserter<std::set<ActiveObject*>>>,
    public ActiveObjectChildRemover
  {
  public:
    explicit
    CompositeSetActiveObject(bool sync_termination = false)
      throw (eh::Exception);

  protected:
    virtual
    ~CompositeSetActiveObject() throw () = default;

    virtual
    void
    remove_child_(ActiveObject* child) throw ();
  };
  typedef ReferenceCounting::QualPtr<CompositeSetActiveObject>
    CompositeSetActiveObject_var;


  /**
   * General implementation of automated ActiveObject removal.
   * Useful with CompositeSetActiveObject.
   * User may override before_remove_child_ function.
   */
  class RemovableActiveObject :
    public virtual ActiveObject,
    public virtual ReferenceCounting::AtomicImpl
  {
  public:
    explicit
    RemovableActiveObject(ActiveObjectChildRemover* owner) throw ();

  protected:
    virtual
    ~RemovableActiveObject() throw () = default;

    virtual
    void
    delete_this_() const throw ();

    virtual
    void
    before_remove_child_() throw ();

    ActiveObjectChildRemover_var owner_;
  };
}

namespace Generics
{
  template <typename Container, typename FrontIns, typename BackIns>
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    CompositeActiveObjectBase(bool sync_termination, bool clear_on_exit)
    throw ()
    : SYNCHRONOUS_(sync_termination), CLEAR_ON_EXIT_(clear_on_exit)
  {
  }

  template <typename Container, typename FrontIns, typename BackIns>
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    ~CompositeActiveObjectBase() throw ()
  {
    if (CLEAR_ON_EXIT_)
    {
      try
      {
        clear();
      }
      catch (...)
      {
      }
    }

    try
    {
      clear_children();
    }
    catch (...)
    {
    }
  }

  template <typename Container, typename FrontIns, typename BackIns>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    activate_object_()
    throw (CompositeAlreadyActive, ChildException, eh::Exception)
  {
    typename Container::iterator it(child_objects_.begin());
    try
    {
      for (; it != child_objects_.end(); ++it)
      {
        (*it)->activate_object();
      }
    }
    catch (const eh::Exception& e)
    {
      Stream::Error all_errors;
      try
      {
        state_ = AS_DEACTIVATING;
        typename Container::reverse_iterator rit(it);
        deactivate_object_(rit);
        wait_for_some_objects_(rit, child_objects_.rend());
      }
      catch (const eh::Exception& e)
      {
        all_errors << e.what();
      }
      state_ = AS_NOT_ACTIVE;
      Stream::Error ostr;
      ostr << FNS << e.what();
      const String::SubString& all_errors_str = all_errors.str();
      if (all_errors_str.size())
      {
        ostr << all_errors_str;
      }
      throw ChildException(ostr);
    }
  }

  template <typename Container, typename FrontIns, typename BackIns>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    deactivate_object_() throw (Exception, eh::Exception)
  {
    deactivate_object_(child_objects_.rbegin());
  }

  template <typename Container, typename FrontIns, typename BackIns>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    wait_object_() throw (Exception, eh::Exception)
  {
    ReferenceCounting::Deque<ActiveObject_var> copy_of_child_objects;
    {
      Sync::ConditionalGuard guard(cond_);
      for (typename Container::iterator itor = child_objects_.begin();
        itor != child_objects_.end(); ++itor)
      {
        copy_of_child_objects.push_back(ReferenceCounting::add_ref(*itor));
      }
    }
    wait_for_some_objects_(copy_of_child_objects.rbegin(),
      copy_of_child_objects.rend());
  }

  template <typename Container, typename FrontIns, typename BackIns>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    add_child_object(ActiveObject* child, bool add_to_head)
    throw (Exception, eh::Exception)
  {
    Sync::PosixGuard guard(cond_);

    try
    {
      if (state_ == AS_ACTIVE)
      {
        if (!child->active())
        {
          child->activate_object();
        }
      }
      else
      {
        if (child->active())
        {
          child->deactivate_object();
          child->wait_object();
        }
      }
      ActiveObject_var ch(ReferenceCounting::add_ref(child));
      if (add_to_head)
      {
        *FrontIns(child_objects_) = std::move(ch);
      }
      else
      {
        *BackIns(child_objects_) = std::move(ch);
      }
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't add object. Caught eh::Exception: " <<
        ex.what();
      throw Exception(ostr);
    }
  }

  template <typename Container, typename FrontIns, typename BackIns>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    clear_children() throw (Exception, eh::Exception)
  {
    Sync::PosixGuard guard(cond_);

    if (state_ != AS_NOT_ACTIVE)
    {
      typename Container::reverse_iterator rit(child_objects_.rbegin());
      deactivate_object_(rit);
      wait_for_some_objects_(rit, child_objects_.rend());
      state_ = AS_NOT_ACTIVE;
    }
    child_objects_.clear();
  }

  template <typename Container, typename FrontIns, typename BackIns>
  template <typename ReverseIterator>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    wait_for_some_objects_(ReverseIterator rit, ReverseIterator rend)
    throw (Exception, eh::Exception)
  {
    Stream::Error all_errors;

    for (; rit != rend; ++rit)
    {
      try
      {
        (*rit)->wait_object();
      }
      catch (const eh::Exception& ex)
      {
        all_errors << ex.what() << std::endl;
      }
    }
    const String::SubString& all_errors_str = all_errors.str();
    if (all_errors_str.size())
    {
      Stream::Error ostr;
      ostr << FNS <<
        "Can't wait child active object. Caught eh::Exception:\n";
      ostr << all_errors_str;
      throw Exception(ostr);
    }
  }

  template <typename Container, typename FrontIns, typename BackIns>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    deactivate_object_(typename Container::reverse_iterator rit)
    throw (Exception, eh::Exception)
  {
    Stream::Error all_errors;

    for (; rit != child_objects_.rend(); ++rit)
    {
      try
      {
        (*rit)->deactivate_object();
        if (SYNCHRONOUS_)
        {
          (*rit)->wait_object();
        }
      }
      catch (const eh::Exception& ex)
      {
        all_errors << ex.what() << std::endl;
      }
    }
    const String::SubString& all_errors_str = all_errors.str();
    if (all_errors_str.size())
    {
      Stream::Error ostr;
      ostr << FNS <<
        "Can't deactivate child active object. Caught eh::Exception:\n";
      ostr << all_errors_str;
      throw Exception(ostr);
    }
  }

  template <typename Container, typename FrontIns, typename BackIns>
  void
  CompositeActiveObjectBase<Container, FrontIns, BackIns>::
    clear() throw (eh::Exception)
  {
    Sync::PosixGuard guard(cond_);

    for (typename Container::iterator it(child_objects_.begin());
      it != child_objects_.end(); ++it)
    {
      (*it)->clear();
    }
  }
}

#endif
