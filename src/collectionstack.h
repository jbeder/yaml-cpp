#ifndef COLLECTIONSTACK_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define COLLECTIONSTACK_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <cassert>
#include <stack>

namespace YAML {
enum class CollectionType { NoCollection, BlockMap, BlockSeq, FlowMap, FlowSeq, CompactMap };

class CollectionStack {
 public:
  CollectionStack() : collectionStack{} {}
  CollectionType GetCurCollectionType() const {
    if (collectionStack.empty())
      return CollectionType::NoCollection;
    return collectionStack.top();
  }

  void PushCollectionType(CollectionType type) {
    collectionStack.push(type);
  }
  void PopCollectionType(CollectionType type) {
    assert(type == GetCurCollectionType());
    (void)type;
    collectionStack.pop();
  }

 private:
  std::stack<CollectionType> collectionStack;
};
}  // namespace YAML

#endif  // COLLECTIONSTACK_H_62B23520_7C8E_11DE_8A39_0800200C9A66
