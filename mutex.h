#ifndef MUTEX_H_
#define MUTEX_H_

#include <set>
#include "utils.h"

namespace lcp
{

template <typename MutexID>
class BasicMutex
{
public:
  typedef BasicMutex<MutexID> Mutex;
  explicit BasicMutex(MutexID id);
  BasicMutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  void Link(Mutex*);

private:
  typedef LinkedList<const Mutex*> MutexList;
  void FindLoop(const Mutex* mutex, MutexList* list) const;

  template <typename Mutex>
  friend void PrintList(LinkedList<Mutex*>* list);

  MutexID id_;
  std::set<Mutex*> refs_;
};

template <typename MutexID>
BasicMutex<MutexID>::BasicMutex(MutexID id):
  id_{id}
{}

template <typename MutexID>
void BasicMutex<MutexID>::Link(Mutex* mutex)
{
  if(refs_.find(mutex) == std::end(refs_))
  {
    refs_.insert(mutex);
    MutexList list {this, nullptr};
    mutex->FindLoop(this, &list);
  }
}

template <typename MutexID>
void BasicMutex<MutexID>::FindLoop(const Mutex* mutex, MutexList* next) const
{
  MutexList list {this, next};
  for(const auto ref: refs_)
  {
    if(ref == mutex)
    {
      printf("!!! Found loop: ");
      PrintList(&list);
      printf("%d\n", mutex->id_);
    }
    else
    {
      ref->FindLoop(mutex, &list);
    }
  }
}

template <typename Mutex>
void PrintList(LinkedList<Mutex*>* list)
{
  if(list->next)
  {
    PrintList(list->next);
  }
  printf("%d - ", list->data->id_);
}

}  /* namespace lcp */

#endif /* MUTEX_H_ */
