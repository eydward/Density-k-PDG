#pragma once
#include <assert.h>

#include <string>

// Represents a simplified fraction n/d.
struct Fraction {
 public:
  // The numerator and denominator of the fraction, respectively.
  int n, d;
  // Constructs a new Fraction with the given numerator and denominator values (will be simplified).
  Fraction(int n_, int d_);
  // Returns a text representation of the fraction.
  std::string to_string() const;
  // Returns a large value that represents infinity (in fact it's 1E8).
  static Fraction infinity();
  // Returns a small value that is close to 0 (in fact it's 1/(8!))
  static Fraction epsilon();

 private:
  void simplify();
};

Fraction operator+(const Fraction& a, const Fraction& b);
bool operator<(const Fraction& a, const Fraction& b);
bool operator<=(const Fraction& a, const Fraction& b);
bool operator>(const Fraction& a, const Fraction& b);
bool operator>=(const Fraction& a, const Fraction& b);
bool operator==(const Fraction& a, const Fraction& b);
bool operator!=(const Fraction& a, const Fraction& b);

// Returns n choose k.
constexpr int compute_binom(int n, int k) {
  assert(0 < n && n <= 12);
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