#include <iostream>
#include <set>
#include <time.h>

#ifndef TEST_SIZE
#define TEST_SIZE 1000000
#endif

int main()
{
  std::set<int> s;
  int array[TEST_SIZE];
  double elapsed;
  struct timespec begin, finish;

  // Set up array data
  for (int i = 0; i < TEST_SIZE; ++i)
  {
    array[i] = i;
  }

  clock_gettime(CLOCK_MONOTONIC, &begin);
  s.insert(array, array+TEST_SIZE);
  clock_gettime(CLOCK_MONOTONIC, &finish);
  elapsed = (finish.tv_sec - begin.tv_sec);
  elapsed += (finish.tv_nsec - begin.tv_nsec) / 1000000000.0;
  std::cout << elapsed << std::endl;
}