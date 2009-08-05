/* Copyright (c) 2005, 2006 by CodeSourcery, LLC.  All rights reserved. */

/** @file    vsip/core/layout.hpp
    @author  Jules Bergmann
    @date    2005-04-08
    @brief   VSIPL++ Library: Data layout within a block.
*/

#ifndef VSIP_CORE_LAYOUT_HPP
#define VSIP_CORE_LAYOUT_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/core/complex_decl.hpp>
#include <vsip/domain.hpp>
#include <vsip/core/length.hpp>
#include <vsip/core/metaprogramming.hpp>

/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{
namespace impl
{

/// Enum to indicate that an Applied_layout or Rt_layout object
/// will not be used and therefor should not be initialized when
/// constructed.
enum empty_layout_type { empty_layout };



/// Class to represent either a interleaved-pointer or a split-pointer.
///
/// Primary definition handles non-complex types.  Functions
/// corresponding to split-pointer cause a runtime error, since
/// "split" does not make sense for non-complex types.
template <typename T>
class Rt_pointer
{
  // Constructors
public:
  // Default constructor creates a NULL pointer.
  Rt_pointer() : ptr_(0) {}

  Rt_pointer(T* ptr) : ptr_(ptr) {}
  Rt_pointer(std::pair<T*, T*> const&) { assert(0); }


  // Accessors.
public:
  T*               as_real() { return ptr_; }
  T*               as_inter() { return ptr_; }
  std::pair<T*,T*> as_split() { assert(0); return std::pair<T*,T*>(0,0); }

  bool is_null() { return ptr_ == 0; }

  // Member data.
private:
  T* ptr_;
};



/// Specialization for complex types.  Whether a Rt_pointer refers to a
/// interleaved or split pointer is determined by the using code.
/// (However, when initializing an interleaved pointer, `ptr1_` is set
/// to 0).
template <typename T>
class Rt_pointer<complex<T> >
{
  // Constructors.
public:
  // Default constructor creates a NULL pointer.
  Rt_pointer() : ptr0_(0), ptr1_(0) {}

  Rt_pointer(complex<T>* ptr)
    : ptr0_(reinterpret_cast<T*>(ptr)), ptr1_(0)
  {}

  Rt_pointer(std::pair<T*, T*> const& ptr)
    : ptr0_(ptr.first), ptr1_(ptr.second)
  {}

  // Acccessors
public:
  T*               as_real() { assert(0); return 0; }
  complex<T>*       as_inter() { return reinterpret_cast<complex<T>*>(ptr0_); }
  std::pair<T*, T*> as_split() { return std::pair<T*,T*>(ptr0_, ptr1_); }

  bool is_null() { return ptr0_ == 0; }

  // Member data.
private:
  T* ptr0_;
  T* ptr1_;
};



/// Runtime dimension-order (corresponds to compile-time tuples).
///
/// Member names are chosen to correspond to tuple's.
///
class Rt_tuple
{
  // Constructors.
public:
  Rt_tuple()
    : impl_dim0(0), impl_dim1(1), impl_dim2(2)
  {}

  Rt_tuple(dimension_type d0, dimension_type d1, dimension_type d2)
    : impl_dim0(d0), impl_dim1(d1), impl_dim2(d2)
  {}

  // Convenience constructor from a compile-time tuple.
  template <dimension_type D0,
	    dimension_type D1,
	    dimension_type D2>
  Rt_tuple(tuple<D0, D1, D2>)
    : impl_dim0(D0), impl_dim1(D1), impl_dim2(D2)
  {}

  // Member data.
public:
  dimension_type impl_dim0;
  dimension_type impl_dim1;
  dimension_type impl_dim2;
};



struct Any_type;


/// Stride_unknown indicates that at compile-time, minor dimension
/// stride is unknown.
struct Stride_unknown
{ static bool const is_ct_unit_stride = false; };

/// Stride_unit indicates that at compile-time, minor dimension is
/// known to be unit-stride.
struct Stride_unit
{ static bool const is_ct_unit_stride = true; };

/// Stride_unit_dense indicates that at compile-time, minor dimension
/// is known to be unit-stride and major dimensions are known to be
/// dense (contiguous).
struct Stride_unit_dense
{ static bool const is_ct_unit_stride = true; };

/// Stride_unit_align indicates that at compile-time, minor dimension
/// is known to be unit-stride and major dimensions are known to
/// start on aligned boundaries.
template <unsigned Align> struct Stride_unit_align
{ static bool const is_ct_unit_stride = true; };


/// Runtime packing format enum.
enum rt_pack_type
{
  stride_unknown,
  stride_unit,
  stride_unit_dense,
  stride_unit_align
};

template <typename PackFmt>
struct Is_stride_unit_align
{
  static bool     const value = false;
  static unsigned const align = 0;
};

template <unsigned Align>
struct Is_stride_unit_align<Stride_unit_align<Align> >
{
  static bool     const value = true;
  static unsigned const align = Align;
};



/// Cmplx_inter_fmt indicates that complex data is stored as an
/// array of type complex, which is assumed to be interleaved.
struct Cmplx_inter_fmt {};
/// Cmplx_split_fmt indicates that complex data is stored in
/// separate arrays of real and imaginary data.
struct Cmplx_split_fmt {};

/// Runtime complex format enum.
enum rt_complex_type
{
  cmplx_split_fmt,
  cmplx_inter_fmt
};



/// Validity check for packing format tags.
template <typename T>
struct Valid_pack_type;

template <> struct Valid_pack_type<Any_type> {};
template <> struct Valid_pack_type<Stride_unit> {};
template <> struct Valid_pack_type<Stride_unit_dense> {};
template <> struct Valid_pack_type<Stride_unknown> {};
template <unsigned Align> struct Valid_pack_type<Stride_unit_align<Align> > {};



/// Validity check for complex format tags.
template <typename T>
struct Valid_complex_fmt;

template <> struct Valid_complex_fmt<Any_type> {};
template <> struct Valid_complex_fmt<Cmplx_inter_fmt> {};
template <> struct Valid_complex_fmt<Cmplx_split_fmt> {};



/// Validity check for dimension-ordering tuples.
template <typename T>
struct Valid_order;

template <> struct Valid_order<Any_type> {};
template <dimension_type Dim0,
	  dimension_type Dim1,
	  dimension_type Dim2>
struct Valid_order<tuple<Dim0, Dim1, Dim2> > {};




/// Layout class encapsulating:
///
///  - Dimension,
///  - Dimension ordering,
///  - Packing format,
///  - Complex format.
template <dimension_type D,
	  typename       Order,
	  typename       PackType,
	  typename	 ComplexType = Cmplx_inter_fmt>
struct Layout
  : Valid_pack_type<PackType>
  , Valid_order<Order>
#if !(defined(__GNUC__) && __GNUC__ < 4)
  // G++ 3.4.4 enters an infinite loop processing this compile-time
  // assertion (060505).  G++ 4.1 is OK.
  , Valid_complex_fmt<ComplexType>
#endif
{
  static dimension_type const dim = D;
  typedef PackType    pack_type;
  typedef Order       order_type;
  typedef ComplexType complex_type;
};



/// Runtime layout class encapsulating:
///  - Dimension,
///  - Dimension ordering,
///  - Packing format,
///  - Complex format.
template <dimension_type Dim>
struct Rt_layout
{
  // Dimension is fixed at compile-time.
  static dimension_type const dim = Dim;

  // Run-time layout.
  rt_pack_type      pack;
  Rt_tuple          order;
  rt_complex_type   complex;
  unsigned          align;	// Only valid if pack == stride_unit_align


  // Construct an empty Rt_layout object.
  Rt_layout() {}

  Rt_layout(
    rt_pack_type    a_pack,
    Rt_tuple const& a_order,
    rt_complex_type a_complex,
    unsigned        a_align)
  : pack    (a_pack),
    order   (a_order),
    complex (a_complex),
    align   (a_align)
  {}


};



/// Applied_layout takes the layout policies encapsulated by a
/// Layout and applys them to map multi-dimensional indices into
/// memory offsets.
template <typename LP>
struct Applied_layout;



template <typename Order,
	  typename ComplexLayout>
class Applied_layout<Layout<1, Order, Stride_unit_dense, ComplexLayout> >
{
public:
  static dimension_type const dim = 1;
  typedef tuple<0, 1, 2>  order_type;
  typedef Stride_unit_dense pack_type;
  typedef ComplexLayout   complex_type;

public:
  Applied_layout(length_type size0)
  {
    size_[0] = size0;
  }

  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
  }

  index_type index(index_type idx0)
    const VSIP_NOTHROW
  {
    assert(idx0 < size_[0]);
    return idx0;
  }

  index_type index(Index<1> idx)
    const VSIP_NOTHROW
    { return idx[0]; }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
    { assert(d <= dim); return 1; }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { assert(d <= dim); return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
    { return size_[0]; }

private:
  length_type size_[dim];
};



template <unsigned Align,
	  typename Order,
	  typename ComplexLayout>
class Applied_layout<Layout<1, Order, Stride_unit_align<Align>, ComplexLayout> >
{
public:
  static dimension_type const dim = 1;
  typedef Order           order_type;
  typedef Stride_unit_dense pack_type;
  typedef ComplexLayout   complex_type;

public:
  Applied_layout(length_type size0)
  {
    size_[0] = size0;
  }

  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
  }

  index_type index(index_type idx0)
    const VSIP_NOTHROW
  {
    assert(idx0 < size_[0]);
    return idx0;
  }

  index_type index(Index<1> idx)
    const VSIP_NOTHROW
    { return idx[0]; }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
    { assert(d <= dim); return 1; }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { assert(d <= dim); return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
    { return size_[0]; }

private:
  length_type size_[dim];
};



template <typename Order,
	  typename ComplexLayout>
class Applied_layout<Layout<1, Order, Stride_unit, ComplexLayout> >
{
public:
  static dimension_type const dim = 1;
  typedef tuple<0, 1, 2>  order_type;
  typedef Stride_unit_dense pack_type;
  typedef ComplexLayout   complex_type;

public:
  Applied_layout(length_type size0)
  {
    size_[0] = size0;
  }

  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
  }

  index_type index(index_type idx0)
    const VSIP_NOTHROW
  {
    assert(idx0 < size_[0]);
    return idx0;
  }

  index_type index(Index<1> idx)
    const VSIP_NOTHROW
    { return idx[0]; }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
    { assert(d <= dim); return 1; }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { assert(d <= dim); return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
    { return size_[0]; }

private:
  length_type size_[dim];
};



template <typename ComplexLayout>
class Applied_layout<Layout<2, tuple<0, 1, 2>, Stride_unit_dense, ComplexLayout> >
{
public:
  static dimension_type const dim = 2;
  typedef tuple<0, 1, 2>  order_type;
  typedef Stride_unit_dense pack_type;
  typedef ComplexLayout   complex_type;

public:
  Applied_layout(length_type size0, length_type size1)
  {
    size_[0] = size0;
    size_[1] = size1;
  }

  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
    size_[1] = size_of_dim(extent, 1);
  }

  index_type index(index_type idx0, index_type idx1)
    const VSIP_NOTHROW
  {
    assert(idx0 < size_[0] && idx1 < size_[1]);
    return idx0 * size_[1] + idx1;
  }

  index_type index(Index<2> idx)
    const VSIP_NOTHROW
    { return idx[0] * size_[1] + idx[1]; }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
    { return d == 0 ? size_[1] : 1; }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
    { return size_[1] * size_[0]; }

private:
  length_type size_[2];
};



template <typename ComplexLayout>
class Applied_layout<Layout<2, tuple<1, 0, 2>, Stride_unit_dense, ComplexLayout> >
{
public:
  static dimension_type const dim = 2;
  typedef tuple<1, 0, 2>  order_type;
  typedef Stride_unit_dense pack_type;
  typedef ComplexLayout   complex_type;

public:
  Applied_layout(length_type size0, length_type size1)
  {
    size_[0] = size0;
    size_[1] = size1;
  }

  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
    size_[1] = size_of_dim(extent, 1);
  }

  index_type index(index_type idx0, index_type idx1)
    const VSIP_NOTHROW
  {
    assert(idx0 < size_[0] && idx1 < size_[1]);
    return idx0 + idx1 * size_[0];
  }

  index_type index(Index<2> idx)
    const VSIP_NOTHROW
    { return idx[0] + idx[1] * size_[0]; }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
    { return d == 0 ? 1 : size_[0]; }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
    { return size_[1] * size_[0]; }

private:
  length_type size_[2];
};



