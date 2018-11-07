/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SIGNALING_SERVER_BASE_STRINGUTILS_H_
#define SIGNALING_SERVER_BASE_STRINGUTILS_H_

#include <ctype.h>

#ifdef _WIN32
#include <malloc.h>
#include <windows.h>
#define alloca _alloca
#else
#ifdef BSD
#include <stdlib.h>
#else  // BSD
#include <alloca.h>
#endif  // !BSD
#endif  // WEBRTC_POSIX
namespace base {
///////////////////////////////////////////////////////////////////////////////
// Generic string/memory utilities
///////////////////////////////////////////////////////////////////////////////

#define STACK_ARRAY(TYPE, LEN) \
  static_cast<TYPE*>(::alloca((LEN) * sizeof(TYPE)))

}
#endif  // SIGNALING_SERVER_BASE_STRINGUTILS_H_
