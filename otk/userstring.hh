// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __userstring_hh
#define   __userstring_hh

#include <string>
#include <vector>

extern "C" {
#include <assert.h>
}

//! userstring is a std::string with an extra flag specifying if the string is
//! UTF-8 encoded.
class userstring : public std::string
{
public:
  //! A vector of userstrings
  typedef std::vector<userstring> vector;
  
private:
  bool _utf8;

public:
  userstring(bool utf8) : std::string(), _utf8(utf8) {}
  userstring(const userstring& s, size_type pos = 0,
	     size_type n = npos) : std::string(s, pos, n), _utf8(s._utf8) {}
  userstring(const char *s, bool utf8) : std::string(s), _utf8(utf8) {}
  userstring(const char *s, size_type n, bool utf8) : std::string(s, n),
						      _utf8(utf8) {}
  userstring(size_type n, char c, bool utf8) : std::string(n, c),
					       _utf8(utf8) {}
  userstring(const_iterator first, const_iterator last, bool utf8) :
    std::string(first, last), _utf8(utf8) {}

  //! Returns if the string is encoded in UTF-8 or not
  inline bool utf8() const { return _utf8; }

  inline void setUtf8(bool u) { _utf8 = u; }

  inline userstring& insert(size_type pos, const userstring& s) {
    assert(s._utf8 == _utf8);
    std::string::insert(pos, s);
    return *this;
  }

  inline userstring& insert(size_type pos,
			    const userstring& s,
			    size_type pos1, size_type n) {
    assert(s._utf8 == _utf8);
    std::string::insert(pos, s, pos1, n);
    return *this;
  }

  inline userstring& append(const userstring& s) {
    assert(s._utf8 == _utf8);
    std::string::append(s);
    return *this;
  }

  inline userstring& append(const userstring& s,
			    size_type pos, size_type n) {
    assert(s._utf8 == _utf8);
    std::string::append(s, pos, n);
    return *this;
  }

  inline userstring& assign(const userstring& s) {
    assert(s._utf8 == _utf8);
    std::string::assign(s);
    return *this;
  }
  
  inline userstring& assign(const userstring& s,
			    size_type pos, size_type n) {
    assert(s._utf8 == _utf8);
    std::string::assign(s, pos, n);
    return *this;
  }

  inline userstring& replace(size_type pos, size_type n,
			     const userstring& s) {
    assert(s._utf8 == _utf8);
    std::string::replace(pos, n, s);
    return *this;
  }

  inline userstring& replace(size_type pos, size_type n,
			     const userstring& s,
			     size_type pos1, size_type n1) {
    assert(s._utf8 == _utf8);
    std::string::replace(pos, n, s, pos1, n1);
    return *this;
  }

  inline userstring& operator=(const userstring& s) {
    return assign(s);
  }
  
  inline userstring& operator+=(const userstring& s) {
    return append(s);
  }
};

#endif // __userstring_hh
