#include "forkjoin.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <atomic>
#include <chrono>
#include <memory>
#include "tbb/tbb.h"

namespace {
using std::vector;
using std::string;
using std::numeric_limits;
using std::cout;
using std::endl;
using std::to_string;
using std::atomic;
using std::atomic_fetch_add;
using std::unique_ptr;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using tbb::parallel_for;
using tbb::blocked_range;
using tbb::parallel_do;
using tbb::parallel_do_feeder;

template <typename T>
void print_array(T* array, const size_t size) {
  string outstr = "{";
  for(int i = 0, end = size; i != end; ++i)
    outstr += to_string(array[i]) + ", ";
  outstr.erase(outstr.size() - 2);
  outstr += "}";
  cout << outstr << endl;
}

template <typename T>
T* median_of_three(T* x, T* y, T* z) {
  // I copied this from the book, I didn't write this madness. I just didn't feel like fixing it.
  return *x < *y ? *y < *z ? y : *x < *z ? z : x : *z < *y ? y : *z < *x ? z : x;
}

template <typename T>
T* choose_partition_key(T* first, T* last) {
  const size_t offset = (last - first) / 8;
  return median_of_three(
    median_of_three(first, first + offset, first + offset * 2),
    median_of_three(first + offset * 3, first + offset * 4, last - (3 * offset + 1)),
    median_of_three(last - (2 * offset + 1), last - (offset + 1), last - 1));
}

template <typename T>
T* divide(T* first, T* last) {
  std::swap(*first, *choose_partition_key(first, last));
  T key = *first;
  T* middle = std::partition(first + 1, last, [key](const T& x) {return x < key;}) - 1;
  if(middle != first) {
    // move partition key to between the partitions
    std::swap(*first, *middle);
  } else {
    // check if keys are equal
    if(last == std::find_if(first + 1, last, [key](const T& x) {return key < x;}))
      return NULL;
  }
  return middle;
}

// Appears in Structured Parallel Programming, published by Elsevier, Inc.
template <typename T>
void quicksort(T* first, T* last) {
  const ptrdiff_t cutoff = 500;
  tbb::task_group g;
  while(last - first > cutoff) {
    T* middle = divide(first, last);
    if(!middle) {
      g.wait();
      return;
    }

    // spawn the smaller problem
    if(middle - first < last - (middle + 1)) {
      g.run([first, middle]{quicksort(first, middle);});
      first = middle + 1; // solve right subproblem in the next iteration
    } else {
      g.run([last, middle]{quicksort(middle + 1, last);});
      last = middle; // solve right subproblem in the next iteration
    }
  }
  std::sort(first, last); // use serial sort for small arrays
  g.wait();
}

} // namespace

namespace patterns {
void forkjoin() {
  srand(time(NULL));

  const int size = 10000000;
  typedef int sort_t;

  cout << "fork-join pattern for parallel quicksort" << endl;
  cout << "---------------------------------------------" << endl;
  cout << "sorting " << size << "x array" << endl;

  unique_ptr<sort_t[]> input(new sort_t[size]);
  for(size_t i = 0, end = size; i != end; ++i)
    input[i] = rand();

  unique_ptr<sort_t[]> input_serial(new sort_t[size]);
  std::copy(input.get(), input.get() + size, input_serial.get());

  {
    const auto start = high_resolution_clock::now();
    quicksort(input.get(), input.get() + size);
    const auto end = high_resolution_clock::now();
    const auto duration = end - start;
    const auto ms = duration_cast<milliseconds>(duration).count();
    cout << "parallel sorted in " << ms << "ms" << endl;
  }
  {
    const auto start = high_resolution_clock::now();
    std::sort(input_serial.get(), input_serial.get() + size);
    const auto end = high_resolution_clock::now();
    const auto duration = end - start;
    const auto ms = duration_cast<milliseconds>(duration).count();
    cout << "serial sorted in " << ms << "ms" << endl;
  }
}
} // namespace patterns
