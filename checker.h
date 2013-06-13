#ifndef CHECKER_H_
#define CHECKER_H_

#include <map>
#include <set>
#include <vector>

#include "exception.h"

namespace lcp
{

template <typename MutexID, typename ThreadID>
class BasicTree
{
public:
  typedef BasicTree<MutexID, ThreadID> Tree;
  typedef BasicMutex<MutexID, ThreadID> Mutex;
  explicit BasicTree(const Mutex*);
  void AddBranch(const Tree& branch) { branches_.push_back(branch); }

private:
  const Mutex* mutex_;
  std::vector<Tree> branches_;
};

template <typename MutexID, typename ThreadID>
class BasicMutex
{
public:
  typedef BasicMutex<MutexID, ThreadID> Mutex;
  typedef BasicDeadlock<MutexID, ThreadID> Deadlock;
  void SetValue(MutexID value) { id_ = value; }
  MutexID Value() const { return id_; }
  void OnLock(ThreadID, Mutex*);
  Mutex* OnUnlock(ThreadID);

private:
  void Link(Mutex*);
  typedef std::vector<const Mutex*> Loop;
  bool FindLoop(const Mutex* mutex, Loop& loop) const;

  MutexID id_ {};
  bool locked_ {};
  bool recursive_ {};
  int lock_count_ {};
  ThreadID owner_;
  Mutex* prev_ {};
  Mutex* next_ {};

  std::set<Mutex*> refs_;
};

template <typename MutexID, typename ThreadID>
class BasicChecker
{
public:
  typedef BasicMutex<MutexID, ThreadID> Mutex;
  typedef BasicDeadlock<MutexID, ThreadID> Deadlock;
  void OnMutexLock(const MutexID, const ThreadID tid);
  void OnMutexUnlock(const MutexID, const ThreadID tid);

private:
  std::map<MutexID, Mutex> mutexes_;
  std::map<ThreadID, Mutex*> heads_;
};

template <typename MutexID, typename ThreadID>
void BasicMutex<MutexID, ThreadID>::OnLock(ThreadID tid, Mutex* head)
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
    }
    else
    {
      printf("[ERROR] locking already locked mutex %p\n", this);
      //error locking already locked mutex
    }
  }
  lock_count_++;

  if(head)
  {
    head->Link(this);
  }
}

template <typename MutexID, typename ThreadID>
BasicMutex<MutexID, ThreadID>* BasicMutex<MutexID, ThreadID>::OnUnlock(ThreadID tid)
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
  lock_count_--;

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

template <typename MutexID, typename ThreadID>
void BasicMutex<MutexID, ThreadID>::Link(Mutex* mutex)
{
  next_ = mutex;
  if(next_)
  {
    printf("link %d to %d. context: %u\n", mutex->id_, id_, owner_);
    next_->prev_ = this;

    if(refs_.find(next_) == std::end(refs_))
    {
      refs_.insert(next_);

      Loop loop;
      if(next_->FindLoop(this, loop))
      {
        loop.push_back(this);
        printf("!!! Found loop:\n");
        for(auto const ref: loop)
        {
          printf("!!! --> %d\n", ref->id_);
        }
      }
    }
  }
}

template <typename MutexID, typename ThreadID>
bool BasicMutex<MutexID, ThreadID>::FindLoop(const Mutex* mutex, Loop& loop) const
{
  for(const auto ref: refs_)
  {
    if(ref == mutex || ref->FindLoop(mutex, loop))
    {
      loop.push_back(this);
      return true;
    }
  }
  return false;
}

template <typename MutexID, typename ThreadID>
void BasicChecker<MutexID, ThreadID>::OnMutexLock(const MutexID mutex_id, const ThreadID thread_id)
{
  Mutex* mutex;
  Mutex* head {};

  auto mutex_pair = mutexes_.find(mutex_id);
  if(mutex_pair == std::end(mutexes_))
  {
    mutex = &mutexes_[mutex_id];
    mutex->SetValue(mutex_id);
  }
  else
  {
    mutex = &mutex_pair->second;
  }

  auto thread_pair = heads_.find(thread_id);
  if(thread_pair != std::end(heads_))
  {
    head = thread_pair->second;
    thread_pair->second = mutex;
  }
  else
  {
    heads_[thread_id] = mutex;
  }

  mutex->OnLock(thread_id, head);
}

template <typename MutexID, typename ThreadID>
void BasicChecker<MutexID, ThreadID>::OnMutexUnlock(const MutexID mutex_id, const ThreadID thread_id)
{
  auto mutex_pair = mutexes_.find(mutex_id);
  if(mutex_pair != std::end(mutexes_))
  {
    auto mutex = &mutex_pair->second;

    auto head_pair = heads_.find(thread_id);
    if(head_pair != std::end(heads_) && head_pair->second == mutex)
    {
      head_pair->second = mutex->OnUnlock(thread_id);
    }
    else
    {
      mutex->OnUnlock(thread_id);
    }
  }
  else
  {
    printf("[ERROR] Unlocking unknown mutex\n");
  }
}

}  /* namespace lcp */

#endif /* CHECKER_H_ */
