/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_StreamConsumer_h
#define js_sandbox_StreamConsumer_h

#include "js/StreamConsumer.h"

namespace js {
namespace sandbox {

JS_PUBLIC_API bool StreamConsumer_consumeChunk(JS::StreamConsumer* consumer, const uint8_t* begin,
                                 size_t length);

JS_PUBLIC_API void StreamConsumer_streamEnd(JS::StreamConsumer* consumer,
                              JS::OptimizedEncodingListener* listener = nullptr);

JS_PUBLIC_API void StreamConsumer_streamError(JS::StreamConsumer* consumer, size_t errorCode);

JS_PUBLIC_API void StreamConsumer_consumeOptimizedEncoding(JS::StreamConsumer* consumer,
                                             const uint8_t* begin,
                                             size_t length);

JS_PUBLIC_API void StreamConsumer_noteResponseURLs(JS::StreamConsumer* consumer,
                                     const char* maybeUrl,
                                     const char* maybeSourceMapUrl);
}  // namespace sandbox
}  // namespace JS

#endif
