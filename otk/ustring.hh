// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __ustring_hh
#define   __ustring_hh

/*! @file ustring.hh
  @brief Provides a simple UTF-8 encoded string
*/

extern "C" {

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#else
#  ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif
#endif

}

#include <string>

namespace otk {


#ifdef HAVE_STDINT_H
typedef uint32_t unichar;
#else
typedef u_int32_t unichar;
#endif


#ifndef DOXYGEN_IGNORE

unichar utf8_get_char(const char *p);

#endif // DOXYGEN_IGNORE

//! The iterator type for ustring
/*!
  Note this is not a random access iterator but a bidirectional one, since all
  index operations need to iterate over the UTF-8 data.  Use std::advance() to
  move to a certain position.
  <p>
  A writeable iterator isn't provided because:  The number of bytes of the old
  UTF-8 character and the new one to write could be different. Therefore, any
  write operation would invalidate all other iterators pointing into the same
  string.
*/

template <class T>
class ustring_Iterator
{
public:
  typedef std::bidirectional_iterator_tag   iterator_category;
  typedef unichar                           value_type;
  typedef std::string::difference_type      difference_type;
  //typedef value_type                        reference;
  typedef void                              pointer;
  
  inline ustring_Iterator() {}
  inline ustring_Iterator(const ustring_Iterator<std::string::iterator>&
			  other) : _pos(other.base()) {}


  inline value_type operator*() const {
    // get an iterator to the internal string
    std::string::const_iterator pos = _pos;
    return utf8_get_char(&(*pos));
  }

  
  inline ustring_Iterator<T> &     operator++() {
    pos_ += g_utf8_skip[static_cast<unsigned char>(*pos_)];
    return *this;
  }
  inline ustring_Iterator<T> &     operator--() {
    do { --_pos; } while((*_pos & '\xC0') == '\x80');
    return *this;
  }

  explicit inline ustring_Iterator(T pos) : _pos(pos) {}
  inline T base() const { return _pos; }

private:
  T _pos;
};


//! This class provides a simple wrapper to a std::string that can be encoded
//! as UTF-8. The ustring::utf() member specifies if the given string is UTF-8
//! encoded. ustrings default to specifying UTF-8 encoding.
/*!
  This class does <b>not</b> handle extended 8-bit ASCII charsets like
  ISO-8859-1.
  <p>
  More info on Unicode and UTF-8 can be found here:
  http://www.cl.cam.ac.uk/~mgk25/unicode.html
  <p>
  This does not subclass std::string, because std::string was intended to be a
  final class.  For instance, it does not have a virtual destructor.
*/
class ustring {
  std::string _string;
  bool _utf8;
  
public:
  typedef std::string::size_type                        size_type;
  typedef std::string::difference_type                  difference_type;

  typedef unichar                                       value_type;
  //typedef unichar &                                     reference;
  //typedef const unichar &                               const_reference;

  //typedef ustring_Iterator<std::string::iterator>       iterator;
  //typedef ustring_Iterator<std::string::const_iterator> const_iterator;

  static const size_type npos = std::string::npos;

  ustring();
  ~ustring();

  // make new strings
  
  ustring(const ustring& other);
  ustring& operator=(const ustring& other);
  ustring(const std::string& src);
  ustring(const char* src);

  // append to the string

  ustring& operator+=(const ustring& src);
  ustring& operator+=(const char* src);
  ustring& operator+=(char c);

  // sizes
  
  ustring::size_type size() const;
  ustring::size_type bytes() const;
  ustring::size_type capacity() const;
  ustring::size_type max_size() const;
  bool empty() const;

  // erase substrings

  void clear();
  ustring& erase(size_type i, size_type n=npos);

  // change the string's size

  void resize(size_type n, char c='\0');

  // extract characters
  
  // No reference return; use replace() to write characters.
  value_type operator[](size_type i) const;

  // internal data

  const char* data()  const;
  const char* c_str() const;

  // encoding
  
  bool utf8() const;
  void setUtf8(bool utf8);
};

}

#endif // __ustring_hh
