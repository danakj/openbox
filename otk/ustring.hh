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

//! The number of bytes to skip to find the next character in the string
const char g_utf8_skip[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

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
  typedef value_type                        reference;
  typedef void                              pointer;

  inline ustring_Iterator() {}
  inline ustring_Iterator(const ustring_Iterator<std::string::iterator>&
			  other) : _pos(other.base()) {}

  inline value_type operator*() const {
    // get a unicode character from the iterator's position

    // get an iterator to the internal string
    std::string::const_iterator pos = _pos;
    
    unichar result = static_cast<unsigned char>(*pos);

    // if its not a 7-bit ascii character
    if((result & 0x80) != 0) {
      // len is the number of bytes this character takes up in the string
      unsigned char len = g_utf8_skip[result];
      result &= 0x7F >> len;
      
      while(--len != 0) {
	result <<= 6;
	result |= static_cast<unsigned char>(*++pos) & 0x3F;
      }
    }
    
    return result;
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

#endif // DOXYGEN_IGNORE

//! This class provides a simple wrapper to a std::string that is encoded as
//! UTF-8.
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

public:
  typedef std::string::size_type                        size_type;
  typedef std::string::difference_type                  difference_type;

  typedef unichar                                       value_type;
  typedef unichar &                                     reference;
  typedef const unichar &                               const_reference;

  typedef ustring_Iterator<std::string::iterator>       iterator;
  typedef ustring_Iterator<std::string::const_iterator> const_iterator;

  static const size_type npos = std::string::npos;

  ustring();
  ~ustring();

  // make new strings
  
  ustring(const ustring& other);
  ustring& operator=(const ustring& other);
  ustring(const std::string& src);
  ustring::ustring(const char* src);

  // sizes
  
  ustring::size_type size() const;
  ustring::size_type length() const;
  ustring::size_type bytes() const;
  ustring::size_type capacity() const;
  ustring::size_type max_size() const;

  // internal data

  const char* data()  const;
  const char* c_str() const;
  
};

}

#endif // __ustring_hh
