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



#ifndef GENERICS_DIRSELECTOR_HPP
#define GENERICS_DIRSELECTOR_HPP

#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>

#include <eh/Errno.hpp>

#include <String/StringManip.hpp>

#include <Generics/BoolFunctors.hpp>
#include <Generics/Function.hpp>
#include <Generics/ArrayAutoPtr.hpp>


namespace Generics
{
  /**
   * Namespace for all directory_selector related types and functions
   */
  namespace DirSelect
  {
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(FailedToOpenDirectory, Exception);
    DECLARE_EXCEPTION(FailedToStatFile, Exception);

    /*
     * Error callback type for directory_selector
     */
    typedef void (*ErrorHandler)(const char* full_path);

    /**
     * Helper base class for all directory_selector predicates
     */
    struct Predicate :
      public std::binary_function<const char*, const struct stat&, bool>
    {
    };

#define DEFINE_MODE_TEST(name, mode_test) \
    struct name : public Predicate \
    { \
      bool \
      operator ()(const char*, const struct stat& st) const \
      { \
        return mode_test(st.st_mode); \
      } \
    };

    /**
     * Predicates test st_mode of stat struct
     */
    DEFINE_MODE_TEST(IsDirectory, S_ISDIR);
    DEFINE_MODE_TEST(IsRegular, S_ISREG);
    DEFINE_MODE_TEST(IsLink, S_ISLNK);

#undef DEFINE_MODE_TEST

    /**
     * Predicate allows to filter files by their names
     */
    class NamePattern : public Predicate
    {
    public:
      /**
       * Constructor
       * @param pattern sh(1)-like pattern
       */
      NamePattern(const char* pattern) throw (eh::Exception);

      /*
       * Matches file name by the pattern
       * @param full_path full path to the file
       * @param st file information
       * @return whether the file name matches the pattern or not
       */
      bool
      operator ()(const char* full_path, const struct stat& st) const
        throw (eh::Exception);

    protected:
      std::string pattern_;
    };

    /**
     * Helper class for wrapping of user-specified non-const functor
     */
    template <typename Functor>
    class FunctorWrapper : public Predicate
    {
    public:
      /**
       * Constructor
       * @param predicate user-specified non-const predicate
       * @param file_name_only if true supply a file name (not a full path)
       * @param result result to return
       * to the functor
       */
      FunctorWrapper(Functor& predicate, bool file_name_only, bool result)
        throw ();

      /**
       * Executes user-specified functor
       * @param full_path the full path of the file
       * @param st file information
       * @return required result
       */
      bool
      operator ()(const char* full_path, const struct stat& st) const
        throw (eh::Exception);

    private:
      Functor& functor_;
      bool file_name_only_;
      bool result_;
    };

    /**
     * Helper function to construct FunctorWrapper
     * @param functor user-specified functor
     * @param file_name_only if functor should take only a file name part
     * of the path
     * @param result result to return
     * @return constructed FunctorWrapper
     */
    template <typename Functor>
    FunctorWrapper<Functor>
    wrap_functor(Functor& functor,
      bool file_name_only = false, bool result = true) throw ();

    /**
     * Helper function returning file name from the full path
     * @param full_path full path to the file
     * @return file name part of the path
     */
    const char*
    file_name(const char* full_path) throw ();

    /**
     * Default handler for opendir(2) fail
     */
    void
    default_failed_to_open_directory(const char* full_path)
      throw (Exception);

    /**
     * Default handler for stat(2) or lstat(2) fail
     * @param full_path full path to failed file
     */
    void
    default_failed_to_stat_file(const char* full_path)
      throw (Exception);

    /**
     * Function crawls the supplied directory calculating supplied predicate
     * for every file
     * For those directories (except . and ..) where predicate returns true
     * recursive crawling is used
     * @param path directory path to start from
     * @param predicate predicate to calculate
     * @param resolve_links treat links using stat(2) or lstat(2)
     * @param failed_to_open_directory function called on opendir(2) failure
     * @param failed_to_stat_file function called on stat(2) or
     * lstat(2) failure
     *
     * Examples of predicates (assuming "user" is a user predicate):
     * All directory entries (recursive)
     *   wrap_predicate(user)
     * All directory entries (non-recursive)
     *   wrap_predicate(user, false)
     * All regular files by mask "*.txt" (non-recursive)
     *   and2(and2(IsRegular(), NamePattern("*.txt")), wrap_predicate(user))
     * All regular files by mask "*.txt" (recursive)
     *   true2(and2(and2(IsRegular(), NamePattern("*.txt")),
     *    wrap_predicate(user)))
     */
    template <typename Predicate>
    void
    directory_selector(const char* path, const Predicate& predicate,
      bool resolve_links = true,
      ErrorHandler failed_to_open_directory =
        default_failed_to_open_directory,
      ErrorHandler failed_to_stat_file =
        default_failed_to_stat_file) throw (eh::Exception);

