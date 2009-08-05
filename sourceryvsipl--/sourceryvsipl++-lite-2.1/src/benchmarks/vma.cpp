/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/vma.cpp
    @author  Jules Bergmann
    @date    2006-05-25
    @brief   VSIPL++ Library: Benchmark for fused multiply-add
             and variants such as axpy.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#define DO_SIMD 0

#include <iostream>

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/math.hpp>
#include <vsip/random.hpp>
#if DO_SIMD
#  include <vsip/opt/simd/simd.hpp>
#  include <vsip/opt/simd/vaxpy.hpp>
#endif
#include <vsip/opt/diag/eval.hpp>

#include <vsip_csl/test-storage.hpp>
#include "benchmarks.hpp"


using namespace vsip;


/***********************************************************************
  Definitions - vector element-wise fused multiply-add
***********************************************************************/

template <typename T1,
	  typename T2>
struct Ops2_info
{
  static unsigned int const div = 1;
  static unsigned int const sqr = 1;
  static unsigned int const mul = 1;
  static unsigned int const add = 1;
};



template <typename T>
struct Ops2_info<T, complex<T> >
{
  static unsigned int const div = 6 + 3 + 2;
  static unsigned int const mul = 2;
  static unsigned int const add = 1;
};



template <typename T>
struct Ops2_info<complex<T>, T >
{
  static unsigned int const div = 2;
  static unsigned int const mul = 2;
  static unsigned int const add = 1;
};

template <typename T>
struct Ops2_info<complex<T>, complex<T> >
{
  static unsigned int const div = 6 + 3 + 2;
  static unsigned int const mul = 4 + 2;
  static unsigned int const add = 2;
};



template <typename T,
	  dimension_type DimA,
	  dimension_type DimB,
	  dimension_type DimC>
struct t_vma : Benchmark_base
{
  char const* what() { return "t_vma"; }
  int ops_per_point(length_type)
    { return vsip::impl::Ops_info<T>::mul + vsip::impl::Ops_info<T>::add; }
  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    Storage<DimA, T> A(Domain<1>(size), T(3));
    Storage<DimB, T> B(Domain<1>(size), T(4));
    Storage<DimC, T> C(Domain<1>(size), T(5));
    Vector<T>        X(size, T(0));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      X = A.view * B.view + C.view;
    t1.stop();
    
    for (index_type i=0; i<size; ++i)
      test_assert(equal(X.get(i), T(3*4+5)));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type const size = 256;

    Storage<DimA, T> A(Domain<1>(size), T(3));
    Storage<DimB, T> B(Domain<1>(size), T(4));
    Storage<DimC, T> C(Domain<1>(size), T(5));
    Vector<T>        X(size, T(0));

    vsip::impl::diagnose_eval_list_std(X, A.view * B.view + C.view);
  }
};



// In-place multiply-add, aka AXPY (Y = A*X + Y)

template <typename TA,
	  typename TB,
	  dimension_type DimA,
	  dimension_type DimB>
struct t_vma_ip : Benchmark_base
{
  typedef typename Promotion<TA, TB>::type T;

  char const* what() { return "t_vma_ip"; }
  int ops_per_point(length_type)
  { return Ops2_info<TA, TB>::mul + Ops2_info<T, T>::add; }

  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    Storage<DimA, TA> A(Domain<1>(size), TA(0));
    Storage<DimB, TB> B(Domain<1>(size), TB(0));
    Vector<T>         X(size, T(5));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      X += A.view * B.view;
    t1.stop();

    A.view = TA(3);
    B.view = TB(4);

    X += A.view * B.view;
    
    for (index_type i=0; i<size; ++i)
      test_assert(equal(X.get(i), T(3*4+5)));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type const size = 256;

    Storage<DimA, TA> A(Domain<1>(size), TA(0));
    Storage<DimB, TB> B(Domain<1>(size), TB(0));
    Vector<T>         X(size, T(5));

    vsip::impl::diagnose_eval_list_std(X, X + A.view * B.view);
  }
};




template <typename T>
struct t_vma_cSC : Benchmark_base
{
  char const* what() { return "t_vma_cSC"; }
  int ops_per_point(length_type)
  {
    return Ops2_info<T, complex<T> >::mul +
	   Ops2_info<complex<T>, complex<T> >::add;
  }
  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    complex<T>          a = complex<T>(3, 0);
    Vector<T>           B(size, T(4));
    Vector<complex<T> > C(size, complex<T>(5, 0));
    Vector<complex<T> > X(size, complex<T>(0, 0));

    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      X = a * B + C;
    t1.stop();
    
