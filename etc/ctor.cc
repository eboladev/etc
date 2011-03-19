#include <stdio.h>

class L {
public:
  L() {};
  ~L() {};
  L(const L &src) {printf("copy\n");};
};

class Kurl {
public:
  Kurl() {};
  ~Kurl() {};
 
  class List: public L {
  public:
    List() {};
    ~List() {};
  };
};

int main() {
  Kurl::List some;
  Kurl::List other(some);
  return 0;
}
