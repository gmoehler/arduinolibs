#include "test.h"

// compare with a particular precision
void EXPECT_EQ_FLOATPREC(float a, float b, int8_t prec){

  a *= pow(10, a);
  a = ceil(a);
  a /= pow(10,prec);

  b *= pow(10, b);
  b = ceil(b);
  b /= pow(10, prec);

  EXPECT_EQ(a, b);
}