template <unsigned Align,
	  typename ComplexLayout>
class Applied_layout<Layout<2, tuple<0, 1, 2>, Stride_unit_align<Align>,
			   ComplexLayout> >
{
public:
  static dimension_type const dim = 2;
  typedef tuple<0, 1, 2>         order_type;
  typedef Stride_unit_align<Align> pack_type;
  typedef ComplexLayout          complex_type;

public:
  Applied_layout(length_type size0, length_type size1)
  {
    size_[0] = size0;
    size_[1] = size1;

    stride_ = size_[1];

    if (stride_ % Align != 0)
      stride_ += (Align - stride_%Align);
  }

  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
    size_[1] = size_of_dim(extent, 1);

    stride_ = size_[1];

    if (stride_ % Align != 0)
      stride_ += (Align - stride_%Align);
  }

  index_type index(index_type idx0, index_type idx1)
    const VSIP_NOTHROW
  {
    assert(idx0 < size_[0] && idx1 < size_[1]);
    return idx0 * stride_ + idx1;
  }

  index_type index(Index<2> idx)
    const VSIP_NOTHROW
    { return idx[0] * stride_ + idx[1]; }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
    { return d == 0 ? stride_ : 1; }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { return size_[d]; }

  length_type total_size() const
    { return stride_ * size_[0] - (stride_-size_[1]); }


