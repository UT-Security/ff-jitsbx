/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef sandbox_RBTree_h
#define sandbox_RBTree_h

#include <stdint.h>

namespace js {
namespace sandbox {

template <class T, class Trait>
class RBTree;

enum Color { Black = 0, Red = 1 };
  
template <typename T>
class RBTreeNode {
  T* left_;
  T* right_;
  T* parentAndColor_;

 public:

  RBTreeNode() : left_(nullptr), right_(nullptr), parentAndColor_(nullptr) {}

  T* left() { return left_; }
  T** leftLink() { return &left_; }
  void setLeft(T* node) { left_ = node; }

  T* right() { return right_; }
  T** rightLink() { return &right_; }
  void setRight(T* node) { right_ = node; }

  inline static T* parent(uintptr_t parentAndColor) {
    return reinterpret_cast<T*>(parentAndColor & uintptr_t(~1));
  }

  inline static Color color(uintptr_t parentAndColor) {
    return static_cast<Color>(parentAndColor &
                              uintptr_t(1));
  }

  T* parent() {
    return RBTreeNode<T>::parent(reinterpret_cast<uintptr_t>(parentAndColor_));
  }
  
  void setParent(T* node) {
    parentAndColor_ = reinterpret_cast<T*>(
        (reinterpret_cast<uintptr_t>(parentAndColor_) & uintptr_t(~1)) |
        color());
  }

  Color color() {
    return RBTreeNode<T>::color(reinterpret_cast<uintptr_t>(parentAndColor_));
  }
  void setColor(Color c) {
    parentAndColor_ = reinterpret_cast<T*>(
        (reinterpret_cast<uintptr_t>(parentAndColor_) & uintptr_t(~1)) | c);
  }

  uintptr_t parentAndColor() {
    return reinterpret_cast<uintptr_t>(parentAndColor_);
  }

  void setParentAndColor(uintptr_t parentAndColor) {
    parentAndColor_ = reinterpret_cast<T*>(parentAndColor);
  }

  void setParentAndColor(T* node, Color c) {
    parentAndColor_ = reinterpret_cast<T*>(
        (reinterpret_cast<uintptr_t>(parentAndColor_) & uintptr_t(~1)) | c);
  }

  bool isRed() { return color() == Color::Red; }
  bool isBlack() { return color() == Color::Black; }
};

/*
 * red-black trees properties:  https://en.wikipedia.org/wiki/Rbtree
 *
 *  1) A node is either red or black
 *  2) The root is black
 *  3) All leaves (NULL) are black
 *  4) Both children of every red node are black
 *  5) Every simple path from root to leaves contains the same number
 *     of black nodes.
 */
template <class T, class Trait>
class RBTree {
  T* root_;

public:
  constexpr RBTree() : root_(nullptr) {}

  bool isEmpty() const { return root_ == nullptr; }  

  T* root() { return root_; }
  T** rootLink() { return &root_; }

  T* last() {
    T* node = root_;

    if (!node) {
      return nullptr;
    }

    while (node->right()) {
      node = node->right();
    }
    return node;
  }

  void link(T* node, T* parent, T** link) {
    node->setParentAndColor(parent, Black);
    node->setLeft(nullptr);
    node->setRight(nullptr);

    *link = node;
  }

  T* redParent(T* red) {
    return reinterpret_cast<T*>(red->parentAndColor());
  }

  void changeChild(T* oldN, T* newN, T* parent) {
    if (parent) {
      if (parent->left() == oldN) {
        parent->setLeft(newN);
      } else {
        parent->setRight(newN);
      }
    } else {
      root_ = newN;
    }
  }

  void rotateSetParents(T* oldN, T* newN, Color color) {
    T* parent = oldN->parent();
    newN->setParentAndColor(oldN->parent(), oldN->color());
    oldN->setParentAndColor(newN, color);
    changeChild(oldN, newN, parent);    
  }

  void insert(T* node) {
    T* parent = redParent(node), *grandParent, *temp; 

    while (true) {
      if (!parent) {
        node->setParentAndColor(nullptr, Color::Black);
        break;
      }

      if (parent->isBlack()) {
        break;
      }

      // we know grandParent is non-null since parent
      // is red and hence can't be the root.
      grandParent = redParent(parent);

      temp = grandParent->right();
      if (parent != temp) { // parent == grandParent->left()
        if (temp && temp->isRed()) {
          temp->setColor(Color::Black);
          parent->setColor(Color::Black);
          node = grandParent;
          parent = node->parent();
          node->setColor(Color::Red);
          continue;
        }

        temp = parent->right();
        if (node == temp) {
          temp = node->left();
          parent->setRight(temp);
          node->setLeft(parent);
          if (temp) {
            temp->setParentAndColor(parent, Color::Black);
          }
          parent->setParentAndColor(node, Color::Red);
          Trait::rotate(parent, node);
          parent = node;
          temp = node->right();
        }

        grandParent->setLeft(temp);
        parent->setRight(grandParent);
        if (temp) {
          temp->setParentAndColor(grandParent, Color::Black);
        }
        rotateSetParents(grandParent, parent, Color::Red);
        Trait::rotate(grandParent, parent);
        break;
      } else {
        temp = grandParent->left();
        if (temp && temp->isRed()) {
          temp->setColor(Color::Black);
          parent->setColor(Color::Black);
          node = grandParent;
          parent = grandParent->parent();
          node->setColor(Color::Red);
          continue;
        }

        temp = parent->left();
        if (node == temp) {
          temp = node->right();
          parent->setLeft(temp);
          node->setRight(parent);
          if (temp) {
            temp->setParentAndColor(parent, Color::Black);
          }
          parent->setParentAndColor(node, Color::Red);
          Trait::rotate(parent, node);
          parent = node;
          temp = node->left();
        }

        grandParent->setRight(temp);
        parent->setLeft(grandParent);
        if (temp) {
          temp->setParentAndColor(grandParent, Color::Black);
        }
        rotateSetParents(grandParent, parent, Color::Red);
        Trait::rotate(grandParent, parent);
        break;
      }
    }
  }

