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





#ifndef STRING_TEXTTEMPLATE_HPP
#define STRING_TEXTTEMPLATE_HPP

#include <istream>
#include <set>

#include <ReferenceCounting/ReferenceCounting.hpp>
#include <ReferenceCounting/Deque.hpp>

#include <Generics/GnuHashTable.hpp>
#include <Generics/HashTableAdapters.hpp>
#include <Generics/Function.hpp>


namespace String
{
  namespace TextTemplate
  {
    DECLARE_EXCEPTION(TextTemplException, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidTemplate, TextTemplException);
    DECLARE_EXCEPTION(UnknownName, TextTemplException);

    /**
     * Callback context. Determines values for keys.
     */
    class ArgsCallback
    {
    public:
      /**
       * Destructor
       */
      virtual
      ~ArgsCallback() throw ();

      /**
       * Returns value for a key.
       * @param key Text of a key.
       * @param result Value corresponding with the key.
       * @param value if false return key name if has value to supply.
       * @return whether key was processed or not
       */
      virtual
      bool
      get_argument(const SubString& key, std::string& result,
        bool value = true) const throw (eh::Exception) = 0;
    };


    typedef std::set<std::string> Keys;

    /**
     * Text template. Replace keys with values in a pattern.
     * Works on SubString supplied.
     */
    class Basic : private Generics::Uncopyable
    {
    public:
      static const SubString DEFAULT_LEXEME;

      /**
       * Constructor.
       */
      Basic() throw ();

      /**
       * Constructor. Calls init.
       * @param str template to parse
       * @param start_lexeme Start lexeme.
       * @param end_lexeme End lexeme.
       * @exception InvalidTemplate Invalid template.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      explicit
      Basic(const SubString& str,
        const SubString& start_lexeme = DEFAULT_LEXEME,
        const SubString& end_lexeme = DEFAULT_LEXEME)
        throw (InvalidTemplate, TextTemplException, eh::Exception);

      /**
       * Destructor.
       */
      virtual
      ~Basic() throw ();

      /**
       * Initializes a pattern.
       * @param str template to parse
       * @param start_lexeme Start lexeme.
       * @param end_lexeme End lexeme.
       * @exception InvalidTemplate Invalid template.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      void
      init(const SubString& str,
        const SubString& start_lexeme = DEFAULT_LEXEME,
        const SubString& end_lexeme = DEFAULT_LEXEME)
        throw (InvalidTemplate, TextTemplException, eh::Exception);

      /**
       * Instantiation of a pattern.
       * @param args supplier of values for found keys
       * @return instantiated template
       * @exception UnknownName Invalid or unknown key.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      std::string
      instantiate(const ArgsCallback& args) const
        throw (UnknownName, TextTemplException, eh::Exception);

      /**
       * Building a set of keys args contains values for.
       * @param args supplier of values for found keys
       * @param keys resulted keys set
       * @exception UnknownName Invalid or unknown key.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      void
      keys(const ArgsCallback& args, Keys& keys) const
        throw (UnknownName, TextTemplException, eh::Exception);

      /**
       * Tests whether the template is contains items or not
       * @return true if contains
       */
      bool
      empty() const throw (eh::Exception);

    private:
      /**
       * Base item interface.
       */
      class Item : public ReferenceCounting::DefaultImpl<>
      {
      protected:
        /**
         * Destructor
         */
        virtual
        ~Item() throw ();

      public:
        /**
         * Add value of item to destination string.
         * @param callback Informational callback
         * @param dst Append value of item to it
         */
        virtual
        void
        append_value(const ArgsCallback& callback, std::string& dst) const
          throw (eh::Exception) = 0;

        /**
         * Checks if callback contains the specific value if it's required
         * @param callback Informational callback
         * @return required key name or empty if not required
         */
        virtual
        std::string
        key(const ArgsCallback& callback) const
          throw (eh::Exception) = 0;
      };
      typedef ReferenceCounting::QualPtr<Item> Item_var;

      /**
       * String item.
       */
      class StringItem : public Item
      {
      public:
        StringItem(const SubString& val) throw (eh::Exception);

        /**
         * Adds stored value to destination string.
         * @param callback Informational callback
         * @param dst Append value of item to it
         */
        virtual
        void
        append_value(const ArgsCallback& callback, std::string& dst) const
          throw (eh::Exception);

        /**
         * Does nothing
         * @param callback Informational callback
         * @return empty SubString
         */
        virtual
        std::string
        key(const ArgsCallback& callback) const
          throw (eh::Exception);

