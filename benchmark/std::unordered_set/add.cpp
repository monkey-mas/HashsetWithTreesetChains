#include <iostream>
#include <set>
#include <time.h>
#include <unordered_set>

#ifndef TEST_SIZE
#define TEST_SIZE 1000000
#endif

int main()
{
  std::unordered_set<int> s;
  double elapsed;
  struct timespec begin, finish;

  clock_gettime(CLOCK_MONOTONIC, &begin);
  for (int i = 0; i < TEST_SIZE; ++i)
  {
    s.insert(i);
  }
  clock_gettime(CLOCK_MONOTONIC, &finish);
  elapsed = (finish.tv_sec - begin.tv_sec);
  elapsed += (finish.tv_nsec - begin.tv_nsec) / 1000000000.0;
  std::cout << elapsed << std::endl;
}