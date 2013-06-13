#ifndef _LCP_CONNECTION_H_
#define _LCP_CONNECTION_H_

namespace lcp
{

class Connection
{
public:
  static Connection* getInstance(void);
  virtual ~Connection();
  virtual void Send(const void* buff, size_t size) const = 0;
};

}  /* namespace lcp */

#endif /* _LCP_CONNECTION_H_ */
