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





#ifndef GENERICS_FILECACHE_HPP
#define GENERICS_FILECACHE_HPP

#include <memory>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <eh/Errno.hpp>

#include <Sync/Condition.hpp>

#include <ReferenceCounting/ReferenceCounting.hpp>
#include <ReferenceCounting/Map.hpp>

#include <String/StringManip.hpp>

#include <Generics/BoundedMap.hpp>


/**X Generics library namespace. */
namespace Generics
{
  /**X Namespace containing file cache classes exceptions. */
  namespace CacheExceptions
  {
    DECLARE_EXCEPTION(CacheException, eh::DescriptiveException);
    DECLARE_EXCEPTION(ImplementationException, CacheException);
    DECLARE_EXCEPTION(NotExistException, CacheException);
  }


  /**
   * Templates defined below use CheckStrategy and UpdateStrategy.
   * Each strategy is [possibly] initialized by the name supplied
   * by the containing class (or such objects are constructed before
   * construction of the containing class).
   *
   * Class CheckStrategy must provide the following interface:
   * class CheckStrategy
   * {
   * public:
   *   // Optional
   *   CheckStratery(const char* name) throw (eh::Exception);
   *
   *   ~CheckStratery() throw ();
   *
   *   // Not required to be MT-safe
   *   bool
   *   object_is_changed() throw (eh::Exception);
   * };
   *
   * objects_is_changed() must return true if the checked object requires
   * reloading (see UpdateStrategy)
   *
   *
   * Class UpdateStrategy must provide the following interface:
   * class UpdateStrategy
   * {
   * public:
   *   typedef SomeType Buffer;
   *
   *   // Optional
   *   UpdateStrategy(const char* name) throw (eh::Exception);
   *
   *   ~UpdateStrategy() throw ();
   *
   *   // Required to be MT-safe
   *   Buffer&
   *   get() throw (eh::Exception);
   *
   *   // Not required to be MT-safe
   *   void
   *   update() throw (eh::Exception);
   * };
   *
   * get() must return reference to the content currently stored in the
   * object
   * update() must reload content from the controlled object (all
   * references returned by get() may become invalid)
   */


  /**
   * Simple file check strategy
   * Every time required it checks if the file is modified comparing to
   * the last check
   */
  class SimpleFileCheckStrategy
  {
  public:
    /**
     * Constructor
     * @param file_name name of the file to check
     */
    SimpleFileCheckStrategy(const char* file_name)
      throw (eh::Exception);

    /**
     * Checks if modifition time of the file increased
     * @return const std::string containing file data
     */
    bool
    object_is_changed()
      throw (eh::Exception, CacheExceptions::ImplementationException);


  private:
    const std::string FILE_NAME_;
    time_t last_modification_time_;
  };


  /**
   * Delayed check strategy
   * Template check strategy is called not more than once in the specified
   * time period
   */
  template <typename CheckStrategy>
  class DelayedCheckStrategy : private CheckStrategy
  {
  public:
    static const time_t TIMEOUT = 60;

    /**
     * Constructor
     * @param check_strategy CheckStrategy to own
     * @param timeout the most frequent checks interval
     */
    DelayedCheckStrategy(CheckStrategy* check_strategy,
      Time timeout = TIMEOUT) throw (eh::Exception);

    /**
     * Constructor
     * @param timeout the most frequent checks interval
     * @param args some args to pass to CheckStrategy constructor
     */
    template <typename... T>
    DelayedCheckStrategy(Time timeout, T... args) throw (eh::Exception);

    /**
     * Content of the loaded file
     * @return const std::string containing file data
     */
    bool
    object_is_changed() throw (eh::Exception);


  private:
    const std::unique_ptr<CheckStrategy> CHECK_STRATEGY_;
    const Time TIMEOUT_;
    Time next_check_;
  };


