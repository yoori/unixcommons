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



#ifndef LANGUAGE_JAPAN_SEGMENTOR_MECAB_HPP
#define LANGUAGE_JAPAN_SEGMENTOR_MECAB_HPP

#include <memory>

#include <Sync/PosixLock.hpp>
#include <Sync/Semaphore.hpp>

#include <ReferenceCounting/Deque.hpp>

#include <Language/SegmentorCommons/SegmentorInterface.hpp>


namespace MeCab
{
  class Tagger;
}

namespace Language
{
  namespace Segmentor
  {
    namespace Japanese
    {
      class MecabSegmentor : public UniqueSegmentorInterface<MecabSegmentor>
      {
      public:
        enum ThreadsLimitViolationPolicy
        {
          TLVP_EXCEPTION,
          TLVP_WAITING
        };

        explicit
        MecabSegmentor(const char* config_file = 0,
          size_t max_threads_count = 1000,
          ThreadsLimitViolationPolicy policy = TLVP_WAITING)
          throw (UniqueException, SegmException);

        virtual
        void
        segmentation(WordsList& result, const char* phrase,
          size_t phrase_len) const throw (SegmException);

        virtual
        void
        put_spaces(std::string& result, const char* phrase,
          size_t phrase_len) const throw (SegmException);

      protected:
        virtual
        ~MecabSegmentor() throw ();

        template <class Target>
        void
        put_parsed_(Target& target, const char* phrase,
          size_t phrase_len) const throw (SegmException);

      private:
        typedef Sync::PosixMutex Mutex_;
        typedef Sync::PosixGuard WriteGuard_;

        class MecabTagger_ : public ReferenceCounting::AtomicImpl
        {
        public:
          explicit
          MecabTagger_(const char* cmd) throw (SegmException);

          bool
          empty() const throw ();

          template <class Target>
          void
          parse_to(Target& result, const char* phrase,
            size_t phrase_len) throw (eh::Exception);

        private:
          virtual
          ~MecabTagger_() throw ();

          std::unique_ptr<MeCab::Tagger> tagger_;
        };
        typedef ReferenceCounting::QualPtr<MecabTagger_> MecabTagger_var_;


        bool
        is_valid_utf8_(const char* str, size_t str_len) const throw ();

        MecabTagger_var_
        init_new_tagger_() const throw (SegmException);

        MecabTagger_var_
        get_tagger_() const throw (SegmException);

        void
        put_tagger_(MecabTagger_var_& tagger) const throw (SegmException);

        std::string command_line_;
        ThreadsLimitViolationPolicy policy_;
        mutable Mutex_ lock_;
        mutable ReferenceCounting::Deque<MecabTagger_var_> taggers_;
        mutable size_t max_threads_count_;
        std::unique_ptr<Sync::Semaphore> waiting_sem_;
        mutable short waiting_num_;
      };
      typedef ReferenceCounting::ConstPtr<MecabSegmentor>
        MecabSegmentor_var;
    } //namespace Japanese
  } //namespace Segmentor
} //namespace Language

#endif