      protected:
        /**
         * Destructor
         */
        virtual
        ~StringItem() throw ();

      private:
        SubString value_;
      };

      /**
       * Variable item.
       */
      class VarItem : public Item
      {
      public:
        VarItem(const SubString& key) throw (eh::Exception);

        /**
         * Founds value for the stored key and adds it to destination
         * string.
         * @param callback Informational callback
         * @param dst Append value to it
         */
        virtual
        void
        append_value(const ArgsCallback& callback, std::string& dst) const
          throw (eh::Exception);

        /**
         * Checks if callback contains the value for the key
         * @param callback Informational callback
         * @return required key name
         */
        virtual
        std::string
        key(const ArgsCallback& callback) const
          throw (eh::Exception);

      protected:
        /**
         * Destructor
         */
        virtual
        ~VarItem() throw ();

      private:
        SubString key_;
      };

    private:
      typedef ReferenceCounting::Deque<Item_var> Items;

      Items items_;
    };

    /**
     * Text template. Replace keys with values in a pattern.
     * Stores SubString supplied in std::string and works on it.
     */
    class String : public Basic
    {
    public:
      String() throw ();

      /**
       * Constructor. Calls init.
       * @param str template to copy and parse
       * @param start_lexeme Start lexeme.
       * @param end_lexeme End lexeme.
       * @exception InvalidTemplate Invalid template.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      explicit
      String(const SubString& str,
        const SubString& start_lexeme = DEFAULT_LEXEME,
        const SubString& end_lexeme = DEFAULT_LEXEME)
        throw (InvalidTemplate, TextTemplException, eh::Exception);

      virtual
      ~String() throw ();

      /**
       * Initializes a pattern.
       * @param str template to copy and parse
       * @param start_lexeme Start lexeme.
       * @param end_lexeme End lexeme.
       * @exception InvalidTemplate Invalid template.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      void
      init(const SubString& str,
        const SubString& start_lexeme = DEFAULT_LEXEME,
        const SubString& end_lexeme = DEFAULT_LEXEME)
        throw (InvalidTemplate, TextTemplException, eh::Exception);

    protected:
      std::string text_template_;
    };

    /**
     * Text template. Replace keys with values in a pattern.
     * Stores content of std::istream supplied in std::string and 
     * works on it.
     */
    class IStream : public String
    {
    public:
      /**
       * Constructor
       */
      IStream() throw ();

      /**
       * Initializes a pattern.
       * @param istr stream to read and parse
       * @param start_lexeme Start lexeme.
       * @param end_lexeme End lexeme.
       * @exception InvalidTemplate Invalid template.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      explicit
      IStream(std::istream& istr,
        const SubString& start_lexeme = DEFAULT_LEXEME,
        const SubString& end_lexeme = DEFAULT_LEXEME)
        throw (InvalidTemplate, TextTemplException, eh::Exception);

      /**
       * Destructor
       */
      virtual
      ~IStream() throw ();

      /**
       * Initializes a pattern.
       * @param istr stream to read and parse
       * @param start_lexeme Start lexeme.
       * @param end_lexeme End lexeme.
       * @exception InvalidTemplate Invalid template.
       * @exception TextTemplException Other errors.
       * @exception eh::Exception std::exception.
       */
      void
      init(std::istream& istr,
        const SubString& start_lexeme = DEFAULT_LEXEME,
        const SubString& end_lexeme = DEFAULT_LEXEME)
        throw (InvalidTemplate, TextTemplException, eh::Exception);
    };


    /**
     * General adapter for ArgsContainer
     */
    struct ArgsContainerAdapter
    {
      static
      const SubString&
      real_key(const SubString& key) throw ();

      template <typename Iterator>
      static
      std::string
      value(const Iterator& itor)
        throw (eh::Exception);
    };

    /**
     * Specific adapter for ArgsContainer
     */
    struct ArgsContainerStringAdapter : public ArgsContainerAdapter
    {
      static
      std::string
      real_key(const SubString& key) throw (eh::Exception);
    };

    /**
     * Implementation of ArgsCallback using Container to find
     * values for corresponding keys
     */
    template <typename Container, typename Adapter = ArgsContainerAdapter>
    class ArgsContainer : public ArgsCallback
    {
    public:
      /**
       * Constructor to save pointer to container that able find keys.
       * @param cont pointer to container
       */
      explicit
      ArgsContainer(const Container* cont) throw ();

