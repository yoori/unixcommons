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





#include <map>

#include <String/StringManip.hpp>
#include <String/TextTemplate.hpp>

#include <Stream/MMapStream.hpp>


namespace String
{
  namespace TextTemplate
  {
    //
    // Basic::Item class
    //

    Basic::Item::~Item() throw ()
    {
    }


    //
    // Basic::StringItem class
    //

    Basic::StringItem::StringItem(const SubString& val)
      throw (eh::Exception)
      : value_(val)
    {
    }

    Basic::StringItem::~StringItem() throw ()
    {
    }


    void
    Basic::StringItem::append_value(const ArgsCallback& /*callback*/,
      std::string& dst) const throw (eh::Exception)
    {
      value_.append_to(dst);
    }

    std::string
    Basic::StringItem::key(const ArgsCallback& /*callback*/) const
      throw (eh::Exception)
    {
      return std::string();
    }


    //
    // Basic::VarItem class
    //

    Basic::VarItem::VarItem(const SubString& key) throw (eh::Exception)
      : key_(key)
    {
    }

    Basic::VarItem::~VarItem() throw ()
    {
    }


    void
    Basic::VarItem::append_value(const ArgsCallback& callback,
      std::string& dst) const throw (eh::Exception)
    {
      std::string result;
      if (!callback.get_argument(key_, result))
      {
        Stream::Error ostr;
        ostr << FNS << "failed to substitute key '" << key_ << "'";
        throw UnknownName(ostr);
      }
      dst += result;
    }

    std::string
    Basic::VarItem::key(const ArgsCallback& callback) const
      throw (eh::Exception)
    {
      std::string result;
      callback.get_argument(key_, result, false);
      return result;
    }



    //
    // SubString class
    //

    const SubString Basic::DEFAULT_LEXEME("%%", 2);

    Basic::Basic(const SubString& str,
      const SubString& start_lexeme, const SubString& end_lexeme)
      throw (InvalidTemplate, TextTemplException, eh::Exception)
    {
      init(str, start_lexeme, end_lexeme);
    }

    void
    Basic::init(const SubString& source,
      const SubString& start_lexeme, const SubString& end_lexeme)
      throw (InvalidTemplate, TextTemplException, eh::Exception)
    {
      items_.clear();

      if (start_lexeme.empty())
      {
        Stream::Error ostr;
        ostr << FNS << "empty start_lexeme.";
        throw TextTemplException(ostr);
      }

      if (end_lexeme.empty())
      {
        Stream::Error ostr;
        ostr << FNS << "empty end_lexeme.";
        throw TextTemplException(ostr);
      }

      SubString str(source);

      // Split a template string on keys into items
      // and create the list from them.
      while (!str.empty())
      {
        SubString::SizeType begin = str.find(start_lexeme);

        if (begin == SubString::NPOS)
        {
          items_.push_back(Item_var(new StringItem(str)));
          break;
        }

        items_.push_back(Item_var(new StringItem(str.substr(0, begin))));

        begin += start_lexeme.length();

        SubString::SizeType end = str.find(end_lexeme, begin);
        if (end == SubString::NPOS)
        {
          Stream::Error ostr;
          ostr << FNS <<
            "invalid template: closing lexeme (" << end_lexeme <<
            ") not found. Template:\n'" << source << "'";
          throw InvalidTemplate(ostr);
        }

        items_.push_back(Item_var(
          new VarItem(str.substr(begin, end - begin))));

        str = str.substr(end + end_lexeme.length());
      }
    }

    std::string
    Basic::instantiate(const ArgsCallback& args) const
      throw (UnknownName, TextTemplException, eh::Exception)
    {
      std::string str;

      // Replace keys with values
      for (Items::const_iterator it = items_.begin();
        it != items_.end(); ++it)
      {
        (*it)->append_value(args, str);
      }

      return str;
    }

    void
    Basic::keys(const ArgsCallback& args, Keys& keys) const
      throw (UnknownName, TextTemplException, eh::Exception)
    {
      keys.clear();
      for (Items::const_iterator it = items_.begin();
        it != items_.end(); ++it)
      {
        std::string&& name = (*it)->key(args);
        if (!name.empty())
        {
          keys.insert(std::move(name));
        }
      }
    }


    //
    // String class
    //

