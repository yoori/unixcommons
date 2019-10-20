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



//@file  Stream/FlagsSaver.hpp
#ifndef STREAM_FLAGS_SAVER_HPP
#define STREAM_FLAGS_SAVER_HPP

#include <ios>

#include <Generics/Uncopyable.hpp>


namespace Stream
{
  /**
   * Guard for saving std streams state (fmtflags)
   * Provide restore ability.
   */
  class FlagsSaver : Generics::Uncopyable
  {
  public:
    typedef ::std::ios_base StateType;
    typedef ::std::ios_base::fmtflags AspectType;

    /**
     * Constructor
     * @param s Independent from the national characteristics
     * the state of input-output to be saved
     */
    explicit
    FlagsSaver(StateType &s) throw ();

    /**
     * Constructor
     * @param s Independent from the national characteristics
     * the state of input-output to be saved
     * @param a flags to be set on s.
     */
    FlagsSaver(StateType& s, const AspectType& a) throw ();

    /**
     * Destructor restore state of stream
     */
    ~FlagsSaver() throw ();

    /**
     * Restore state of stream
     */
    void
    restore() throw ();

  private:
    StateType& state_;
    const AspectType ASPECT_;
  };
}  // namespace Stream

//////////////////////////////////////////////////////////////////////////
// Inlines implementations
//////////////////////////////////////////////////////////////////////////

namespace Stream
{
  //
  //  class FlagsSaver
  //

  inline
  FlagsSaver::FlagsSaver(StateType &state) throw ()
    : state_(state), ASPECT_(state.flags())
  {
  }

  inline
  FlagsSaver::FlagsSaver(StateType& state, const AspectType& aspect) throw ()
    : state_(state), ASPECT_(state.flags(aspect))
  {
  }

  inline
  void
  FlagsSaver::restore() throw ()
  {
    state_.flags(ASPECT_);
  }

  inline
  FlagsSaver::~FlagsSaver() throw ()
  {
    restore();
  }
}  // namespace Stream

#endif
