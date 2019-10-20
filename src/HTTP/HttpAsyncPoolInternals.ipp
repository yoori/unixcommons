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



namespace HTTP
{
  namespace HttpInternals
  {
    //
    // SignalQueue class
    //

    template <typename Object, typename Data>
    SignalQueue<Object, Data>::SignalQueue(Object& object,
      DataCallback data_callback, QuitCallback quit_callback,
      CheckCallback check_callback)
      throw (eh::Exception, SyscallFailure)
      : object_(object),
        data_callback_(data_callback), quit_callback_(quit_callback),
        check_callback_(check_callback), removed_(true)
    {
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::register_event(event_base& base)
      throw (eh::Exception, Exception)
    {
      event_set(&pipe_read_, pipe_.read_descriptor(), EV_READ | EV_PERSIST,
        read_callback_, this);
      event_base_set(&base, &pipe_read_);
      if (event_add(&pipe_read_, 0) == -1)
      {
        Stream::Error ostr;
        ostr << FNS << "event_add() failed.";
        throw Exception(ostr);
      }
      removed_ = false;
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::add(Data& data)
      throw (eh::Exception, SyscallFailure)
    {
      bool empty;

      {
        Sync::PosixGuard guard(mutex_);
        empty = queue_.empty();
        queue_.push_back(data);
      }

      if (empty)
      {
        signal(RT_DATA);
      }
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::quit() throw (SyscallFailure)
    {
      signal(RT_QUIT);
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::check() throw (SyscallFailure)
    {
      signal(RT_CHECK);
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::flush() throw (eh::Exception)
    {
      remove_event_();

      for (;;)
      {
        Data data;
        {
          Sync::PosixGuard guard(mutex_);
          if (queue_.empty())
          {
            break;
          }
          data = queue_.front();
          queue_.pop_front();
        }
        (object_.*data_callback_)(data);
      }

      for (unsigned char data[256];
        pipe_.read(data, sizeof(data)) == sizeof(data);)
      {
      }
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::handle_read_() throw ()
    {
      bool states[RT_LAST];
      std::fill(states, states + RT_LAST, false);

      for (;;)
      {
        unsigned char data[256];
        ssize_t res = pipe_.read(data, sizeof(data) - 1);
        switch (res)
        {
        case -1:
          if (errno == EAGAIN)
          {
            break;
          }

        case 0:
          states[RT_QUIT] = true;
          break;

        default:
          const bool cont = res == sizeof(data);
          for (unsigned char* ptr = data; res--; ptr++)
          {
            states[*ptr] = true;
          }
          if (cont)
          {
            continue;
          }
          break;
        }
        break;
      }

      if (states[RT_DATA])
      {
        for (;;)
        {
          Data data;
          {
            Sync::PosixGuard guard(mutex_);
            if (queue_.empty())
            {
              break;
            }
            data = queue_.front();
            queue_.pop_front();
          }
          (object_.*data_callback_)(data);
        }
      }

      if (states[RT_QUIT])
      {
        terminate_();
      }
      else
      {
        if (states[RT_CHECK])
        {
          (object_.*check_callback_)();
        }
      }
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::read_callback_(
      int /*fd*/, short /*type*/, void* arg) throw ()
    {
      static_cast<SignalQueue<Object, Data>*>(arg)->handle_read_();
    }


    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::signal(unsigned char data)
      throw (SyscallFailure)
    {
      for (;;)
      {
        ssize_t res = pipe_.signal(data);
        switch (res)
        {
        case -1:
          eh::throw_errno_exception<SyscallFailure>(FNE, "send");

        case 0:
          {
            Stream::Error ostr;
            ostr << FNS << "send error: connection closed";
            throw SyscallFailure(ostr);
          }

        default:
          break;
        }
        break;
      }
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::terminate_() throw ()
    {
      remove_event_();
      (object_.*quit_callback_)();
    }

    template <typename Object, typename Data>
    void
    SignalQueue<Object, Data>::remove_event_() throw ()
    {
      if (!removed_)
      {
        event_del(&pipe_read_);
        removed_ = true;
      }
    }
  }
}
