/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/vmul.cpp
    @author  Jules Bergmann
    @date    2005-07-11
    @brief   VSIPL++ Library: Benchmark for vector multiply.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>

#include <vsip/initfin.hpp>

#include "vmul.hpp"

using namespace vsip;



/***********************************************************************
  Definitions
***********************************************************************/

void
defaults(Loop1P&)
{
}



int
test(Loop1P& loop, int what)
{
  switch (what)
  {
  case   1: loop(t_vmul1<float>()); break;
  case   2: loop(t_vmul1<complex<float> >()); break;
#ifdef VSIP_IMPL_SOURCERY_VPP
  case   3: loop(t_vmul2<complex<float>, impl::Cmplx_inter_fmt>()); break;
  case   4: loop(t_vmul2<complex<float>, impl::Cmplx_split_fmt>()); break;
#endif
  case   5: loop(t_rcvmul1<float>()); break;

  case  21: loop(t_vmul_dom1<float>()); break;
  case  22: loop(t_vmul_dom1<complex<float> >()); break;

  case  31: loop(t_vmul_ip1<float>()); break;
  case  32: loop(t_vmul_ip1<complex<float> >()); break;

    // Using function
  case  51: loop(t_vmul_func<float>()); break;
  case  52: loop(t_vmul_func<complex<float> >()); break;

  // Double-precision

  case 101: loop(t_vmul1<double>()); break;
  case 102: loop(t_vmul1<complex<double> >()); break;
#ifdef VSIP_IMPL_SOURCERY_VPP
  case 103: loop(t_vmul2<complex<double>, impl::Cmplx_inter_fmt>()); break;
  case 104: loop(t_vmul2<complex<double>, impl::Cmplx_split_fmt>()); break;
#endif
  case 105: loop(t_rcvmul1<double>()); break;

  case 131: loop(t_vmul_ip1<double>()); break;
  case 132: loop(t_vmul_ip1<complex<double> >()); break;

  case 0:
    std::cout
      << "vmul -- vector multiplication\n"
      << "single-precision:\n"
      << " Vector-Vector:\n"
      << "   -1 -- Vector<        float > * Vector<        float >\n"
      << "   -2 -- Vector<complex<float>> * Vector<complex<float>>\n"
      << "   -3 -- Vector<complex<float>> * Vector<complex<float>> (INTER)\n"
      << "   -4 -- Vector<complex<float>> * Vector<complex<float>> (SPLIT)\n"
      << "   -5 -- Vector<        float > * Vector<complex<float>>\n"
      << "\n"
      << "  -21 -- t_vmul_dom1\n"
      << "  -22 -- t_vmul_dom1\n"
      << "  -31 -- t_vmul_ip1\n"
      << "  -32 -- t_vmul_ip1\n"
      << " Vector-Vector (using mul() function):\n"
      << "  -51 -- mul(Vector<        float >, Vector<        float >)\n"
      << "  -52 -- mul(Vector<complex<float>>, Vector<complex<float>>)\n"
      << "\ndouble-precision:\n"
      << "  (101-113)\n"
      << "  (131-132)\n"
      ;

  default:
    return 0;
  }
  return 1;
}