    for (index_type i=0; i<size; ++i)
      test_assert(equal(X.get(i), complex<T>(3*4+5, 0)));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type const size = 256;

    complex<T>          a = complex<T>(3, 0);
    Vector<T>           B(size, T(4));
    Vector<complex<T> > C(size, complex<T>(5, 0));
    Vector<complex<T> > X(size, complex<T>(0, 0));

    vsip::impl::diagnose_eval_list_std(X, a * B + C);
  }
};



#if DO_SIMD
template <typename T>
struct t_vma_cSC_simd : Benchmark_base
{
  char const* what() { return "t_vma_cSC_simd"; }
  int ops_per_point(length_type)
  {
    return Ops2_info<T, complex<T> >::mul +
	   Ops2_info<complex<T>, complex<T> >::add;
  }
  int riob_per_point(length_type) { return 2*sizeof(T); }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    using vsip::impl::Ext_data;

    typedef Dense<1, T>           sblock_type;
    typedef Dense<1, complex<T> > cblock_type;

    complex<T>          a = complex<T>(3, 0);
    Vector<T, sblock_type>          B(size, T(4));
    Vector<complex<T>, cblock_type> C(size, complex<T>(5, 0));
    Vector<complex<T>, cblock_type> X(size, complex<T>(0, 0));

    vsip::impl::profile::Timer t1;

    {
      Ext_data<sblock_type> B_ext(B.block());
      Ext_data<cblock_type> C_ext(C.block());
      Ext_data<cblock_type> X_ext(X.block());
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      vsip::impl::simd::vma_cSC(a, B_ext.data(), C_ext.data(), X_ext.data(),
				size);
    t1.stop();
    }
    
    for (index_type i=0; i<size; ++i)
    {
      if (!equal(X.get(i), complex<T>(3*4+5, 0)))
	std::cout << i << " = " << X.get(i) << std::endl;
      test_assert(equal(X.get(i), complex<T>(3*4+5, 0)));
    }
    
    time = t1.delta();
  }

  void diag()
  {
    using vsip::impl::simd::Is_algorithm_supported;
    using vsip::impl::simd::Alg_vma_cSC;

    static bool const Is_vectorized =
      Is_algorithm_supported<std::complex<T>, false, Alg_vma_cSC>::value;

    std::cout << "is_vectorized: "
	      << (Is_vectorized ? "yes" : "no")
	      << std::endl;
  }
};
#endif



void
defaults(Loop1P&)
{
}



int
test(Loop1P& loop, int what)
{
  typedef float           SF;
  typedef complex<float>  CF;
  typedef double          SD;
  typedef complex<double> CD;

  switch (what)
  {
  case   1: loop(t_vma<SF, 1, 1, 1>()); break;
  case   2: loop(t_vma<SF, 0, 1, 1>()); break;
  case   3: loop(t_vma<SF, 0, 1, 0>()); break;

  case  11: loop(t_vma<CF, 1, 1, 1>()); break;
  case  12: loop(t_vma<CF, 0, 1, 1>()); break;
  case  13: loop(t_vma<CF, 0, 1, 0>()); break;

  case  21: loop(t_vma_ip<SF, SF, 1, 1>()); break;
  case  22: loop(t_vma_ip<SF, SF, 0, 1>()); break;

  case  31: loop(t_vma_ip<CF, CF, 1, 1>()); break;
  case  32: loop(t_vma_ip<CF, CF, 0, 1>()); break;

  case  41: loop(t_vma_ip<CF, SF, 0, 1>()); break;

  case 141: loop(t_vma_ip<CD, SD, 0, 1>()); break;

  case 201: loop(t_vma_cSC<SF>()); break;
#if DO_SIMD
  case 202: loop(t_vma_cSC_simd<SF>()); break;
#endif
  case 203: loop(t_vma_cSC<SD>()); break;
#if DO_SIMD
  case 204: loop(t_vma_cSC_simd<SD>()); break;
#endif

  case 0:
    std::cout
      << "vma -- vector multiply-add\n"
      << "  -21 -- V += A * B [float]\n"
      << "  -22 -- V += a * B [float]\n"
      << "  -31 -- V += A * B [complex]\n"
      << "  -32 -- V += a * B [complex]\n"
      << " -201 -- V = a * B + C [complex*float + complex]\n"
      << std::endl;
  default:
    return 0;
  }
  return 1;
}