  void erase(T* node) {
    T* child = node->right();
    T* temp = node->left();
    T* parent, *rebalance;
    uintptr_t pc;
    
    if (!temp) {
      pc = node->parentAndColor();
      parent = RBTreeNode<T>::parent(pc);     
      changeChild(node, child, parent);
      if (child) {
        child->setParentAndColor(pc);
        rebalance = nullptr;
      } else {
        rebalance = RBTreeNode<T>::color(pc) == Color::Black ? parent : nullptr;
      }

      temp = parent;
    } else if (!child) {
      temp->setParentAndColor(node->parentAndColor());
      pc = node->parentAndColor();
      parent = RBTreeNode<T>::parent(pc);
      changeChild(node, temp, parent);
      rebalance = nullptr;
      temp = parent;
    } else {
      T* successor = child, *child2;

      temp = child->left();
      if (!temp) {
        parent = successor;
        child2 = successor->right();

        Trait::copy(node, successor);
      } else {
        do {
          parent = successor;
          successor = temp;
          temp =  temp->left();
        } while (temp);

        child2 = successor->right();
        parent->setLeft(child2);
        successor->setRight(child);
        child->setParent(successor);

        Trait::copy(node, successor);
        Trait::propagate(parent, successor);
      }

      temp = node->left();
      successor->setLeft(temp);
      temp->setParent(successor);

      pc = node->parentAndColor();
      temp = RBTreeNode<T>::parent(pc);
      changeChild(node, successor, temp);

      if (child2) {
        child->setParentAndColor(parent, Color::Black);
        rebalance = nullptr;
      } else {
        rebalance = successor->isBlack() ? parent : nullptr;
      }

      successor->setParentAndColor(pc);
      temp = successor;
    }

    Trait::propagate(temp, nullptr);

    if (rebalance) {
      eraseColor(rebalance);
    }
  }

  inline void eraseColor(T* parent) {
    T* node = nullptr, *sibling, *temp1, *temp2;

    while (true) {
      sibling = parent->right();
      if (node != sibling) {
        if (sibling->isRed()) {
          temp1 = sibling->left();
          parent->setRight(temp1);
          sibling->setLeft(parent);
          temp1->setParentAndColor(parent, Color::Black);
          rotateSetParents(parent, sibling, Color::Red);
          Trait::rotate(parent, sibling);
          sibling = temp1;
        }
        temp1 = sibling->right();
        if (!temp1 || temp1->isBlack()) {
          temp2 = sibling->left();
          if (!temp2 || temp2->isBlack()) {
            sibling->setParentAndColor(parent, Color::Red);
            if (parent->isRed()) {
              parent->setColor(Color::Black);
            } else {
              node = parent;
              parent = node->parent();
              if (parent) {
                continue;
              }
            }
            break;
          }

          temp1 = temp2->right();
          sibling->setLeft(temp1);
          temp2->setRight(sibling);
          parent->setRight(temp2);
          if (temp1) {
            temp1->setParentAndColor(sibling, Color::Black);
          }
          Trait::rotate(sibling, temp2);
          temp1 = sibling;
          sibling = temp2;
        }

        temp2 = sibling->left();
        parent->setRight(temp2);
        sibling->setLeft(parent);
        temp1->setParentAndColor(sibling, Color::Black);
        if (temp2) {
          temp2->setParent(parent);
        }
        rotateSetParents(parent, sibling, Color::Black);
        Trait::rotate(parent, sibling);
        break;
      } else {
        sibling = parent->left();
        if (sibling->isRed()) {
          temp1 = sibling->right();
          parent->setLeft(temp1);
          sibling->setRight(parent);
          temp1->setParentAndColor(parent, Color::Black);
          rotateSetParents(parent, sibling, Color::Red);
          Trait::rotate(parent, sibling);
          sibling = temp1;
        }
        temp1 = sibling->left();
        if (!temp1 || temp1->isBlack()) {
          temp2 = sibling->right();
          if (!temp2 || temp2->isBlack()) {
            sibling->setParentAndColor(parent, Color::Red);
            if (parent->isRed()) {
              parent->setColor(Color::Black);
            } else {
              node = parent;
              parent = node->parent();
              if (parent) {
                continue;
              }
            }
            break;
          }
          temp1 = temp2->left();
          sibling->setRight(temp1);
          temp2->setLeft(sibling);
          parent->setLeft(temp2);
          if (temp1) {
            temp1->setParentAndColor(sibling, Color::Black);
          }
          Trait::rotate(sibling, temp2);
          temp1 = sibling;
          sibling = temp2;
        }
        temp2 = sibling->right();
        parent->setLeft(temp2);
        sibling->setRight(parent);
        temp1->setParentAndColor(sibling, Color::Black);
        if (temp2) {
          temp2->setParent(parent);
        }
        rotateSetParents(parent, sibling, Color::Black);
        Trait::rotate(parent, sibling);
        break;
      }
    }
  }
};

}  // namespace sandbox
} // namespace js

#endif