      /**
       * Returns value for a key.
       * @param key Text of a key.
       * @param result Value corresponding with the key.
       * @param value if false return key name if has value to supply.
       * @return whether key was processed or not
       */
      virtual
      bool
      get_argument(const SubString& key, std::string& result,
        bool value = true) const throw (eh::Exception);

    private:
      const Container* cont_;
    };

    /**
     * Implementation of ArgsCallback using default values found in keys
     * names or using additional ArgsCallback for lacking ones.
     */
    class DefaultValue : public ArgsCallback
    {
    public:
      /**
       * Constructor
       * @param callback callback for keys lacking default values
       */
      explicit
      DefaultValue(const ArgsCallback* callback) throw ();

      /**
       * Returns value for a key.
       * @param key Text of a key.
       * @param result Value corresponding with the key.
       * @param value if false return key name if has value to supply.
       * @return whether key was processed or not
       */
      virtual
      bool
      get_argument(const SubString& key, std::string& result,
        bool value = true) const throw (eh::Exception);

      const ArgsCallback* callback_;
    };

    /**
     * ArgsEncoder implements simple encoding enabled text template
     * arguments provider.
     */
    class ArgsEncoder : public ArgsCallback
    {
    public:
      typedef void (*ValueEncoder)(std::string&& value,
        std::string& encoded);

      /**
       * EncoderItem class is required for
       * 1. Registration of default encoding types
       * 2. Transfer of (probable default) encoding type into
       *    Args
       */
      class EncoderItem
      {
      public:
        /**
         * Constructor
         * @param encode Function providing encoding
         */
        explicit
        EncoderItem(ValueEncoder encode) throw ();

        /**
         * Constructor
         * Registers encoder in the common EncoderHolder
         * @param key Unique encoder key, static constant only
         * @param encoder Function providing encoding
         */
        EncoderItem(const char* key, ValueEncoder encoder)
          throw (eh::Exception);

        /**
         * Getter for saved encoder
         * @return Saved encoder
         */
        ValueEncoder
        get_encoder_() const throw ();

      private:
        ValueEncoder encoder_;
      };

    public:
      static const EncoderItem EI_UTF8;
      static const EncoderItem EI_MIME_URL;
      static const EncoderItem EI_XML;
      static const EncoderItem EI_JS;
      static const EncoderItem EI_JS_UNICODE;

      /**
       * A constructor.
       * @param args_container Container of arguments
       * @param encode Inform get_argument to distinguish encoding prefix in
       * key string
       * @param error_if_no_key if true raise ::UnknownName
       * exception when key not found.
       * @param default_encoding Text using encoding
       */
      explicit
      ArgsEncoder(ArgsCallback* args_container,
        bool encode = true, bool error_if_no_key = true,
        const EncoderItem& default_encoding = EI_UTF8)
        throw (UnknownName, eh::Exception);

      /**
       * Set callback
       * @param args_container Pointer to the new callback
       */
      void
      set_callback(ArgsCallback* args_container)
        throw ();

      /**
       * Returns value for a key.
       * @param key Text of a key.
       * @param result Value corresponding with the key.
       * @param value if false return key name if has value to supply.
       * @return whether key was processed or not
       * @exception eh::Exception std::exception.
       */
      virtual
      bool
      get_argument(const SubString& key, std::string& result,
        bool value = true) const throw (eh::Exception);

    protected:
      ArgsCallback* args_container_;
      const bool ENCODE_;
      const bool ERROR_IF_NO_KEY_;
      const ValueEncoder DEFAULT_ENCODER_;
    };


    /**
     * Args class
     */
    class Args :
      public ArgsEncoder,
      public Generics::GnuHashTable<
        Generics::StringHashAdapter, std::string>
    {
    public:
      /**
       * A constructor.
       * @param encode Inform get_argument to distinguish encoding prefix in
       * key string
       * @param table_size defines hash table size
       * @param error_if_no_key if true raise ::UnknownName
       * exception when key not found.
       * @param default_encoding Text using encoding
       * @param has_defaults True switch on callback for default values.
       */
      explicit
      Args(bool encode = true, unsigned long table_size = 200,
        bool error_if_no_key = true,
        const EncoderItem& default_encoding = EI_UTF8,
        bool has_defaults = true)
        throw (UnknownName, eh::Exception);

      ~Args() throw ();

    protected:
      typedef Generics::GnuHashTable<
        Generics::StringHashAdapter, std::string> ValueContainer;

      ArgsContainer<ValueContainer> args_container_;
      DefaultValue default_value_callback_;
    };

