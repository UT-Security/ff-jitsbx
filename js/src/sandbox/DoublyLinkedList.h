/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef sandbox_DoublyLinkedList_h
#define sandbox_DoublyLinkedList_h

namespace js {
namespace sandbox {

template <typename T>
class DoublyLinkedListElement {
private:
  T* next_;
  T* prev_;

public:
  DoublyLinkedListElement() : next_(nullptr), prev_(nullptr) {}

  T* next() { return next_; }
  T* prev() { return prev_; }

  void setNext(T* next) {
    next_ = next;
  }

  void setPrev(T* prev) {
    prev_ = prev;
  }
};

template <typename T>
class DoublyLinkedList {

private:
  T* head_;

public:
  DoublyLinkedList() : head_(nullptr) {}

  T* head() { return head_; }
  void setHead(T* head) { head_ = head; }
  
};
 
}
}

#endif
