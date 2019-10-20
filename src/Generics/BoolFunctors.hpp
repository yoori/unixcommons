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



#ifndef GENERICS_BOOLFUNCTORS_HPP
#define GENERICS_BOOLFUNCTORS_HPP

#include <utility>

#include <eh/Exception.hpp>


namespace Generics
{
  /**
   * Namespace for helper classes for BoolFunctors classes
   */
  namespace BoolFunctorsHelper
  {
    /**
     * Holder of const references to one predicate object
     */
    template <typename Predicate1>
    class PredicatesHolder1
    {
    protected:
      /**
       * Constructor
       * @param predicate1 the only predicate
       */
      PredicatesHolder1(const Predicate1& predicate1) throw ();

      const Predicate1& predicate1_;
    };

    /**
     * Holder of const references to two predicate objects
     */
    template <typename Predicate1, typename Predicate2>
    class PredicatesHolder2
    {
    protected:
      /**
       * Constructor
       * @param predicate1 the first predicate
       * @param predicate2 the second predicate
       */
      PredicatesHolder2(const Predicate1& predicate1,
        const Predicate2& predicate2) throw ();

      const Predicate1& predicate1_;
      const Predicate2& predicate2_;
    };

    /**
     * Holder of const references to three predicate objects
     */
    template <typename Predicate1, typename Predicate2,
              typename Predicate3>
    class PredicatesHolder3
    {
    protected:
      /**
       * Constructor
       * @param predicate1 the first predicate
       * @param predicate2 the second predicate
       * @param predicate3 the third predicate
       */
      PredicatesHolder3(const Predicate1& predicate1,
        const Predicate2& predicate2, const Predicate3& predicate3)
        throw ();

      const Predicate1& predicate1_;
      const Predicate2& predicate2_;
      const Predicate3& predicate3_;
    };
  }


  /**
   * Predicate calculates holding predicate and returns true
   * Predicate is called with one argument
   */
  template <typename Predicate1>
  class True1 :
    public std::unary_function<typename Predicate1::argument_type,
                               bool>,
    protected BoolFunctorsHelper::PredicatesHolder1<Predicate1>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the only predicate
     */
    True1(const Predicate1& predicate1) throw ();

    /**
     * Calls predicate(arg)
     * @param arg the only argument to pass to the predicate
     * @return true
     */
    bool
    operator ()(typename Predicate1::argument_type arg) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates holding predicate and returns true
   * Predicate is called with two arguments
   */
  template <typename Predicate1>
  class True2 :
    public std::binary_function<typename Predicate1::first_argument_type,
                                typename Predicate1::second_argument_type,
                                bool>,
    protected BoolFunctorsHelper::PredicatesHolder1<Predicate1>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the only predicate
     */
    True2(const Predicate1& predicate1) throw ();

    /**
     * Calls predicate(arg1, arg2)
     * @param arg1 the first argument to pass to the predicate
     * @param arg2 the second argument to pass to the predicate
     * @return true
     */
    bool
    operator ()(typename Predicate1::first_argument_type arg1,
      typename Predicate1::second_argument_type arg2) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates holding predicate and returns false
   * Predicate is called with one argument
   */
  template <typename Predicate1>
  class False1 :
    public std::unary_function<typename Predicate1::argument_type,
                               bool>,
    protected BoolFunctorsHelper::PredicatesHolder1<Predicate1>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the only predicate
     */
    False1(const Predicate1& predicate1) throw ();

    /**
     * Calls predicate(arg)
     * @param arg the only argument to pass to the predicate
     * @return false
     */
    bool
    operator ()(typename Predicate1::argument_type arg) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates holding predicate and returns false
   * Predicate is called with two arguments
   */
  template <typename Predicate1>
  class False2 :
    public std::binary_function<typename Predicate1::first_argument_type,
                                typename Predicate1::second_argument_type,
                                bool>,
    protected BoolFunctorsHelper::PredicatesHolder1<Predicate1>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the only predicate
     */
    False2(const Predicate1& predicate1) throw ();

