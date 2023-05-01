#include "thread_pool.hpp"

#include <iostream>
#include <unistd.h>

void hello() {
  std::cout << "Hello, Thread Pool!" << std::endl;
}

int main() {
  ThreadPool pool(4);
  pool.submit(hello);
  sleep(2);
  return 0;
}