    enum DIRECTORY_SELECTOR_FLAGS
    {
      DSF_NON_RECURSIVE        = 0,
      DSF_RECURSIVE            = 1,

      DSF_REGULAR_ONLY         = 0,
      DSF_ALL_FILES            = 2,

      DSF_RESOLVE_LINKS        = 0,
      DSF_DONT_RESOLVE_LINKS   = 4,

      DSF_NO_EXCEPTION_ON_OPEN = 0,
      DSF_EXCEPTION_ON_OPEN    = 8,

      DSF_NO_EXCEPTION_ON_STAT = 0,
      DSF_EXCEPTION_ON_STAT    = 16,

      DSF_FULL_PATH            = 0,
      DSF_FILE_NAME_ONLY       = 32,

      DSF_DEFAULT = // == 0
        DSF_NON_RECURSIVE |
        DSF_REGULAR_ONLY |
        DSF_RESOLVE_LINKS |
        DSF_NO_EXCEPTION_ON_OPEN |
        DSF_NO_EXCEPTION_ON_STAT |
        DSF_FULL_PATH |
        0
    };

    /**
     * Wrapper for original directory_selector
     * @param path directory path to start from
     * @param functor user-supplied functor to calculate
     * @param mask sh(1)-like mask
     * @param flags flags determining the behaviour
     */
    template <typename Functor>
    void
    directory_selector(const char* path, Functor&& functor,
      const char* mask, int flags = DSF_DEFAULT) throw (eh::Exception);

    /**
     * Creates a list of matched file inside the supplied container
     */
    template <typename Iterator>
    class ListCreator
    {
    public:
      /**
       * Constructor
       * @param iterator output iterator to fill in
       */
      explicit
      ListCreator(Iterator iterator)
        throw ();

      /*
       * Adds another full file name to the container
       * @param full_path full path to the file
       * @param st file information
       */
      bool
      operator ()(const char* full_path, const struct stat& st)
        throw (eh::Exception);

    private:
      Iterator iterator_;
    };

    template <typename Iterator>
    ListCreator<Iterator>
    list_creator(Iterator iterator) throw (eh::Exception);
  }
}

/*
 * INLINES
 */
namespace Generics
{
  namespace DirSelect
  {
    /*
     * NamePattern class
     */
    inline
    NamePattern::NamePattern(const char* pattern) throw (eh::Exception)
      : pattern_(pattern)
    {
    }

    inline
    bool
    NamePattern::operator ()(const char* full_path,
      const struct stat&) const
      throw (eh::Exception)
    {
      return !fnmatch(pattern_.c_str(), file_name(full_path), FNM_PATHNAME);
    }

    inline
    const char*
    file_name(const char* full_path) throw ()
    {
      const char* ptr = strrchr(full_path, '/');
      return ptr ? ptr + 1 : full_path;
    }

    template <typename Functor>
    inline
    FunctorWrapper<Functor>::FunctorWrapper(
      Functor& functor, bool file_name_only, bool result)
      throw ()
      : functor_(functor), file_name_only_(file_name_only),
        result_(result)
    {
    }

    template <typename Functor>
    bool
    FunctorWrapper<Functor>::operator ()(
      const char* full_path, const struct stat& st) const
      throw (eh::Exception)
    {
      return functor_(file_name_only_ ? file_name(full_path) : full_path,
        st), result_;
    }

    template <typename Functor>
    FunctorWrapper<Functor>
    wrap_functor(Functor& functor,
      bool file_name_only, bool result) throw ()
    {
      return FunctorWrapper<Functor>(functor, file_name_only, result);
    }

    inline
    void
    default_failed_to_open_directory(const char* full_path)
      throw (Exception)
    {
      eh::throw_errno_exception<FailedToOpenDirectory>(FNE,
        "'", full_path, "'");
    }

    inline
    void
    default_failed_to_stat_file(const char* full_path)
      throw (Exception)
    {
      eh::throw_errno_exception<FailedToStatFile>(FNE,
        "'", full_path, "'");
    }

    namespace DirectoryFilterHelper
    {
      typedef int (*StatFunc)(const char*, struct stat*);

      class DirPtr : private Uncopyable
      {
      public:
        DirPtr(const char* path) throw ();
        ~DirPtr() throw ();

        DIR*
        get() const throw ();

      private:
        DIR* dir;
      };