private:
  length_type size_[2];
  length_type stride_;
};



template <unsigned Align,
	  typename ComplexLayout>
class Applied_layout<Layout<2, tuple<1, 0, 2>, Stride_unit_align<Align>,
			   ComplexLayout> >
{
public:
  static dimension_type const dim = 2;
  typedef tuple<1, 0, 2>           order_type;
  typedef Stride_unit_align<Align> pack_type;
  typedef ComplexLayout            complex_type;

public:
  Applied_layout(length_type size0, length_type size1)
  {
    size_[0] = size0;
    size_[1] = size1;

    stride_ = size_[1];

    if (stride_ % Align != 0)
      stride_ += (Align - stride_%Align);
  }

  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
    size_[1] = size_of_dim(extent, 1);

    stride_ = size_[0];

    if (stride_ % Align != 0)
      stride_ += (Align - stride_%Align);
  }

  index_type index(index_type idx0, index_type idx1)
    const VSIP_NOTHROW
  {
    assert(idx0 < size_[0] && idx1 < size_[1]);
    return idx0 + idx1 * stride_;
  }

  index_type index(Index<2> idx)
    const VSIP_NOTHROW
    { return idx[0] + idx[1] * stride_; }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
    { return d == 1 ? stride_ : 1; }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { return size_[d]; }

  length_type total_size() const
    { return stride_ * size_[1] - (stride_-size_[0]); }


