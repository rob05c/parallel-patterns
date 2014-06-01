#include "patterns.h"
#include <iostream>
#include <cstdlib>
#include "tbb/tbb.h"

namespace {
using std::string;
using std::cout;
using std::endl;
using std::to_string;
using tbb::parallel_for;
using tbb::blocked_range;

int add_five(const int num) {
  return num + 5;
}

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
T sum_array(T* array, const size_t len) {
  T sum = 0;
  for(size_t i = 0, end = len; i != end; ++i)
    sum += array[i];
  return sum;
}

/// body for tbb::parallel_scan
/// used by scan()
class Body {
  int result;
  int* const y;
  const int* const z;
public:

  Body(int y_[], const int z_[]) : result(0), z(z_), y(y_) {}
  Body(Body& b) : z(b.z), y(b.y), result(0) {}
  Body(Body& b, tbb::split) : z(b.z), y(b.y), result(0) {}
  void assign(Body& b, tbb::split) {result = b.result;}
  void assign(Body& b) {result = b.result;}
  void reverse_join(Body& a) {result = a.result;}
  int get_result() const {return result;}
  template<typename Tag>
  void operator()(const blocked_range<int>& r, Tag) {
    int temp = result;
    for(int i = r.begin(); i < r.end(); ++i) {
      temp = temp + z[i];
      if(Tag::is_final_scan())
        y[i] = temp;
    }
    result = temp;
  }
};


}

namespace patterns {

void map() {
  cout << "map pattern" << endl;
  cout << "-----------" << endl;
  cout << "mapping add_five() onto each element of ";

  const int size = 10;
  int input[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  print_array(input, size);

  int output[size];
  parallel_for(0, size, 1, [input, &output](int i) {
      output[i] = add_five(input[i]);
  });
  cout << "output: ";
  print_array(output, size);
  cout << endl;
}

void stencil() {
  cout << "stencil pattern" << endl;
  cout << "---------------" << endl;
  cout << "summing each element with its neighbors, of ";

  const int size = 10;
  int input[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  print_array(input, size);

  int output[size] = {0};
  output[0] = sum_array(input, 2);
  output[size-1] = sum_array(&input[size-2], 2);
  parallel_for(1, size-1, 1, [input, &output](int i) {
      output[i] = sum_array((int*)&input[i-1], 3);
  });
  
  cout << "output: ";
  print_array(output, size);
  cout << endl;
}

void reduce() {
  cout << "reduce pattern" << endl;
  cout << "--------------" << endl;
  cout << "multiplying each element of ";

  const int size = 10;
  int input[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  print_array(input, size);

  auto multFunc = [](int x, int y) -> int {
    return x * y;
  };

  auto combineFunc = [](const blocked_range<int*>& r, int init) -> int {
    for(int* i = r.begin(); i != r.end(); ++i)
      init *= *i;
    return init;
  };

  const int product = parallel_reduce(blocked_range<int*>(input, input+size), 1, combineFunc, multFunc);

  cout << "product: " << product << endl << endl;
}

void scan() {
  using namespace tbb; //debug

  cout << "scan pattern (aka parallel prefix)" << endl;
  cout << "----------------------------------" << endl;
  cout << "computing the sum of each previous element in ";

  const int size = 10;
  int input[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  print_array(input, size);

  int output[size] = {0};
  Body body(output, input);
  parallel_scan(blocked_range<int>(0, size), body);

  cout << "output: ";
  print_array(output, size);
  cout << endl;
}

} // namespace patterns