  /**
   * Simple file update strategy
   * Loads the file with the specified file name into std::string
   * Concurent writing to the file impacts the process
   */
  class SimpleFileUpdateStrategy
  {
  public:
    typedef const std::string Buffer;

    /**
     * Constructor
     * @param file_name name of the file to load
     */
    SimpleFileUpdateStrategy(const char* file_name)
      throw (eh::Exception);

    /**
     * Content of the loaded file
     * @return const std::string containing file data
     */
    Buffer&
    get() throw ();

    /**
     * Loads the file's data into internal member
     */
    void
    update() throw (CacheExceptions::CacheException, eh::Exception);

  private:
    const std::string FILE_NAME_;
    std::string content_;
  };


  /**
   * Cache provides means to update system state (via UpdateStrategy)
   * in case CheckStrategy says the controlling object is updated.
   * Such process is executed every time get() method is called.
   * When the state is changed but previous state is still used outside
   * the class it blocks until all references to the previous state are gone
   * and only then updates the current state
   */
  template <typename CheckStrategy, typename UpdateStrategy>
  class Cache : public ReferenceCounting::AtomicImpl
  {
  public:
    typedef typename UpdateStrategy::Buffer Buffer;
    typedef ReferenceCounting::QualPtr<Cache> Cache_var;

    /**
     * Smart pointer for Buffer.
     * It is either referenced by buffer_ (and owns Cache as well)
     * and shared by some other pointers
     * or is solely owned by unreferenced_buffer_ (and not owns Cache)
     * allowing to be destroyed or updated by Cache at any moment
     */
    class BufferHolder : public ReferenceCounting::AtomicImpl
    {
    public:
      /**
       * Implements dereferencing poiner semantics.
       * @return reference to UPDATER_'s buffer
       */
      Buffer&
      operator *() const throw (eh::Exception);

      /**
       * Implements dereferencing pointer semantics.
       * @return pointer to UPDATER_'s buffer
       */
      Buffer*
      operator ->() const throw (eh::Exception);

    protected:
      /**
       * Constructor
       * @param mutex shared (with Cache) mutex
       */
      BufferHolder(Sync::PosixMutex& mutex) throw ();

      /**
       * Destructor
       */
      virtual
      ~BufferHolder() throw ();

      /**
       * If it was shared by several outer pointers, it calls reset_buffer_
       * to inform Cache that it's the only owner of BufferHolder and
       * clears pointer to Cache
       * Otherwise (it is already the only owner) it removes itself
       */
      virtual
      bool
      remove_ref_no_delete_() const throw ();

      /**
       * Allows Cache to be referenced by BufferHolder
       * Called by cache
       * @param cache pointer to Cache to own
       */
      void
      set_cache_(Cache* cache) throw ();


    protected:
      Sync::PosixMutex& mutex_;
      mutable Cache_var cache_;

      friend class Cache;
    };
    friend class BufferHolder;
    typedef ReferenceCounting::QualPtr<BufferHolder> BufferHolder_var;

  public:
    /**
     * Constructor
     * Constructs Cache object that keeps an eye on resource identified by
     * 'name'. Creates CheckStrategy and UpdateStrategy objects and
     * initializes them with 'name'.
     * @param name some name meaningful for both CheckStrategy and
     * UpdateStrategy
     */
    Cache(const char* name) throw (eh::Exception);

    /**
     * Constructor
     * Constructs Cache object that keeps an eye on file identified by
     * 'name'. Also take ownership on CheckStrategy and UpdateStrategy.
     * @param checker CheckStrategy to own and free on destruction
     * @param updater UpdateStategy to own and free on destruction
     */
    Cache(CheckStrategy* checker, UpdateStrategy* updater)
      throw (eh::Exception);

    /**
     * If the CHECKER_ says it's modified it waits until all of the
     * references to Buffer will be freed, then calls owned UPDATER_
     * to renew memory data
     * @return smart pointer holding reference to UPDATER_'s buffer
     */
    BufferHolder_var
    get() throw (CacheExceptions::ImplementationException, eh::Exception);