private:
  length_type size_[2];
  length_type stride_;
};



template <dimension_type Dim0,
	  dimension_type Dim1,
	  dimension_type Dim2,
	  typename       ComplexLayout>
class Applied_layout<Layout<3, tuple<Dim0, Dim1, Dim2>, Stride_unit_dense,
			   ComplexLayout> >
{
public:
  static dimension_type const dim = 3;
  typedef tuple<Dim0, Dim1, Dim2> order_type;
  typedef Stride_unit_dense         pack_type;
  typedef ComplexLayout           complex_type;

public:
  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
    size_[1] = size_of_dim(extent, 1);
    size_[2] = size_of_dim(extent, 2);
  }

  index_type index(Index<3> idx)
    const VSIP_NOTHROW
  {
    assert(idx[0] < size_[0] && idx[1] < size_[1] && idx[2] < size_[2]);
    return idx[Dim0]*size_[Dim1]*size_[Dim2] +
           idx[Dim1]*size_[Dim2] + 
           idx[Dim2];
  }

  index_type index(index_type idx0, index_type idx1, index_type idx2)
    const VSIP_NOTHROW
    { return index(Index<3>(idx0, idx1, idx2)); }


  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
  {
    return d == Dim2 ? 1 :
           d == Dim1 ? size_[Dim2]
                     : size_[Dim1] * size_[Dim2];

  }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
    { return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
    { return size_[2] * size_[1] * size_[0]; }

private:
  length_type size_[3];
};



