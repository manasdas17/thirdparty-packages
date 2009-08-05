/* Copyright (c) 2008 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/stream.cpp
    @author  Jules Bergmann
    @date    2008-10-08
    @brief   VSIPL++ Library: VSIPL++ STEAM Benchmark.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>

#include <vsip/initfin.hpp>
#include <vsip/math.hpp>
#include <vsip/random.hpp>
#include <vsip/selgen.hpp>
#include <vsip/core/setup_assign.hpp>
#include <vsip/opt/diag/eval.hpp>

#include "benchmarks.hpp"
#include "create_map.hpp"

using namespace vsip;


/***********************************************************************
  STREAM Parameters
***********************************************************************/

template<typename T=double>
struct parameters
{
  // Type to use in checking code.  It might not be the same
  // as T.
  typedef double check_type;
  // Tolerance for comparisons.
  double const EPSILON() const { return 1.e-8; }
  char const* TYPESTR() const { return "DOUBLE PRECISION"; }
};

template<>
struct parameters<float>
{
  typedef double check_type;
  double const EPSILON() const { return 1.e-5; }
  char const*  const TYPESTR() const { return "FLOAT"; }
};

template<>
struct parameters<int>
{
  typedef int check_type;
  int    const EPSILON() const { return 0; }
  char const* const TYPESTR() const { return "INT"; }
};

/***********************************************************************
  STREAM Composite: Copy -> Scale -> Add -> Triad
***********************************************************************/

template <typename T,
	  typename MapT = Local_map,
	  typename SP   = No_barrier>
struct t_composite_vpp : Benchmark_base
{
  typedef Dense<1, T, row1_type, MapT> block_type;

  char const* what() { return "t_composite_vpp"; }
  int  ops_per_point(length_type)  { return  5; }
  int riob_per_point(length_type)  { return  6*sizeof(T); }
  int wiob_per_point(length_type)  { return  4*sizeof(T); }
  int  mem_per_point(length_type)  { return  3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
//  if (loop < 10) loop = 10;

    std::cout << "STREAM_vpp: size=" << size << ", loop=" << loop << "\n";

    parameters<T> param;

    MapT map = create_map<1, MapT>('a');

    Vector<T, block_type> A(size, T(), map);
    Vector<T, block_type> B(size,      map);
    Vector<T, block_type> C(size,      map);
    T s = T(3);

    A = T(2);

    float times[4][loop];
    float avgtime[4], maxtime[4], mintime[4];

    float   bytes[4];
    bytes[0] = 2 * sizeof(T) * size;
    bytes[1] = 2 * sizeof(T) * size;
    bytes[2] = 3 * sizeof(T) * size;
    bytes[3] = 3 * sizeof(T) * size;

    vsip::impl::profile::Acc_timer t1;
    SP sp;
    
    sp.sync();
    for (index_type l=0; l<loop; ++l)
    {
      		t1.start();
      C = A;
      		t1.stop();	times[0][l] = t1.delta();
      		t1.start();
      B = s*C;
      		t1.stop();	times[1][l] = t1.delta();
      		t1.start();
      C = A+B;
      		t1.stop();	times[2][l] = t1.delta();
      		t1.start();
      A = B+s*C;
      		t1.stop();	times[3][l] = t1.delta();
    }
    sp.sync();
    time = t1.total();

    /*  --- SUMMARY --- */

    for (int j=0; j<4; j++)
    {
      avgtime[j] = 0;
      maxtime[j] = 0;
      mintime[j] = 1000000;
    }
    for (index_type k=1; k<loop; k++) /* note -- skip first iteration */
      {   
      for (int j=0; j<4; j++)
	{   
	avgtime[j] = avgtime[j] + times[j][k];
	mintime[j] = std::min(mintime[j], times[j][k]);
	maxtime[j] = std::max(maxtime[j], times[j][k]);
	}   
      }   

/*
    if (!verbose)
      printf("Stream Benchmark for %s\n",params.TYPESTR());
*/

    static char const *label[4] = {"Copy:      ", "Scale:     ",
                                   "Add:       ", "Triad:     "};

    if (streamprnt_)
    {
      printf("Function      Rate (MB/s)   Avg time     Min time     Max time\n");
      for (int j=0; j<4; j++)
	{
	avgtime[j] = avgtime[j]/(double)(loop-1);

	printf("%s%11.4f  %11.4f  %11.4f  %11.4f\n", label[j],
	       1.0E-06 * bytes[j]/mintime[j],
	       avgtime[j],
	       mintime[j],
	       maxtime[j]);
	}
    }
    
    if (check_)
    {
      typedef typename parameters<T>::check_type CHKT;
      CHKT a = T(2), b = T(2), c = T(0);
      for (index_type l=0; l<loop; ++l)
      {
	c = a;
	b = s*c;
	c = a+b;
	a = b+s*c;
      }
      a *= (CHKT)size;
      b *= (CHKT)size;
      c *= (CHKT)size;
      CHKT asum = 0;
      CHKT bsum = 0;
      CHKT csum = 0;
      for (int i=0; i<size; i++)
      {
        asum += A(i);
        bsum += B(i);
        csum += C(i);
      }
      asum = std::abs(a-asum)/asum;
      bsum = std::abs(b-bsum)/bsum;
      csum = std::abs(c-csum)/csum;
      CHKT epsilon = param.EPSILON();
      test_assert( asum <= epsilon );
      test_assert( bsum <= epsilon );
      test_assert( csum <= epsilon );
    }
  }

