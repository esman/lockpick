#ifndef CHECKER_H_
#define CHECKER_H_

#include <map>
#include <set>
#include <vector>

#include "exception.h"

namespace lcp
{

template <typename _MutexType, typename _ThreadType>
class BasicTree
{
public:
  typedef BasicTree<_MutexType, _ThreadType> Tree;
  typedef BasicMutex<_MutexType, _ThreadType> Mutex;
  explicit BasicTree(const Mutex*);
  void AddBranch(const Tree& branch) { branches_.push_back(branch); }

private:
  const Mutex* mutex_;
  std::vector<Tree> branches_;
};

template <typename _MutexType, typename _ThreadType>
class BasicMutex
{
public:
  typedef BasicMutex<_MutexType, _ThreadType> Mutex;
  typedef BasicDeadlock<_MutexType, _ThreadType> Deadlock;
  void SetValue(_MutexType value) { value_ = value; }
  _MutexType Value() const { return value_; }
  void OnLock(_ThreadType, Mutex*);
  Mutex* OnUnlock(_ThreadType);

private:
  void Link(Mutex*);
  void CheckTree(const Mutex* mutex) const;

  _MutexType value_ {};
  bool locked_ {};
  bool recursive_ {};
  int lock_count_ {};
  _ThreadType owner_;
  Mutex* prev_ {};
  Mutex* next_ {};

  std::map<_ThreadType, std::set<Mutex*>> tree_;
};

template <typename _MutexType, typename _ThreadType>
class BasicChecker
{
public:
  typedef BasicMutex<_MutexType, _ThreadType> Mutex;
  typedef BasicDeadlock<_MutexType, _ThreadType> Deadlock;
  void OnMutexLock(const _MutexType, const _ThreadType tid);
  void OnMutexUnlock(const _MutexType, const _ThreadType tid);

private:
  std::map<_MutexType, Mutex> mutexes_;
  std::map<_ThreadType, Mutex*> heads_;
};

template <typename _MutexType, typename _ThreadType>
void BasicMutex<_MutexType, _ThreadType>::OnLock(_ThreadType tid, Mutex* head)
{
  if(!locked_)
  {
    locked_ = true;
    owner_ = tid;
  }
  else
  {
    if(tid == owner_)
    {
      if(!recursive_)
      {
        recursive_ = true;
        printf("[WARN] recursive mutex %p\n", this);
        //warn on recursion
      }
      lock_count_++;
    }
    else
    {
      printf("[ERROR] locking already locked mutex %p\n", this);
      //error locking already locked mutex
    }
  }

  if(head)
  {
    head->Link(this);
    CheckTree(head);
  }
}

template <typename _MutexType, typename _ThreadType>
BasicMutex<_MutexType, _ThreadType>* BasicMutex<_MutexType, _ThreadType>::OnUnlock(_ThreadType tid)
{
  if(locked_)
  {
    if(tid == owner_)
    {
      if(!recursive_ || --lock_count_ == 0)
      {
        locked_ = false;
      }
    }
    else
    {
      printf("[ERROR] unlocking from not owning thread. mutex %p\n", this);
      //error unlocking from not owning thread
    }
  }
  else
  {
    printf("[ERROR] unlocking not locked mutex %p\n", this);
    //error unlocking not locked mutex
  }

  if(prev_)
  {
    prev_->Link(next_);
  }
  else if(next_)
  {
    next_->prev_ = nullptr;
  }

  return prev_;
}

template <typename _MutexType, typename _ThreadType>
void BasicMutex<_MutexType, _ThreadType>::Link(Mutex* mutex)
{
  next_ = mutex;
  if(next_)
  {
    printf("link %d to %d. context: %u\n", mutex->value_, value_, owner_);
    tree_[owner_].insert(next_);
    next_->prev_ = this;
  }
}

template <typename _MutexType, typename _ThreadType>
void BasicMutex<_MutexType, _ThreadType>::CheckTree(const Mutex* mutex) const
{
  for(const auto p: tree_)
  {
    if(p.first != mutex->owner_)
    {
      for(const auto m: p.second)
      {
        printf("check mutex %d:%d == %d:%d\n", m->value_, p.first, mutex->value_, mutex->owner_);
        if(m != mutex)
        {
          m->CheckTree(mutex);
        }
        else
        {
          throw Deadlock(this, mutex);
        }
      }
    }
  }
}

template <typename _MutexType, typename _ThreadType>
void BasicChecker<_MutexType, _ThreadType>::OnMutexLock(const _MutexType mutex, const _ThreadType tid)
{
  auto& m = mutexes_[mutex];
  m.SetValue(mutex);
  auto pair = heads_.find(tid);
  Mutex* head {};

  if(pair != std::end(heads_))
  {
    head = pair->second;
    pair->second = &m;
  }
  else
  {
    heads_[tid] = &m;
  }

  m.OnLock(tid, head);
}

template <typename _MutexType, typename _ThreadType>
void BasicChecker<_MutexType, _ThreadType>::OnMutexUnlock(const _MutexType mutex, const _ThreadType tid)
{
  auto& m = mutexes_.at(mutex);
  if(heads_[tid] == &m)
  {
    heads_[tid] = m.OnUnlock(tid);
  }
  else
  {
    m.OnUnlock(tid);
  }
}

}  /* namespace lcp */

#endif /* CHECKER_H_ */
