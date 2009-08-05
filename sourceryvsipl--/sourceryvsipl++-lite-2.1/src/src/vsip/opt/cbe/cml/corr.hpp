/* Copyright (c) 2008 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip/opt/cbe/cml/corr.hpp
    @author  Mike LeBlanc
    @date    2008-05-21
    @brief   VSIPL++ Library: Correlation class implementation using CML.
*/

#ifndef VSIP_OPT_CBE_CML_CORR_HPP
#define VSIP_OPT_CBE_CML_CORR_HPP

#if VSIP_IMPL_REF_IMPL
# error "vsip/opt files cannot be used as part of the reference impl."
#endif

#include <vsip/support.hpp>
#include <vsip/domain.hpp>
#include <vsip/vector.hpp>
#include <vsip/matrix.hpp>
#include <vsip/core/domain_utils.hpp>
#include <vsip/core/signal/types.hpp>
#include <vsip/core/profile.hpp>
#include <vsip/core/signal/corr_common.hpp>
#include <vsip/opt/dispatch.hpp>

#include <vsip/core/block_traits.hpp>
#include <vsip/opt/expr/serial_evaluator.hpp>
#include <vsip/core/expr/scalar_block.hpp>
#include <vsip/core/expr/unary_block.hpp>
#include <vsip/core/expr/binary_block.hpp>
#include <vsip/core/expr/operations.hpp>
#include <vsip/core/fns_elementwise.hpp>
#include <vsip/core/extdata.hpp>
#include <vsip/core/adjust_layout.hpp>

#include <vsip/math.hpp>
#include <vsip/signal.hpp>

#include <cml.h>

namespace vsip
{
namespace impl
{
namespace cml
{

// Correlation

// Wrappers for CML functions.
inline
void
corr(
  float const* coeff, length_type c_size,
  float const* in,    length_type /*i_size*/, stride_type s_in,
  float*       out,   length_type o_size, stride_type s_out,
  length_type decimation)
{
  cml_corr1d_f(coeff,1,in,s_in,out,s_out,decimation,c_size,o_size);
}

inline
void
corr(
  std::complex<float> const* coeff, length_type c_size,
  std::complex<float> const* in,    length_type /*i_size*/, stride_type s_in,
  std::complex<float>*       out,   length_type o_size, stride_type s_out,
  length_type decimation)
{
  float const* fcoeff = reinterpret_cast<float const*>(coeff);
  float const* fin    = reinterpret_cast<float const*>(in);
  float*       fout   = reinterpret_cast<float*>(out);
  cml_ccorr1d_f(fcoeff,1,fin,s_in,fout,s_out,decimation,c_size,o_size);
}

inline
void
corr(
  std::pair<float*,float*> coeff, length_type c_size,
  std::pair<float*,float*> in,    length_type /*i_size*/, stride_type s_in,
  std::pair<float*,float*> out,   length_type o_size, stride_type s_out,
  length_type decimation)
{
  float const* fcoeff_re = reinterpret_cast<float const*>(coeff.first);
  float const* fcoeff_im = reinterpret_cast<float const*>(coeff.second);
  float const* fin_re    = reinterpret_cast<float const*>(in.first);
  float const* fin_im    = reinterpret_cast<float const*>(in.second);
  float*       fout_re   = reinterpret_cast<float*>(out.first);
  float*       fout_im   = reinterpret_cast<float*>(out.second);
  cml_zcorr1d_f(
    fcoeff_re,fcoeff_im,1,
    fin_re,fin_im,s_in,
    fout_re,fout_im,s_out,
    decimation,c_size,o_size
    );
}

inline
void
corr2(
  float const* ref,
    length_type Mr, length_type Mc, stride_type Mr_stride, stride_type Mc_stride,
  float const* in, 
                                    stride_type Nr_stride, stride_type Nc_stride,
  float*       out,
    length_type Pr, length_type Pc, stride_type Pr_stride, stride_type Pc_stride
  )
{
  cml_corr2d_f(
    ref,Mr_stride,Mc_stride,
    in, Nr_stride,Nc_stride,
    out,Pr_stride,Pc_stride,
    1,1,Mr,Mc,Pr,Pc);
}

inline
void
corr2(
  std::complex<float> const* ref,
    length_type Mr, length_type Mc, stride_type Mr_stride, stride_type Mc_stride,
  std::complex<float> const* in, 
                                    stride_type Nr_stride, stride_type Nc_stride,
  std::complex<float>*       out,
    length_type Pr, length_type Pc, stride_type Pr_stride, stride_type Pc_stride
  )
{
  float const* fref = reinterpret_cast<float const*>(ref);
  float const* fin  = reinterpret_cast<float const*>(in);
  float*       fout = reinterpret_cast<float*>(out);
  cml_ccorr2d_f(
    fref,Mr_stride,Mc_stride,
    fin, Nr_stride,Nc_stride,
    fout,Pr_stride,Pc_stride,
    1,1,Mr,Mc,Pr,Pc);
}

inline
void
corr2(
  std::pair<float*,float*> ref,
    length_type Mr, length_type Mc, stride_type Mr_stride, stride_type Mc_stride,
  std::pair<float*,float*> in, 
                                    stride_type Nr_stride, stride_type Nc_stride,
  std::pair<float*,float*> out,
    length_type Pr, length_type Pc, stride_type Pr_stride, stride_type Pc_stride
  )
{
  cml_zcorr2d_f(
    ref.first,ref.second,Mr_stride,Mc_stride,
    in.first, in.second, Nr_stride,Nc_stride,
    out.first,out.second,Pr_stride,Pc_stride,
    1,1,Mr,Mc,Pr,Pc);
}


template <template <typename, typename> class V,
	  support_region_type R,
	  typename            T,
	  unsigned            N,
          alg_hint_type       H>
class Correlation
{
  static dimension_type const dim = Dim_of_view<V>::dim;

