#include <cstdlib>
#include <cstdio>

#include "checker.h"

int main(int argc, char* argv[])
{
  (void) argc;
  (void) argv;

  typedef lcp::BasicChecker<int, int> Checker;

  try
  {
    int A = 1;
    int B = 2;
    int C = 3;
//    int X1 = 11;
//    int X2 = 12;
//    int X3 = 13;

    int t1 = 1;
    int t2 = 1;
    int t3 = 3;

    Checker c;

    c.OnMutexLock(A, t1);
    c.OnMutexLock(B, t1);
    c.OnMutexUnlock(B, t1);
    c.OnMutexUnlock(A, t1);

    c.OnMutexLock(B, t2);
    c.OnMutexLock(C, t2);
    c.OnMutexUnlock(C, t2);
    c.OnMutexUnlock(B, t2);

    c.OnMutexLock(C, t3);
    c.OnMutexLock(A, t3);
    c.OnMutexUnlock(A, t3);
    c.OnMutexUnlock(C, t3);
}
  catch(const std::exception& e)
  {
    printf("%s\n", e.what());
  }

  return EXIT_SUCCESS;
}
