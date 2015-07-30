#include "data.h"
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
}

namespace patterns {
stars::stars(size_t count_) {
  count = count_;
  ascension = new double[count];
  declination = new double[count];
  fields = new int[count * FieldCount];
}

stars::~stars() {
  delete[] ascension;
  delete[] declination;
  delete[] fields;
}

double random_ascension() {
  return (((double)rand()) / (double)RAND_MAX) * 24.0;
}

double random_declination() {
  return (((double)rand()) / (double)RAND_MAX) * 180.0 - 90.0;
}

void populate_star_array(star* s, const size_t count) {
  for(int i = 0, end = count; i != end; ++i) {
    s[i].ascension = random_ascension();
    s[i].declination = random_declination();
  }
}

void populate_stars(stars* s) {
  for(int i = 0, end = s->count; i != end; ++i) {
    s->ascension[i] = random_ascension();
    s->declination[i] = random_declination();
  }
}

__attribute__((optimize("unroll-loops")))
void reduce_floats() {
  srand(time(NULL));
  const int numStars = 5000;
  star* array_of_structs = new star[numStars];
  stars struct_of_arrays(numStars);
  populate_star_array(array_of_structs, numStars);
  populate_stars(&struct_of_arrays);

  cout << "map pattern with arrays-of-structures" << endl;
  cout << "and structures-of-arrays" << endl;
  cout << "----------------------------------------" << endl;
  cout << "calculating longitude from right ascension of stars" << endl;

  auto get_longitude = [](const double ascension) {
    const double degrees = ascension * 15.0;
    const double adjusted_degrees = degrees > 360.0 ? degrees - 180.0 : degrees;
    return adjusted_degrees;
  };

  double output[numStars];
  const int numIterations = 10;

  clock_t start = clock();
  for(int i = 0, end = numIterations; i != end; ++i) {
    parallel_for(0, numStars, 1, [&struct_of_arrays, &output, get_longitude](int i) {
        output[i] = get_longitude(struct_of_arrays.ascension[i]);
      });
  }
  double seconds = (clock() - start) / (double)CLOCKS_PER_SEC;
  cout << "structure-of-arrays: " << seconds << " seconds." << endl;

  start = clock();
  for(int i = 0, end = numIterations; i != end; ++i) {
    parallel_for(0, numStars, 1, [&array_of_structs, &output, get_longitude](int i) {
        output[i] = get_longitude(array_of_structs[i].ascension);
      });
  }
  seconds = (clock() - start) / (double)CLOCKS_PER_SEC;
  cout << "array-of-structures: " << seconds << " seconds." << endl;
}
} // namespace patterns
