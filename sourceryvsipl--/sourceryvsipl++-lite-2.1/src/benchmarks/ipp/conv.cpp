/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/conv_ipp.cpp
    @author  Jules Bergmann
    @date    2005-08-24
    @brief   VSIPL++ Library: Benchmark for IPP's FFT.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>
#include <cmath>
#include <complex>

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/math.hpp>
#include <vsip/signal.hpp>

#include <vsip/opt/profile.hpp>
#include <vsip/core/ops_info.hpp>

#include <ipps.h>

#include "loop.hpp"

using std::complex;


/***********************************************************************
  Declarations
***********************************************************************/



template <typename T>
bool
almost_equal(
  T	A,
  T	B,
  T	rel_epsilon = 1e-4,
  T	abs_epsilon = 1e-6)
{
  if (std::abs(A - B) < abs_epsilon)
    return true;

  T relative_error;

  if (std::abs(B) > std::abs(A))
    relative_error = std::abs((A - B) / B);
  else
    relative_error = std::abs((B - A) / A);

  return (relative_error <= rel_epsilon);
}



template <typename T>
struct t_conv_ipp;

template <>
struct t_conv_ipp<float> : Benchmark_base
{
  char* what() { return "t_conv_ipp"; }
  float ops_per_point(size_t size)
  {
    size_t output_size = size+coeff_size_+1; 

    float ops = coeff_size_ * output_size *
      (vsip::impl::Ops_info<float>::mul + vsip::impl::Ops_info<float>::add);

    return ops / size;
  }
  int riob_per_point(size_t) { return 1*sizeof(Ipp32f); }
  int wiob_per_point(size_t) { return 1*sizeof(Ipp32f); }
  int mem_per_point(size_t)  { return 2*sizeof(Ipp32f); }

  void operator()(size_t size, size_t loop, float& time)
  {
    size_t output_size = size+coeff_size_-1; 

    Ipp32f*            A;
    Ipp32f*            Z;
    Ipp32f*            Coeff;

    Coeff  = ippsMalloc_32f(coeff_size_);
    A      = ippsMalloc_32f(size);
    Z      = ippsMalloc_32f(output_size);


    if (!Coeff || !A || !Z)
      throw(std::bad_alloc());
      

    for (size_t i=0; i<size; ++i)
      A[i] = float(1.f * rand()/(RAND_MAX+1.0)) - float(0.5);

    for (size_t i=0; i<coeff_size_; ++i)
      Coeff[i] = float(1.f * rand()/(RAND_MAX+1.0)) - float(0.5);
    
    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (size_t l=0; l<loop; ++l)
    {
      ippsConv_32f(A, size, Coeff, coeff_size_, Z);
    }
    t1.stop();

    float max_err = 0.f;
    // We should be comparing up to output_size, but the last few
    // values are always fishy ...
    for (int n=0; n<static_cast<int>(size); ++n)
    {
      float sum   = 0;
      float guage = 0;
      for (int k=0; k<=n; ++k)
	if (0 <= n-k && n-k < static_cast<int>(coeff_size_))
	{
	  sum   += A[k]*Coeff[n-k];
	  guage += std::abs(A[k])*std::abs(Coeff[n-k]);
	}
      float err = std::abs(Z[n] - sum) / guage;
      if (err > max_err)
	max_err = err;
    }
    assert(max_err < 0.001);

    time = t1.delta();

    ippsFree(Coeff);
    ippsFree(A);
    ippsFree(Z);
  }

  t_conv_ipp(size_t coeff_size) : coeff_size_(coeff_size) {}

  size_t coeff_size_;
};



void
defaults(Loop1P& loop)
{
  loop.loop_start_ = 5000;
  loop.start_ = 4;
  loop.user_param_ = 16;
}


int
test(Loop1P& loop, int what)
{
  switch (what)
  {
  case  1: loop(t_conv_ipp<float>(loop.user_param_)); break;
  default: return 0;
  }
  return 1;
}