    void
    String::init(const SubString& str,
      const SubString& start_lexeme, const SubString& end_lexeme)
      throw (InvalidTemplate, TextTemplException, eh::Exception)
    {
      str.assign_to(text_template_);

      Basic::init(text_template_, start_lexeme, end_lexeme);
    }

    String::String(const SubString& str,
      const SubString& start_lexeme, const SubString& end_lexeme)
      throw (InvalidTemplate, TextTemplException, eh::Exception)
    {
      init(str, start_lexeme, end_lexeme);
    }


    //
    // IStream class
    //

    void
    IStream::init(std::istream& istr,
      const SubString& start_lexeme, const SubString& end_lexeme)
      throw (InvalidTemplate, TextTemplException, eh::Exception)
    {
      // Create a big string from a template file.
      std::getline(istr, text_template_, '\0');
      if (istr.bad() || !istr.eof())
      {
        Stream::Error ostr;
        ostr << FNS << "unable to read from istream.";
        throw TextTemplException(ostr);
      }

      Basic::init(text_template_, start_lexeme, end_lexeme);
    }

    IStream::IStream(std::istream& istr,
      const SubString& start_lexeme, const SubString& end_lexeme)
      throw (InvalidTemplate, TextTemplException, eh::Exception)
    {
      init(istr, start_lexeme, end_lexeme);
    }

    namespace
    {
      /**
       * utf-8 => utf-8 encoder
       * @param value string for convertation
       * @param encoded resulted encoded string
       */
      void
      encode_utf8_(std::string&& value, std::string& encoded)
        throw (eh::Exception)
      {
        encoded = std::move(value);
      }

      /**
       * utf-8 => xml encoder
       * @param value string for conversion
       * @param encoded resulted encoded string
       */
      void
      encode_xml_(std::string&& value, std::string& encoded)
        throw (StringManip::InvalidFormatException, eh::Exception)
      {
        StringManip::xml_encode(value.c_str(), encoded);
      }

      void
      encode_mime_(std::string&& value, std::string& encoded)
        throw (StringManip::InvalidFormatException, eh::Exception)
      {
        StringManip::mime_url_encode(value, encoded);
      }

      void
      encode_js_unicode_(std::string&& value, std::string& encoded)
        throw (StringManip::InvalidFormatException, eh::Exception)
      {
        StringManip::js_unicode_encode(value.c_str(), encoded);
      }

      void
      encode_js_(std::string&& value, std::string& encoded)
        throw (StringManip::InvalidFormatException, eh::Exception)
      {
        StringManip::js_encode(value.c_str(), encoded);
      }

      /**
       * EncoderHolder class is a holder of default encoders
       */
      class EncoderHolder
      {
      public:
        /**
         * Finds previously registered encoder by the key
         * @param key unique keys determining encoder
         * @return Previously registered encoder associated with the key
                   or zero if not found
         */
        Args::ValueEncoder
        get_value_encoder(const SubString& key) const throw ();

        /**
         * Performs registration of encoder
         * @param key the unique key
         * @param encoder encoder associated with the key
         */
        void
        register_value_encoder(const char* key,
          Args::ValueEncoder encoder) throw (eh::Exception);

      private:
        typedef std::map<const SubString, Args::ValueEncoder> RelationType;

        RelationType relation_;
      };

      inline
      Args::ValueEncoder
      EncoderHolder::get_value_encoder(const SubString& key) const throw ()
      {
        RelationType::const_iterator found(relation_.find(key));
        return found == relation_.end() ? 0 : found->second;
      }

      inline
      void
      EncoderHolder::register_value_encoder(const char* key,
        Args::ValueEncoder encoder) throw (eh::Exception)
      {
        relation_[SubString(key)] = encoder;
      }

      /**
       * Global encoder holder
       */
      EncoderHolder encoder_holder;
    }


    //
    // DefaultValue class
    //

    DefaultValue::DefaultValue(const ArgsCallback* callback) throw ()
      : callback_(callback)
    {
    }

    bool
    DefaultValue::get_argument(const SubString& key, std::string& result,
      bool value) const throw (eh::Exception)
    {
      SubString::SizeType pos = key.find('=');

      if (pos == 0)
      {
        return false;
      }

      if (pos == SubString::NPOS)
      {
        return callback_->get_argument(key, result, value);
      }

      if (callback_->get_argument(key.substr(0, pos), result, value))
      {
        return true;
      }
      (value ? key.substr(pos + 1) : key.substr(0, pos)).assign_to(result);
      return true;
    }