    /**
     * Calls predicate(arg1, arg2)
     * @param arg1 the first argument to pass to the predicate
     * @param arg2 the second argument to pass to the predicate
     * @return false
     */
    bool
    operator ()(typename Predicate1::first_argument_type arg1,
      typename Predicate1::second_argument_type arg2) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates holding predicate and returns negation
   * Predicate is called with one argument
   */
  template <typename Predicate1>
  class Not1 :
    public std::unary_function<typename Predicate1::argument_type,
                               bool>,
    protected BoolFunctorsHelper::PredicatesHolder1<Predicate1>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the only predicate
     */
    Not1(const Predicate1& predicate1) throw ();

    /**
     * Calls !predicate(arg)
     * @param arg the only argument to pass to the predicate
     * @return negation
     */
    bool
    operator ()(typename Predicate1::argument_type arg) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates holding predicate and returns negation
   * Predicate is called with two arguments
   */
  template <typename Predicate1>
  class Not2 :
    public std::binary_function<typename Predicate1::first_argument_type,
                                typename Predicate1::second_argument_type,
                                bool>,
    protected BoolFunctorsHelper::PredicatesHolder1<Predicate1>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the only predicate
     */
    Not2(const Predicate1& predicate1) throw ();

    /**
     * Calls !predicate(arg1, arg2)
     * @param arg1 the first argument to pass to the predicate
     * @param arg2 the second argument to pass to the predicate
     * @return negation
     */
    bool
    operator ()(typename Predicate1::first_argument_type arg1,
      typename Predicate1::second_argument_type arg2) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates the first holding predicate
   * The second predicate is calculated only when the first returns true
   * "Logical and" of results is returned
   * Predicates are called with one argument
   */
  template <typename Predicate1, typename Predicate2>
  class And1 :
    public std::unary_function<typename Predicate1::argument_type,
                               bool>,
    protected BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the first predicate
     * @param predicate2 the second predicate
     */
    And1(const Predicate1& predicate1, const Predicate2& predicate2)
      throw ();

    /**
     * Calls predicate1(arg) && predicate2(arg)
     * @param arg the only argument to pass to the predicates
     * @return "logical and" of predicates results
     */
    bool
    operator ()(typename Predicate1::argument_type arg) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates the first holding predicate
   * The second predicate is calculated only when the first returns true
   * "Logical and" of results is returned
   * Predicates are called with two arguments
   */
  template <typename Predicate1, typename Predicate2>
  class And2 :
    public std::binary_function<typename Predicate1::first_argument_type,
                                typename Predicate1::second_argument_type,
                                bool>,
    protected BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the first predicate
     * @param predicate2 the second predicate
     */
    And2(const Predicate1& predicate1, const Predicate2& predicate2)
      throw ();

    /**
     * Calls predicate1(arg1, arg2) && predicate2(arg1, arg2)
     * @param arg1 the first argument to pass to the predicates
     * @param arg2 the second argument to pass to the predicates
     * @return "logical and" of predicates results
     */
    bool
    operator ()(typename Predicate1::first_argument_type arg1,
      typename Predicate1::second_argument_type arg2) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates the first holding predicate
   * The second predicate is calculated only when the first returns false
   * "Logical or" of results is returned
   * Predicates are called with one argument
   */
  template <typename Predicate1, typename Predicate2>
  class Or1 :
    public std::unary_function<typename Predicate1::argument_type,
                               bool>,
    protected BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the first predicate
     * @param predicate2 the second predicate
     */
    Or1(const Predicate1& predicate1, const Predicate2& predicate2)
      throw ();

    /**
     * Calls predicate1(arg) || predicate2(arg)
     * @param arg the only argument to pass to the predicates
     * @return "logical or" of predicates results
     */
    bool
    operator ()(typename Predicate1::argument_type arg) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates the first holding predicate
   * The second predicate is calculated only when the first returns false
   * "Logical or" of results is returned
   * Predicates are called with two arguments
   */
  template <typename Predicate1, typename Predicate2>
  class Or2 :
    public std::binary_function<typename Predicate1::first_argument_type,
                                typename Predicate1::second_argument_type,
                                bool>,
    protected BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the first predicate
     * @param predicate2 the second predicate
     */
    Or2(const Predicate1& predicate1, const Predicate2& predicate2)
      throw ();

