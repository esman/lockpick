#ifndef TREE_H_
#define TREE_H_

#include <cassert>
#include <map>
#include "utils.h"

namespace lcp
{

template <typename Mutex>
class BasicTree
{
public:
  typedef BasicTree<Mutex> Tree;
  BasicTree() = default;
  ~BasicTree();

  BasicTree(const Tree&) = delete;
  const Tree& operator=(const Tree&) = delete;

  Tree* OnMutexLock(Mutex* mutex);
  Tree* OnMutexUnlock(const Mutex* mutex);

private:
  typedef LinkedList<Mutex*> MutexList;
  BasicTree(Tree* parent, Mutex* mutex);
  BasicTree(Tree* parent, const MutexList* list);
  Tree* FindNode(MutexList* list, const Mutex* mutex);
  Tree* Branch(MutexList* list);

  Tree* parent_ {};
  Mutex* mutex_ {};
  std::map<const Mutex*, Tree*> nodes_;
};

template <typename Mutex>
BasicTree<Mutex>::BasicTree(Tree* parent, Mutex* mutex):
  parent_{parent},
  mutex_{mutex}
{}

template <typename Mutex>
BasicTree<Mutex>::BasicTree(Tree* parent, const MutexList* list):
  BasicTree{parent, list->data}
{
  list = list->next;
  if(list)
  {
    nodes_[list->data] = new Tree(this, list);
  }
}

template <typename Mutex>
BasicTree<Mutex>::~BasicTree()
{
  for(auto pair: nodes_)
  {
    delete pair.second;
  }
}

template <typename Mutex>
BasicTree<Mutex>* BasicTree<Mutex>::OnMutexLock(Mutex* mutex)
{
  Tree* node;

  if(mutex_)
  {
    mutex_->Link(mutex);
  }

  auto node_pair = nodes_.find(mutex);
  if(node_pair != std::end(nodes_))
  {
    node = node_pair->second;
  }
  else
  {
    node = new Tree(this, mutex);
    nodes_[mutex] = node;
  }

  return node;
}

template <typename Mutex>
BasicTree<Mutex>* BasicTree<Mutex>::OnMutexUnlock(const Mutex* mutex)
{
  Tree* node;

  assert(mutex_ && parent_);

  if(mutex == mutex_)
  {
    node = parent_;
  }
  else
  {
    MutexList list {mutex_, nullptr};
    node = parent_->FindNode(&list, mutex);
  }

  return node;
}

template <typename Mutex>
BasicTree<Mutex>* BasicTree<Mutex>::FindNode(MutexList* next, const Mutex* mutex)
{
  Tree* node;

  assert(mutex_ && parent_);

  if(mutex == mutex_)
  {
    node = parent_->Branch(next);
  }
  else
  {
    MutexList list {mutex_, next};
    node = parent_->FindNode(&list, mutex);
  }

  return node;
}

template <typename Mutex>
BasicTree<Mutex>* BasicTree<Mutex>::Branch(MutexList* list)
{
  Tree* node;

  auto node_pair = nodes_.find(list->data);
  if(node_pair == std::end(nodes_))
  {
    node = new Tree(this, list);
    nodes_[list->data] = node;
  }
  else if(list->next)
  {
    node = node_pair->second->Branch(list->next);
  }
  else
  {
    node = this;
  }

  return node;
}

}  /* namespace lcp */

#endif /* TREE_H_ */
