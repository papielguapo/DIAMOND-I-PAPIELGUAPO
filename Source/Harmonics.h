#pragma once
#include <array>
#include <cmath>
namespace diamond {
// Tn(0) is removed so a silent channel never excites the even generators.
// DC produced on non-silent material is removed by the branch DC servo.
inline double residual(double x, const std::array<float,12>& amount) noexcept {
 if (!std::isfinite(x)) return 0;
 const double a=std::abs(x), u= a>1 ? std::copysign(1.0,x) : x;
 double prev=1, curr=u, sum=0;
 for(int n=2;n<=13;++n) {
  double next=2*u*curr-prev; prev=curr; curr=next;
  // Linear continuation avoids the exponential polynomial growth outside [-1,1].
  double basis=next;
  if(a>1) basis += ((x<0 && n%2)?-1:1)*n*n*(a-1);
  const double atZero=n%2 ? 0 : (n%4==0 ? 1 : -1);
  double gain=amount[(size_t)n-2]*.01;
  gain=gain*gain/(double)n; // independent progressive taper; no shared normalisation
  sum+=gain*(basis-atZero);
 }
 return sum;
}
}