      inline
      DirPtr::DirPtr(const char* path) throw ()
      {
        dir = opendir(path);
      }

      inline
      DirPtr::~DirPtr() throw ()
      {
        if (dir)
        {
          closedir(dir);
        }
      }

      inline
      DIR*
      DirPtr::get() const throw ()
      {
        return dir;
      }

      template <typename Predicate>
      inline
      void
      walkthrough(const char* path, StatFunc stat_func,
        const Predicate& predicate,
        ErrorHandler failed_to_open_directory,
        ErrorHandler failed_to_stat_file) throw (eh::Exception)
      {
        struct stat st;

        DirPtr dir(path);
        if (!dir.get())
        {
          if (failed_to_open_directory)
          {
            failed_to_open_directory(path);
          }
          return;
        }

        int path_len = strlen(path);
        ArrayChar full_path(path_len + NAME_MAX + 2);
        if (path_len)
        {
          memcpy(&full_path[0], path, path_len);
          if (full_path[path_len - 1] != '/')
          {
            full_path[path_len] = '/';
            path_len++;
          }
        }
        char* name = &full_path[path_len];

        while (dirent* entry = readdir(dir.get()))
        {
          if (String::StringManip::strlcpy(name, entry->d_name, NAME_MAX)
              >= NAME_MAX)
          {
            //TODO
          }
          if (stat_func(&full_path[0], &st) < 0)
          {
            if (failed_to_stat_file)
            {
              failed_to_stat_file(&full_path[0]);
            }
            continue;
          }
          if (predicate(&full_path[0], st) && S_ISDIR(st.st_mode) &&
            (name[0] != '.' || (name[1] != '\0' &&
              (name[1] != '.' || name[2] != '\0'))))
          {
            walkthrough(&full_path[0], stat_func, predicate,
              failed_to_open_directory, failed_to_stat_file);
          }
        }
      }
    }

    template <typename Predicate>
    void
    directory_selector(const char* path, const Predicate& predicate,
      bool resolve_links, ErrorHandler failed_to_open_directory,
      ErrorHandler failed_to_stat_file) throw (eh::Exception)
    {
      DirectoryFilterHelper::StatFunc stat_func =
        resolve_links ? stat : lstat;

      DirectoryFilterHelper::walkthrough(
        path, stat_func, predicate, failed_to_open_directory,
        failed_to_stat_file);
    }

    template <typename Functor>
    void
    directory_selector(const char* path, Functor&& functor,
      const char* mask, int flags) throw (eh::Exception)
    {
      ErrorHandler failed_to_open_directory =
        flags & DSF_EXCEPTION_ON_OPEN ?
        default_failed_to_open_directory : 0;
      ErrorHandler failed_to_stat_file =
        flags & DSF_EXCEPTION_ON_STAT ?
        default_failed_to_stat_file : 0;
      bool recursive = flags & DSF_RECURSIVE;
      bool only_regular = !(flags & DSF_ALL_FILES);
      bool resolve_links = !(flags & DSF_DONT_RESOLVE_LINKS);
      bool file_name_only = flags & DSF_FILE_NAME_ONLY;

      if (recursive)
      {
        if (only_regular)
        {
          directory_selector(path,
            true2(and2(and2(IsRegular(), NamePattern(mask)),
              wrap_functor(functor, file_name_only))),
            resolve_links, failed_to_open_directory, failed_to_stat_file);
        }
        else
        {
          directory_selector(path,
            true2(and2(NamePattern(mask),
              wrap_functor(functor, file_name_only))),
            resolve_links, failed_to_open_directory, failed_to_stat_file);
        }
      }
      else
      {
        if (only_regular)
        {
          directory_selector(path,
            and2(and2(IsRegular(), NamePattern(mask)),
              wrap_functor(functor, file_name_only)),
            resolve_links, failed_to_open_directory, failed_to_stat_file);
        }
        else
        {
          directory_selector(path,
            and2(NamePattern(mask),
              wrap_functor(functor, file_name_only, false)),
            resolve_links, failed_to_open_directory, failed_to_stat_file);
        }
      }
    }

    /*
     * ListCreator class
     */
    template <typename Iterator>
    ListCreator<Iterator>::ListCreator(Iterator iterator) throw ()
      : iterator_(iterator)
    {
    }

    template <typename Iterator>
    bool
    ListCreator<Iterator>::operator ()(
      const char* full_path, const struct stat&)
      throw (eh::Exception)
    {
      *iterator_++ = full_path;
      return true;
    }


    template <typename Iterator>
    ListCreator<Iterator>
    list_creator(Iterator iterator) throw (eh::Exception)
    {
      return ListCreator<Iterator>(iterator);
    }
  }
}

#endif
