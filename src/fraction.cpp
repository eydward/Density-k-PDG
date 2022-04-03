#include "fraction.h"

#include <algorithm>

int gcd(int a, int b) {
  while (b != 0) {
    a = a % b;
    std::swap(a, b);
  }
  return a;
}

Fraction::Fraction(int n_, int d_) : n(n_), d(d_) {
  assert(d_ > 0);
  simplify();
}

std::string Fraction::to_string() const { return std::to_string(n) + "/" + std::to_string(d); }

void Fraction::simplify() {
  int g = gcd(n, d);
  if (g != 1) {
    n /= g;
    d /= g;
  }
}

Fraction Fraction::infinity() { return Fraction(1E8, 1); }
Fraction Fraction::epsilon() { return Fraction(1, 40320); }

Fraction operator+(const Fraction& a, const Fraction& b) {
  return Fraction(a.n * b.d + a.d * b.n, a.d * b.d);
}

bool operator<(const Fraction& a, const Fraction& b) { return a.n * b.d < a.d * b.n; }
bool operator>(const Fraction& a, const Fraction& b) { return b < a; }
bool operator<=(const Fraction& a, const Fraction& b) { return !(b < a); }
bool operator>=(const Fraction& a, const Fraction& b) { return !(a < b); }
bool operator==(const Fraction& a, const Fraction& b) { return a.n == b.n && a.d == b.d; }
bool operator!=(const Fraction& a, const Fraction& b) { return !(a == b); }
