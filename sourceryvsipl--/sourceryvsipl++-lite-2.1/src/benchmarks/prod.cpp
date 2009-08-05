/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/prod.cpp
    @author  Jules Bergmann
    @date    2005-10-11
    @brief   VSIPL++ Library: Benchmark for matrix-matrix produtcs.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/math.hpp>
#include <vsip/signal.hpp>

#include <vsip/core/profile.hpp>

#include <vsip_csl/plainblock.hpp>
#include <vsip_csl/test.hpp>
#include "loop.hpp"


using namespace vsip;


/***********************************************************************
  Definition
***********************************************************************/

// Matrix-matrix product benchmark class.

template <typename T>
struct t_prod1 : Benchmark_base
{
  static length_type const Dec = 1;

  char const* what() { return "t_prod1"; }
  float ops_per_point(length_type M)
  {
    length_type N = M;
    length_type P = M;

    float ops = /*M * */ P * N * 
      (vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add);

    return ops;
  }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 0; }
  int mem_per_point(length_type M) { return 3*M*M*sizeof(T); }

  void operator()(length_type M, length_type loop, float& time)
  {
    length_type N = M;
    length_type P = M;

    Matrix<T>   A (M, N, T(1));
    Matrix<T>   B (N, P, T(1));
    Matrix<T>   Z (M, P, T(1));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Z = prod(A, B);
    t1.stop();
    
    time = t1.delta();
  }

  t_prod1() {}
};



// Matrix-matrix product (with hermetian) benchmark class.

template <typename T>
struct t_prodh1 : Benchmark_base
{
  static length_type const Dec = 1;

  char const* what() { return "t_prodh1"; }
  float ops_per_point(length_type M)
  {
    length_type N = M;
    length_type P = M;

    float ops = /*M * */ P * N * 
      (vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add);

    return ops;
  }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 0; }
  int mem_per_point(length_type M) { return 3*M*M*sizeof(T); }

  void operator()(length_type M, length_type loop, float& time)
  {
    length_type N = M;
    length_type P = M;

    Matrix<T>   A (M, N, T());
    Matrix<T>   B (P, N, T());
    Matrix<T>   Z (M, P, T());

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Z = prodh(A, B);
    t1.stop();
    
    time = t1.delta();
  }

  t_prodh1() {}
};



// Matrix-matrix product (with tranpose) benchmark class.

template <typename T>
struct t_prodt1 : Benchmark_base
{
  static length_type const Dec = 1;

  char const* what() { return "t_prodt1"; }
  float ops_per_point(length_type M)
  {
    length_type N = M;
    length_type P = M;

    float ops = /*M * */ P * N * 
      (vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add);

    return ops;
  }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 0; }
  int mem_per_point(length_type M) { return 3*M*M*sizeof(T); }

  void operator()(length_type M, length_type loop, float& time)
  {
    length_type N = M;
    length_type P = M;

    Matrix<T>   A (M, N, T());
    Matrix<T>   B (P, N, T());
    Matrix<T>   Z (M, P, T());

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Z = prodt(A, B);
    t1.stop();
    
    time = t1.delta();
  }

  t_prodt1() {}
};



// Matrix-matrix product benchmark class with particular ImplTag.

template <typename ImplTag,
	  typename T>
struct t_prod2 : Benchmark_base
{
  static length_type const Dec = 1;

  char const* what() { return "t_prod2"; }
  float ops_per_point(length_type M)
  {
    length_type N = M;
    length_type P = M;

    float ops = /*M * */ P * N * 
      (vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add);

    return ops;
  }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 0; }
  int mem_per_point(length_type M) { return 3*M*M*sizeof(T); }

  void operator()(length_type M, length_type loop, float& time)
  {
    length_type N = M;
    length_type P = M;

    typedef Dense<2, T, row2_type> a_block_type;
    typedef Dense<2, T, row2_type> b_block_type;
    typedef Dense<2, T, row2_type> z_block_type;

    Matrix<T, a_block_type>   A(M, N, T());
    Matrix<T, b_block_type>   B(N, P, T());
    Matrix<T, z_block_type>   Z(M, P, T());


    typedef impl::dispatcher::Evaluator<impl::dispatcher::Op_prod_mm, ImplTag,
      void(z_block_type &, a_block_type const &, b_block_type const &)>
      Eval;

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Eval::exec(Z.block(), A.block(), B.block());
    t1.stop();
    
    time = t1.delta();
  }

  t_prod2() {}
};



template <typename ImplTag,
	  typename T>
struct t_prod2pb : Benchmark_base
{
  static length_type const Dec = 1;

  char const* what() { return "t_prod2"; }
  float ops_per_point(length_type M)
  {
    length_type N = M;
    length_type P = M;

    float ops = /*M * */ P * N * 
      (vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add);

    return ops;
  }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 0; }
  int mem_per_point(length_type M) { return 3*M*M*sizeof(T); }

  void operator()(length_type M, length_type loop, float& time)
  {
    length_type N = M;
    length_type P = M;

    typedef Plain_block<2, T, row2_type> a_block_type;
    typedef Plain_block<2, T, row2_type> b_block_type;
    typedef Plain_block<2, T, row2_type> z_block_type;

    Matrix<T, a_block_type>   A(M, N, T());
    Matrix<T, b_block_type>   B(N, P, T());
    Matrix<T, z_block_type>   Z(M, P, T());


    typedef impl::dispatcher::Evaluator<impl::dispatcher::Op_prod_mm, ImplTag,
      void(z_block_type &, a_block_type const &, b_block_type const &)>
		Eval;

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      Eval::exec(Z.block(), A.block(), B.block());
    t1.stop();
    
    time = t1.delta();
  }

  t_prod2pb() {}
};



void
defaults(Loop1P& loop)
{
  loop.loop_start_ = 5000;
  loop.start_ = 4;
  loop.stop_  = 8;
}



int
test(Loop1P& loop, int what)
{
  switch (what)
  {
  case  1: loop(t_prod1<float>()); break;
  case  2: loop(t_prod1<complex<float> >()); break;

  case  3: loop(t_prod2<impl::Generic_tag, float>()); break;
  case  4: loop(t_prod2<impl::Generic_tag, complex<float> >()); break;

#if VSIP_IMPL_HAVE_BLAS
  case  5: loop(t_prod2<impl::Blas_tag, float>()); break;
    // The BLAS backend doesn't handle split-complex, so don't attempt
    // to instantiate that code if we are using split-complex blocks.
# if !VSIP_IMPL_PREFER_SPLIT_COMPLEX
  case  6: loop(t_prod2<impl::Blas_tag, complex<float> >()); break;
# endif
#endif

  case  11: loop(t_prodt1<float>()); break;
  case  12: loop(t_prodt1<complex<float> >()); break;
  case  13: loop(t_prodh1<complex<float> >()); break;

    
  case  103: loop(t_prod2pb<impl::Generic_tag, float>()); break;
  case  104: loop(t_prod2pb<impl::Generic_tag, complex<float> >()); break;

  default: return 0;
  }
  return 1;
}