    //
    // UpdateStrategy class
    //

    /**
     * UpdateStrategy provides an interface for Default class
     * to be used in conjunction with FileCache to provide
     * "cacheable text file template" functionality.
     */
    class UpdateStrategy
    {
    public:
      /**
       * Declare IStream class to be a FileCache buffer
       */
      typedef const IStream Buffer;

      /**
       * Constructs UpdateStrategy object that will hold
       * text template file name and use it to update Default instance.
       * @param fname file name.
       */
      explicit
      UpdateStrategy(const char* fname) throw (eh::Exception);

      /**
       * Destructs UpdateStrategy object
       */
      virtual
      ~UpdateStrategy() throw ();

      /**
       * Provides reference to Default object as a in-memory buffer of a
       * template file.
       * @return Returns reference to the stored Default object.
       */
      Buffer&
      get() throw ();

      /**
       * Updates stored Default object from a template file.
       * Called by FileCache when file changes.
       */
      void
      update() throw (TextTemplException, eh::Exception);

      /**
       * Provides text template lexeme which starts template variable entry.
       * Should be implemented in derived class.
       * @return Returns starting lexeme.
       */
      virtual
      SubString
      start_lexeme() const throw (eh::Exception) = 0;

      /**
       * Provides text template lexeme which ends template variable entry.
       * Should be implemented in derived class.
       * @return Returns ending lexeme.
       */
      virtual
      SubString
      end_lexeme() const throw (eh::Exception) = 0;

    private:
      IStream text_template_;
      std::string fname_;
    };
  }
}

//
// INLINES
//

namespace String
{
  namespace TextTemplate
  {
    //
    // ArgsCallback class
    //

    inline
    ArgsCallback::~ArgsCallback() throw ()
    {
    }


    //
    // Basic class
    //

    inline
    Basic::Basic() throw ()
    {
    }

    inline
    Basic::~Basic() throw ()
    {
    }

    inline
    bool
    Basic::empty() const throw (eh::Exception)
    {
      return items_.empty();
    }


    //
    // String class
    //

    inline
    String::String() throw ()
    {
    }

    inline
    String::~String() throw ()
    {
    }


    //
    // IStream class
    //

    inline
    IStream::IStream() throw ()
    {
    }

    inline
    IStream::~IStream() throw ()
    {
    }


    //
    // ArgsContainerAdapter class
    //

    inline
    const SubString&
    ArgsContainerAdapter::real_key(const SubString& key) throw ()
    {
      return key;
    }

    template <typename Iterator>
    std::string
    ArgsContainerAdapter::value(const Iterator& itor)
      throw (eh::Exception)
    {
      return itor->second;
    }


    //
    // ArgsContainerAdapter class
    //

    inline
    std::string
    ArgsContainerStringAdapter::real_key(const SubString& key)
      throw (eh::Exception)
    {
      return key.str();
    }


    //
    // ArgsContainer class
    //

    template <typename Container, typename Adapter>
    ArgsContainer<Container, Adapter>::ArgsContainer(const Container* cont)
      throw ()
      : cont_(cont)
    {
    }

    template <typename Container, typename Adapter>
    bool
    ArgsContainer<Container, Adapter>::get_argument(
      const SubString& key, std::string& result, bool value) const
      throw (eh::Exception)
    {
      if (!value)
      {
        key.assign_to(result);
        return true;
      }

      typename Container::const_iterator it =
        cont_->find(Adapter::real_key(key));
      if (it == cont_->end())
      {
        return false;
      }
      result = Adapter::value(it);
      return true;
    }


    //
    // Args::EncoderItem class
    //

    inline
    ArgsEncoder::EncoderItem::EncoderItem(ValueEncoder encoder) throw ()
      : encoder_(encoder)
    {
    }

    inline
    ArgsEncoder::ValueEncoder
    ArgsEncoder::EncoderItem::get_encoder_() const throw ()
    {
      return encoder_;
    }


    //
    // Args class
    //

    inline
    Args::~Args() throw ()
    {
    }


    //
    // UpdateStrategy class
    //

    inline
    UpdateStrategy::UpdateStrategy(const char* fname)
      throw (eh::Exception)
      : fname_(fname ? fname : "")
    {
    }

    inline
    UpdateStrategy::~UpdateStrategy() throw ()
    {
    }

    inline
    UpdateStrategy::Buffer&
    UpdateStrategy::get() throw ()
    {
      return text_template_;
    }
  }
}

#endif
