#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

namespace lcp
{

template <typename T>
struct LinkedList
{
  T data;
  LinkedList* next;
};

}  /* namespace lcp */

#endif /* LINKEDLIST_H_ */
