#include "Numerov.hpp"
#include "QuantumDefect.hpp"

#include <string>
#include <complex>
#include <vector>
#include <cmath>

//#include <boost/math/special_functions/spherical_harmonic.hpp>


Numerov::Numerov(std::string species, int n, int l, real_t j)
  : species(species), n(n), l(l), j(j), qd(species,n,l,j),
    nsteps(nsteps_), xmin(xmin_), xmax(xmax_), dx(dx_) {
  dx_ = 0.001*10; // TODO
  xmin_ = n*n - n*sqrt(n*n-(l-1)*(l-1));
  xmin_ = floor(sqrt(( 2.08 > xmin_ ? 2.08 : xmin_ )));
  xmax_ = sqrt(2*n*(n+15));
  nsteps_ = ceil((xmax_ - xmin_)/dx_);
  x.resize(nsteps_);
  for (int i = 0; i < nsteps_; i++) {
    x[i] = xmin_ + i*dx_;
  }
  y.assign(nsteps_,0.0);
}


std::vector<real_t> Numerov::axis() {
  return x;
}


std::vector<real_t> Numerov::integrate() {
  // Set the initial condition
  if ( (n-l) % 2 == 0 )
    y[nsteps_-2] = -1e-10;
  else
    y[nsteps_-2] = 1e-10;

  // Perform the integration using Numerov's scheme
  for (int i = nsteps_-3; i >= 0; i--)
    y[i] = step(i);

  // Normalization
  real_t norm = 0;
  for (int i = 0; i < nsteps_; i++)
    norm += y[i]*y[i] * x[i]*x[i] * dx_;
  norm = sqrt(2*norm);

  if ( norm != 0.0 ) {
    for (int i = 0; i < nsteps_; i++)
      y[i] /= norm;
  }

  /*
  for (int i = 0; i < nsteps_; i++)
    y[i] /= pow(x[i],1.5);
  */

  return y;
}


real_t Numerov::V(real_t x) {
  real_t Z_l = 1 + (qd.Z - 1)*exp(-qd.a1*x)
    - x*(qd.a3 + qd.a4*x)*exp(-qd.a2*x);
  real_t V_c = -Z_l / x;
  real_t V_p = -qd.ac/(2.*x*x*x*x) * (1-exp(-pow(x/qd.rc,6)));
  real_t V_so = 0.0;
  if ( l < 4 ) {
    real_t alpha = 7.2973525664e-3;// ~1/137 fine-structure constant CODATA 2014
    V_so = alpha*alpha/(4 * x*x*x) * (j*(j+1) - l*(l+1) - 0.5*(1+0.5));
  }
  return V_c + V_p + V_so;
}


real_t Numerov::g(real_t x) {
  //return l*(l+1)/(x*x) + 2.*(V(x) - qd.energy);
  return (2.*l+.5)*(2.*l+1.5)/(x*x) + 8.*(x*x)*(V(x*x) - qd.energy);
}


real_t Numerov::step(int i) {
  real_t A = (2. + 5./6. * dx_*dx_ * g(x[i+1])) * y[i+1];
  real_t B = (1. - 1./12.* dx_*dx_ * g(x[i+2])) * y[i+2];
  real_t C =  1. - 1./12.* dx_*dx_ * g(x[i]);
  return (A - B)/C  ;
}