  typedef dense_complex_type complex_type;
  typedef Storage<complex_type, T> storage_type;
  typedef typename storage_type::type ptr_type;

  // Compile-time constants.
public:
  static support_region_type const supprt  = R;

  // Constructors, copies, assignments, and destructors.
public:
  Correlation(Domain<dim> const& ref_size,
              Domain<dim> const& input_size)
    VSIP_THROW((std::bad_alloc));

  Correlation(Correlation const&) VSIP_NOTHROW;
  Correlation& operator=(Correlation const&) VSIP_NOTHROW;
  ~Correlation() VSIP_NOTHROW;

  // Accessors.
public:
  Domain<dim> const& reference_size() const VSIP_NOTHROW   { return ref_size_; }
  Domain<dim> const& input_size() const VSIP_NOTHROW   { return input_size_; }
  Domain<dim> const& output_size() const VSIP_NOTHROW  { return output_size_; }
  support_region_type support() const VSIP_NOTHROW     { return supprt; }

  float impl_performance(char* what) const
  {
    if (!strcmp(what, "in_ext_cost")) return pm_in_ext_cost_;
    else if (!strcmp(what, "out_ext_cost")) return pm_out_ext_cost_;
    else if (!strcmp(what, "non-opt-calls")) return pm_non_opt_calls_;
    else return 0.f;
  }

  // Implementation functions.
protected:
  template <typename Block0,
	    typename Block1,
	    typename Block2>
  Vector<T, Block2>
  operator()(
    bias_type,
    const_Vector<T, Block0>,
    const_Vector<T, Block1>,
    Vector<T, Block2>)
    VSIP_NOTHROW;

  template <typename Block0,
	    typename Block1,
	    typename Block2>
  Matrix<T, Block2>
  operator()(
    bias_type,
    const_Matrix<T, Block0>,
    const_Matrix<T, Block1>,
    Matrix<T, Block2>)
    VSIP_NOTHROW;

public:
  template <typename Block0,
	    typename Block1,
	    typename Block2>
  void
  impl_correlate(
    bias_type,
    const_Vector<T, Block0>,
    const_Vector<T, Block1>,
    Vector<T, Block2>)
    VSIP_NOTHROW;

  template <typename Block0,
	    typename Block1,
	    typename Block2>
  void
  impl_correlate(
    bias_type,
    const_Matrix<T, Block0>,
    const_Matrix<T, Block1>,
    Matrix<T, Block2>)
    VSIP_NOTHROW;

protected:
  typedef Layout<dim,
                 typename Row_major<dim>::type,
		 Stride_unit,
		 complex_type>
    layout_type;
  typedef typename View_of_dim<dim, T, Dense<dim, T> >::type coeff_view_type;
  typedef Ext_data<typename coeff_view_type::block_type, layout_type>
    c_ext_type;

  // Member data.
private:
  Domain<dim>     ref_size_;
  Domain<dim>     input_size_;
  Domain<dim>     output_size_;

  aligned_array<T>        in_buffer_;
  aligned_array<T>        out_buffer_;
  aligned_array<T>        ref_buffer_;

