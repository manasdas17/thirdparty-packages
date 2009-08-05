/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/dot.cpp
    @author  Jules Bergmann
    @date    2005-10-11
    @brief   VSIPL++ Library: Benchmark for dot-produtcs.

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

#include <vsip_csl/test.hpp>
#include "loop.hpp"

using namespace vsip;


/***********************************************************************
  Definition
***********************************************************************/

// Dot-product benchmark class.

template <typename T>
struct t_dot1 : Benchmark_base
{
  static length_type const Dec = 1;

  char const* what() { return "t_dot1"; }
  float ops_per_point(length_type)
  {
    float ops = (vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add);

    return ops;
  }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 0; }
  int mem_per_point(length_type) { return 2*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
  {
    Vector<T>   A (size, T());
    Vector<T>   B (size, T());
    T r = T();

    A(0) = T(3);
    B(0) = T(4);

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      r = dot(A, B);
    t1.stop();

    if (r != T(3)*T(4))
      abort();
    
    time = t1.delta();
  }

  t_dot1() {}
};



// Dot-product benchmark class with particular ImplTag.

template <typename ImplTag,
          typename T,
          bool     IsValid = impl::dispatcher::Evaluator<
            impl::dispatcher::Op_prod_vv_dot, ImplTag,
            T(typename Vector<T>::block_type const&, 
              typename Vector<T>::block_type const&) >::ct_valid>
struct t_dot2 : Benchmark_base
{
  static length_type const Dec = 1;

  char const* what() { return "t_dot2"; }
  float ops_per_point(length_type)
  {
    float ops = (vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add);

    return ops;
  }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 0; }
  int mem_per_point(length_type) { return 2*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
  {
    Vector<T>   A (size, T());
    Vector<T>   B (size, T());
    T r = T();

    A(0) = T(3);
    B(0) = T(4);

    typedef typename Vector<T>::block_type block_type;

    typedef impl::dispatcher::Evaluator<impl::dispatcher::Op_prod_vv_dot, 
        ImplTag, T(block_type const&, block_type const&) > 
      Eval;

    assert(Eval::ct_valid);
  
    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      r = Eval::exec(A.block(), B.block());
    t1.stop();

    if (r != T(3)*T(4))
      abort();
    
    time = t1.delta();
  }

  t_dot2() {}
};


template <typename ImplTag,
              typename T>
struct t_dot2<ImplTag, T, false> : Benchmark_base
{
  void operator()(length_type, length_type, float& time)
  {
    std::cout << "t_dot2: evaluator not implemented\n";
  }
}; 

void
defaults(Loop1P& loop)
{
  loop.loop_start_ = 5000;
  loop.start_ = 4;
}



int
test(Loop1P& loop, int what)
{
  switch (what)
  {
  case  1: loop(t_dot1<float>()); break;
  case  2: loop(t_dot1<complex<float> >()); break;

  case  3: loop(t_dot2<impl::Generic_tag, float>()); break;
  case  4: loop(t_dot2<impl::Generic_tag, complex<float> >()); break;

#if VSIP_IMPL_HAVE_BLAS
  case  5: loop(t_dot2<impl::Blas_tag, float>()); break;
  case  6: loop(t_dot2<impl::Blas_tag, complex<float> >()); break;
#endif

#if VSIP_IMPL_HAVE_CUDA
  case  7: loop(t_dot2<impl::Cuda_tag, float>()); break;
  case  8: loop(t_dot2<impl::Cuda_tag, complex<float> >()); break;
#endif

  default: return 0;
  }
  return 1;
}
