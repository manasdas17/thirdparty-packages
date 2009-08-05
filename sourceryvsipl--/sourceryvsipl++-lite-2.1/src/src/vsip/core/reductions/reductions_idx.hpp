/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved. */

/** @file    vsip/core/reductions/reductions_idx.hpp
    @author  Jules Bergmann
    @date    2005-07-11
    @brief   VSIPL++ Library: Reduction functions returning indices.
	     [math.fns.reductidx].

*/

#ifndef VSIP_CORE_REDUCTIONS_REDUCTIONS_IDX_HPP
#define VSIP_CORE_REDUCTIONS_REDUCTIONS_IDX_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/support.hpp>
#include <vsip/vector.hpp>
#include <vsip/matrix.hpp>
#include <vsip/tensor.hpp>
#include <vsip/core/reductions/functors.hpp>
#include <vsip/core/dispatch.hpp>
#if !VSIP_IMPL_REF_IMPL
#  include <vsip/opt/dispatch.hpp>
#  include <vsip/opt/reductions/par_reductions.hpp>
#  ifdef VSIP_IMPL_HAVE_SAL
#    include <vsip/opt/sal/eval_reductions.hpp>
#  endif
#endif
#if VSIP_IMPL_HAVE_CVSIP
#  include <vsip/core/cvsip/eval_reductions_idx.hpp>
#endif



/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{

namespace impl
{

namespace dispatcher
{

/***********************************************************************
  Generic evaluators.
***********************************************************************/

#ifndef VSIP_IMPL_REF_IMPL
template<template <typename> class ReduceT>
struct List<Op_reduce_idx<ReduceT> >
{
  typedef Make_type_list<Parallel_tag, Cvsip_tag, Mercury_sal_tag,
    Generic_tag>::type type;
};
#endif


/// Generic evaluator for vector reductions.
template <template <typename> class ReduceT,
          typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<1>&, row1_type)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<1>&, row1_type)
  { return true; }

  static void exec(T& r, Block const& a, Index<1>& idx, row1_type)
  {
    typedef typename Block::value_type VT;
    index_type maxi = 0;

    ReduceT<VT> maxv(a.get(maxi));

    for (index_type i=0; i<a.size(1, 0); ++i)
      if (maxv.next_value(a.get(i)))
	maxi = i;

    idx = Index<1>(maxi);
    r =  maxv.value();
  }
};


/// Generic evaluator for matrix reductions (tuple<0, 1, 2>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<2>&, row2_type)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<2>&, row2_type)
  { return true; }

  static void exec(T& r, Block const& a, Index<2>& idx, row2_type)
  {
    typedef typename Block::value_type VT;
    index_type maxi = 0;
    index_type maxj = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj));

    for (index_type i=0; i<a.size(2, 0); ++i)
    for (index_type j=0; j<a.size(2, 1); ++j)
    {
      if (maxv.next_value(a.get(i, j)))
      {
	maxi = i;
	maxj = j;
      }
    }
  
    idx = Index<2>(maxi, maxj);
    r   = maxv.value();
  }
};


/// Generic evaluator for matrix reductions (tuple<2, 1, 0>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<2>&, col2_type)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<2>&, col2_type)
  { return true; }

  static void exec(T& r, Block const& a, Index<2>& idx, col2_type)
  {
    typedef typename Block::value_type VT;
    index_type maxi = 0;
    index_type maxj = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj));

    for (index_type j=0; j<a.size(2, 1); ++j)
    for (index_type i=0; i<a.size(2, 0); ++i)
    {
      if (maxv.next_value(a.get(i, j)))
      {
	maxi = i;
	maxj = j;
      }
    }
  
    idx = Index<2>(maxi, maxj);
    r   = maxv.value();
  }
};


/// Generic evaluator for tensor reductions (tuple<0, 1, 2>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<3>&, tuple<0, 1, 2>)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<3>&, tuple<0, 1, 2>)
  { return true; }

  static void exec(T& r, Block const& a, Index<3>& idx, tuple<0, 1, 2>)
  {
    typedef typename Block::value_type VT;
    
    index_type maxi = 0;
    index_type maxj = 0;
    index_type maxk = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj, maxk));

    for (index_type i=0; i<a.size(3, 0); ++i)
    for (index_type j=0; j<a.size(3, 1); ++j)
    for (index_type k=0; k<a.size(3, 2); ++k)
    {
      if (maxv.next_value(a.get(i, j, k)))
      {
	maxi = i;
	maxj = j;
	maxk = k;
      }
    }

    idx = Index<3>(maxi, maxj, maxk);
    r   = maxv.value();
  }
};