  int             pm_non_opt_calls_;
  size_t          pm_in_ext_cost_;
  size_t          pm_out_ext_cost_;
};


/// Construct a correlation object.

template <template <typename, typename> class ConstViewT,
	  support_region_type                 Supp,
	  typename                            T,
	  unsigned                            n_times,
          alg_hint_type                       a_hint>
Correlation<ConstViewT, Supp, T, n_times, a_hint>::Correlation(
  Domain<dim> const&   ref_size,
  Domain<dim> const&   input_size)
VSIP_THROW((std::bad_alloc))
  : 
    ref_size_   (ref_size),
    input_size_ (input_size),
    output_size_(impl::conv_output_size(Supp, ref_size_, input_size, 1)),
    in_buffer_  (input_size_.size()),
    out_buffer_ (output_size_.size()),
    ref_buffer_ (ref_size_.size()),
    pm_non_opt_calls_ (0)
{
}

template <template <typename, typename> class ConstViewT,
	  support_region_type                 Supp,
	  typename                            T,
	  unsigned                            n_times,
          alg_hint_type                       a_hint>
Correlation<ConstViewT, Supp, T, n_times, a_hint>::~Correlation()
  VSIP_NOTHROW
{
}



// Perform 1-D correlation.

template <template <typename, typename> class ConstViewT,
	  support_region_type Supp,
	  typename            T,
	  unsigned            n_times,
          alg_hint_type       a_hint>
template <typename Block0,
	  typename Block1,
	  typename Block2>
void
Correlation<ConstViewT, Supp, T, n_times, a_hint>::impl_correlate(
  bias_type               bias,
  const_Vector<T, Block0> ref,
  const_Vector<T, Block1> in,
  Vector<T, Block2>       out)
VSIP_NOTHROW
{
  length_type const M = this->ref_size_[0].size();
  length_type const N = this->input_size_[0].size();
  length_type const P = this->output_size_[0].size();

  assert(P == out.size());

  typedef vsip::impl::Ext_data<Block0> ref_ext_type;
  typedef vsip::impl::Ext_data<Block1>  in_ext_type;
  typedef vsip::impl::Ext_data<Block2> out_ext_type;

  ref_ext_type ref_ext(ref.block(), vsip::impl::SYNC_IN,  array_cast<complex_type>(ref_buffer_));
  in_ext_type   in_ext( in.block(), vsip::impl::SYNC_IN,  array_cast<complex_type>( in_buffer_));
  out_ext_type out_ext(out.block(), vsip::impl::SYNC_OUT, array_cast<complex_type>(out_buffer_));

  VSIP_IMPL_PROFILE(pm_in_ext_cost_  += in_ext.cost());
  VSIP_IMPL_PROFILE(pm_out_ext_cost_ += out_ext.cost());

  stride_type s_in  =  in_ext.stride(0);
  stride_type s_out = out_ext.stride(0);

  if (Supp == support_full)
  {
    VSIP_IMPL_PROFILE(pm_non_opt_calls_++);
    corr_full(bias, ref_ext.data(), M, 1, in_ext.data(), N, s_in, out_ext.data(), P, s_out);
  }
  else if (Supp == support_same)
  {
    VSIP_IMPL_PROFILE(pm_non_opt_calls_++);
    corr_same(bias, ref_ext.data(), M, 1, in_ext.data(), N, s_in, out_ext.data(), P, s_out);
  }
  // (Supp == support_min)
  else if (bias == biased)
  {
    VSIP_IMPL_PROFILE(pm_non_opt_calls_++);
    corr_min(bias, ref_ext.data(), M, 1, in_ext.data(), N, s_in, out_ext.data(), P, s_out);
  }
  else
  {
    corr(ref_ext.data(), M, in_ext.data(), N, s_in, out_ext.data(), P, s_out, 1);
  }
}

template <template <typename, typename> class ConstViewT,
	  support_region_type Supp,
	  typename            T,
	  unsigned            n_times,
          alg_hint_type       a_hint>
template <typename Block0,
	  typename Block1,
	  typename Block2>
Vector<T, Block2>
Correlation<ConstViewT, Supp, T, n_times, a_hint>::operator()(
  bias_type               bias,
  const_Vector<T, Block0> ref,
  const_Vector<T, Block1> in,
  Vector<T, Block2>       out)
VSIP_NOTHROW
{
  impl_correlate(bias, ref, in, out);
  return out;
}


// Perform 2-D correlation.

template <template <typename, typename> class ConstViewT,
	  support_region_type Supp,
	  typename            T,
	  unsigned            n_times,
          alg_hint_type       a_hint>
template <typename Block0,
	  typename Block1,
	  typename Block2>
void
Correlation<ConstViewT, Supp, T, n_times, a_hint>::impl_correlate(
  bias_type               bias,
  const_Matrix<T, Block0> ref,
  const_Matrix<T, Block1> in,
  Matrix<T, Block2>       out)
VSIP_NOTHROW
{
  length_type const Mr = this->ref_size_[0].size();
  length_type const Mc = this->ref_size_[1].size();
  length_type const Nr = this->input_size_[0].size();
  length_type const Nc = this->input_size_[1].size();
  length_type const Pr = this->output_size_[0].size();
  length_type const Pc = this->output_size_[1].size();

  assert(Pr == out.size(0) && Pc == out.size(1));

  typedef vsip::impl::Ext_data<Block0> ref_ext_type;
  typedef vsip::impl::Ext_data<Block1>  in_ext_type;
  typedef vsip::impl::Ext_data<Block2> out_ext_type;

  ref_ext_type ref_ext(ref.block(), vsip::impl::SYNC_IN,  array_cast<complex_type>(ref_buffer_));
  in_ext_type   in_ext( in.block(), vsip::impl::SYNC_IN,  array_cast<complex_type>( in_buffer_));
  out_ext_type out_ext(out.block(), vsip::impl::SYNC_OUT, array_cast<complex_type>(out_buffer_));

  VSIP_IMPL_PROFILE(pm_in_ext_cost_  += in_ext.cost());
  VSIP_IMPL_PROFILE(pm_out_ext_cost_ += out_ext.cost());

  stride_type ref_row_stride = ref_ext.stride(0);
  stride_type ref_col_stride = ref_ext.stride(1);
  stride_type in_row_stride  = in_ext.stride(0);
  stride_type in_col_stride  = in_ext.stride(1);
  stride_type out_row_stride = out_ext.stride(0);
  stride_type out_col_stride = out_ext.stride(1);

  if (bias == biased)
  {
    VSIP_IMPL_PROFILE(pm_non_opt_calls_++);
    corr_min(
      bias,
      ref_ext.data(), Mr, Mc, ref_row_stride, ref_col_stride,
       in_ext.data(), Nr, Nc,  in_row_stride,  in_col_stride,
      out_ext.data(), Pr, Pc, out_row_stride, out_col_stride
      );
  }
  else
  {
    corr2(
      ref_ext.data(), Mr, Mc, ref_row_stride, ref_col_stride,
       in_ext.data(),          in_row_stride,  in_col_stride,
      out_ext.data(), Pr, Pc, out_row_stride, out_col_stride
      );
  }
}

template <template <typename, typename> class ConstViewT,
	  support_region_type Supp,
	  typename            T,
	  unsigned            n_times,
          alg_hint_type       a_hint>
template <typename Block0,
	  typename Block1,
	  typename Block2>
Matrix<T, Block2>
Correlation<ConstViewT, Supp, T, n_times, a_hint>::operator()(
  bias_type               bias,
  const_Matrix<T, Block0> ref,
  const_Matrix<T, Block1> in,
  Matrix<T, Block2>       out)
VSIP_NOTHROW
{
  impl_correlate(bias, ref, in, out);
  return out;
}



} // namespace vsip::impl::cml

namespace dispatcher
{

template <support_region_type R,
	  unsigned            N,
          alg_hint_type       H>
struct Evaluator<Corr_tag<1, R, float, N, H>, Cml_tag>
{
  static bool const ct_valid = R==support_min;
  typedef cml::Correlation<const_Vector, R, float, N, H> backend_type;
};

template <support_region_type R,
	  unsigned            N,
          alg_hint_type       H>
struct Evaluator<Corr_tag<1, R, std::complex<float>, N, H>, Cml_tag>
{
  static bool const ct_valid = R==support_min;
  typedef cml::Correlation<const_Vector, R, std::complex<float>, N, H> backend_type;
};

template <support_region_type R,
	  unsigned            N,
          alg_hint_type       H>
struct Evaluator<Corr_tag<2, R, float, N, H>, Cml_tag>
{
  static bool const ct_valid = R==support_min;
  typedef cml::Correlation<const_Matrix, R, float, N, H> backend_type;
};

template <support_region_type R,
	  unsigned            N,
          alg_hint_type       H>
struct Evaluator<Corr_tag<2, R, std::complex<float>, N, H>, Cml_tag>
{
  static bool const ct_valid = R==support_min;
  typedef cml::Correlation<const_Matrix, R, std::complex<float>, N, H> backend_type;
};

} // namespace vsip::impl::dispatcher

} // namespace vsip::impl
} // namespace vsip

#endif // VSIP_OPT_CBE_CML_CORR_HPP
