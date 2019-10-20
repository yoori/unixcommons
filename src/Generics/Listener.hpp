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



// Generics/Listener.hpp
#ifndef DESCRIPTOR_LISTENER_HPP
#define DESCRIPTOR_LISTENER_HPP

#include <event.h>

#include <Generics/ActiveObject.hpp>
#include <Generics/ArrayAutoPtr.hpp>
#include <Generics/Descriptors.hpp>
#include <Generics/Time.hpp>


namespace Generics
{
  class DescriptorListener;
  class ActiveDescriptorListener;
  typedef ReferenceCounting::QualPtr<ActiveDescriptorListener>
    ActiveDescriptorListener_var;

  /**
   * Callback for generalized DescriptorListener
   */
  template <typename Listener, typename ListenerHolder>
  class DescriptorListenerCallbackTempl :
    public virtual ActiveObjectCallback
  {
  public:
    DescriptorListenerCallbackTempl() throw (eh::Exception);

    /**
     * Stored listener in the internal variable.
     * Useful for default on_all_closed() implementation.
     * @param new_listener pointer to object that calls the callback.
     */
    void
    listener(ListenerHolder new_listener) throw ();

    /**
     * Stored listener
     * @return stored listener pointer
     */
    Listener*
    listener() throw ();

    /**
     * Event data available, data string is not zero terminated!
     * @param fd file descriptor which was the cause of the event.
     * @param fd_index index fd in original array of descriptors, used for
     * listener construction.
     * @param str string with data, not zero terminated.
     * @param size length for data string.
     */
    virtual
    void
    on_data_ready(int fd, size_t fd_index, const char* str, size_t size)
      throw () = 0;

    /**
     * Called when a read on a descriptor does not provide data.
     * By default does nothing.
     * @param fd descriptor that has been closed or read fails.
     * @param fd_index index fd in original array of descriptors, used for
     * listener construction.
     * @param error 0 if someone closed descriptor, non zero
     * errno value if read() failed.
     */
    virtual
    void
    on_closed(int fd, size_t fd_index, int error) throw ();

    /**
     * Called when all descriptors used for Listener creation are closed.
     * Useful reaction is to terminate the listener and destroy it
     * as useless.
     */
    virtual
    void
    on_all_closed() throw () = 0;

    /**
     * Periodically called.
     * By default does nothing.
     */
    virtual
    void
    on_periodic() throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~DescriptorListenerCallbackTempl() throw ();

  private:
    ListenerHolder listener_;
  };

  /**
   * Callback for the-same-thread DescriptorListener.
   * Called when data packs available for using.
   * Closes descriptor listener when all descriptors are gone.
   */
  class DescriptorListenerCallback :
    public DescriptorListenerCallbackTempl<DescriptorListener,
      DescriptorListener*>
  {
  public:
    /**
     * Calls terminate() for the listener.
     */
    virtual
    void
    on_all_closed() throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~DescriptorListenerCallback() throw ();
  };
  typedef ReferenceCounting::QualPtr<DescriptorListenerCallback>
    DescriptorListenerCallback_var;

  /**
   * Hang on descriptors and call callbacks when data available.
   * Don't use heap for message buffering.
   * The same thread version.
   */
  class DescriptorListener
  {
  public:
    /**
     * Base exception for all class exceptions.
     */
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    /**
     * Raise if incorrect argument specified.
     */
    DECLARE_EXCEPTION(InvalidArgument, Exception);
    /**
     * Raise if system API errors occurred.
     */
    DECLARE_EXCEPTION(SysCallFailure, Exception);
    /**
     * Raise if libevent errors occurred.
     */
    DECLARE_EXCEPTION(EventFailure, Exception);

    /**
     * Construct finish events.
     * @param callback reporting errors and actions callback.
     * @param descriptors file descriptors, we will read data from it and
     * call callbacks when data has been got.
     * @param dscs_amount number of file descriptors.
     * @param buffers_size buffer size for each descriptor, used to read data.
     * Cannot be 0.
     * @param full_lines_only switch on buffering mode, false mean
     * immediately call data_ready() callback.
     */
    DescriptorListener(DescriptorListenerCallback* callback,
      const int* descriptors, size_t dscs_amount,
      size_t buffers_size = 4096, bool full_lines_only = false)
      throw (InvalidArgument, SysCallFailure, EventFailure, eh::Exception);

    /**
     * Destructor
     */
    ~DescriptorListener() throw ();

    /**
     * Demultiplex events and call callbacks.
     */
    void
    listen() throw (eh::Exception, EventFailure);

    /**
     * Put stop message in special pipe, when DescriptorListener read it
     * listen call exit.
     */
    void
    terminate() throw ();

  protected:
    DescriptorListenerCallback_var callback_;