    /**
     * Calls predicate1(arg1, arg2) || predicate2(arg1, arg2)
     * @param arg1 the first argument to pass to the predicates
     * @param arg2 the second argument to pass to the predicates
     * @return "logical or" of predicates results
     */
    bool
    operator ()(typename Predicate1::first_argument_type arg1,
      typename Predicate1::second_argument_type arg2) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates the first holding predicate
   * The second predicate is calculated only when the first returns true
   * The third predicate is calculated only when the first returns false
   * The result of the last calculated predicate is returned
   * Predicates are called with one argument
   */
  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  class Conditional1 :
    public std::unary_function<typename Predicate1::argument_type,
                               bool>,
    protected BoolFunctorsHelper::PredicatesHolder3<Predicate1, Predicate2,
      Predicate3>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the first predicate
     * @param predicate2 the second predicate
     * @param predicate3 the third predicate
     */
    Conditional1(const Predicate1& predicate1,
      const Predicate2& predicate2, const Predicate3& predicate3)
      throw ();

    /**
     * Calls predicate1(arg) ? predicate2(arg) : predicate3(arg)
     * @param arg the only argument to pass to the predicates
     * @return the result of the last calculated predicate
     */
    bool
    operator ()(typename Predicate1::argument_type arg) const
      throw (eh::Exception);
  };

  /**
   * Predicate calculates the first holding predicate
   * The second predicate is calculated only when the first returns true
   * The third predicate is calculated only when the first returns false
   * The result of the last calculated predicate is returned
   * Predicates are called with two arguments
   */
  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  class Conditional2 :
    public std::binary_function<typename Predicate1::first_argument_type,
                                typename Predicate1::second_argument_type,
                                bool>,
    protected BoolFunctorsHelper::PredicatesHolder3<Predicate1, Predicate2,
      Predicate3>
  {
  public:
    /**
     * Constructor
     * @param predicate1 the first predicate
     * @param predicate2 the second predicate
     * @param predicate3 the third predicate
     */
    Conditional2(const Predicate1& predicate1,
      const Predicate2& predicate2, const Predicate3& predicate3)
      throw ();

    /**
     * Calls predicate1(arg1, arg2) ? predicate2(arg1, arg2) :
     * predicate3(arg1, arg2)
     * @param arg1 the first argument to pass to the predicates
     * @param arg2 the second argument to pass to the predicates
     * @return the result of the last calculated predicate
     */
    bool
    operator ()(typename Predicate1::first_argument_type arg1,
      typename Predicate1::second_argument_type arg2) const
      throw (eh::Exception);
  };

  /**
   * Helper function to construct True1 predicate
   * @param predicate1 the only predicate
   * @return Constructed True1 predicate
   */
  template <typename Predicate1>
  True1<Predicate1>
  true1(const Predicate1& predicate1) throw ();

  /**
   * Helper function to construct True2 predicate
   * @param predicate1 the only predicate
   * @return Constructed True2 predicate
   */
  template <typename Predicate1>
  True2<Predicate1>
  true2(const Predicate1& predicate1) throw ();

  /**
   * Helper function to construct False1 predicate
   * @param predicate1 the only predicate
   * @return Constructed False1 predicate
   */
  template <typename Predicate1>
  False1<Predicate1>
  false1(const Predicate1& predicate1) throw ();

  /**
   * Helper function to construct False2 predicate
   * @param predicate1 the only predicate
   * @return Constructed False2 predicate
   */
  template <typename Predicate1>
  False2<Predicate1>
  false2(const Predicate1& predicate1) throw ();

  /**
   * Helper function to construct Not1 predicate
   * @param predicate1 the only predicate
   * @return Constructed Not1 predicate
   */
  template <typename Predicate1>
  Not1<Predicate1>
  not1(const Predicate1& predicate1) throw ();

  /**
   * Helper function to construct Not2 predicate
   * @param predicate1 the only predicate
   * @return Constructed Not2 predicate
   */
  template <typename Predicate1>
  Not2<Predicate1>
  not2(const Predicate1& predicate1) throw ();

  /**
   * Helper function to construct And1 predicate
   * @param predicate1 the first predicate
   * @param predicate2 the second predicate
   * @return Constructed And1 predicate
   */
  template <typename Predicate1, typename Predicate2>
  And1<Predicate1, Predicate2>
  and1(const Predicate1& predicate1, const Predicate2& predicate2) throw ();

  /**
   * Helper function to construct And2 predicate
   * @param predicate1 the first predicate
   * @param predicate2 the second predicate
   * @return Constructed And2 predicate
   */
  template <typename Predicate1, typename Predicate2>
  And2<Predicate1, Predicate2>
  and2(const Predicate1& predicate1, const Predicate2& predicate2) throw ();

  /**
   * Helper function to construct Or1 predicate
   * @param predicate1 the first predicate
   * @param predicate2 the second predicate
   * @return Constructed Or1 predicate
   */
  template <typename Predicate1, typename Predicate2>
  Or1<Predicate1, Predicate2>
  or1(const Predicate1& predicate1, const Predicate2& predicate2) throw ();

  /**
   * Helper function to construct Or2 predicate
   * @param predicate1 the first predicate
   * @param predicate2 the second predicate
   * @return Constructed Or2 predicate
   */
  template <typename Predicate1, typename Predicate2>
  Or2<Predicate1, Predicate2>
  or2(const Predicate1& predicate1, const Predicate2& predicate2) throw ();

  /**
   * Helper function to construct Conditional1 predicate
   * @param predicate1 the first predicate
   * @param predicate2 the second predicate
   * @param predicate3 the third predicate
   * @return Constructed Conditional1 predicate
   */
  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  Conditional1<Predicate1, Predicate2, Predicate3>
  conditional1(const Predicate1& predicate1, const Predicate2& predicate2,
    const Predicate3& predicate3) throw ();

  /**
   * Helper function to construct Conditional2 predicate
   * @param predicate1 the first predicate
   * @param predicate2 the second predicate
   * @param predicate3 the third predicate
   * @return Constructed Conditional2 predicate
   */
  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  Conditional2<Predicate1, Predicate2, Predicate3>
  conditional2(const Predicate1& predicate1, const Predicate2& predicate2,
    const Predicate3& predicate3) throw ();
}

