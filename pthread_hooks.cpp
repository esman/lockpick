#include <pthread.h>
#include <stdio.h>
#include <dlfcn.h>
#include <cstring>

#include "connection.h"
#include "checker.h"

namespace lcp
{

template <size_t _Size>
class Transaction final
{
public:
  ~Transaction()
  {
    Connection::getInstance()->Send(buff_, size_);
  }

  void Send(const void* buff, size_t size)
  {
    ::memcpy(buff_ + size_, buff, size);
    size_ += size;
  }

  template <typename T>
  void Send(const T& msg)
  {
    Send(&msg, sizeof(T));
  }

private:
  char buff_[_Size];
  size_t size_ {};
};

extern "C" int pthread_mutex_lock(pthread_mutex_t* mutex)
{
  static int (*func)(pthread_mutex_t*);

  if(!func)
  {
    func = reinterpret_cast<decltype(func)>(::dlsym(RTLD_NEXT, "pthread_mutex_lock"));
  }

//  printf("lock %p\n", mutex);

  return func(mutex);
}

extern "C" int pthread_mutex_unlock(pthread_mutex_t* mutex)
{
  static int (*func)(pthread_mutex_t*);

  if(!func)
  {
    func = reinterpret_cast<decltype(func)>(::dlsym(RTLD_NEXT, "pthread_mutex_unlock"));
  }

//  printf("unlock %p\n", mutex);

  return func(mutex);
}

} /* namespace lcp */
