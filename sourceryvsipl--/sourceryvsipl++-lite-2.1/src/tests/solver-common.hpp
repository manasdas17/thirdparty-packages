/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/solver-common.cpp
    @author  Jules Bergmann
    @date    2005-09-07
    @brief   VSIPL++ Library: Common classes for solver (covsol, llsql,
	     qr) tests.
*/

#ifndef VSIP_TESTS_SOLVER_COMMON_HPP
#define VSIP_TESTS_SOLVER_COMMON_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>

#include <vsip/support.hpp>
#include <vsip/complex.hpp>
#include <vsip/matrix.hpp>

#include <vsip_csl/test.hpp>
#include <vsip_csl/test-precision.hpp>



/***********************************************************************
  Definitions
***********************************************************************/

template <typename T>
vsip::const_Vector<T>
test_ramp(
  T                 a,
  T                 b,
  vsip::length_type len)
VSIP_NOTHROW
{
  vsip::Vector<T> r(len);

  for (vsip::index_type i=0; i<len; ++i)
    r.put(i, a + T(i)*b);

  return r;
}

template <typename T,
	  typename BlockT>
vsip::Vector<T, BlockT>
test_ramp(
  vsip::Vector<T, BlockT> view,
  T                       a,
  T                       b)
VSIP_NOTHROW
{
  for (vsip::index_type i=0; i<view.size(); ++i)
    view.put(i, a + T(i)*b);

  return view;
}

template <typename T>
struct Test_traits
{
  static T offset() { return T(0);    }
  static T value1() { return T(2);    }
  static T value2() { return T(0.5);  }
  static T value3() { return T(-0.5); }
  static T conj(T a) { return a; }
  static bool is_positive(T a) { return (a > T(0)); }

  static vsip::mat_op_type const trans = vsip::mat_trans;
};

template <typename T>
struct Test_traits<vsip::complex<T> >
{
  static vsip::complex<T> offset() { return vsip::complex<T>(0, 2);      }
  static vsip::complex<T> value1() { return vsip::complex<T>(2);      }
  static vsip::complex<T> value2() { return vsip::complex<T>(0.5, 1); }
  static vsip::complex<T> value3() { return vsip::complex<T>(1, -1); }
  static vsip::complex<T> conj(vsip::complex<T> a) { return vsip::conj(a); }
  static bool is_positive(vsip::complex<T> a)
  { return (a.real() > T(0)) && (equal(a.imag(), T(0))); }

  static vsip::mat_op_type const trans = vsip::mat_herm;
};



template <typename T>
T tconj(T const& val)
{
  return Test_traits<T>::conj(val);
}

template <typename T>
bool
is_positive(T const& val)
{
  return Test_traits<T>::is_positive(val);
}



// Compute matrix-matrix produce C = A B

template <typename T,
	  typename Block1,
	  typename Block2,
	  typename Block3>
void
prod(
  vsip::const_Matrix<T, Block1> a,
  vsip::const_Matrix<T, Block2> b,
  vsip::Matrix      <T, Block3> c)
{
  using vsip::index_type;

  assert(a.size(0) == c.size(0));
  assert(b.size(1) == c.size(1));
  assert(a.size(1) == b.size(0));

  for (index_type i=0; i<c.size(0); ++i)
    for (index_type j=0; j<c.size(1); ++j)
    {
      T tmp = T();
      for (index_type k=0; k<a.size(1); ++k)
	tmp += a.get(i, k) * b.get(k, j);
      c(i, j) = tmp;
    }
}



// Check error between matrix-matrix product A B and expected C.

template <typename T,
	  typename Block1,
	  typename Block2,
	  typename Block3>
float
prod_check(
  vsip::const_Matrix<T, Block1> a,
  vsip::const_Matrix<T, Block2> b,
  vsip::Matrix<T, Block3> c)
{
  using vsip::index_type;
  typedef typename vsip::impl::Scalar_of<T>::type scalar_type;

  assert(a.size(0) == c.size(0));
  assert(b.size(1) == c.size(1));
  assert(a.size(1) == b.size(0));

  float err = 0.f;

  for (index_type i=0; i<c.size(0); ++i)
    for (index_type j=0; j<c.size(1); ++j)
    {
      T           tmp   = T();
      scalar_type guage = scalar_type();

      for (index_type k=0; k<a.size(1); ++k)
      {
	tmp   += a.get(i, k) * b.get(k, j);
	guage += mag(a.get(i, k)) * mag(b.get(k, j));
      }

      float err_ij = mag(tmp - c(i, j)) / 
        vsip_csl::Precision_traits<scalar_type>::eps;
      if (guage > scalar_type())
	err_ij = err_ij/guage;
      err = std::max(err, err_ij);
    }

  return err;
}



// Compute matrix-matrix produce C = A' B

template <typename T,
	  typename Block1,
	  typename Block2,
	  typename Block3>
void
prodh(
  vsip::const_Matrix<T, Block1> a,
  vsip::const_Matrix<T, Block2> b,
  vsip::Matrix      <T, Block3> c)
{
  using vsip::index_type;

  assert(a.size(1) == c.size(0));
  assert(b.size(1) == c.size(1));
  assert(a.size(0) == b.size(0));

  for (index_type i=0; i<c.size(0); ++i)
    for (index_type j=0; j<c.size(1); ++j)
    {
      T tmp = T();
      for (index_type k=0; k<a.size(0); ++k)
	tmp += Test_traits<T>::conj(a.get(k, i)) * b.get(k, j);
      c(i, j) = tmp;
    }
}

#endif // VSIP_TESTS_SOLVER_COMMON_HPP
