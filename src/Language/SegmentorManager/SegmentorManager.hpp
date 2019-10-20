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



#ifndef LANGUAGE_SEGMENTOR_MANAGER_SEGMENTOR_MANAGER_HPP
#define LANGUAGE_SEGMENTOR_MANAGER_SEGMENTOR_MANAGER_HPP

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Language/SegmentorCommons/SegmentorInterface.hpp>


namespace Language
{
  namespace Segmentor
  {
    //
    // class CompositeSegmentor
    //

    class CompositeSegmentor : public SegmentorInterface
    {
    public:
      CompositeSegmentor() throw ();

      template <typename Iterator>
      CompositeSegmentor(const Iterator& begin, const Iterator& end)
        throw (SegmException);

      void
      add_segmentor(const SegmentorInterface* segmentor)
        throw (SegmException);

      virtual
      void
      segmentation(WordsList& result, const char* phrase,
        size_t phrase_len) const throw (SegmException);

      virtual
      void
      put_spaces(std::string& result, const char* phrase,
        size_t phrase_len) const
        throw (SegmException);

    protected:
      virtual
      ~CompositeSegmentor() throw ();

    private:
      typedef std::list<SegmentorInterface_var> SegmentorList;
      SegmentorList segmentors_;
    };
    typedef ReferenceCounting::QualPtr<CompositeSegmentor>
      CompositeSegmentor_var;
  } //namespace Segmentor
} //namespace Language

namespace Language
{
  namespace Segmentor
  {
    //
    // class CompositeSegmentor
    //

    inline
    CompositeSegmentor::CompositeSegmentor() throw ()
    {
    }

    inline
    CompositeSegmentor::~CompositeSegmentor() throw ()
    {
    }

    template <typename Iterator>
    CompositeSegmentor::CompositeSegmentor(
      const Iterator& begin, const Iterator& end) throw (SegmException)
    {
      try
      {
        segmentors_.assign(begin, end);
      }
      catch (const eh::Exception& e)
      {
        Stream::Error error;
        error << FNS << "eh::Exception caught: " << e.what();
        throw SegmException(error);
      }
    }

    inline
    void
    CompositeSegmentor::add_segmentor(const SegmentorInterface* segmentor)
      throw (SegmException)
    {
      try
      {
        segmentors_.push_back(SegmentorInterface_var(
          ReferenceCounting::add_ref(segmentor)));
      }
      catch (const eh::Exception& e)
      {
        Stream::Error error;
        error << FNS << "eh::Exception caught: " << e.what();
        throw SegmException(error);
      }
    }

    inline
    void
    CompositeSegmentor::segmentation(WordsList& result, const char* phrase,
      size_t phrase_len) const throw (SegmException)
    {
      try
      {
        result.clear();
        result.emplace_back(phrase, phrase_len);

        for (SegmentorList::const_iterator it = segmentors_.begin();
          it != segmentors_.end(); ++it)
        {
          WordsList new_result;

          for (WordsList::const_iterator w_it = result.begin();
            w_it != result.end(); ++w_it)
          {
            WordsList local_result;
            (*it)->segmentation(local_result, w_it->data(), w_it->length());
            new_result.splice(new_result.end(), local_result);
          }

          new_result.swap(result);
        }
      }
      catch (const SegmException&)
      {
        throw;
      }
      catch (const eh::Exception& e)
      {
        Stream::Error error;
        error << FNS << "eh::Exception caught: " << e.what();
        throw SegmException(error);
      }
    }

    inline
    void
    CompositeSegmentor::put_spaces(std::string& result, const char* phrase,
      size_t phrase_len) const throw (SegmException)
    {
      try
      {
        std::string(phrase, phrase_len).swap(result);

        for (SegmentorList::const_iterator it = segmentors_.begin();
          it != segmentors_.end(); ++it)
        {
          (*it)->put_spaces(result, result.data(), result.length());
        }
      }
      catch (const SegmException&)
      {
        throw;
      }
      catch (const eh::Exception& e)
      {
        Stream::Error error;
        error << FNS << "eh::Exception caught: " << e.what();
        throw SegmException(error);
      }
    }
  } //namespace Segmentor
} //namespace Language

#endif
