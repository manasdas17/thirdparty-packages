/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/solver-llsqsol.cpp
    @author  Jules Bergmann
    @date    2005-09-07
    @brief   VSIPL++ Library: Unit tests for llsqsol() solver.
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <cassert>

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/tensor.hpp>
#include <vsip/solvers.hpp>

#include <vsip_csl/test.hpp>
#include <vsip_csl/test-precision.hpp>
#include "test-random.hpp"
#include "solver-common.hpp"

#define VERBOSE  0
#define DO_SWEEP 0

#if VERBOSE
#  include <iostream>
#  include <vsip_csl/output.hpp>
#  include "extdata-output.hpp"
#endif

using namespace std;
using namespace vsip;
using namespace vsip_csl;


/***********************************************************************
  llsqsol function using rsol tests
***********************************************************************/

template <return_mechanism_type RtM,
	  typename              T>
void
test_llsqsol_diag(
  length_type m,
  length_type n,
  length_type p)
{
  test_assert(m >= n);

  Matrix<T> a(m, n);
  Matrix<T> x(n, p);
  Matrix<T> b(m, p);

  a        = T();
  a.diag() = T(1);
  if (n > 0) a(0, 0)  = Test_traits<T>::value1();
  if (n > 2) a(2, 2)  = Test_traits<T>::value2();
  if (n > 3) a(3, 3)  = Test_traits<T>::value3();

  for (index_type i=0; i<p; ++i)
    b.col(i) = test_ramp(T(1), T(i), m);
  if (p > 1)
    b.col(1) += Test_traits<T>::offset();

  if (RtM == by_value)
    x = llsqsol(a, b);
  else
    llsqsol(a, b, x);

#if VERBOSE
  cout << "a = " << endl << a << endl;
  cout << "x = " << endl << x << endl;
  cout << "b = " << endl << b << endl;
#endif

  for (index_type c=0; c<p; ++c)
    for (index_type r=0; r<n; ++r)
      test_assert(equal(b(r, c),
		   a(r, r) * x(r, c)));
}



template <return_mechanism_type RtM,
	  typename              T>
void
test_llsqsol_random(
  length_type m,
  length_type n,
  length_type p)
{
  test_assert(m >= n);

  Matrix<T> a(m, n);
  Matrix<T> x(n, p);
  Matrix<T> b(m, p);
  Matrix<T> chk(m, p);

  randm(a);
  randm(b);

  // If m > n, min || AX - B || may not be zero,
  // Need way to check that X is best solution
  //
  // In the meantime, limit rank of A, B to produce zero minimum.

  for (index_type i=n; i<m; ++i)
  {
    a.row(i) = T(i-n+2) * a.row(i-n);
    b.row(i) = T(i-n+2) * b.row(i-n);
  }

  if (RtM == by_value)
    x = llsqsol(a, b);
  else
    llsqsol(a, b, x);

  prod(a, x, chk);
  float err = prod_check(a, x, b);

#if VERBOSE
  cout << "a = " << endl << a << endl;
  cout << "x = " << endl << x << endl;
  cout << "b = " << endl << b << endl;
  cout << "chk = " << endl << chk << endl;
  cout << "lsqsol<" << Type_name<T>::name()
       << ">(" << m << ", " << n << ", " << p << "): " << err << endl;
#endif

  typedef typename impl::Scalar_of<T>::type scalar_type;
#if NORMAL_EPSILON
  // These are almost_equal()'s normal epsilon.  They work fine for Lapack.
  scalar_type rel_epsilon = 1e-3;
  scalar_type abs_epsilon = 1e-5;
  float       err_bound   = 10.0;
#else
  // These are looser bounds.  They are necessary for SAL.
  scalar_type rel_epsilon = 1e-3;
  scalar_type abs_epsilon = 1e-5;
  float       err_bound   = 50.0;
#endif

  if (err > err_bound)
  {
    for (index_type r=0; r<m; ++r)
      for (index_type c=0; c<p; ++c)
	test_assert(almost_equal(b.get(r, c), chk.get(r, c),
				 rel_epsilon, abs_epsilon));
  }
}



template <return_mechanism_type RtM,
	  typename              T>
void
llsqsol_cases(vsip::impl::Bool_type<true>)
{
  test_llsqsol_diag<RtM, T>(1,   1, 2);
  test_llsqsol_diag<RtM, T>(5,   5, 2);
  test_llsqsol_diag<RtM, T>(6,   6, 2);
  test_llsqsol_diag<RtM, T>(17, 17, 2);

  test_llsqsol_diag<RtM, T>(1,   1, 3);
  test_llsqsol_diag<RtM, T>(5,   5, 3);
  test_llsqsol_diag<RtM, T>(17, 17, 3);

  test_llsqsol_diag<RtM, T>(3,   1, 3);
  test_llsqsol_diag<RtM, T>(5,   3, 3);
  test_llsqsol_diag<RtM, T>(17, 11, 3);

  test_llsqsol_random<RtM, T>(1,   1, 2);
  test_llsqsol_random<RtM, T>(5,   5, 2);
  test_llsqsol_random<RtM, T>(17, 17, 2);

  test_llsqsol_random<RtM, T>(1,   1, 3);
  test_llsqsol_random<RtM, T>(5,   5, 3);
  test_llsqsol_random<RtM, T>(17, 17, 3);

  test_llsqsol_random<RtM, T>(3,   1, 3);
  test_llsqsol_random<RtM, T>(5,   3, 3);
  test_llsqsol_random<RtM, T>(17, 11, 3);

#if DO_SWEEP
  for (index_type i=1; i<100; i+= 8)
    for (index_type j=1; j<10; j += 4)
    {
      test_llsqsol_random<RtM, T>(i,   i,   j+1);
      test_llsqsol_random<RtM, T>(i+1, i+1, j);
      test_llsqsol_random<RtM, T>(i+2, i+2, j+2);
    }
#endif
}



template <return_mechanism_type RtM,
	  typename              T>
void
llsqsol_cases(vsip::impl::Bool_type<false>)
{
}



// Front-end function for llsqsol_cases.

// This function dispatches to either real set of tests or an empty
// function depending on whether the QR backends configured in support
// value type T.  llsqsol is implemented with QR, and not all QR backends
// support all value types.

template <return_mechanism_type RtM,
	  typename              T>
void
llsqsol_cases()
{
  using vsip::impl::Bool_type;
  using vsip::impl::Type_equal;
  using vsip::impl::Choose_qrd_impl;
  using vsip::impl::None_type;
  using vsip::impl::Mercury_sal_tag;

  llsqsol_cases<RtM, T>(
	Bool_type<!Type_equal<typename Choose_qrd_impl<T>::type,
                              None_type>::value>());
}



/***********************************************************************
  Main
***********************************************************************/

template <> float  Precision_traits<float>::eps = 0.0;
template <> double Precision_traits<double>::eps = 0.0;



int
main(int argc, char** argv)
{
  vsipl init(argc, argv);

  Precision_traits<float>::compute_eps();
  Precision_traits<double>::compute_eps();

  llsqsol_cases<by_reference, float>();
  llsqsol_cases<by_reference, double>();
  llsqsol_cases<by_reference, complex<float> >();
  llsqsol_cases<by_reference, complex<double> >();

  llsqsol_cases<by_value, float>();
  llsqsol_cases<by_value, double>();
  llsqsol_cases<by_value, complex<float> >();
  llsqsol_cases<by_value, complex<double> >();
}
