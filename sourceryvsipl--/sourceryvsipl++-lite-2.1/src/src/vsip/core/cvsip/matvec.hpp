/* Copyright (c) 2006 by CodeSourcery.  All rights reserved. */

/** @file    vsip/core/cvsip/matvec.hpp
    @author  Stefan Seefeld
    @date    2006-10-25
    @brief   VSIPL++ Library: C-VSIPL bindings for matvec operations.

*/

#ifndef VSIP_CORE_CVSIP_MATVEC_HPP
#define VSIP_CORE_CVSIP_MATVEC_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/core/config.hpp>
#include <vsip/core/order_traits.hpp>
#include <vsip/core/cvsip/block.hpp>
#include <vsip/core/cvsip/view.hpp>
#include <vsip/core/coverage.hpp>
extern "C" {
#include <vsip.h>
}

namespace vsip
{
namespace impl
{

namespace cvsip
{

template <typename T> struct Op_traits { static bool const valid = false;};

#if VSIP_IMPL_CVSIP_HAVE_FLOAT
template <>
struct Op_traits<float>
{
  typedef float value_type;
  typedef vsip_vview_f vector_type;
  typedef vsip_mview_f matrix_type;

  static bool const valid = true;

  static value_type dot(vector_type const *a, vector_type const *b)
  { return vsip_vdot_f(a, b);}
  static void outer(value_type s, vector_type const *a, vector_type const *b,
                    matrix_type *r)
  { vsip_vouter_f(s, a, b, r);}
  static void gemp(matrix_type *c, value_type alpha,
                   matrix_type *a, matrix_type *b, value_type beta)
  { vsip_gemp_f(alpha, a, VSIP_MAT_NTRANS, b, VSIP_MAT_NTRANS, beta, c);}
  static void prod(matrix_type *a, matrix_type *b, matrix_type *c)
  { vsip_mprod_f(a, b, c);}
  static void prod(matrix_type *a, vector_type *x, vector_type *y)
  { vsip_mvprod_f(a, x, y);}
};

template <>
struct Op_traits<std::complex<float> >
{
  typedef std::complex<float> value_type;
  typedef vsip_cvview_f vector_type;
  typedef vsip_cmview_f matrix_type;

  static bool const valid = true;

  static value_type dot(vector_type const *a, vector_type const *b)
  {
    vsip_cscalar_f retn = vsip_cvdot_f(a, b);
    return value_type(retn.r, retn.i);
  }
  static value_type cvjdot(vector_type const *a, vector_type const *b)
  {
    vsip_cscalar_f retn = vsip_cvjdot_f(a, b);
    return value_type(retn.r, retn.i);
  }
  static void outer(value_type s, vector_type const *a, vector_type const *b,
                    matrix_type *r)
  {
    vsip_cscalar_f scale = {s.real(), s.imag()};
    vsip_cvouter_f(scale, a, b, r);
  }
  static void gemp(matrix_type *c, value_type alpha,
                   matrix_type *a, matrix_type *b, value_type beta)
  {
    vsip_cscalar_f al = {alpha.real(), alpha.imag()};
    vsip_cscalar_f be = {beta.real(), beta.imag()};
    vsip_cgemp_f(al, a, VSIP_MAT_NTRANS, b, VSIP_MAT_NTRANS, be, c);
  }
  static void prod(matrix_type *a, matrix_type *b, matrix_type *c)
  { vsip_cmprod_f(a, b, c);}
  static void prod(matrix_type *a, vector_type *x, vector_type *y)
  { vsip_cmvprod_f(a, x, y);}
};
#endif
#if VSIP_IMPL_CVSIP_HAVE_DOUBLE
template <>
struct Op_traits<double>
{
  typedef double value_type;
  typedef vsip_vview_d vector_type;
  typedef vsip_mview_d matrix_type;

  static bool const valid = true;

