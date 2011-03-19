#include <iostream>
#include <cstdlib>
 
using namespace std;
 
unsigned long fib(unsigned long n) {
  if (n < 2) return (1);
 
  unsigned long i;
  unsigned int fibnm1 = 1, fibn = 1;
  unsigned int fib_tmp;
 
  for (i = 2; i <= n; i++) {
    fib_tmp = fibn + fibnm1;
    fibnm1 = fibn;
    fibn = fib_tmp;
  } 
 
return(fibn);
}
 
int main(int argc, char *argv[]) {
  int n = ((argc == 2) ? atoi(argv[1]) : 1);
  cout << fib(n) << endl;
  return(0);
}
