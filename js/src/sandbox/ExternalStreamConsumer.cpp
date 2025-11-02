/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/sandbox/StreamConsumer.h"

namespace js {
namespace sandbox {

JS_PUBLIC_API bool StreamConsumer_consumeChunk(JS::StreamConsumer* consumer, const uint8_t* begin,
                                 size_t length) {
  return consumer->consumeChunk(begin, length);
}

JS_PUBLIC_API void StreamConsumer_streamEnd(JS::StreamConsumer* consumer,
                              JS::OptimizedEncodingListener* listener) {
  return consumer->streamEnd(listener);
}

JS_PUBLIC_API void StreamConsumer_streamError(JS::StreamConsumer* consumer, size_t errorCode) {
  return consumer->streamError(errorCode);
}

JS_PUBLIC_API void StreamConsumer_consumeOptimizedEncoding(JS::StreamConsumer* consumer,
                                             const uint8_t* begin,
                                             size_t length) {
  return consumer->consumeOptimizedEncoding(begin, length);
}

JS_PUBLIC_API void StreamConsumer_noteResponseURLs(JS::StreamConsumer* consumer,
                                     const char* maybeUrl,
                                     const char* maybeSourceMapUrl) {
  return consumer->noteResponseURLs(maybeUrl, maybeSourceMapUrl);
}

}
}