/// Generic evaluator for tensor reductions (tuple<0, 2, 1>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<3>&, tuple<0, 2, 1>)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<3>&, tuple<0, 2, 1>)
  { return true; }

  static void exec(T& r, Block const& a, Index<3>& idx, tuple<0, 2, 1>)
  {
    typedef typename Block::value_type VT;
    
    index_type maxi = 0;
    index_type maxj = 0;
    index_type maxk = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj, maxk));

    for (index_type i=0; i<a.size(3, 0); ++i)
    for (index_type k=0; k<a.size(3, 2); ++k)
    for (index_type j=0; j<a.size(3, 1); ++j)
    {
      if (maxv.next_value(a.get(i, j, k)))
      {
	maxi = i;
	maxj = j;
	maxk = k;
      }
    }

    idx = Index<3>(maxi, maxj, maxk);
    r   = maxv.value();
  }
};


/// Generic evaluator for tensor reductions (tuple<1, 0, 2>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<3>&, tuple<1, 0, 2>)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<3>&, tuple<1, 0, 2>)
  { return true; }

  static void exec(T& r, Block const& a, Index<3>& idx, tuple<1, 0, 2>)
  {
    typedef typename Block::value_type VT;
    
    index_type maxi = 0;
    index_type maxj = 0;
    index_type maxk = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj, maxk));

    for (index_type j=0; j<a.size(3, 1); ++j)
    for (index_type i=0; i<a.size(3, 0); ++i)
    for (index_type k=0; k<a.size(3, 2); ++k)
    {
      if (maxv.next_value(a.get(i, j, k)))
      {
	maxi = i;
	maxj = j;
	maxk = k;
      }
    }

    idx = Index<3>(maxi, maxj, maxk);
    r   = maxv.value();
  }
};


/// Generic evaluator for tensor reductions (tuple<1, 2, 0>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<3>&, tuple<1, 2, 0>)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<3>&, tuple<1, 2, 0>)
  { return true; }

  static void exec(T& r, Block const& a, Index<3>& idx, tuple<1, 2, 0>)
  {
    typedef typename Block::value_type VT;
    
    index_type maxi = 0;
    index_type maxj = 0;
    index_type maxk = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj, maxk));

    for (index_type j=0; j<a.size(3, 1); ++j)
    for (index_type k=0; k<a.size(3, 2); ++k)
    for (index_type i=0; i<a.size(3, 0); ++i)
    {
      if (maxv.next_value(a.get(i, j, k)))
      {
	maxi = i;
	maxj = j;
	maxk = k;
      }
    }

    idx = Index<3>(maxi, maxj, maxk);
    r   = maxv.value();
  }
};


/// Generic evaluator for tensor reductions (tuple<2, 0, 1>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<3>&, tuple<2, 0, 1>)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<3>&, tuple<2, 0, 1>)
  { return true; }

  static void exec(T& r, Block const& a, Index<3>& idx, tuple<2, 0, 1>)
  {
    typedef typename Block::value_type VT;
    
    index_type maxi = 0;
    index_type maxj = 0;
    index_type maxk = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj, maxk));

    for (index_type k=0; k<a.size(3, 2); ++k)
    for (index_type i=0; i<a.size(3, 0); ++i)
    for (index_type j=0; j<a.size(3, 1); ++j)
    {
      if (maxv.next_value(a.get(i, j, k)))
      {
	maxi = i;
	maxj = j;
	maxk = k;
      }
    }

    idx = Index<3>(maxi, maxj, maxk);
    r   = maxv.value();
  }
};


/// Generic evaluator for tensor reductions (tuple<2, 1, 0>).
template <template <typename> class ReduceT,
	  typename                  T,
	  typename                  Block>
