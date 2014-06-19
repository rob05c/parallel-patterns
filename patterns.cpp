#include "patterns.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <atomic>
#include "tbb/tbb.h"

namespace {
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::to_string;
using std::atomic;
using std::atomic_fetch_add;
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

/// body for tbb::parallel_scan
/// used by scan()
class TbbScanBody {
  int result;
  int* const y;
  const int* const z;
public:
  TbbScanBody(int y_[], const int z_[]) : result(0), z(z_), y(y_) {}
  TbbScanBody(TbbScanBody& b) : z(b.z), y(b.y), result(0) {}
  TbbScanBody(TbbScanBody& b, tbb::split) : z(b.z), y(b.y), result(0) {}
  void assign(TbbScanBody& b, tbb::split) {result = b.result;}
  void assign(TbbScanBody& b) {result = b.result;}
  void reverse_join(TbbScanBody& a) {result = a.result;}
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

  auto add_five = [](const int num) {
    return num + 5;
  };

  int output[size];
  parallel_for(0, size, 1, [input, &output, add_five](int i) {
      output[i] = add_five(input[i]);
  });
  cout << "output: ";
  print_array(output, size);
  cout << endl;
}

/// Note this is also implemented using the geometric decomposition pattern.
/// The array is _decomposed_ into overlapping regions which are fed to sum_array.
void stencil() {
  cout << "stencil pattern" << endl;
  cout << "---------------" << endl;
  cout << "summing each element with its neighbors, of ";

  const int size = 10;
  int input[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  print_array(input, size);

  auto sum_array = [](int* array, const size_t len) {
    int sum = 0;
    for(size_t i = 0, end = len; i != end; ++i)
      sum += array[i];
    return sum;
  };

  int output[size] = {0};
  output[0] = sum_array(input, 2);
  output[size-1] = sum_array(&input[size-2], 2);
  parallel_for(1, size-1, 1, [input, &output, sum_array](int i) {
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
  cout << "scan pattern (aka parallel prefix)" << endl;
  cout << "----------------------------------" << endl;
  cout << "computing the sum of each previous element in ";

  const int size = 10;
  int input[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  print_array(input, size);

  int output[size] = {0};
  TbbScanBody body(output, input);
  parallel_scan(blocked_range<int>(0, size), body);

  cout << "output: ";
  print_array(output, size);
  cout << endl;
}

void pack() {
  cout << "pack pattern" << endl;
  cout << "------------" << endl;
  cout << "packing array ";

  const int size = 20;
  int input[] = {1, 2, 0, 0, 3, 0, 4, 0, 5, 0, 6, 7, 0, 8, 9, 0, 0, 10, 0, 0};
  print_array(input, size);

  int output[size] = {0};
  atomic<unsigned int> next = ATOMIC_VAR_INIT(0);
  parallel_for(0, size, 1, [input, &output, &next](int i) {
      if(input[i] == 0)
        return;
      const int outi = atomic_fetch_add(&next, 1u);
      output[outi] = input[i];
  });
  const size_t output_size = next.load();

  cout << "output: ";
  print_array(output, output_size);
  cout << endl;
}

void gather() {
  cout << "gather pattern" << endl;
  cout << "------------" << endl;
  cout << "gathering array ";

  int input[] = {1, 2, 0, 0, 3, 0, 4, 0, 5, 0, 6, 7, 0, 8, 9, 0, 0, 10, 0, 0};
  const int size = sizeof(input) / sizeof(int);
  print_array(input, size);

  cout << "indices ";
  int indices[] = {0, 1, 6, 8, 17};
  const int indices_size = sizeof(indices) / sizeof(int);
  print_array(indices, indices_size);

  int output[indices_size];
  parallel_for(0, indices_size, 1, [input, indices, &output](int i) {
      output[i] = input[indices[i]];
  });

  cout << "output: ";
  print_array(output, indices_size);
  cout << endl;
}

void workpile() {
  cout << "workpile pattern" << endl;
  cout << "------------" << endl;
  cout << "adding 2 for i < 5, subtracting 10 from i ≥ 5 and adding to the workpile " << endl;
  cout << "for i ∈ ";

  int input[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int size = sizeof(input) / sizeof(int);
  vector<int*> dolist = {input, input+1, input+2, input+3, input+4, input+5, input+6, input+7, input+8, input+9};
  print_array(input, size);

  parallel_do(dolist.begin(), dolist.end(), [](int* i, parallel_do_feeder<int*>& feeder) {
      if(*i < 5) {
        *i += 2;
      } else {
        *i -= 10;
        feeder.add(i);
      }
  });

  cout << "output: ";
  print_array(input, size);
}

} // namespace patterns