/*
 * INLINES
 */

namespace Generics
{
  namespace BoolFunctorsHelper
  {
    /*
     * PredicatesHolder1 class
     */
    template <typename Predicate1>
    inline
    PredicatesHolder1<Predicate1>::PredicatesHolder1(
      const Predicate1& predicate1) throw ()
      : predicate1_(predicate1)
    {
    }

    /*
     * PredicatesHolder2 class
     */
    template <typename Predicate1, typename Predicate2>
    inline
    PredicatesHolder2<Predicate1, Predicate2>::PredicatesHolder2(
      const Predicate1& predicate1, const Predicate2& predicate2)
      throw ()
      : predicate1_(predicate1), predicate2_(predicate2)
    {
    }

    /*
     * PredicatesHolder3 class
     */
    template <typename Predicate1, typename Predicate2,
              typename Predicate3>
    inline
    PredicatesHolder3<Predicate1, Predicate2, Predicate3>::PredicatesHolder3(
      const Predicate1& predicate1, const Predicate2& predicate2,
      const Predicate3& predicate3) throw ()
      : predicate1_(predicate1), predicate2_(predicate2),
        predicate3_(predicate3)
    {
    }
  }


  /*
   * True1 class
   */
  template <typename Predicate1>
  inline
  True1<Predicate1>::True1(const Predicate1& predicate1) throw ()
    : BoolFunctorsHelper::PredicatesHolder1<Predicate1>(predicate1)
  {
  }

