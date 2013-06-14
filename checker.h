#ifndef CHECKER_H_
#define CHECKER_H_

#include <vector>

#include "tree.h"
#include "mutex.h"

namespace lcp
{

template <typename MutexID, typename ThreadID>
class BasicChecker
{
public:
  ~BasicChecker();
  typedef BasicMutex<MutexID> Mutex;
  typedef BasicTree<Mutex> Tree;
  void OnMutexLock(const MutexID, const ThreadID tid);
  void OnMutexUnlock(const MutexID, const ThreadID tid);

private:
  std::map<MutexID, Mutex*> mutexes_;
  std::vector<Tree*> trees_;
  std::map<ThreadID, Tree*> heads_;
};

template <typename MutexID, typename ThreadID>
BasicChecker<MutexID, ThreadID>::~BasicChecker()
{
  for(auto pair: mutexes_)
  {
    delete pair.second;
  }

  for(auto tree: trees_)
  {
    delete tree;
  }
}

template <typename MutexID, typename ThreadID>
void BasicChecker<MutexID, ThreadID>::OnMutexLock(const MutexID mutex_id, const ThreadID thread_id)
{
  auto mutex = mutexes_[mutex_id];
  if(mutex == nullptr)
  {
    mutex = new Mutex(mutex_id);
    mutexes_[mutex_id] = mutex;
  }

  auto tree = heads_[thread_id];
  if(tree == nullptr)
  {
    tree = new Tree;
    trees_.push_back(tree);
  }

  heads_[thread_id] = tree->OnMutexLock(mutex);
}

template <typename MutexID, typename ThreadID>
void BasicChecker<MutexID, ThreadID>::OnMutexUnlock(const MutexID mutex_id, const ThreadID thread_id)
{
  auto tree = heads_[thread_id];
  if(tree)
  {
    auto mutex = mutexes_[mutex_id];
    if(mutex)
    {
      heads_[thread_id] = tree->OnMutexUnlock(mutex);
    }
    else
    {
      printf("[ERROR] Unlocking unknown mutex %d\n", mutex_id);
    }
  }
  else
  {
    printf("[ERROR] Thread %d has no locked mutexes\n", thread_id);
  }
}

}  /* namespace lcp */

#endif /* CHECKER_H_ */
