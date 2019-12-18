#include "util.h"

int min(int a, int b) { return (a < b) ? a : b; }
int max(int a, int b) { return (a > b) ? a : b; }
int mid(int lo, int val, int hi) { return min(hi, max(lo, val)); }