  protected:
    /**
     * Destructor
     */
    virtual
    ~Cache() throw ();

    /**
     * @returns reference to UPDATER_'s buffer
     */
    Buffer&
    get_buffer_() throw (eh::Exception);

    /**
     * It's called when all of the outer references to BufferHolder are gone
     * Takes ownership over buffer_
     */
    void
    reset_buffer_() throw ();

  private:
    const std::unique_ptr<CheckStrategy> CHECKER_;
    const std::unique_ptr<UpdateStrategy> UPDATER_;

    Sync::PosixMutex mutex_;
    Sync::Conditional condition_;

    BufferHolder* buffer_;
    BufferHolder_var unreferenced_buffer_;
  };

  /**
   * CacheManager is a set of 'Cache's. It allows to keep many
   * caches in one place and access them by corresponding names.
   * Caches are destroyed after specified period of inactivity or
   * on exceeding of bound level (see BoundedMap template class for
   * details).
   *
   * Each cache must define Cache_var and BufferHolder_var types inside.
   * get() member function must return BufferHolder_var value.
   * CacheFactory must produce Cache_var from supplied const char*.
   */
  template <typename Cache,
    typename SizePolicy =
      DefaultSizePolicy<std::string, typename Cache::Cache_var>,
    typename CacheFactory = typename Cache::Cache_var (*)(const char*)>
  class CacheManager
  {
  public:
    /**
     * Default limitation parameters
     */
    static const time_t THRESHOLD_SEC = 30;
    static const size_t BOUND_LIMIT = 1000;

    typedef typename Cache::BufferHolder_var
      BufferHolder_var;

    /**
     * Constructor
     * @param threshold_timeout Threshold timeout (passed into BoundedMap)
     * @param bound_limit Size limitation (passed into BoundedMap)
     * @param size_policy Size policy (passed into BoundedMap)
     */
    CacheManager(Time threshold_timeout = THRESHOLD_SEC,
      size_t bound_limit = BOUND_LIMIT,
      SizePolicy size_policy = SizePolicy()) throw (eh::Exception);

    /**
     * Constructor
     * @param cache_factory Factory for Cache objects
     * @param threshold_timeout Threshold timeout (passed into BoundedMap)
     * @param bound_limit Size limitation (passed into BoundedMap)
     * @param size_policy Size policy (passed into BoundedMap)
     */
    CacheManager(CacheFactory cache_factory,
      Time threshold_timeout = Generics::Time(THRESHOLD_SEC),
      size_t bound_limit = BOUND_LIMIT,
      SizePolicy size_policy = SizePolicy()) throw (eh::Exception);

    /**
     * Destructor
     */
    virtual
    ~CacheManager() throw ();

    /**
     * Gets buffer corresponding to cache identified by 'name'.
     * If such cache does not already exist in the manager then
     * it is created.
     * @param name name of cache
     */
    BufferHolder_var
    get(const char* name)
      throw (CacheExceptions::ImplementationException, eh::Exception);

    /**
     * See BoundedMap for details
     * @return current threshold timeout
     */
    Time
    threshold_timeout() throw ();

    /**
     * Sets new threshold timeout
     * See BoundedMap for details
     * @param timeout new timeout
     */
    void
    threshold_timeout(Time timeout) throw ();

    /**
     * See BoundedMap for details
     * return current bound limit
     */
    size_t
    bound_limit() throw ();

    /**
     * Sets new size limit
     * See BoundedMap for details
     * @param new_bound_limit new limit
     */
    void
    bound_limit(size_t new_bound_limit) throw ();


  private:
    typedef BoundedMap<std::string, typename Cache::Cache_var,
      SizePolicy, Sync::Policy::Null,
      ReferenceCounting::Map<std::string,
        typename BoundedMapTypes<std::string,
          typename Cache::Cache_var>::Item> >
      CacheDescriptorMap;

