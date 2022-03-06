#pragma once
#include <assert.h>

// Represents a simplified fraction n/d.
struct Fraction {
 public:
  int n, d;

  Fraction(int n_, int d_);

 private:
  void simplify();
};

bool operator<(const Fraction& a, const Fraction& b);
bool operator<=(const Fraction& a, const Fraction& b);
bool operator>(const Fraction& a, const Fraction& b);
bool operator>=(const Fraction& a, const Fraction& b);
bool operator==(const Fraction& a, const Fraction& b);
bool operator!=(const Fraction& a, const Fraction& b);

// Returns n choose k.
constexpr int compute_binom(int n, int k) {
  assert(0 < n && n <= 8);
  assert(0 < k && k <= n);
  int result = 1;
  for (int i = 0; i < k; i++) {
    result *= n - i;
  }
  for (int i = 1; i <= k; i++) {
    result /= i;
  }
  return result;
}