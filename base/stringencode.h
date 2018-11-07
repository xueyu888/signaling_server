/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SIGNALING_SERVER_BASE_STRINGENCODE_H_
#define SIGNALING_SERVER_BASE_STRINGENCODE_H_

#include <string>

namespace base {
//////////////////////////////////////////////////////////////////////
// String Encoding Utilities
//////////////////////////////////////////////////////////////////////

// Note: in-place decoding (buffer == source) is allowed.
size_t url_decode(char* buffer,
                  size_t buflen,
                  const char* source,
                  size_t srclen);

std::string hex_encode(const char* source, size_t srclen);

typedef size_t (*Transform)(char* buffer,
                            size_t buflen,
                            const char* source,
                            size_t srclen);

size_t transform(std::string& value,
                 size_t maxlen,
                 const std::string& source,
                 Transform t);

// Return the result of applying transform t to source.
std::string s_transform(const std::string& source, Transform t);

// Convenience wrappers.
inline std::string s_url_decode(const std::string& source) {
  return s_transform(source, url_decode);
}

} // namespace base
 
#endif  // SIGNALING_SERVER_BASE_STRINGENCODE_H_
