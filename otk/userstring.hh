// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __userstring_hh
#define   __userstring_hh

#include <string>

//! userstring is a std::string with an extra flag specifying if the string is
//! UTF-8 encoded.
class userstring : public std::string
{
private:
  bool _utf8;

public:
  userstring(bool utf8) : std::string(), _utf8(utf8) {}
  userstring(const userstring& s, bool utf8, size_type pos = 0,
	     size_type n = npos) : std::string(s, pos, n), _utf8(utf8) {}
  userstring(const charT *s, bool utf8) : std::string(s), _utf8(utf8) {}
  userstring(const charT* s, size_type n, bool utf8) : std::string(s, n),
						       _utf8(utf8) {}
  userstring(size_type n, charT c, bool utf8) : std::string(n, c),
						_utf8(utf8) {}

  //! Returns if the string is encoded in UTF-8 or not
  inline bool utf8() const { return _utf8; }
};

#endif // __userstring_hh