struct Evaluator<Op_reduce_idx<ReduceT>, Generic_tag,
                 void(T&, Block const&, Index<3>&, tuple<2, 1, 0>)>
{
  static bool const ct_valid = true;
  static bool rt_valid(T&, Block const&, Index<3>&, tuple<2, 1, 0>)
  { return true; }

  static void exec(T& r, Block const& a, Index<3>& idx, tuple<2, 1, 0>)
  {
    typedef typename Block::value_type VT;
    
    index_type maxi = 0;
    index_type maxj = 0;
    index_type maxk = 0;

    ReduceT<VT> maxv(a.get(maxi, maxj, maxk));

    for (index_type k=0; k<a.size(3, 2); ++k)
    for (index_type j=0; j<a.size(3, 1); ++j)
    for (index_type i=0; i<a.size(3, 0); ++i)
    {
      if (maxv.next_value(a.get(i, j, k)))
      {
	maxi = i;
	maxj = j;
	maxk = k;
      }
    }

    idx = Index<3>(maxi, maxj, maxk);
    r   = maxv.value();
  }
};

} // namespace vsip::impl::dispatcher


template <template <typename> class ReduceT,
	  typename                  ViewT>
typename ReduceT<typename ViewT::value_type>::result_type
reduce_idx(ViewT v, Index<ViewT::dim>& idx)
{
  typedef typename ViewT::value_type T;
  typedef typename ReduceT<T>::result_type result_type;
  typedef typename ViewT::block_type block_type;
  typedef Index<ViewT::dim> index_type;
  typedef typename Block_layout<typename ViewT::block_type>::order_type
		order_type;

  result_type r;

#if VSIP_IMPL_REF_IMPL
  dispatcher::Evaluator<dispatcher::Op_reduce_idx<ReduceT>, Cvsip_tag,
    void(result_type&, block_type const&, index_type&, order_type)>::
    exec(r, v.block(), idx, order_type());
#else

  dispatch<dispatcher::Op_reduce_idx<ReduceT>, void,
    result_type&, block_type const&, index_type&, order_type>
    (r, v.block(), idx, order_type());
#endif

  return r;
}

} // namespace vsip::impl
	     


/***********************************************************************
  API Reduction-Index Functions
***********************************************************************/

template <typename                            T,
	  template <typename, typename> class ViewT,
	  typename                            BlockT>
T
maxval(
   ViewT<T, BlockT>              v,
   Index<ViewT<T, BlockT>::dim>& idx)
VSIP_NOTHROW
{
  typedef typename impl::Block_layout<BlockT>::order_type order_type;
  return impl::reduce_idx<impl::Max_value>(v, idx);
}



template <typename                            T,
	  template <typename, typename> class ViewT,
	  typename                            BlockT>
T
minval(
   ViewT<T, BlockT>              v,
   Index<ViewT<T, BlockT>::dim>& idx)
VSIP_NOTHROW
{
  typedef typename impl::Block_layout<BlockT>::order_type order_type;
  return impl::reduce_idx<impl::Min_value>(v, idx);
}



template <typename                            T,
	  template <typename, typename> class ViewT,
	  typename                            BlockT>
typename impl::Scalar_of<T>::type
maxmgval(
   ViewT<T, BlockT>              v,
   Index<ViewT<T, BlockT>::dim>& idx)
VSIP_NOTHROW
{
  typedef typename impl::Block_layout<BlockT>::order_type order_type;
  return impl::reduce_idx<impl::Max_mag_value>(v, idx);
}



template <typename                            T,
	  template <typename, typename> class ViewT,
	  typename                            BlockT>
typename impl::Scalar_of<T>::type
minmgval(
   ViewT<T, BlockT>              v,
   Index<ViewT<T, BlockT>::dim>& idx)
VSIP_NOTHROW
{
  typedef typename impl::Block_layout<BlockT>::order_type order_type;
  return impl::reduce_idx<impl::Min_mag_value>(v, idx);
}



template <typename                            T,
	  template <typename, typename> class ViewT,
	  typename                            BlockT>
T
maxmgsqval(
   ViewT<complex<T>, BlockT>              v,
   Index<ViewT<complex<T>, BlockT>::dim>& idx)
VSIP_NOTHROW
{
  typedef typename impl::Block_layout<BlockT>::order_type order_type;
  return impl::reduce_idx<impl::Max_magsq_value>(v, idx);
}



template <typename                            T,
	  template <typename, typename> class ViewT,
	  typename                            BlockT>
T
minmgsqval(
   ViewT<complex<T>, BlockT>              v,
   Index<ViewT<complex<T>, BlockT>::dim>& idx)
VSIP_NOTHROW
{
  typedef typename impl::Block_layout<BlockT>::order_type order_type;
  return impl::reduce_idx<impl::Min_magsq_value>(v, idx);
}

} // namespace vsip

#endif // VSIP_CORE_REDUCTIONS_REDUCTIONS_IDX_HPP