    static
    typename Cache::Cache_var
    default_factory_(const char* name) throw (eh::Exception);

    Sync::PosixMutex mutex_;
    CacheDescriptorMap caches_;
    CacheFactory factory_;
  };


  /**
   * FileCache is a kind of Cache designed to work with files as resources
   */
  template <typename UpdateStrategy = SimpleFileUpdateStrategy,
    typename CheckStrategy = SimpleFileCheckStrategy>
  class FileCache : public Cache<CheckStrategy, UpdateStrategy>
  {
  public:
    /**
     * Constructor
     * @param file_name file name to check and update from
     */
    explicit
    FileCache(const char* file_name) throw (eh::Exception);


  protected:
    /**
     * Destructor
     */
    ~FileCache() throw ();
  };


  /**
   * FileCacheManager is a kind of CacheManager designed to work with
   * files as resources
   */
  template <typename UpdateStrategy = SimpleFileUpdateStrategy,
    typename CheckStrategy = SimpleFileCheckStrategy,
    typename SizePolicy =
      DefaultSizePolicy<std::string,
        typename Cache<CheckStrategy, UpdateStrategy>::Cache_var> >
  class FileCacheManager :
    public CacheManager<Cache<CheckStrategy, UpdateStrategy>, SizePolicy,
      typename Cache<CheckStrategy, UpdateStrategy>::Cache_var (*)(
        const char*)>
  {
  private:
    typedef CacheManager<Cache<CheckStrategy, UpdateStrategy>, SizePolicy,
      typename Cache<CheckStrategy, UpdateStrategy>::Cache_var (*)(
        const char*)> Parent;

  public:
    using Parent::THRESHOLD_SEC;
    using Parent::BOUND_LIMIT;

    /**
     * Constructor
     * @param threshold_timeout Threshold timeout (passed into CacheManager)
     * @param bound_limit Size limitation (passed into CacheManager)
     * @param size_policy Size policy (passed into CacheManager)
     */
    explicit
    FileCacheManager(Time threshold_timeout = Generics::Time(THRESHOLD_SEC),
      size_t bound_limit = BOUND_LIMIT,
      SizePolicy size_policy = SizePolicy()) throw (eh::Exception);

    /**
     * Destructor
     */
    virtual
    ~FileCacheManager() throw ();
  };


  class FileAccessCacheManager;
  /**
   * Checks accessibility of the file for reading
   */
  class FileAccessCache : public ReferenceCounting::AtomicImpl
  {
  public:
    typedef ReferenceCounting::QualPtr<FileAccessCache> Cache_var;
    typedef bool BufferHolder_var;

    /**
     * Constructor
     * @param file_name name of the file to check
     * @param checker checker routine
     */
    FileAccessCache(const char* file_name, FileAccessCacheManager& checker)
      throw (eh::Exception);

    /**
     * Returns file's access status
     * @return true if the plain file is accessible for read
     */
    bool
    get() throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~FileAccessCache() throw ();

  private:
    std::string file_name_;
    FileAccessCacheManager& checker_;
    Sync::PosixMutex mutex_;
    Time last_check_;
    bool last_result_;
  };

  /**
   * Adapter for FileAccessCacheManager to pass into CacheManager as
   * a CacheFactory
   */
  class FileAccessCacheFactory
  {
  public:
    /**
     * Constructor
     * @param factory real factory
     */
    FileAccessCacheFactory(FileAccessCacheManager& factory) throw ();

    /**
     * Produces instance of FileAccessCache bu the call to the real factory
     * @param file_name name of the file to check
     * @return pointer to the newly created instance
     */
    FileAccessCache*
    operator ()(const char* file_name) throw (eh::Exception);

  private:
    FileAccessCacheManager& factory_;
  };

