/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/fft_expr_arg.cpp
    @author  Jules Bergmann
    @date    2006-05-10
    @brief   VSIPL++ Library: Regression test for passing an expression
             as an argument to Fft (Issue #125).
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/matrix.hpp>
#include <vsip/signal.hpp>

#include <vsip_csl/test.hpp>

using namespace std;
using namespace vsip;


/***********************************************************************
  Definitions
***********************************************************************/

template <typename T1,
	  typename T2>
struct Fft_dir_trait;

template <typename T>
struct Fft_dir_trait<complex<T>, complex<T> >
{
  static int const fwd = fft_fwd;
  static int const inv = fft_inv;
};

template <typename T>
struct Fft_dir_trait<T, complex<T> >
{
  static int const fwd = 0;
  // cannot perform inverse
};

template <typename T>
struct Fft_dir_trait<complex<T>, T>
{
  // cannot perform forward
  static int const inv = 0;
};



template <typename View1, typename View2, typename View3>
void pulseCompression(int decimationFactor, 
                      View1 in, View2 ref, View3 out) {
  int size = in.size() / decimationFactor;

  Domain<1> decimatedDomain(0, decimationFactor, size);

  typedef typename View1::value_type T1;
  typedef typename View2::value_type T2;
  typedef typename View3::value_type T3;

  Fft<const_Vector, T1, T2, Fft_dir_trait<T1, T2>::fwd>
    forwardFft (size, 1);
  Fft<const_Vector, T2, T3, Fft_dir_trait<T2, T3>::inv, by_reference> 
    inverseFft (size, 1.0/size);

  T1 alpha = T1(1);

  inverseFft(ref * forwardFft(alpha * in(decimatedDomain)), out);
}

int
main(int argc, char** argv)
{
  vsipl init(argc, argv);

  length_type size = 16;

  Vector<complex<float> > in(size, complex<float>());
  Vector<complex<float> > ref(size, complex<float>());
  Vector<complex<float> > out(size);

  pulseCompression(1, in, ref, out);


  Vector<float>           in2(size,      float());
  Vector<complex<float> > ref2(size/2+1, complex<float>());
  Vector<float>           out2(size);

  pulseCompression(1, in2, ref2, out2);

  return 0;
}
