/* Copyright (c) 2008 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip/opt/ukernel/kernels/params/fft_param.hpp
    @author  Jules Bergmann
    @date    2008-06-13
    @brief   VSIPL++ Library: Parameters for FFT kernels.
*/

#ifndef VSIP_OPT_UKERNEL_KERNELS_PARAMS_FFT_PARAM_HPP
#define VSIP_OPT_UKERNEL_KERNELS_PARAMS_FFT_PARAM_HPP

typedef struct
{
  int           dir; // -1 forward, +1 inverse
  unsigned int  size;
  float         scale;
} Uk_fft_params;

#endif // VSIP_OPT_UKERNEL_KERNELS_PARAMS_FFT_PARAM_HPP
