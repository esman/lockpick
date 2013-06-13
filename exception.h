#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <string>
#include <sstream>
#include <stdexcept>

namespace lcp
{

template <typename _MutexType, typename _ThreadType>
class BasicMutex;

class LockError: public std::exception
{
public:
  const char* what() const throw() override { return what_.c_str(); }

protected:
  std::string what_;
};

template <typename _MutexType, typename _ThreadType>
class BasicDeadlock: public LockError
{
public:
  typedef BasicMutex<_MutexType, _ThreadType> Mutex;
  BasicDeadlock(const Mutex* mutex1, const Mutex* mutex2);
  const Mutex* getMutex1() const;
  const Mutex* getMutex2() const;

private:
  const Mutex* first_;
  const Mutex* second_;
};

template <typename _MutexType, typename _ThreadType>
BasicDeadlock<_MutexType, _ThreadType>::BasicDeadlock(const Mutex* mutex1, const Mutex* mutex2):
  first_(mutex1),
  second_(mutex2)
{
  std::stringstream strm;
  strm << "Possible deadlock found: [" << first_->Value() << "] - [" << second_->Value() << ']';
  what_ = strm.str();
}

}  /* namespace lcp */

#endif /* EXCEPTION_H_ */
