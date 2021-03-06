/* Copyright (c) 2008 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip/opt/ukernel/kernels/cbe_accel/cfconv_f.hpp
    @author  Jules Bergmann
    @date    2008-08-07
    @brief   VSIPL++ Library: Inter-complex fastconv ukernel.
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/opt/ukernel/kernels/params/fused_param.hpp>

#include <vsip/opt/ukernel/kernels/cbe_accel/fused_kernel.hpp>
#include <vsip/opt/ukernel/kernels/cbe_accel/cfft_f.hpp>
#include <vsip/opt/ukernel/kernels/cbe_accel/cvmmul_f.hpp>

typedef Fused_kernel<
		Fft_kernel, Vmmul_kernel<std::complex<float> >, Fft_kernel>
	kernel_type;

char Fft_kernel::buf1[FFT_BUF1_SIZE_BYTES] __attribute((aligned(128)));
char Fft_kernel::buf2[FFT_BUF2_SIZE_BYTES] __attribute((aligned(128)));

#include <vsip/opt/ukernel/cbe_accel/alf_base.hpp>
