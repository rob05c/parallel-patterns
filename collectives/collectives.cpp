#include "collectives.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <atomic>
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
void reduce_floats() {
  srand(time(NULL));
  const int size = 100000;
  const size_t product_count = 10;

  cout << "reduce pattern with floating-point arithmetic" << endl;
  cout << "---------------------------------------------" << endl;
  cout << "the purpose of this function is to demonstrate the approximate commutativity of floating-point arithmetic." << endl;
  cout << "multiplying each element of " << size << "x array, with values approaching 1.0, " << product_count << " times:" << endl;

  double input[size];
  for(size_t i = 0, end = size; i != end; ++i)
    input[i] = 1.0 + (((double)rand()) / ((double)RAND_MAX)) / 1000.0;

  auto multFunc = [](double x , double y) -> double {
    return x * y;
  };

  auto combineFunc = [](const blocked_range<double*>& r, double init) -> double {
    for(double* i = r.begin(); i != r.end(); ++i)
      init *= *i;
    return init;
  };


  cout << std::fixed;
  for(size_t i = 0, end = product_count; i != end; ++i) {
    const double product = parallel_reduce(blocked_range<double*>(input, input+size), 1.0, combineFunc, multFunc);
    cout << "product " << i << ": " << product << endl;
  }
  cout << endl;

}
} // namespace patterns
