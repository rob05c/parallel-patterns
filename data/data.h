#ifndef hello_h
#define hello_h
#include <cstring> // for size_t
namespace patterns {

//#define STAR_FIELD_COUNT 50000

class star {
public:
  enum {FieldCount = 50000};
  double ascension;
  double declination;
  int fields[FieldCount];
};

class stars {
public:
  enum {FieldCount = 50000};
  stars(size_t count);
  virtual ~stars();
  double* ascension;
  double* declination;
  int* fields;
  int count;
private:
  stars();
  stars(const stars&);
  stars& operator=(const stars& rhs);
};

void reduce_floats();
} // namespace patterns
#endif