  void diag()
  {
    length_type const size = 256;

    MapT map = create_map<1, MapT>('a');

    Vector<T, block_type> A(size, T(), map);
    Vector<T, block_type> B(size,      map);
    Vector<T, block_type> C(size,      map);

    vsip::impl::diagnose_eval_list_std(C, A);
    vsip::impl::diagnose_eval_list_std(B, T(3)*C);
    vsip::impl::diagnose_eval_list_std(C, A+B);
    vsip::impl::diagnose_eval_list_std(A, B+T(3)*C);
  }

  t_composite_vpp(bool check, bool streamprnt) : check_(check), streamprnt_(streamprnt) {}

  // Member data
  bool check_;
  bool streamprnt_;
};

template <typename T,
	  typename MapT = Local_map,
	  typename SP   = No_barrier>
struct t_composite_c : Benchmark_base
{
  typedef Dense<1, T, row1_type, MapT> block_type;

  char const* what() { return "t_composite_c"; }
  int  ops_per_point(length_type)  { return  5; }
  int riob_per_point(length_type)  { return  6*sizeof(T); }
  int wiob_per_point(length_type)  { return  4*sizeof(T); }
  int  mem_per_point(length_type)  { return  3*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
    VSIP_IMPL_NOINLINE
  {
    parameters<T> param;

    MapT map = create_map<1, MapT>('a');

    Vector<T, block_type> A(size, T(), map);
    Vector<T, block_type> B(size,      map);
    Vector<T, block_type> C(size,      map);
    T s = T(3);

    A = T(2);

    float times[4][loop];
    float avgtime[4], maxtime[4], mintime[4];

    float   bytes[4];
    bytes[0] = 2 * sizeof(T) * size; 
    bytes[1] = 2 * sizeof(T) * size; 
    bytes[2] = 3 * sizeof(T) * size; 
    bytes[3] = 3 * sizeof(T) * size;

    vsip::impl::profile::Acc_timer t1;
    SP sp;
    
    {
      typedef typename Vector<T, block_type>::local_type l_view_type;
      typedef typename l_view_type::block_type l_block_type;

      impl::Ext_data<l_block_type> ext_a(A.local().block(), impl::SYNC_IN);
      impl::Ext_data<l_block_type> ext_b(B.local().block(), impl::SYNC_OUT);
      impl::Ext_data<l_block_type> ext_c(C.local().block(), impl::SYNC_OUT);

      int const N = ext_a.size();
      T* a        = ext_a.data();
      T* b        = ext_b.data();
      T* c        = ext_c.data();
    
      sp.sync();
      for (index_type l=0; l<loop; ++l)
      {
	int j;
                t1.start();
	for (j=0; j<N; j++)
	  c[j] = a[j];
                t1.stop();   times[0][l] = t1.delta();
                t1.start();
	for (j=0; j<N; j++)
	  b[j] = s*c[j];
                t1.stop();   times[1][l] = t1.delta();
                t1.start();
	for (j=0; j<N; j++)
	  c[j] = a[j]+b[j];
                t1.stop();   times[2][l] = t1.delta();
                t1.start();
	for (j=0; j<N; j++)
	  a[j] = b[j]+s*c[j];
                t1.stop();   times[3][l] = t1.delta();
      }
      sp.sync();
    }
    
    time = t1.total();

    /*  --- SUMMARY --- */

    for (int j=0; j<4; j++)
    {
      avgtime[j] = 0;
      maxtime[j] = 0;
      mintime[j] = 1000000;
    }
    for (index_type k=1; k<loop; k++) /* note -- skip first iteration */
      {
      for (int j=0; j<4; j++)
        {
        avgtime[j] = avgtime[j] + times[j][k];
        mintime[j] = std::min(mintime[j], times[j][k]);
        maxtime[j] = std::max(maxtime[j], times[j][k]);
        }
      }

/*
     if (!verbose)
       printf("Stream Benchmark for %s\n",params.TYPESTR());
 */

    static char const *label[4] = {"Copy:      ", "Scale:     ",
                                   "Add:       ", "Triad:     "};

    if (streamprnt_)
    {
      printf("Function      Rate (MB/s)   Avg time     Min time     Max time\n");
      for (int j=0; j<4; j++)
	{
	avgtime[j] = avgtime[j]/(double)(loop-1);

	printf("%s%11.4f  %11.4f  %11.4f  %11.4f\n", label[j],
	       1.0E-06 * bytes[j]/mintime[j],
	       avgtime[j],
	       mintime[j],
	       maxtime[j]);
	}
    }

    if (check_)
    {
      typedef typename parameters<T>::check_type CHKT;
      CHKT a = T(2), b = T(2), c = T(0);
      for (index_type l=0; l<loop; ++l)
      {
	c = a;
	b = s*c;
	c = a+b;
	a = b+s*c;
      }
      a *= (CHKT)size;
      b *= (CHKT)size;
      c *= (CHKT)size;
      CHKT asum = 0;
      CHKT bsum = 0;
      CHKT csum = 0;
      for (int i=0; i<size; i++)
      {
        asum += A(i);
        bsum += B(i);
        csum += C(i);
      }
      asum = std::abs(a-asum)/asum;
      bsum = std::abs(b-bsum)/bsum;
      csum = std::abs(c-csum)/csum;
      CHKT epsilon = param.EPSILON();
      test_assert( asum <= epsilon );
      test_assert( bsum <= epsilon );
      test_assert( csum <= epsilon );
    }
  }

  void diag()
  {
    std::cout << "Scalar code () code\n";
  }

  t_composite_c(bool check, bool streamprnt) : check_(check), streamprnt_(streamprnt) {}

  // Member data
  bool check_;
  bool streamprnt_;
};






/***********************************************************************
  Definitions
***********************************************************************/

void
defaults(Loop1P& loop)
{
  loop.start_      = 21;
  loop.stop_       = 21;
  loop.loop_start_ = 10;
  loop.fix_loop_   = true;

  loop.param_["check"] = "y";
  loop.param_["streamprnt"] = "n";
}



int
test(Loop1P& loop, int what)
{
  bool check  = (loop.param_["check"] == "1" ||
		 loop.param_["check"] == "y");
  bool streamprnt  = (loop.param_["streamprnt"] == "1" ||
		      loop.param_["streamprnt"] == "y");

  switch (what)
  {
  case   1: loop(t_composite_vpp<float> (check,streamprnt)); break;
  case   2: loop(t_composite_vpp<double>(check,streamprnt)); break;
  case   3: loop(t_composite_vpp<int>   (check,streamprnt)); break;

  case 101: loop(t_composite_c<float>   (check,streamprnt)); break;
  case 102: loop(t_composite_c<double>  (check,streamprnt)); break;
  case 103: loop(t_composite_c<int>     (check,streamprnt)); break;

  case 0:
    std::cout
      << "stream -- VSIPL++ STREAM benchmark\n"
      << "STREAM_Composite:\n"
      << "   -1 -- composite (VSIPL++ float)\n"
      << "   -2 -- composite (VSIPL++ double)\n"
      << "   -3 -- composite (VSIPL++ int)\n"
      << " -101 -- composite (C float)\n"
      << " -102 -- composite (C double)\n"
      << " -103 -- composite (C int)\n"
      << "\n"
      << " Parameters\n"
      << "  -p:check      {0,1,n,y} -- check benchmark result (default y)\n"
      << "  -p:streamprnt {0,1,n,y} -- print STREAM results   (default n)\n"
      ;

  default:
    return 0;
  }
  return 1;
}