  template <typename Predicate1>
  inline
  bool
  True1<Predicate1>::operator ()(
    typename Predicate1::argument_type arg) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg), true;
  }

  /*
   * True2 class
   */
  template <typename Predicate1>
  inline
  True2<Predicate1>::True2(const Predicate1& predicate1) throw ()
    : BoolFunctorsHelper::PredicatesHolder1<Predicate1>(predicate1)
  {
  }

  template <typename Predicate1>
  inline
  bool
  True2<Predicate1>::operator ()(
    typename Predicate1::first_argument_type arg1,
    typename Predicate1::second_argument_type arg2) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg1, arg2), true;
  }

  /*
   * False1 class
   */
  template <typename Predicate1>
  inline
  False1<Predicate1>::False1(const Predicate1& predicate1) throw ()
    : BoolFunctorsHelper::PredicatesHolder1<Predicate1>(predicate1)
  {
  }

  template <typename Predicate1>
  inline
  bool
  False1<Predicate1>::operator ()(
    typename Predicate1::argument_type arg) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg), false;
  }

  /*
   * False2 class
   */
  template <typename Predicate1>
  inline
  False2<Predicate1>::False2(const Predicate1& predicate1) throw ()
    : BoolFunctorsHelper::PredicatesHolder1<Predicate1>(predicate1)
  {
  }

  template <typename Predicate1>
  inline
  bool
  False2<Predicate1>::operator ()(
    typename Predicate1::first_argument_type arg1,
    typename Predicate1::second_argument_type arg2) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg1, arg2), false;
  }

  /*
   * Not1 class
   */
  template <typename Predicate1>
  inline
  Not1<Predicate1>::Not1(const Predicate1& predicate1) throw ()
    : BoolFunctorsHelper::PredicatesHolder1<Predicate1>(predicate1)
  {
  }

  /*
   * Not1 class
   */
  template <typename Predicate1>
  inline
  bool
  Not1<Predicate1>::operator ()(
    typename Predicate1::argument_type arg) const
    throw (eh::Exception)
  {
    return !this->predicate1_(arg);
  }

  /*
   * Not2 class
   */
  template <typename Predicate1>
  inline
  Not2<Predicate1>::Not2(const Predicate1& predicate1) throw ()
    : BoolFunctorsHelper::PredicatesHolder1<Predicate1>(predicate1)
  {
  }

  template <typename Predicate1>
  inline
  bool
  Not2<Predicate1>::operator ()(
    typename Predicate1::first_argument_type arg1,
    typename Predicate1::second_argument_type arg2) const
    throw (eh::Exception)
  {
    return !this->predicate1_(arg1, arg2);
  }

  /*
   * And1 class
   */
  template <typename Predicate1, typename Predicate2>
  inline
  And1<Predicate1, Predicate2>::And1(const Predicate1& predicate1,
    const Predicate2& predicate2) throw ()
    : BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>(
        predicate1, predicate2)
  {
  }

  template <typename Predicate1, typename Predicate2>
  inline
  bool
  And1<Predicate1, Predicate2>::operator ()(
    typename Predicate1::argument_type arg) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg) && this->predicate2_(arg);
  }

  /*
   * And2 class
   */
  template <typename Predicate1, typename Predicate2>
  inline
  And2<Predicate1, Predicate2>::And2(const Predicate1& predicate1,
    const Predicate2& predicate2) throw ()
    : BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>(
        predicate1, predicate2)
  {
  }

  template <typename Predicate1, typename Predicate2>
  inline
  bool
  And2<Predicate1, Predicate2>::operator ()(
    typename Predicate1::first_argument_type arg1,
    typename Predicate1::second_argument_type arg2) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg1, arg2) && this->predicate2_(arg1, arg2);
  }

  /*
   * Or1 class
   */
  template <typename Predicate1, typename Predicate2>
  inline
  Or1<Predicate1, Predicate2>::Or1(const Predicate1& predicate1,
    const Predicate2& predicate2) throw ()
    : BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>(
        predicate1, predicate2)
  {
  }

  template <typename Predicate1, typename Predicate2>
  inline
  bool
  Or1<Predicate1, Predicate2>::operator ()(
    typename Predicate1::argument_type arg) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg) || this->predicate2_(arg);
  }

  /*
   * Or2 class
   */
  template <typename Predicate1, typename Predicate2>
  inline
  Or2<Predicate1, Predicate2>::Or2(const Predicate1& predicate1,
    const Predicate2& predicate2) throw ()
    : BoolFunctorsHelper::PredicatesHolder2<Predicate1, Predicate2>(
        predicate1, predicate2)
  {
  }

  template <typename Predicate1, typename Predicate2>
  inline
  bool
  Or2<Predicate1, Predicate2>::operator ()(
    typename Predicate1::first_argument_type arg1,
    typename Predicate1::second_argument_type arg2) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg1, arg2) || this->predicate2_(arg1, arg2);
  }

  /*
   * Conditional1 class
   */
  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  inline
  Conditional1<Predicate1, Predicate2, Predicate3>::Conditional1(
    const Predicate1& predicate1, const Predicate2& predicate2,
    const Predicate3& predicate3) throw ()
    : BoolFunctorsHelper::PredicatesHolder3
        <Predicate1, Predicate2, Predicate3>(
        predicate1, predicate2, predicate3)
  {
  }

  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  inline
  bool
  Conditional1<Predicate1, Predicate2, Predicate3>::operator ()(
    typename Predicate1::argument_type arg) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg) ? this->predicate2_(arg) :
      this->predicate3_(arg);
  }

  /*
   * Conditional2 class
   */
  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  inline
  Conditional2<Predicate1, Predicate2, Predicate3>::Conditional2(
    const Predicate1& predicate1, const Predicate2& predicate2,
    const Predicate3& predicate3) throw ()
    : BoolFunctorsHelper::PredicatesHolder3
        <Predicate1, Predicate2, Predicate3>(
        predicate1, predicate2, predicate3)
  {
  }

  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  inline
  bool
  Conditional2<Predicate1, Predicate2, Predicate3>::operator ()(
    typename Predicate1::first_argument_type arg1,
    typename Predicate1::second_argument_type arg2) const
    throw (eh::Exception)
  {
    return this->predicate1_(arg1, arg2) ? this->predicate2_(arg1, arg2) :
      this->predicate3_(arg1, arg2);
  }


  /*
   * Helper functions
   */
  template <typename Predicate1>
  True1<Predicate1>
  true1(const Predicate1& predicate1) throw ()
  {
    return True1<Predicate1>(predicate1);
  }

  template <typename Predicate1>
  True2<Predicate1>
  true2(const Predicate1& predicate1) throw ()
  {
    return True2<Predicate1>(predicate1);
  }

  template <typename Predicate1>
  False1<Predicate1>
  false1(const Predicate1& predicate1) throw ()
  {
    return False1<Predicate1>(predicate1);
  }

  template <typename Predicate1>
  False2<Predicate1>
  false2(const Predicate1& predicate1) throw ()
  {
    return False2<Predicate1>(predicate1);
  }

  template <typename Predicate1>
  Not1<Predicate1>
  not1(const Predicate1& predicate1) throw ()
  {
    return Not1<Predicate1>(predicate1);
  }

  template <typename Predicate1>
  Not2<Predicate1>
  not2(const Predicate1& predicate1) throw ()
  {
    return Not2<Predicate1>(predicate1);
  }

  template <typename Predicate1, typename Predicate2>
  And1<Predicate1, Predicate2>
  and1(const Predicate1& predicate1, const Predicate2& predicate2) throw ()
  {
    return And1<Predicate1, Predicate2>(predicate1, predicate2);
  }

  template <typename Predicate1, typename Predicate2>
  And2<Predicate1, Predicate2>
  and2(const Predicate1& predicate1, const Predicate2& predicate2) throw ()
  {
    return And2<Predicate1, Predicate2>(predicate1, predicate2);
  }

  template <typename Predicate1, typename Predicate2>
  Or1<Predicate1, Predicate2>
  or1(const Predicate1& predicate1, const Predicate2& predicate2) throw ()
  {
    return Or1<Predicate1, Predicate2>(predicate1, predicate2);
  }

  template <typename Predicate1, typename Predicate2>
  Or2<Predicate1, Predicate2>
  or2(const Predicate1& predicate1, const Predicate2& predicate2) throw ()
  {
    return Or2<Predicate1, Predicate2>(predicate1, predicate2);
  }

  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  Conditional1<Predicate1, Predicate2, Predicate3>
  conditional1(const Predicate1& predicate1, const Predicate2& predicate2,
    const Predicate3& predicate3) throw ()
  {
    return Conditional1<Predicate1, Predicate2, Predicate3>(
      predicate1, predicate2, predicate3);
  }

  template <typename Predicate1, typename Predicate2,
            typename Predicate3>
  Conditional2<Predicate1, Predicate2, Predicate3>
  conditional2(const Predicate1& predicate1, const Predicate2& predicate2,
    const Predicate3& predicate3) throw ()
  {
    return Conditional2<Predicate1, Predicate2, Predicate3>(
      predicate1, predicate2, predicate3);
  }
}

#endif