  /**
   * Holder of files' access checking objects, providing access to the
   * stored access status. Access to each file is checked not more than
   * once in timeout.
   */
  class FileAccessCacheManager :
    public CacheManager<FileAccessCache,
      DefaultSizePolicy<std::string, FileAccessCache::Cache_var>,
      FileAccessCacheFactory>
  {
  public:
    /**
     * Constructor
     * @param timeout time interval when stored access status is valid
     * @param bound_limit number of files information to have stored
     */
    explicit
    FileAccessCacheManager(const Time& timeout,
      size_t bound_limit = BOUND_LIMIT)
      throw (eh::Exception);

  protected:
    /**
     * Performes check of file's access status (called by FileAccessCache)
     * Updates last_check and last_status with current values if timeout
     * has gone
     * @param file_name name of the file to check
     * @param last_check timestamp of the last check
     * @param last_result result of the last check
     */
    void
    check_(const char* file_name, Time& last_check, bool& last_result) const
      throw ();

    /**
     * Creates a new instance of FileAccessCache (called by
     * FileAccessCacheFactory)
     * @param file_name name of the file to check
     * @return newly created instance of FileAccessCheck
     */
    FileAccessCache*
    create_(const char* file_name) throw (eh::Exception);

    friend class FileAccessCache;
    friend class FileAccessCacheFactory;


  private:
    Time timeout_;
  };
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace Generics
{
  //////////////////////////////////////////////////////////////
  // SimpleFileCheckStrategy
  //////////////////////////////////////////////////////////////

  inline
  SimpleFileCheckStrategy::SimpleFileCheckStrategy(const char* file_name)
    throw (eh::Exception)
    : FILE_NAME_(file_name ? file_name : ""), last_modification_time_(0)
  {
  }

  inline
  bool
  SimpleFileCheckStrategy::object_is_changed()
    throw (eh::Exception, CacheExceptions::ImplementationException)
  {
    struct stat st;

    if (::stat(FILE_NAME_.c_str(), &st) == -1)
    {
      eh::throw_errno_exception<CacheExceptions::ImplementationException>(
        FNE, "unable to ::stat() file '", FILE_NAME_.c_str(), "'");
    }

    if (last_modification_time_ < st.st_mtime)
    {
      last_modification_time_ = st.st_mtime;
      return true;
    }

    return false;
  }

  //////////////////////////////////////////////////////////////
  // DelayedFileCheckStrategy
  //////////////////////////////////////////////////////////////

  template <typename CheckStrategy>
  const time_t DelayedCheckStrategy<CheckStrategy>::TIMEOUT;

  template <typename CheckStrategy>
  DelayedCheckStrategy<CheckStrategy>::DelayedCheckStrategy(
    CheckStrategy* check_strategy, Time timeout)
    throw (eh::Exception)
    : CHECK_STRATEGY_(check_strategy), TIMEOUT_(timeout)
  {
  }

  template <typename CheckStrategy>
  template <typename... T>
  DelayedCheckStrategy<CheckStrategy>::DelayedCheckStrategy(Time timeout,
    T... args) throw (eh::Exception)
    : CHECK_STRATEGY_(new CheckStrategy(std::forward<T>(args)...)),
      TIMEOUT_(timeout)
  {
  }


  template <typename CheckStrategy>
  bool
  DelayedCheckStrategy<CheckStrategy>::object_is_changed()
    throw (eh::Exception)
  {
    const Time NOW(Time::get_time_of_day());
    if (NOW < next_check_)
    {
      return false;
    }

    next_check_ = NOW + TIMEOUT_;

    return CheckStrategy::object_is_changed();
  }

  //////////////////////////////////////////////////////////////
  // SimpleFileUpdateStrategy
  //////////////////////////////////////////////////////////////

  inline
  SimpleFileUpdateStrategy::SimpleFileUpdateStrategy(const char* file_name)
    throw (eh::Exception)
    : FILE_NAME_(file_name ? file_name : "")
  {
  }

  inline
  SimpleFileUpdateStrategy::Buffer&
  SimpleFileUpdateStrategy::get() throw ()
  {
    return content_;
  }

  inline
  void
  SimpleFileUpdateStrategy::update()
    throw (CacheExceptions::CacheException, eh::Exception)
  {
    int fildes = open(FILE_NAME_.c_str(), O_RDONLY);
    if (fildes == -1)
    {
      eh::throw_errno_exception<CacheExceptions::CacheException>(
        FNE, "failed to open file '", FILE_NAME_.c_str(), "'");
    }

    std::string content;
    char buf[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(fildes, buf, sizeof(buf))) > 0)
    {
      content.append(buf, bytes_read);
    }
    int error = errno;
    close(fildes);
    if (bytes_read == -1)
    {
      eh::throw_errno_exception<CacheExceptions::CacheException>(
        error, FNE, "error reading from file '", FILE_NAME_.c_str(), "'");
    }
    content_.swap(content);
  }