    //
    // ArgsEncoder::EncoderItem class
    //

    ArgsEncoder::EncoderItem::EncoderItem(const char* key,
      ValueEncoder encoder) throw (eh::Exception)
      : encoder_(encoder)
    {
      assert(key);
      encoder_holder.register_value_encoder(key, encoder);
    }


    //
    // ArgsEncoder class
    //

    /**X
     * Default encodings
     */
    const ArgsEncoder::EncoderItem
      ArgsEncoder::EI_UTF8("utf8", encode_utf8_);
    const ArgsEncoder::EncoderItem
      ArgsEncoder::EI_MIME_URL("mime-url", encode_mime_);
    const ArgsEncoder::EncoderItem
      ArgsEncoder::EI_XML("xml", encode_xml_);
    const ArgsEncoder::EncoderItem
      ArgsEncoder::EI_JS_UNICODE("js-unicode", encode_js_unicode_);
    const ArgsEncoder::EncoderItem
      ArgsEncoder::EI_JS("js", encode_js_);

    ArgsEncoder::ArgsEncoder(ArgsCallback* args_container,
      bool encode, bool error_if_no_key,
      const EncoderItem& default_encoding)
      throw (UnknownName, eh::Exception)
      : args_container_(args_container),
        ENCODE_(encode), ERROR_IF_NO_KEY_(error_if_no_key),
        DEFAULT_ENCODER_(default_encoding.get_encoder_())
    {
      if (!DEFAULT_ENCODER_)
      {
        Stream::Error ostr;
        ostr << FNS << "invalid key";
        throw UnknownName(ostr);
      }
    }

    void
    ArgsEncoder::set_callback(ArgsCallback* args_container) throw ()
    {
      args_container_ = args_container;
    }

    bool
    ArgsEncoder::get_argument(const SubString& key, std::string& result,
      bool value) const throw (eh::Exception)
    {
      if (key.empty())
      {
        return false;
      }

      ValueEncoder encoder = DEFAULT_ENCODER_;

      SubString key_val(key);

      if (ENCODE_)
      {
        SubString::SizeType pos = key_val.find(':');
        if (pos != SubString::NPOS)
        {
          ValueEncoder found =
            encoder_holder.get_value_encoder(key_val.substr(0, pos));
          if (found)
          {
            encoder = found;
            key_val = key_val.substr(pos + 1);
          }
        }
      }

      std::string found;

      if (!args_container_->get_argument(key_val, found, value))
      {
        if (ERROR_IF_NO_KEY_)
        {
          return false;
        }
        else
        {
          if (value)
          {
            result.clear();
          }
          else
          {
            key_val.assign_to(result);
          }
          return true;
        }
      }

      if (!value)
      {
        result = std::move(found);
        return true;
      }

      // Special case: no encode
      if (encoder == EI_UTF8.get_encoder_())
      {
        result = std::move(found);
        return true;
      }

      (*encoder)(std::move(found), result);

      return true;
    }


    //
    // Args class
    //

    Args::Args(bool encode, unsigned long table_size,
      bool error_if_no_key, const EncoderItem& default_encoding,
      bool has_defaults) throw (UnknownName, eh::Exception)
      : ArgsEncoder(0, encode, error_if_no_key, default_encoding),
        ValueContainer(table_size), args_container_(this),
        default_value_callback_(&args_container_)
    {
      ArgsCallback* callback = &args_container_;
      if (has_defaults)
      {
        callback = &default_value_callback_;
      }
      set_callback(callback);
    }


    //
    // UpdateStrategy class
    //

    void
    UpdateStrategy::update()
      throw (TextTemplException, eh::Exception)
    {
      try
      {
        Stream::FileParser file(fname_.c_str());

        try
        {
          text_template_.init(file, start_lexeme(), end_lexeme());
        }
        catch (const TextTemplException& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "failed to initialize with file " << fname_ <<
            ": " << ex.what();
          throw TextTemplException(ostr);
        }
      }
      catch (const TextTemplException&)
      {
        throw;
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "failed to open file '" << fname_ << "': " <<
          ex.what();
        throw TextTemplException(ostr);
      }
    }
  }
}
