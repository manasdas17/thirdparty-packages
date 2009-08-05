/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/vmul.cpp
    @author  Jules Bergmann
    @date    2006-06-01
    @brief   VSIPL++ Library: Benchmark for vector magsq.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/math.hpp>
#include <vsip/random.hpp>
#include <vsip/core/metaprogramming.hpp>
#include <vsip/opt/diag/eval.hpp>

#include "benchmarks.hpp"

using namespace vsip;


/***********************************************************************
  Definitions - vector element-wise multiply
***********************************************************************/

template <typename T>
struct t_vmagsq1 : Benchmark_base
{
  typedef typename impl::Scalar_of<T>::type scalar_type;

  char const* what() { return "t_vmagsq1"; }
  int ops_per_point(length_type)
  {
    if (impl::Is_complex<T>::value)
      return 2*vsip::impl::Ops_info<scalar_type>::mul + 
        vsip::impl::Ops_info<scalar_type>::add;
    else
      return vsip::impl::Ops_info<T>::mul;
  }
  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    Vector<T>           A(size, T());
    Vector<scalar_type> Z(size);

    Rand<T> gen(0, 0);
    A = gen.randu(size);

    A.put(0, T(3));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Z = magsq(A);
    t1.stop();
    
    for (index_type i=0; i<size; ++i)
      test_assert(equal(Z.get(i), magsq(A.get(i))));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type size = 256;
    Vector<T>           A(size, T());
    Vector<scalar_type> Z(size);

    vsip::impl::diagnose_eval_list_std(Z, magsq(A));
  }
};



template <typename T>
struct t_vmag1 : Benchmark_base
{
  typedef typename impl::Scalar_of<T>::type scalar_type;

  char const* what() { return "t_vmag1"; }
  int ops_per_point(length_type)
  {
    if (impl::Is_complex<T>::value)
      return 2*vsip::impl::Ops_info<scalar_type>::mul + 
        vsip::impl::Ops_info<scalar_type>::add;
    else
      return vsip::impl::Ops_info<T>::mul;
  }
  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    Vector<T>           A(size, T());
    Vector<scalar_type> Z(size);

    Rand<T> gen(0, 0);
    A = gen.randu(size);

    A.put(0, T(3));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Z = mag(A);
    t1.stop();
    
    for (index_type i=0; i<size; ++i)
      test_assert(equal(Z.get(i), mag(A.get(i))));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type size = 256;
    Vector<T>           A(size, T());
    Vector<scalar_type> Z(size);

    vsip::impl::diagnose_eval_list_std(Z, mag(A));
  }
};



template <typename T>
struct t_vmag_dense_mat : Benchmark_base
{
  typedef typename impl::Scalar_of<T>::type scalar_type;

  static length_type const rows = 2;

  char const* what() { return "t_vmag_dense_mat"; }
  int ops_per_point(length_type)
  {
    if (impl::Is_complex<T>::value)
      return rows * (2*vsip::impl::Ops_info<scalar_type>::mul + 
		     vsip::impl::Ops_info<scalar_type>::add);
    else
      return rows * vsip::impl::Ops_info<T>::mul;
  }
  int riob_per_point(length_type) { return rows*2*sizeof(T); }
  int wiob_per_point(length_type) { return rows*1*sizeof(T); }
  int mem_per_point(length_type)  { return rows*3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    Matrix<T>           A(rows, size, T());
    Matrix<scalar_type> Z(rows, size);

    Rand<T> gen(0, 0);
    A = gen.randu(rows, size);

    A.put(0, 0, T(3));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Z = mag(A);
    t1.stop();
    
    for (index_type r=0; r<rows; ++r)
      for (index_type i=0; i<size; ++i)
	test_assert(equal(Z.get(r, i), mag(A.get(r, i))));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type size = 256;
    Matrix<T>           A(rows, size, T());
    Matrix<scalar_type> Z(rows, size);

    vsip::impl::diagnose_eval_list_std(Z, mag(A));
  }
};



template <typename T>
struct t_vmag_nondense_mat : Benchmark_base
{
  typedef typename impl::Scalar_of<T>::type scalar_type;

  static length_type const rows = 2;

  char const* what() { return "t_vmag_nondense_mat"; }
  int ops_per_point(length_type)
  {
    if (impl::Is_complex<T>::value)
      return rows * (2*vsip::impl::Ops_info<scalar_type>::mul + 
		     vsip::impl::Ops_info<scalar_type>::add);
    else
      return rows * vsip::impl::Ops_info<T>::mul;
  }
  int riob_per_point(length_type) { return rows*2*sizeof(T); }
  int wiob_per_point(length_type) { return rows*1*sizeof(T); }
  int mem_per_point(length_type)  { return rows*3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    Matrix<T>           Asup(rows, size + 16, T());
    Matrix<scalar_type> Zsup(rows, size + 16);

    typename Matrix<T>::subview_type A = Asup(Domain<2>(rows, size));
    typename Matrix<T>::subview_type Z = Zsup(Domain<2>(rows, size));

    Rand<T> gen(0, 0);
    A = gen.randu(rows, size);

    A.put(0, 0, T(3));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Z = mag(A);
    t1.stop();
    
    for (index_type r=0; r<rows; ++r)
      for (index_type i=0; i<size; ++i)
	test_assert(equal(Z.get(r, i), mag(A.get(r, i))));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type size = 256;

    Matrix<T>           Asup(rows, size + 16, T());
    Matrix<scalar_type> Zsup(rows, size + 16);

    typename Matrix<T>::subview_type A = Asup(Domain<2>(rows, size));
    typename Matrix<T>::subview_type Z = Zsup(Domain<2>(rows, size));

    vsip::impl::diagnose_eval_list_std(Z, mag(A));
  }
};



void
defaults(Loop1P&)
{
}



int
test(Loop1P& loop, int what)
{
  switch (what)
  {
  case  1: loop(t_vmagsq1<        float   >()); break;
  case  2: loop(t_vmagsq1<complex<float > >()); break;
  case  3: loop(t_vmagsq1<        double  >()); break;
  case  4: loop(t_vmagsq1<complex<double> >()); break;

  case 11: loop(t_vmag1<        float   >()); break;
  case 12: loop(t_vmag1<complex<float > >()); break;
  case 13: loop(t_vmag1<        double  >()); break;
  case 14: loop(t_vmag1<complex<double> >()); break;

  case 111: loop(t_vmag_dense_mat<float>()); break;
  case 112: loop(t_vmag_nondense_mat<float>()); break;

  default:
    return 0;
  }
  return 1;
}