  /////////////////////////////////////////////////////////
  // Cache<CheckStrategy, UpdateStrategy>::BufferHolder
  /////////////////////////////////////////////////////////

  template <typename CheckStrategy, typename UpdateStrategy>
  Cache<CheckStrategy, UpdateStrategy>::BufferHolder::BufferHolder(
    Sync::PosixMutex& mutex) throw ()
    : mutex_(mutex)
  {
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  Cache<CheckStrategy, UpdateStrategy>::BufferHolder::~BufferHolder()
    throw ()
  {
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  bool
  Cache<CheckStrategy, UpdateStrategy>::BufferHolder::
    remove_ref_no_delete_() const throw ()
  {
    Cache_var cache;

    {
      Sync::PosixGuard guard(mutex_);

      // Not the last reference
      if (!ReferenceCounting::AtomicImpl::remove_ref_no_delete_())
      {
        return false;
      }

      // The last reference and [was] owned only by Cache - number of
      // references cannot be changed by anyone
      if (!cache_)
      {
        return true;
      }

      // The last reference but owned not by Cache
      // Cache cannot increase number of references because of mutex
      cache = cache_.retn();
      cache->reset_buffer_();
    }

    // Destruction of cache may trigger destruction of both Cache and
    // Buffer calling remove_ref_no_delete again - cannot make it under
    // locked mutex
    return false;
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  void
  Cache<CheckStrategy, UpdateStrategy>::BufferHolder::set_cache_(
    Cache* cache) throw ()
  {
    assert(cache && cache->UPDATER_.get());

    cache_ = ReferenceCounting::add_ref(cache);
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  typename Cache<CheckStrategy, UpdateStrategy>::Buffer&
  Cache<CheckStrategy, UpdateStrategy>::BufferHolder::operator *() const
    throw (eh::Exception)
  {
    return cache_->get_buffer_();
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  typename Cache<CheckStrategy, UpdateStrategy>::Buffer*
  Cache<CheckStrategy, UpdateStrategy>::BufferHolder::operator ->() const
    throw (eh::Exception)
  {
    return &cache_->get_buffer_();
  }

  //////////////////////////////////////////////////////////////
  // Cache
  //////////////////////////////////////////////////////////////

  template <typename CheckStrategy, typename UpdateStrategy>
  Cache<CheckStrategy, UpdateStrategy>::Cache(const char* name)
    throw (eh::Exception)
    : CHECKER_(new CheckStrategy(name)), UPDATER_(new UpdateStrategy(name)),
      buffer_(0), unreferenced_buffer_(new BufferHolder(mutex_))
  {
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  Cache<CheckStrategy, UpdateStrategy>::Cache(CheckStrategy* checker,
    UpdateStrategy* updater) throw (eh::Exception)
    : CHECKER_(checker), UPDATER_(updater),
      buffer_(0), unreferenced_buffer_(new BufferHolder(mutex_))
  {
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  Cache<CheckStrategy, UpdateStrategy>::~Cache() throw ()
  {
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  typename Cache<CheckStrategy, UpdateStrategy>::Buffer&
  Cache<CheckStrategy, UpdateStrategy>::get_buffer_() throw (eh::Exception)
  {
    return UPDATER_->get();
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  void
  Cache<CheckStrategy, UpdateStrategy>::reset_buffer_() throw ()
  {
    unreferenced_buffer_ = ReferenceCounting::add_ref(buffer_);
    buffer_ = 0;
    condition_.broadcast();
  }

  template <typename CheckStrategy, typename UpdateStrategy>
  typename Cache<CheckStrategy, UpdateStrategy>::BufferHolder_var
  Cache<CheckStrategy, UpdateStrategy>::get()
    throw (CacheExceptions::ImplementationException, eh::Exception)
  {
    BufferHolder_var return_buffer;

    do
    {
      Sync::ConditionalGuard guard(condition_, mutex_);

      // No calls to buffer_->remove_ref under this mutex

      if (CHECKER_->object_is_changed())
      {
        if (buffer_)
        {
          // Waiting for all of the references to BufferHolder to be released
          guard.wait();
        }
        // Maybe we were not the only waitor of BufferHolder's release
        if (!buffer_)
        {
          UPDATER_->update();
        }
      }

      if (buffer_)
      {
        // BufferHolder is shared with someone
        return_buffer = ReferenceCounting::add_ref(buffer_);
        break;
      }

      // We are the only owner of BufferHolder
      return_buffer = buffer_ = unreferenced_buffer_.retn();
      buffer_->set_cache_(this);
    }
    while (false);

    return return_buffer;
  }

  //////////////////////////////////////////////////////////////
  // CacheManager
  //////////////////////////////////////////////////////////////

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  const time_t CacheManager<Cache, SizePolicy, CacheFactory>::THRESHOLD_SEC;
  template <typename Cache, typename SizePolicy, typename CacheFactory>
  const size_t CacheManager<Cache, SizePolicy, CacheFactory>::BOUND_LIMIT;

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  CacheManager<Cache, SizePolicy, CacheFactory>::CacheManager(
    Time threshold_timeout, size_t bound_limit,
    SizePolicy size_policy) throw (eh::Exception)
    : caches_(bound_limit, threshold_timeout, size_policy),
      factory_(default_factory_)
  {
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  CacheManager<Cache, SizePolicy, CacheFactory>::CacheManager(
    CacheFactory factory, Time threshold_timeout,
    size_t bound_limit, SizePolicy size_policy) throw (eh::Exception)
    : caches_(bound_limit, threshold_timeout, size_policy),
      factory_(factory)
  {
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  CacheManager<Cache, SizePolicy, CacheFactory>::~CacheManager()
    throw ()
  {
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  typename CacheManager<Cache, SizePolicy, CacheFactory>::BufferHolder_var
  CacheManager<Cache, SizePolicy, CacheFactory>::get(const char* name)
    throw (CacheExceptions::ImplementationException, eh::Exception)
  {
    const std::string string_name(name);

    Sync::PosixGuard guard(mutex_);

    typename CacheDescriptorMap::iterator it =
      caches_.find(string_name);

    if (it == caches_.end())
    {
      // create and insert it
      typename Cache::Cache_var cache(factory_(name));
      caches_.insert(
        typename CacheDescriptorMap::value_type(string_name, cache));
      return cache->get();
    }

    BufferHolder_var buffer = it->second->get();
    // Size may be changed in get() call
    caches_.update(it);
    return buffer;
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  Time
  CacheManager<Cache, SizePolicy, CacheFactory>::threshold_timeout()
    throw ()
  {
    Sync::PosixGuard guard(mutex_);
    return caches_.timeout();
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  void
  CacheManager<Cache, SizePolicy, CacheFactory>::threshold_timeout(
    Time timeout) throw ()
  {
    Sync::PosixGuard guard(mutex_);
    caches_.timeout(timeout);
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  size_t
  CacheManager<Cache, SizePolicy, CacheFactory>::bound_limit() throw ()
  {
    Sync::PosixGuard guard(mutex_);
    return caches_.bound();
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  void
  CacheManager<Cache, SizePolicy, CacheFactory>::bound_limit(
    size_t new_bound_limit) throw ()
  {
    Sync::PosixGuard guard(mutex_);
    caches_.bound(new_bound_limit);
  }

  template <typename Cache, typename SizePolicy, typename CacheFactory>
  typename Cache::Cache_var
  CacheManager<Cache, SizePolicy, CacheFactory>::default_factory_(
    const char* name) throw (eh::Exception)
  {
    return typename Cache::Cache_var(new Cache(name));
  }

  //////////////////////////////////////////////////////////////
  // FileCache
  //////////////////////////////////////////////////////////////

  template <typename UpdateStrategy, typename CheckStrategy>
  FileCache<UpdateStrategy, CheckStrategy>::FileCache(
    const char* file_name) throw (eh::Exception)
    : Cache<CheckStrategy, UpdateStrategy>(file_name)
  {
  }

  template <typename UpdateStrategy, typename CheckStrategy>
  FileCache<UpdateStrategy, CheckStrategy>::~FileCache() throw ()
  {
  }

  //////////////////////////////////////////////////////////////
  // FileCacheManager
  //////////////////////////////////////////////////////////////

  template <typename CheckStrategy, typename UpdateStrategy,
    typename SizePolicy>
  FileCacheManager<CheckStrategy, UpdateStrategy, SizePolicy>::
    FileCacheManager(Time threshold_timeout, size_t bound_limit,
    SizePolicy size_policy) throw (eh::Exception)
    : Parent(threshold_timeout, bound_limit, size_policy)
  {
  }

  template <typename CheckStrategy, typename UpdateStrategy,
    typename SizePolicy>
  FileCacheManager<CheckStrategy, UpdateStrategy, SizePolicy>::
    ~FileCacheManager() throw ()
  {
  }


  //
  // FileAccessCache class
  //

  inline
  FileAccessCache::FileAccessCache(const char* file_name,
    FileAccessCacheManager& checker) throw (eh::Exception)
    : file_name_(file_name), checker_(checker)
  {
  }

  inline
  FileAccessCache::~FileAccessCache() throw ()
  {
  }

  inline
  bool
  FileAccessCache::get() throw ()
  {
    Sync::PosixGuard guard(mutex_);
    checker_.check_(file_name_.c_str(), last_check_, last_result_);
    return last_result_;
  }

  //
  // FileAccessCacheFactory class
  //

  inline
  FileAccessCacheFactory::FileAccessCacheFactory(
    FileAccessCacheManager& factory) throw ()
    : factory_(factory)
  {
  }

  inline
  FileAccessCache*
  FileAccessCacheFactory::operator ()(const char* file_name)
    throw (eh::Exception)
  {
    return factory_.create_(file_name);
  }

  //
  // FileAccessCacheManager class
  //

  inline
  FileAccessCacheManager::FileAccessCacheManager(const Time& timeout,
    size_t bound_limit) throw (eh::Exception)
    : CacheManager<FileAccessCache,
      DefaultSizePolicy<std::string, FileAccessCache::Cache_var>,
      FileAccessCacheFactory>(
        FileAccessCacheFactory(*this), timeout, bound_limit),
      timeout_(timeout)
  {
  }

  inline
  void
  FileAccessCacheManager::check_(const char* file_name, Time& last_check,
    bool& last_result) const throw ()
  {
    Time now(Time::get_time_of_day());
    if (last_check < now - timeout_)
    {
      struct stat st;
      last_result = !stat(file_name, &st) && S_ISREG(st.st_mode) &&
        !access(file_name, R_OK);
      last_check = now;
    }
  }

  inline
  FileAccessCache*
  FileAccessCacheManager::create_(const char* file_name)
    throw (eh::Exception)
  {
    return new FileAccessCache(file_name, *this);
  }
}

#endif