  private:
    /**
     * Context object for each descriptor, contain
     * buffers, data, etc.
     */
    struct DescriptorActionContext
    {
      /**
       * Need for arrays of DescriptorActionContext
       * initialization.
       * @param base general event_base used for event demultiplexing.
       * @param host pointer to object that holds descriptors contexts.
       * @param buffer_size size of static memory used for descriptor read.
       * @param descriptor, open for reading, nonblocking descriptor.
       */
      void
      init(event_base* base, DescriptorListener* host, size_t buffer_size,
        int descriptor) throw (EventFailure, eh::Exception);

      DescriptorListener* owner;
      ArrayChar buffer;
      size_t used_buffer;
      event read_event;
    };

    /**
     * Called when data for reading is available.
     * @param fd descriptor which allow reading.
     * @param context structure for fd maintenance
     */
    void
    handle_read_(int fd, DescriptorActionContext& context) throw ();

    /**
     * Translate system callbacks to class method handle_read_.
     * @param fd file descriptor
     * @param type type of fd
     * @param arg supplementary info share when event registered.
     */
    static
    void
    read_callback_(int fd, short type, void* arg) throw ();

    /**
     * Calls when anyone writes data to termination pipe.
     * @param fd file descriptor
     * @param type type of fd
     * @param arg supplementary info share when event registered.
     */
    static
    void
    terminate_callback_(int fd, short type, void* arg) throw ();

    /**
     * Called periodically.
     * @param fd file descriptor
     * @param type type of fd
     * @param arg supplementary info share when event registered.
     */
    static
    void
    periodic_callback_(int fd, short type, void* arg) throw ();

    typedef ArrayAutoPtr<DescriptorActionContext> ReadContexts;

    static const Time PERIOD;

    const size_t DESCRIPTORS_COUNT_;
    const size_t BUFFERS_LENGTH_;
    const bool FULL_LINES_ONLY_;
    ReadContexts read_contexts_;
    size_t closed_descriptors_;
    event_base* base_;
    NonBlockingReadPipe termination_pipe_;
    event termination_;
    event periodic_;
  };

  /**
   * Callback for ActiveDescriptorListener
   */
  class ActiveDescriptorListenerCallback :
    public DescriptorListenerCallbackTempl<ActiveDescriptorListener,
      ActiveDescriptorListener_var>
  {
  public:
    /**
     * Calls deactivate_object() for the listener.
     */
    virtual
    void
    on_all_closed() throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~ActiveDescriptorListenerCallback() throw ();
  };
  typedef ReferenceCounting::QualPtr<ActiveDescriptorListenerCallback>
    ActiveDescriptorListenerCallback_var;

  /**
   * Hangs on descriptors in separate thread.
   * Call ActiveDescriptorListenerCallback callbacks,
   * when actions occurs on descriptors.
   */
  class ActiveDescriptorListener : public ActiveObjectCommonImpl
  {
  public:
    /**
     * In particular construct finish events.
     * @param callback reporting errors and actions callback.
     * @param descriptors file descriptors, we will read data from them and
     * call callbacks when data has been got.
     * @param dscs_amount number of file descriptors.
     * @param buffers_size buffer size for each descriptor, used to read data.
     * Cannot be 0.
     * @param full_lines_only switch on buffering mode, false mean
     * immediately call data_ready() callback.
     */
    ActiveDescriptorListener(ActiveDescriptorListenerCallback* callback,
      const int* descriptors, size_t dscs_amount,
      size_t buffers_size = 4096, bool full_lines_only = false)
      throw (eh::Exception);

  protected:
    /**
     * Destructor check active state and stop object if require.
     */
    virtual
    ~ActiveDescriptorListener() throw ();

  private:
    class ListenerJob :
      public SingleJob,
      private DescriptorListener
    {
    public:
      ListenerJob(ActiveDescriptorListenerCallback* callback,
        const int* descriptors, size_t number_of_descriptors,
        size_t buffers_size, bool full_lines_only)
        throw (eh::Exception);

      void
      active_listener(ActiveDescriptorListener* active_listener)
        throw ();

      virtual
      void
      work() throw ();

      virtual
      void
      terminate() throw ();

    protected:
      virtual
      ~ListenerJob() throw ();

    private:
      /**
       * Implement delegation calls from DLCallback
       * to ActiveDLCallback.
       */
      class DLCAdapter :
        public DescriptorListenerCallback,
        public ReferenceCounting::AtomicImpl
      {
      public:
        /**
         * @param active_listener active object of listener for
         * delegation calls * from DLCallback to ActiveDLCallback.
         * Must be != 0.
         * @param active_callback adapting to DLCallback ActiveDLCallback.
         * Must be != 0.
         */
        DLCAdapter(ActiveDescriptorListenerCallback* active_callback)
          throw ();

        void
        active_listener(ActiveDescriptorListener* active_listener)
          throw ();

        /**
         * @param listener pointer to object which called callback method.
         * @param fd file descriptor which was the cause of event.
         * @param fd_index index fd in original array of descriptors, used for
         * listener construction.
         * @param str string with data, not zero terminated.
         * @param size length for data string.
         */
        virtual
        void
        on_data_ready(int fd, size_t fd_index, const char* str, size_t size)
          throw ();

        /**
         * @param listener pointer to object which called callback method.
         * @param fd descriptor that has been closed or read fails.
         * @param fd_index index fd in original array of descriptors, used
         * for listener construction.
         * @param error 0 if someone closed descriptor, non zero
         * errno value if read() failed.
         */
        virtual
        void
        on_closed(int fd, size_t fd_index, int error) throw ();

        /**
         * Call when all descriptors used for DescriptorListener
         * creation closed. Excluding termination descriptor.
         * @param listener pointer to object that call callback.
         */
        virtual
        void
        on_all_closed() throw ();

        /**
         * Sink for Active object errors.
         * @param object calling Active object.
         * @param severity severity for trouble.
         * @param description text that describe trouble.
         */
        virtual
        void
        report_error(Severity severity,
          const String::SubString& description,
          const char* error_code = 0) throw ();

      protected:
        /**
         * protected destructor because reference counting object.
         */
        virtual
        ~DLCAdapter() throw ();

      private:
        ActiveDescriptorListenerCallback_var active_callback_;
      };
      typedef ReferenceCounting::QualPtr<DLCAdapter> DLCAdapter_var;
    };
    typedef ReferenceCounting::QualPtr<ListenerJob> ListenerJob_var;
  };