template <dimension_type Dim0,
	  dimension_type Dim1,
	  dimension_type Dim2,
	  unsigned       Align,
	  typename       ComplexLayout>
class Applied_layout<Layout<3, tuple<Dim0, Dim1, Dim2>,
			    Stride_unit_align<Align>,
			    ComplexLayout> >
{
public:
  static dimension_type const dim = 3;
  typedef tuple<Dim0, Dim1, Dim2>  order_type;
  typedef Stride_unit_align<Align> pack_type;
  typedef ComplexLayout            complex_type;

public:
  template <typename ExtentT>
  Applied_layout(ExtentT const& extent)
  {
    size_[0] = size_of_dim(extent, 0);
    size_[1] = size_of_dim(extent, 1);
    size_[2] = size_of_dim(extent, 2);

    stride_[Dim2] = 1;
    stride_[Dim1] = size_[Dim2];
    if (stride_[Dim1] % Align != 0)
      stride_[Dim1] += (Align - stride_[Dim1]%Align);
    stride_[Dim0] = size_[Dim1] * stride_[Dim1];
  }

  index_type index(Index<3> idx)
    const VSIP_NOTHROW
  {
    assert(idx[0] < size_[0] && idx[1] < size_[1] && idx[2] < size_[2]);
    return idx[Dim0]*stride_[Dim0] +
           idx[Dim1]*stride_[Dim1] + 
           idx[Dim2];
  }

  index_type index(index_type idx0, index_type idx1, index_type idx2)
    const VSIP_NOTHROW
    { return index(Index<3>(idx0, idx1, idx2)); }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
  {
    return stride_[d];
  }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
  { return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
  { return size_[Dim0] * stride_[Dim0]; }

private:
  length_type size_  [3];
  stride_type stride_[3];
};



/// Applied run-time layout.
///
/// This object gets created for run-time extdata access.
///
/// Efficiency is important to reduce library interface overhead.
///  - Don't store the whole Rt_layout, only the parts we need:
///    the complex_format and part of the dimension-order.
///
template <dimension_type Dim>
class Applied_layout<Rt_layout<Dim> >
{
public:
  static dimension_type const dim = Dim;

public:
  // Construct an empty Applied_layout.  Used when it is known that object
  // will not be used.
  Applied_layout(empty_layout_type) {}

  /// Construct Applied_layout object.
  ///
  /// Template parameters:
  ///
  ///   :ExtentT: to be a type capable of encoding an extent (Length or Domain)
  ///
  /// Arguments:
  ///
  ///   :layout:    to be the run-time layout.
  ///   :extent:    to be the extent of the data to layout.
  ///   :elem_size: to be the size of a data element (in bytes).
  template <typename ExtentT>
  Applied_layout(
    Rt_layout<Dim> const& layout,
    ExtentT const&        extent,
    length_type           elem_size = 1)
  : cformat_(layout.complex)
  {
    assert(layout.pack != stride_unit_align || 
           layout.align == 0 || layout.align % elem_size == 0);

    for (dimension_type d=0; d<Dim; ++d)
      size_[d] = size_of_dim(extent, d);

    if (Dim == 3)
    {
      order_[2] = layout.order.impl_dim2;
      order_[1] = layout.order.impl_dim1;
      order_[0] = layout.order.impl_dim0;

      stride_[order_[2]] = 1;
      stride_[order_[1]] = size_[order_[2]];
      if (layout.pack == stride_unit_align && 
	  layout.align != 0 &&
	  (elem_size*stride_[order_[1]]) % layout.align != 0)
      {
	stride_type adjust =
	  layout.align - (stride_[order_[1]] * elem_size)%layout.align;
	assert(adjust > 0 && adjust % elem_size == 0);
	adjust /= elem_size;
	stride_[order_[1]] += adjust;
	assert((stride_[order_[1]] * elem_size)%layout.align == 0);
      }
      stride_[order_[0]] = size_[order_[1]] * stride_[order_[1]];
    }
    else if (Dim == 2)
    {
      // Copy only the portion of the dimension-order that we use.
      order_[1] = layout.order.impl_dim1;
      order_[0] = layout.order.impl_dim0;

      stride_[order_[1]] = 1;
      stride_[order_[0]] = size_[order_[1]];

      if (layout.pack == stride_unit_align && 
	  layout.align != 0 &&
	  (elem_size*stride_[order_[0]]) % layout.align != 0)
      {
	stride_type adjust =
	  layout.align - (stride_[order_[0]] * elem_size)%layout.align;
	assert(adjust > 0 && adjust % elem_size == 0);
	adjust /= elem_size;
	stride_[order_[0]] += adjust;
	assert((stride_[order_[0]] * elem_size)%layout.align == 0);
      }
    }
    else  // (Dim == 1)
    {
      // Copy only the portion of the dimension-order that we use.
      order_[0] = layout.order.impl_dim0;

      stride_[0] = 1;
    }
  }

  index_type index(Index<Dim> idx)
    const VSIP_NOTHROW
  {
    if (Dim == 3)
    {
      assert(idx[0] < size_[0] && idx[1] < size_[1] && idx[2] < size_[2]);
      return idx[order_[0]]*stride_[order_[0]] +
	     idx[order_[1]]*stride_[order_[1]] + 
	     idx[order_[2]];
    }
    else if (Dim == 2)
    {
      assert(idx[0] < size_[0] && idx[1] < size_[1]);
      return idx[order_[0]]*stride_[order_[0]] +
	     idx[order_[1]];
    }
    else // (Dim == 1)
    {
      assert(idx[0] < size_[0]);
      return idx[0];
    }
  }

  stride_type stride(dimension_type d)
    const VSIP_NOTHROW
  {
    return stride_[d];
  }

  length_type size(dimension_type d)
    const VSIP_NOTHROW
  { return size_[d]; }

  length_type total_size()
    const VSIP_NOTHROW
  { return size_[order_[0]] * stride_[order_[0]]; }

public:
  rt_complex_type cformat_;

private:
  dimension_type order_ [Dim];
  length_type    size_  [Dim];
  stride_type    stride_[Dim];
};

} // namespace vsip::impl
} // namespace vsip

#endif // VSIP_CORE_LAYOUT_HPP
