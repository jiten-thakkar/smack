#include <stdio.h>
#include <stdlib.h>
#include "smack.h"

// @expect 2 verified, 2 errors?

int g[10];

int main(void) {
  ensures(forall("x", g[qvar("x")] == 0 || qvar("x") < 0 || qvar("x") > 10));
  return 0;
}