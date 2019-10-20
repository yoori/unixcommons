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




#include <eh/Errno.hpp>

#include <Generics/ArrayAutoPtr.hpp>
#include <Generics/Rand.hpp>

#include <Logger/FileLogger.hpp>


namespace Logging
{
  namespace File
  {
    namespace Helper
    {
      //
      // Handler class
      //

      Handler::Handler(File::Config&& config)
        throw (Exception, eh::Exception)
        : POLICIES_(std::move(config.policies)),
          TIME_ZONE_(config.time_zone),
          EXTENDED_NAME_FORMAT_(config.extended_name_format),
          FROM_NUM_(config.from_num), ORDER_NUM_(config.order_num),
          formatter_(config.formatter, config.preallocated_size)
      {
        if (config.file_name.empty())
        {
          Stream::Error ostr;
          ostr << FNS << "file_name not specified";
          throw Exception(ostr);
        }

        String::StringManip::strlcpy(file_name_, config.file_name.c_str(),
          sizeof(file_name_));

        if (EXTENDED_NAME_FORMAT_)
        {
          Stream::FileName ostr(cur_file_name_);
          ostr << file_name_ << "." << ORDER_NUM_ << "." << FROM_NUM_;
        }
        else
        {
          String::StringManip::strlcpy(cur_file_name_, file_name_,
            sizeof(cur_file_name_));
        }

        if (::stat(cur_file_name_, &file_stat_) == 0 &&
          file_stat_.st_size > 0)
        {
          log_create_time_ = Generics::Time::ZERO;
        }
        else
        {
          log_create_time_ = Generics::Time::get_time_of_day();
        }

        outfile_ = std::fopen(cur_file_name_, "a");
        if (!outfile_)
        {
          eh::throw_errno_exception<Exception>(FNE,
            "failed to open file '", cur_file_name_, "'");
        }
      }

      bool
      Handler::rotate_if_required(const Generics::ExtendedTime& time)
        throw (Exception, eh::Exception)
      {
        if (!outfile_ || ::stat(cur_file_name_, &file_stat_) != 0)
        {
          if (outfile_)
          {
            std::fclose(outfile_);
            outfile_ = 0;
          }
          outfile_ = std::fopen(cur_file_name_, "a");

          if (!outfile_)
          {
            eh::throw_errno_exception<Exception>(FNE,
              "failed to open file '", cur_file_name_, "'");
          }

          if (::stat(cur_file_name_, &file_stat_) != 0)
          {
            eh::throw_errno_exception<Exception>(FNE,
              "failed to stat file '", cur_file_name_, "'");
          }
        }

        for (Policies::PolicyList::iterator it = POLICIES_.begin();
          it != POLICIES_.end(); ++it)
        {
          if ((*it)->need_rotation(*this))
          {
            rotate(time);
            return true;
          }
        }

        return false;
      }

      void
      Handler::publish(const LogRecord& record)
        throw (Exception, eh::Exception)
      {
        if (log_time_ < record.time)
        {
          log_time_ = record.time;
        }

        rotate_if_required(record.time.get_time(record.time_zone));

        FormatWrapper::Result line(formatter_.format(record));
        if (!line.get())
        {
          Stream::Error ostr;
          ostr << FNS << "failed to format log record";
          throw Exception(ostr);
        }

        if (std::fputs(line.get(), outfile_) == EOF ||
          std::fflush(outfile_) == EOF ||
          ::stat(cur_file_name_, &file_stat_) != 0)
        {
          if (outfile_)
          {
            std::fclose(outfile_);
            outfile_ = 0;
          }
          outfile_ = std::fopen(cur_file_name_, "a");
          if (!outfile_)
          {
            eh::throw_errno_exception<Exception>(FNE,
              "failed to reopen file '", cur_file_name_, "'");
          }

          if (std::fputs(line.get(), outfile_) == EOF ||
            std::fflush(outfile_) == EOF ||
            ::stat(cur_file_name_, &file_stat_) != 0)
          {
            eh::throw_errno_exception<Exception>(FNE,
              "permanently fail to log message to file '",
              cur_file_name_, "'");
          }
        }
      }

      void
      Handler::rotate(const Generics::ExtendedTime& time)
      throw (Exception, eh::Exception)
      {
        log_create_time_ = log_time_;

        if (outfile_)
        {
          std::fclose(outfile_);
          outfile_ = 0;
        }

        // Format new_file_name.
        FileName new_name;
        {
          Stream::FileName ostr(new_name);
          if (EXTENDED_NAME_FORMAT_)
          {
            // <file_name_>_<order_num_>.<from_num_>_YYMMDD.HHMMSSFFFFFF.<RND>
            const unsigned RND = Generics::safe_rand(1000, 9999);
            ostr << file_name_ << "_" << ORDER_NUM_ << "." <<
              FROM_NUM_ << "_" << time.format("%Y%m%d.%H%M%S%q") <<
              "." << RND;
          }
          else
          {
            ostr << file_name_ << "." << time.format("%Y%m%d.%H%M%S%q");
          }
        }

        if (rename(cur_file_name_, new_name) != 0)
        {
          eh::throw_errno_exception<Exception>(FNE,
            "failed to rename file '", cur_file_name_, "' to '",
            new_name, "'");
        }

        outfile_ = std::fopen(cur_file_name_, "a");
        if (!outfile_)
        {
          eh::throw_errno_exception<Exception>(FNE,
            "failed to open file '", cur_file_name_, "'");
        }
      }
    }

    namespace Policies
    {
      //
      // TimeSpanPolicy class
      //

      TimeSpanPolicy::~TimeSpanPolicy() throw ()
      {
      }

      bool
      TimeSpanPolicy::need_rotation(const Helper::Handler& file_handler)
        throw (Exception, eh::Exception)
      {
        if (file_handler.file_stat().st_size == 0)
        {
          return false;
        }

        return file_handler.log_time() >=
          file_handler.log_create_time() + rotation_time;
      }


      //
      // SizeSpanPolicy class
      //

      SizeSpanPolicy::~SizeSpanPolicy() throw ()
      {
      }

      bool
      SizeSpanPolicy::need_rotation(const Helper::Handler& file_handler)
        throw (Exception, eh::Exception)
      {
        unsigned long long file_size = file_handler.file_stat().st_size;

        return file_size >= rotation_size;
      }


      //
      // AlignedTimeSpanPolicy class
      //

      AlignedTimeSpanPolicy::~AlignedTimeSpanPolicy() throw ()
      {
      }

      bool
      AlignedTimeSpanPolicy::need_rotation(
        const Helper::Handler& file_handler)
        throw (Exception, eh::Exception)
      {
        if (file_handler.file_stat().st_size == 0)
        {
          return false;
        }

        if (border_time_ == Generics::Time::ZERO ||
          file_handler.log_create_time() != last_create_time_)
        {
          Generics::Time log_create_time(
            file_handler.log_create_time() == Generics::Time::ZERO ?
            Generics::Time(file_handler.file_stat().st_mtime) :
            file_handler.log_create_time());
          Generics::ExtendedTime cur(log_create_time.get_time(
            file_handler.get_time_zone()));
          cur.set_time(start_time_);
          border_time_ = cur;
          if (border_time_ > log_create_time)
          {
            border_time_ -= 24 * 60 * 60;
          }
          while (border_time_ < log_create_time)
          {
            border_time_ += rotation_time;
          }
          last_create_time_ = file_handler.log_create_time();
        }

        return file_handler.log_time() > border_time_;
      }
    }
  }
}
