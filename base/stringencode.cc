/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "stringencode.h"

#include <cstdio>
#include "stringutils.h"


/////////////////////////////////////////////////////////////////////////////
// String Encoding Utilities
/////////////////////////////////////////////////////////////////////////////

size_t url_decode(char* buffer,
                  size_t buflen,
                  const char* source,
                  size_t srclen) {
  if (nullptr == buffer)
    return srclen + 1;
  if (buflen <= 0)
    return 0;

  unsigned char h1, h2;
  size_t srcpos = 0, bufpos = 0;
  while ((srcpos < srclen) && (bufpos + 1 < buflen)) {
    unsigned char ch = source[srcpos++];
    if (ch == '+') {
      buffer[bufpos++] = ' ';
    } else if ((ch == '%') && (srcpos + 1 < srclen) &&
               hex_decode(source[srcpos], &h1) &&
               hex_decode(source[srcpos + 1], &h2)) {
      buffer[bufpos++] = (h1 << 4) | h2;
      srcpos += 2;
    } else {
      buffer[bufpos++] = ch;
    }
  }
  buffer[bufpos] = '\0';
  return bufpos;
}

bool hex_decode(char ch, unsigned char* val) {
  if ((ch >= '0') && (ch <= '9')) {
    *val = ch - '0';
  } else if ((ch >= 'A') && (ch <= 'F')) {
    *val = (ch - 'A') + 10;
  } else if ((ch >= 'a') && (ch <= 'f')) {
    *val = (ch - 'a') + 10;
  } else {
    return false;
  }
  return true;
}


size_t transform(std::string& value,
                 size_t maxlen,
                 const std::string& source,
                 Transform t) {
  char* buffer = STACK_ARRAY(char, maxlen + 1);
  size_t length = t(buffer, maxlen + 1, source.data(), source.length());
  value.assign(buffer, length);
  return length;
}

std::string s_transform(const std::string& source, Transform t) {
  // Ask transformation function to approximate the destination size (returns
  // upper bound)
  size_t maxlen = t(nullptr, 0, source.data(), source.length());
  char* buffer = STACK_ARRAY(char, maxlen);
  size_t len = t(buffer, maxlen, source.data(), source.length());
  std::string result(buffer, len);
  return result;
}