  class ExecuteAndListenCallback :
    public virtual DescriptorListenerCallback
  {
  public:
    virtual
    void
    set_pid(pid_t pid) throw ();

  protected:
    virtual
    ~ExecuteAndListenCallback() throw ();
  };
  typedef ReferenceCounting::QualPtr<ExecuteAndListenCallback>
    ExecuteAndListenCallback_var;

  /**
   * Pass some descriptors numbers that we know program will write
   * to. Not necessarily that the descriptors were open.
   * We avoid possible clash between parent and child.
   *
   * @param callback Callback to be notified about data actions.
   * @param program_name The name for the process to be executed.
   * @param argv array of command line parameters execvp compatible.
   * @param descriptors_amount number of descriptors to redefine.
   * @param descriptors array of descriptors to redefine.
   * @param redirect_descriptors_amount number of descriptors to redirect
   * @param redirect_descriptors array of descriptors to redirect to /dev/null
   * @param listener_buffers_size buffer size for each descriptor,
   * used to read data. Cannot be 0.
   * @param listener_full_lines_only switch on buffering mode, false mean
   * immediately call data_ready() callback.
   * @param error_pipe create additional pipe for child error reporting
   * @return child termination status.
   */
  int
  execute_and_listen(ExecuteAndListenCallback* callback,
    const char* program_name, char* const argv[],
    size_t descriptors_amount, const int* descriptors,
    size_t redirect_descriptors_amount = 0,
    const int* redirect_descriptors = 0,
    size_t listener_buffers_size = 4096,
    bool listener_full_lines_only = false, bool error_pipe = false)
    throw (eh::Exception);
}

//
// INLINES
//

namespace Generics
{
  //
  // DescriptorListenerCallbackTempl class
  //

  template <typename Listener, typename ListenerHolder>
  DescriptorListenerCallbackTempl<Listener, ListenerHolder>::
    DescriptorListenerCallbackTempl() throw (eh::Exception)
    : listener_(ListenerHolder())
  {
  }

  template <typename Listener, typename ListenerHolder>
  DescriptorListenerCallbackTempl<Listener, ListenerHolder>::
    ~DescriptorListenerCallbackTempl() throw ()
  {
  }

  template <typename Listener, typename ListenerHolder>
  void
  DescriptorListenerCallbackTempl<Listener, ListenerHolder>::listener(
    ListenerHolder new_listener) throw ()
  {
    listener_ = std::move(new_listener);
  }

  template <typename Listener, typename ListenerHolder>
  Listener*
  DescriptorListenerCallbackTempl<Listener, ListenerHolder>::listener()
    throw ()
  {
    return listener_;
  }

  template <typename Listener, typename ListenerHolder>
  void
  DescriptorListenerCallbackTempl<Listener, ListenerHolder>::on_closed(
    int /*fd*/, size_t /*fd_index*/, int /*error*/) throw ()
  {
  }

  template <typename Listener, typename ListenerHolder>
  void
  DescriptorListenerCallbackTempl<Listener, ListenerHolder>::on_periodic()
    throw ()
  {
  }

  //
  // DescriptorListenerCallback class
  //

  inline
  DescriptorListenerCallback::~DescriptorListenerCallback() throw ()
  {
  }


  //
  // ActiveDescriptorListenerCallback class
  //

  inline
  ActiveDescriptorListenerCallback::~ActiveDescriptorListenerCallback()
    throw ()
  {
  }


  //
  // ExecuteAndListenCallback class
  //

  inline
  ExecuteAndListenCallback::~ExecuteAndListenCallback() throw ()
  {
  }
}

#endif
