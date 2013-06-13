#include <cstdio>

#include "connection.h"

namespace lcp
{

class LocalConnection: public Connection
{
public:
  void Send(const void* buff, size_t size) const override
  {
    ::printf("Send buff %p of size %lu\n", buff, size);
  }

  ~LocalConnection() override {}
};

static LocalConnection instance;

Connection::~Connection() {}

Connection* Connection::getInstance(void)
{
  return &instance;
}

}  /* namespace lcp */
