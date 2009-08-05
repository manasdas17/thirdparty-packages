/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip_csl/load_view.hpp
    @author  Jules Bergmann
    @date    2005-09-30
    @brief   VSIPL++ CodeSourcery Library: Utility to load a view from disk.
*/

#ifndef VSIP_CSL_LOAD_VIEW_HPP
#define VSIP_CSL_LOAD_VIEW_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>
#include <string.h>
#include <errno.h>
#include <memory>

#include <vsip/vector.hpp>
#include <vsip/matrix.hpp>
#include <vsip/tensor.hpp>
#include <vsip/core/noncopyable.hpp>
#include <vsip/core/working_view.hpp>
#include <vsip/core/view_cast.hpp>

#include <vsip_csl/matlab.hpp>


namespace vsip_csl
{

/***********************************************************************
  Definitions
***********************************************************************/

/// Load values from a file descriptor into a VSIPL++ view.

/// Note: assumes complex data on disk is always interleaved. 

template <typename ViewT>
void
load_view(
  FILE* fd,
  ViewT view,
  bool  swap_bytes = false)
{
  using vsip::impl::Block_layout;
  using vsip::impl::Ext_data;
  using vsip::impl::Adjust_layout_complex;
  using vsip::impl::Cmplx_inter_fmt;

  if (subblock(view) != vsip::no_subblock && subblock_domain(view).size() > 0)
  {
    vsip::dimension_type const Dim = ViewT::dim;

    typedef typename ViewT::value_type       value_type;
    typedef typename ViewT::local_type       l_view_type;
    typedef typename l_view_type::block_type l_block_type;
    typedef typename Block_layout<l_block_type>::order_type order_type;

    typedef typename Block_layout<l_block_type>::layout_type layout_type;
    typedef typename Adjust_layout_complex<Cmplx_inter_fmt, layout_type>::type
      use_layout_type;

    l_view_type l_view = view.local();

    vsip::Domain<Dim> g_dom = global_domain(view);
    vsip::Domain<Dim> l_dom = subblock_domain(view);

    Ext_data<l_block_type, use_layout_type> ext(l_view.block());

    // Check that subblock is dense.
    if (!vsip::impl::is_ext_dense<order_type>(Dim, ext))
      VSIP_IMPL_THROW(vsip::impl::unimplemented(
	"load_view can only handle dense subblocks"));

    long l_pos = 0;

    if (Dim >= 1)
    {
      l_pos += g_dom[order_type::impl_dim0].first();
    }

    if (Dim >= 2)
    {
      l_pos *= g_dom[order_type::impl_dim1].size();
      l_pos += g_dom[order_type::impl_dim1].first();
    }

    if (Dim >= 3)
    {
      l_pos *= g_dom[order_type::impl_dim2].size();
      l_pos += g_dom[order_type::impl_dim2].first();
    }

    l_pos *= sizeof(value_type);

    size_t l_size = l_dom.size();

    if (fseek(fd, l_pos, SEEK_SET) == -1)
    {
      fprintf(stderr, "load_view: error on fseek.\n");
      exit(1);
    }

    size_t l_read = fread(ext.data(), sizeof(value_type), l_size, fd);
    if (l_read != l_size)
    {
      std::cout << "load_view: error reading file %s." << std::endl;
      std::cout << "         : read " << l_read << " elements" << std::endl;
      std::cout << "         : expecting " << l_size << std::endl;
      exit(1);
    }

    // Swap from either big- to little-endian, or vice versa.  We can do this
    // as if it were a 1-D view because it is guaranteed to be dense.
    if ( swap_bytes )
    {
      value_type* p_data = ext.data();
      for (size_t i = 0; i < l_size; ++i)
        matlab::Swap_value<value_type,true>::swap(p_data++);
    }
  }
}



/// Load values from a file into a VSIPL++ view.

template <typename ViewT>
void
load_view(
  char const* filename,
  ViewT       view,
  bool        swap_bytes = false)
{
  if (subblock(view) != vsip::no_subblock && subblock_domain(view).size() > 0)
  {
    FILE*  fd;
    
    if (!(fd = fopen(filename, "r")))
    {
      fprintf(stderr, "load_view: error opening '%s'.\n", filename);
      exit(1);
    }

    load_view(fd, view, swap_bytes);

    fclose(fd);
  }
}



/// Load a view from a file with another value type.

/// Requires:
///   T to be the type on disk.
///   FILENAME to be filename.
///   VIEW to be a VSIPL++ view.
///
/// All other layout parameters (dimension-ordering and parallel
/// distribution) are preserved.

template <typename T,
          typename ViewT>
void
load_view_as(
  char const* filename,
  ViewT       view,
  bool        swap_bytes = false)
{
  using vsip::impl::View_of_dim;
  using vsip::impl::Block_layout;
  using vsip::impl::clone_view;

  typedef typename ViewT::block_type                    block_type;
  typedef typename Block_layout<block_type>::order_type order_type;
  typedef typename ViewT::block_type::map_type          map_type;

  typedef vsip::Dense<ViewT::dim, T, order_type, map_type> new_block_type;

  typedef
    typename View_of_dim<ViewT::dim, T, new_block_type>::type
    view_type;

  view_type disk_view = clone_view<view_type>(view, view.block().map());

  load_view(filename, disk_view, swap_bytes);

  view = vsip::impl::view_cast<typename ViewT::value_type>(disk_view);
} 




/// Load values from a file into a VSIPL++ view.

/// Requires
///   DIM to be the dimension of the data/view,
///   T to be the value type of the data/view
///   ORDERT to be the dimension ordering of the data and view
///      (row-major by default).
///   MAPT to be the mapping of the view
///      (Local_map by default).

template <vsip::dimension_type Dim,
	  typename          T,
	  typename          OrderT = typename vsip::impl::Row_major<Dim>::type,
	  typename          MapT = vsip::Local_map>
class Load_view : vsip::impl::Non_copyable
{
public:
  typedef T value_type;
  typedef vsip::Dense<Dim, T, OrderT, MapT> block_type;
  typedef typename vsip::impl::View_of_dim<Dim, T, block_type>::type view_type;

public:
  Load_view(char const*              filename,
	    vsip::Domain<Dim> const& dom,
            MapT const&              map = MapT(),
            bool                     swap_bytes = false)
    : block_ (dom, map),
      view_  (block_)
  {
    load_view(filename, view_, swap_bytes);
  }



  Load_view(FILE*                    fd,
	    vsip::Domain<Dim> const& dom,
            MapT const&              map = MapT(),
            bool                     swap_bytes = false)
    : block_ (dom, map),
      view_  (block_)
  {
    load_view(fd, view_, swap_bytes);
  }

  view_type view() { return view_; }

private:
  block_type                block_;
  view_type                 view_;
};





} // namespace vsip_csl

#endif // VSIP_CSL_LOAD_VIEW_HPP