  static value_type dot(vector_type const *a, vector_type const *b)
  { return vsip_vdot_d(a, b);}
  static void outer(value_type s, vector_type const *a, vector_type const *b,
                    matrix_type *r)
  { vsip_vouter_d(s, a, b, r);}
  static void gemp(matrix_type *c, value_type alpha,
                   matrix_type *a, matrix_type *b, value_type beta)
  { vsip_gemp_d(alpha, a, VSIP_MAT_NTRANS, b, VSIP_MAT_NTRANS, beta, c);}
  static void prod(matrix_type *a, matrix_type *b, matrix_type *c)
  { vsip_mprod_d(a, b, c);}
  static void prod(matrix_type *a, vector_type *x, vector_type *y)
  { vsip_mvprod_d(a, x, y);}
};

template <>
struct Op_traits<std::complex<double> >
{
  typedef std::complex<double> value_type;
  typedef vsip_cvview_d vector_type;
  typedef vsip_cmview_d matrix_type;

  static bool const valid = true;

  static value_type dot(vector_type const *a, vector_type const *b)
  {
    vsip_cscalar_d retn = vsip_cvdot_d(a, b);
    return value_type(retn.r, retn.i);
  }  
  static value_type cvjdot(vector_type const *a, vector_type const *b)
  {
    vsip_cscalar_d retn = vsip_cvjdot_d(a, b);
    return value_type(retn.r, retn.i);
  }  
  static void outer(value_type s, vector_type const *a, vector_type const *b,
                    matrix_type *r)
  {
    vsip_cscalar_d scale = {s.real(), s.imag()};
    vsip_cvouter_d(scale, a, b, r);
  }
  static void gemp(matrix_type *c, value_type alpha,
                   matrix_type *a, matrix_type *b, value_type beta)
  {
    vsip_cscalar_d al = {alpha.real(), alpha.imag()};
    vsip_cscalar_d be = {beta.real(), beta.imag()};
    vsip_cgemp_d(al, a, VSIP_MAT_NTRANS, b, VSIP_MAT_NTRANS, be, c);
  }
  static void prod(matrix_type *a, matrix_type *b, matrix_type *c)
  { vsip_cmprod_d(a, b, c);}
  static void prod(matrix_type *a, vector_type *x, vector_type *y)
  { vsip_cmvprod_d(a, x, y);}
};
#endif

} // namespace vsip::impl::cvsip

namespace dispatcher
{

template <typename T,
          typename Block0,
          typename Block1>
struct Evaluator<Op_prod_vv_dot, Cvsip_tag,
                 T(Block0 const&, Block1 const&)>
{
  typedef cvsip::Op_traits<T> traits;

  // Note: C-VSIPL backends set ct_valid to false if the block types don't 
  // allow direct data access (i.e. Ext_data_cost > 0), which occurs when
  // an operator like conj() is used (see cvjdot()).  This prevents this
  // evaluator from being used if invoked through the normal dispatch 
  // mechanism.  For the reference implementation, we call the backend 
  // explicitly, which is ok, since the Ext_data handles the layout 
  // adjustments.

  static bool const ct_valid = 
    traits::valid &&
    Type_equal<T, typename Block0::value_type>::value &&
    Type_equal<T, typename Block1::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0;

  static bool rt_valid(Block0 const&, Block1 const&) { return true;}

  static T exec(Block0 const& a, Block1 const& b)
  {
    VSIP_IMPL_COVER_FCN("Op_prod_vv_dot/cvsip", exec);
    assert(a.size(1, 0) == b.size(1, 0));

    Ext_data<Block0 const> ext_a(a);
    Ext_data<Block1 const> ext_b(b);
    cvsip::View<1, T> aview(ext_a.data(), 0, ext_a.stride(0), a.size(1, 0));
    cvsip::View<1, T> bview(ext_b.data(), 0, ext_b.stride(0), b.size(1, 0));
    return traits::dot(aview.ptr(), bview.ptr());
  }
};


template <typename T,
          typename Block0,
          typename Block1>
struct Evaluator<Op_prod_vv_dot, Cvsip_tag,
                 std::complex<T>(Block0 const&, 
                   Unary_expr_block<1, conj_functor, Block1, std::complex<T> > const&)>
{
  typedef cvsip::Op_traits<std::complex<T> > traits;
  typedef Unary_expr_block<1, conj_functor, Block1, std::complex<T> > block1_type;

  static bool const ct_valid = 
    traits::valid &&
    Type_equal<complex<T>, typename Block0::value_type>::value &&
    Type_equal<complex<T>, typename Block1::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0;

  static bool rt_valid(Block0 const&, block1_type const&)
  { return true; }

  static complex<T> exec(Block0 const& a, block1_type const& b)
  {
    VSIP_IMPL_COVER_FCN("Op_prod_vv_dot(conj)/cvsip", exec);
    assert(a.size(1, 0) == b.size(1, 0));

    Ext_data<Block0 const> ext_a(a);
    Ext_data<Block1 const> ext_b(b.op());
    cvsip::View<1, std::complex<T> >
      aview(ext_a.data(), 0, ext_a.stride(0), a.size(1, 0));
    cvsip::View<1, std::complex<T> >
      bview(ext_b.data(), 0, ext_b.stride(0), b.size(1, 0));
    return traits::cvjdot(aview.ptr(), bview.ptr());
  }
};


template <typename T,
          typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_vv_outer, Cvsip_tag,
                 void(Block0&, T, Block1 const&, Block2 const&)>
{
  typedef cvsip::Op_traits<T> traits;

  static bool const ct_valid = 
    traits::valid &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0;

  static bool rt_valid(Block0&, T, Block1 const&, Block2 const&)
  { return true;}

  static void exec(Block0& r, T s, Block1 const& a, Block2 const& b)
  {
    VSIP_IMPL_COVER_FCN("Op_prod_vv_outer/cvsip", exec);
    typedef typename Block_layout<Block0>::order_type order0_type;
    VSIP_IMPL_STATIC_ASSERT((Is_order_valid<2, order0_type>::value));

    assert(a.size(1, 0) == r.size(2, 0));
    assert(b.size(1, 0) == r.size(2, 1));

    Ext_data<Block0> ext_r(r);
    Ext_data<Block1 const> ext_a(a);
    Ext_data<Block2 const> ext_b(b);

    cvsip::View<1, T> aview(ext_a.data(), 0, ext_a.stride(0), a.size(1, 0));
    cvsip::View<1, T> bview(ext_b.data(), 0, ext_b.stride(0), b.size(1, 0));
    cvsip::View<2, T> rview(ext_r.data(), 0,
                            ext_r.stride(0), r.size(2, 0),
                            ext_r.stride(1), r.size(2, 1));

    traits::outer(s, aview.ptr(), bview.ptr(), rview.ptr());
  }
};

template <typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_vm, Cvsip_tag,
                 void(Block0&, Block1 const&, Block2 const&)>
{
  typedef typename Block0::value_type T;
  typedef cvsip::Op_traits<T> traits;

  static bool const ct_valid = 
    traits::valid &&
    Type_equal<T, typename Block0::value_type>::value &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0;

  static bool rt_valid(Block0& y, Block1 const& x, Block2 const& a)
  { return true;}

  static void exec(Block0& y, Block1 const& x, Block2 const& a)
  {
    VSIP_IMPL_COVER_FCN("Op_prod_vm/cvsip", exec);
    assert(x.size(1, 0) == a.size(2, 0));

    Ext_data<Block0> ext_y(y);
    Ext_data<Block1 const> ext_x(x);
    Ext_data<Block2 const> ext_a(a);

    cvsip::View<1, T> yview(ext_y.data(), 0, ext_y.stride(0), y.size(1, 0));
    cvsip::View<1, T> xview(ext_x.data(), 0, ext_x.stride(0), x.size(1, 0));
    cvsip::View<2, T> aview(ext_a.data(), 0,
                            ext_a.stride(1), a.size(2, 1),
                            ext_a.stride(0), a.size(2, 0));

    traits::prod(aview.ptr(), xview.ptr(), yview.ptr());
  }
};

template <typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_mv, Cvsip_tag, 
                 void(Block0&, Block1 const&, Block2 const&)>
{
  typedef typename Block0::value_type T;
  typedef typename Block_layout<Block1>::order_type order1_type;
  typedef cvsip::Op_traits<T> traits;

  static bool const ct_valid = 
    traits::valid &&
    Type_equal<T, typename Block0::value_type>::value &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0;

  static bool rt_valid(Block0&, Block1 const&, Block2 const&)
  { return true;}

  static void exec(Block0& y, Block1 const& a, Block2 const& x)
  {
    VSIP_IMPL_COVER_FCN("Op_prod_mv/cvsip", exec);
    assert(x.size(1, 0) == a.size(2, 1));

    Ext_data<Block0> ext_y(y);
    Ext_data<Block1 const> ext_a(a);
    Ext_data<Block2 const> ext_x(x);

    cvsip::View<1, T> yview(ext_y.data(), 0, ext_y.stride(0), y.size(1, 0));
    cvsip::View<2, T> aview(ext_a.data(), 0,
                            ext_a.stride(0), a.size(2, 0),
                            ext_a.stride(1), a.size(2, 1));
    cvsip::View<1, T> xview(ext_x.data(), 0, ext_x.stride(0), x.size(1, 0));

    traits::prod(aview.ptr(), xview.ptr(), yview.ptr());
  }
};

template <typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_mm, Cvsip_tag,
                 void(Block0&, Block1 const&, Block2 const&)>
{
  typedef typename Block0::value_type T;
  typedef cvsip::Op_traits<T> traits;

  static bool const ct_valid = 
    traits::valid &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0;

  static bool rt_valid(Block0&, Block1 const&, Block2 const&)
  { return true;}

  static void exec(Block0& c, Block1 const& a, Block2 const& b)
  {
    VSIP_IMPL_COVER_FCN("Op_prod_mm/cvsip", exec);
    Ext_data<Block0> ext_c(c);
    Ext_data<Block1 const> ext_a(a);
    Ext_data<Block2 const> ext_b(b);

    cvsip::View<2, T> cview(ext_c.data(), 0,
                            ext_c.stride(0), c.size(2, 0),
                            ext_c.stride(1), c.size(2, 1));
    cvsip::View<2, T> aview(ext_a.data(), 0,
                            ext_a.stride(0), a.size(2, 0),
                            ext_a.stride(1), a.size(2, 1));
    cvsip::View<2, T> bview(ext_b.data(), 0,
                            ext_b.stride(0), b.size(2, 0),
                            ext_b.stride(1), b.size(2, 1));
    traits::prod(aview.ptr(), bview.ptr(), cview.ptr());
  }
};

template <typename T,
          typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_gemp, Cvsip_tag,
                 void(Block0&, T, Block1 const&, Block2 const&, T)>
{
  typedef cvsip::Op_traits<T> traits;

  static bool const ct_valid = 
    traits::valid &&
    Type_equal<T, typename Block0::value_type>::value &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0;

  static bool rt_valid(Block0&, T, Block1 const&, Block2 const&, T)
  { return true;}
  static void exec(Block0& c, T alpha,
                   Block1 const& a, Block2 const& b, T beta)
  {
    VSIP_IMPL_COVER_FCN("Op_prod_gemp/cvsip", exec);
    Ext_data<Block0> ext_c(c);
    Ext_data<Block1 const> ext_a(a);
    Ext_data<Block2 const> ext_b(b);

    cvsip::View<2, T> cview(ext_c.data(), 0,
                            ext_c.stride(0), c.size(2, 0),
                            ext_c.stride(1), c.size(2, 1));
    cvsip::View<2, T> aview(ext_a.data(), 0,
                            ext_a.stride(0), a.size(2, 0),
                            ext_a.stride(1), a.size(2, 1));
    cvsip::View<2, T> bview(ext_b.data(), 0,
                            ext_b.stride(0), b.size(2, 0),
                            ext_b.stride(1), b.size(2, 1));
    traits::gemp(cview.ptr(), alpha, aview.ptr(), bview.ptr(), beta);
  }
};

} // namespace vsip::impl::dispatcher

} // namespace vsip::impl
} // namespace vsip

#endif
