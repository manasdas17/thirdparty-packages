/* Copyright (c) 2005, 2006, 2008 by CodeSourcery.  All rights reserved. */

/** @file    vsip/core/subblock.hpp
    @author  Zack Weinberg
    @date    2005-01-28
    @brief   VSIPL++ Library: Subblock classes.

  This file implements Block-interface classes which wrap other Blocks
  and rearrange the data they carry.  Currently we provide four wrappers:

  1) Component_block: For blocks with complex values, expose only the
     real or imaginary components of those values.  Could be
     generalized to blocks with other non-scalar element types,
     e.g. quaternions, if anyone ever wants them.

  2) Transposed_block, Permuted_block: Reorder the indices of an
     N-dimensional block (N>1).

  3) Sliced_block: Bind one of the indices of an N-dimensional block (N>1)
     to a constant, producing an (N-1)-dimensional block.  For example,
     this can be used to extract a row or column vector from a matrix.

  4) Subset_block: Without changing the dimensionality of a block,
     restrict access to a subset of its values: every other element of
     a 1-dimensional vector, or the upper left 3x3 of a 9x9 matrix.
     The subset pattern is restricted to what can be expressed by a
     Domain<N> where N is the underlying block's dimensionality.   */

#ifndef VSIP_CORE_SUBBLOCK_HPP
#define VSIP_CORE_SUBBLOCK_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/support.hpp>
#include <vsip/domain.hpp>
#include <vsip/core/map_fwd.hpp>
#include <vsip/core/refcount.hpp>
#include <vsip/core/static_assert.hpp>
#include <vsip/core/noncopyable.hpp>
#include <vsip/core/domain_utils.hpp>
#include <vsip/core/storage.hpp>
#include <vsip/core/parallel/local_map.hpp>
#include <vsip/core/parallel/subset_map_decl.hpp>
#include <vsip/core/parallel/transpose_map_decl.hpp>
#include <complex>

/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{

namespace impl
{

/// The Component_block class applies an "Extractor" policy to all
/// accesses via get() and put().  Everything else is deferred to the
/// underlying block.

template <typename Block,
          template <typename> class Extractor>
class Component_block : public impl::Non_assignable
{
  typedef Extractor<typename Block::value_type> extr_;
public:
  // Compile-time values and types.
  // The type of elements of this block is determined by the Extractor
  // instantiated for the Block's value_type.
  static dimension_type const dim = Block::dim;
  typedef typename extr_::value_type  value_type;
  typedef value_type&       reference_type;
  typedef value_type const& const_reference_type;
  typedef typename Block::map_type map_type;

  // Constructors and destructors.
  Component_block(Component_block const& b) : blk_(&*b.blk_) {}
  Component_block(Block &blk) VSIP_NOTHROW : blk_ (&blk) {}
  ~Component_block() VSIP_NOTHROW {}

  // Accessors.
  length_type size() const VSIP_NOTHROW { return blk_->size();}
  length_type size(dimension_type block_d, dimension_type d) const VSIP_NOTHROW
  { return blk_->size(block_d, d);}
  // These are noops as Component_block is helt by-value.
  void increment_count() const VSIP_NOTHROW {}
  void decrement_count() const VSIP_NOTHROW {}
  map_type const& map() const { return blk_->map();}

  // Data accessors.
  value_type get(index_type i) const VSIP_NOTHROW
  { return extr_::get(blk_->get(i));}
  value_type get(index_type i, index_type j) const VSIP_NOTHROW
  { return extr_::get(blk_->get(i, j));}
  value_type get(index_type i, index_type j, index_type k) const VSIP_NOTHROW
  { return extr_::get(blk_->get(i, j, k));}

  void put(index_type i, value_type val) VSIP_NOTHROW
  {
    typename Block::value_type tmp = blk_->get(i);
    extr_::set(tmp, val);
    blk_->put(i, tmp);
  }
  void put(index_type i, index_type j, value_type val) VSIP_NOTHROW
  {
    typename Block::value_type tmp = blk_->get(i, j);
    extr_::set(tmp, val);
    blk_->put(i, j, tmp);
  }
  void put(index_type i, index_type j, index_type k, value_type val) VSIP_NOTHROW
  {
    typename Block::value_type tmp = blk_->get(i, j, k);
    extr_::set(tmp, val);
    blk_->put(i, j, k, tmp);
  }

  // Support Direct_data interface.
public:
  typedef impl::Storage<typename Block_layout<Block>::complex_type, value_type>
		storage_type;
  typedef typename storage_type::type       data_type;
  typedef typename storage_type::const_type const_data_type;

  data_type       impl_data()       VSIP_NOTHROW
  { 
    return extr_::get_ptr(blk_->impl_data());
  }

  const_data_type impl_data() const VSIP_NOTHROW
  { 
    return extr_::get_ptr(blk_->impl_data());
  }

  stride_type impl_stride(dimension_type Dim, dimension_type d)
     const VSIP_NOTHROW
  {
    assert(Dim == dim && d<dim);
    if (Type_equal<typename Block_layout<Block>::complex_type,
	           Cmplx_split_fmt>::value)
      return 1*blk_->impl_stride(dim, d);
    else
      return 2*blk_->impl_stride(dim, d);
  }

  // Implementation specific.
public:
  Block const& impl_block() const { return *this->blk_; }

private:
  // Data members.
  typename View_block_storage<Block>::type blk_;
};

// Store Component_blocks by-value.
template <typename Block,
          template <typename> class Extractor>
struct View_block_storage<Component_block<Block, Extractor> >
  : By_value_block_storage<Component_block<Block, Extractor> >
{};


// Extractor classes for real and imaginary components.
template <typename Cplx>
class Real_extractor
{
public:
  typedef typename Cplx::value_type  value_type;

  static value_type get(Cplx val) VSIP_NOTHROW { return val.real();}
  static void set(Cplx& elt, value_type val) VSIP_NOTHROW 
  {
    elt = Cplx(val, elt.imag());
  }

  
  static value_type* get_ptr(complex<value_type>* data)
    { return reinterpret_cast<value_type*>(data); }

  static value_type* get_ptr(std::pair<value_type*, value_type*> const& data)
    { return data.first; }
};

template <typename Cplx>
class Imag_extractor
{
public:
  typedef typename Cplx::value_type  value_type;

  static value_type get(Cplx val) VSIP_NOTHROW { return val.imag();}
  static void set(Cplx& elt, value_type val) VSIP_NOTHROW 
  {
    elt = Cplx(elt.real(), val);
  }

  static value_type* get_ptr(complex<value_type>* data)
    { return reinterpret_cast<value_type*>(data) + 1; }

  static value_type* get_ptr(std::pair<value_type*, value_type*> const& data)
    { return data.second; }
};



template <typename Block,
          template <typename> class Extractor>
struct Block_layout<Component_block<Block, Extractor> >
{
  // Dimension: Same
  // Access   : Same
  // Order    : Same
  // Stride   : If split -> same
  //          : else     -> unknown
  // Cmplx    : Same

  static dimension_type const dim = Block::dim;

  typedef typename Block_layout<Block>::access_type  access_type;
  typedef typename Block_layout<Block>::order_type   order_type;
  typedef typename Block_layout<Block>::complex_type complex_type;

  typedef typename
    ITE_Type<Type_equal<complex_type, Cmplx_split_fmt>::value,
	     As_type<typename Block_layout<Block>::pack_type>,
	     As_type<Stride_unknown> >::type pack_type;

  typedef Layout<dim, order_type, pack_type, complex_type> layout_type;
};

template <typename BlockT, template <typename> class Extractor>
struct Is_modifiable_block<Component_block<BlockT, Extractor> >
  : Is_modifiable_block<BlockT>
{};



// Functor to create map for a subset block from its parent block's map.
// (not a map class)

template <dimension_type Dim,
	  typename       MapT>
struct Subset_block_map
{
  typedef typename Map_subdomain<Dim, MapT>::type type;

  static type convert_map(MapT const&        map,
			  Domain<Dim> const& dom)
  {
    return Map_subdomain<Dim, MapT>::project(map, dom);
  }

  static index_type parent_subblock(MapT const&        map,
				    Domain<Dim> const& dom,
				    index_type         sb)
  {
    return Map_subdomain<Dim, MapT>::parent_subblock(map, dom, sb);
  }
};



/// The Subset_block class maps all accesses through a Domain instance
/// before forwarding to the underlying block.  Thus, for instance, if
/// a 1-dimensional vector of length N is the underlying block, and the
/// Domain instance in use is Domain<1>(N/2)*2, then the Subset_block class
/// will expose every other element of the underlying block.
template <typename Block>
class Subset_block : public Non_assignable
{
public:
  // Compile-time values and types.
  static dimension_type const dim = Block::dim;
  typedef typename Block::value_type value_type;
  typedef value_type&                reference_type;
  typedef value_type const&          const_reference_type;

  typedef Subset_block_map<dim, typename Block::map_type> map_functor;
  typedef typename map_functor::type map_type;

  // Constructors and destructors.
  Subset_block(Domain<dim> const& dom, Block &blk) VSIP_NOTHROW
    : blk_ (&blk),
      dom_ (dom),
      map_ (map_functor::convert_map(blk.map(), dom_))
  {
    // Sanity check that all of the Domain indices are within the
    // underlying block's range.  (If domain is empty, value
    // returned by impl_last() is not valid.)
    for (dimension_type d = 0; d < dim; d++)
    {
      assert(dom_[d].size() == 0 || dom_[d].first() < blk_->size(dim, d));
      assert(dom_[d].size() == 0 || dom_[d].impl_last() < blk_->size(dim, d));
    }
    map_.impl_apply(block_domain<dim>(*this));
  }

  Subset_block(Subset_block const& b)
    : blk_ (&*b.blk_),
      dom_ (b.dom_),
      map_ (b.map_)
  {
    map_.impl_apply(block_domain<dim>(*this));
  }

  ~Subset_block() VSIP_NOTHROW {}

  // Accessors.
  // The size of a Subset is the (total) size of its Domain(s), not
  // the size of the underlying block.
  length_type size() const VSIP_NOTHROW { return dom_.size();}
  length_type size(dimension_type block_d, dimension_type d) const VSIP_NOTHROW
  {
    assert(block_d == dim);
    assert(d < block_d);
    return dom_[d].size();
  }

  // These are noops as Subset_block is held by-value.
  void increment_count() const VSIP_NOTHROW {}
  void decrement_count() const VSIP_NOTHROW {}
  map_type const& map() const { return this->map_;}

  value_type get(index_type i) const VSIP_NOTHROW
  {
    assert(i < this->size(1, 0));
    return blk_->get(dom_[0].impl_nth(i));
  }
  value_type get(index_type i, index_type j) const VSIP_NOTHROW
  {
    assert(i < this->size(2, 0));
    assert(j < this->size(2, 1));
    return blk_->get(dom_[0].impl_nth(i),
		     dom_[1].impl_nth(j));
  }
  value_type get(index_type i, index_type j, index_type k) const VSIP_NOTHROW
  {
    assert(i < this->size(3, 0));
    assert(j < this->size(3, 1));
    assert(k < this->size(3, 2));
    return blk_->get(dom_[0].impl_nth(i),
		     dom_[1].impl_nth(j),
		     dom_[2].impl_nth(k));
  }

  void put(index_type i, value_type val) VSIP_NOTHROW
  {
    blk_->put(dom_[0].impl_nth(i), val);
  }
  void put(index_type i, index_type j, value_type val) VSIP_NOTHROW
  {
    blk_->put(dom_[0].impl_nth(i),
	      dom_[1].impl_nth(j), val);
  }
  void put(index_type i, index_type j, index_type k, value_type val) VSIP_NOTHROW
  {
    blk_->put(dom_[0].impl_nth(i),
	      dom_[1].impl_nth(j),
	      dom_[2].impl_nth(k), val);
  }


  Block const& impl_block() const { return *this->blk_; }
  Domain<dim> const& impl_domain() const { return this->dom_; }


  // Lvalue interface
  reference_type impl_ref(index_type i) VSIP_NOTHROW
  {
    return blk_->impl_ref(dom_[0].impl_nth(i));
  }
  reference_type impl_ref(index_type i, index_type j) VSIP_NOTHROW
  {
    return blk_->impl_ref(dom_[0].impl_nth(i),
                          dom_[1].impl_nth(j));
  }
  reference_type impl_ref(index_type i, index_type j, index_type k) VSIP_NOTHROW
  {
    return blk_->impl_ref(dom_[0].impl_nth(i),
                          dom_[1].impl_nth(j),
                          dom_[2].impl_nth(k));
  }

  // Support Direct_data interface.
public:
  typedef impl::Storage<typename Block_layout<Block>::complex_type, value_type>
		storage_type;
  typedef typename storage_type::type       data_type;
  typedef typename storage_type::const_type const_data_type;

  par_ll_pbuf_type impl_ll_pbuf() VSIP_NOTHROW
  { return blk_->impl_ll_pbuf(); }

  stride_type impl_offset() VSIP_NOTHROW
  {
    stride_type offset = blk_->impl_offset();
    for (dimension_type d=0; d<dim; ++d)
      offset += dom_[d].first() * blk_->impl_stride(dim, d);
    return offset;
  }

  data_type       impl_data()       VSIP_NOTHROW
  { 
    data_type ptr = blk_->impl_data();
    for (dimension_type d=0; d<dim; ++d)
      ptr = storage_type::offset(ptr,
				 dom_[d].first() * blk_->impl_stride(dim, d));
    return ptr;
  }

  const_data_type impl_data() const VSIP_NOTHROW
  { 
    data_type ptr = blk_->impl_data();
    for (dimension_type d=0; d<dim; ++d)
      ptr = storage_type::offset(ptr,
				 dom_[d].first() * blk_->impl_stride(dim, d));
    return ptr;
  }

  stride_type impl_stride(dimension_type Dim, dimension_type d)
     const VSIP_NOTHROW
  {
    assert(Dim == dim && d<dim);
    return blk_->impl_stride(dim, d) * dom_[d].stride();
  }

private:
  // Data members.
  typename View_block_storage<Block>::type blk_;
  Domain<dim> dom_;
  map_type    map_;
};

// Store Subset_blocks by-value.
template <typename Block>
struct View_block_storage<Subset_block<Block> >
  : By_value_block_storage<Subset_block<Block> >
{};


template <typename BlockT>
struct Block_root<Subset_block<BlockT> >
{
  typedef typename Block_root<BlockT>::type type;
};

template <typename       BlockT>
typename Block_root<Subset_block<BlockT> >::type const&
block_root(Subset_block<BlockT> const& block)
{
  return block_root(block.impl_block());
}



template <typename Block>
struct Block_layout<Subset_block<Block> >
{
  // Dimension: Same
  // Access   : Same
  // Order    : Same
  // Stride   : Stride_unknown
  // Cmplx    : Same

  static dimension_type const dim = Block::dim;

  typedef typename Block_layout<Block>::access_type  access_type;
  typedef typename Block_layout<Block>::order_type   order_type;
  typedef Stride_unknown                             pack_type;
  typedef typename Block_layout<Block>::complex_type complex_type;

  typedef Layout<dim, order_type, pack_type, complex_type> layout_type;
};

// Subset_block has impl_ref if the underlying block has impl_ref.
template <typename Block>
struct Lvalue_factory_type<Subset_block<Block> >
{
  typedef typename Lvalue_factory_type<Block>
    ::template Rebind<Subset_block<Block> >::type type;
  template <typename OtherBlock>
  struct Rebind {
    typedef typename Lvalue_factory_type<Block>
      ::template Rebind<OtherBlock>::type type;
  };
};

template <typename BlockT>
struct Is_modifiable_block<Subset_block<BlockT> >
  : Is_modifiable_block<BlockT>
{};



/// The Transposed_block class exchanges the order of indices to a
/// 2-dimensional block, and the dimensions visible via 2-argument
/// size().
template <typename Block> 
class Transposed_block;


// Store Transposed_block by-value.
template <typename Block>
struct View_block_storage<Transposed_block<Block> >
  :  By_value_block_storage<Transposed_block<Block> >
{};

template <typename Block>
class Transposed_block
    : public impl::Compile_time_assert<Block::dim == 2>,
      public impl::Non_assignable
{
  // Compile-time values and types (implementation detail).
private:
  typedef Transpose_map_of<2, typename Block::map_type> map_functor;

  // Compile-time values and types (part of block interface).
public:
  static dimension_type const dim = Block::dim;
  typedef typename Block::value_type value_type;
  typedef value_type&                reference_type;
  typedef value_type const&          const_reference_type;
  typedef typename map_functor::type map_type;

  // Constructors and destructors.
  Transposed_block(Block &blk) VSIP_NOTHROW
    : blk_ (&blk)
    , map_ (map_functor::project(blk.map()))
  { map_.impl_apply(block_domain<dim>(*this)); }

  Transposed_block(Transposed_block const& b)
    : blk_ (&*b.blk_)        // &* work's around holder's lack of copy-cons.
    , map_ (b.map_)
  { map_.impl_apply(block_domain<dim>(*this)); }

  ~Transposed_block() VSIP_NOTHROW
    {}

  // Accessors.
  length_type size() const VSIP_NOTHROW
    { return blk_->size(); }
  length_type size(dimension_type block_d, dimension_type d) const VSIP_NOTHROW
    {
      assert (block_d == 2);
      assert (d <= 1);
      return blk_->size(block_d, !d);
    }
  // These are noops as Transposed_block is held by-value.
  void increment_count() const VSIP_NOTHROW {}
  void decrement_count() const VSIP_NOTHROW {}
  map_type const& map() const { return map_; }

  // Data accessors.
  value_type get(index_type i, index_type j) const VSIP_NOTHROW
    { return blk_->get(j, i); }

  void put(index_type i, index_type j, value_type val) VSIP_NOTHROW
    { return blk_->put(j, i, val); }

  reference_type impl_ref(index_type i, index_type j) VSIP_NOTHROW
    { return blk_->impl_ref(j, i); }

  // Support Direct_data interface.
 public:
  typedef impl::Storage<typename Block_layout<Block>::complex_type, value_type>
		storage_type;
  typedef typename storage_type::type       data_type;
  typedef typename storage_type::const_type const_data_type;

  data_type       impl_data()       VSIP_NOTHROW
  { return blk_->impl_data(); }

  const_data_type impl_data() const VSIP_NOTHROW
  { return blk_->impl_data(); }

  stride_type impl_stride(dimension_type Dim, dimension_type d)
     const VSIP_NOTHROW
  {
    assert(Dim == dim && d<dim);
    return blk_->impl_stride(dim, 1 - d);
  }

public:
  Block const& impl_block() const { return *this->blk_; }

 private:
  // Data members.
  typename View_block_storage<Block>::type blk_;
  map_type                                 map_;
};

// Transposed_block impl_ref if the underlying block has impl_ref.
template <typename Block>
struct Lvalue_factory_type<Transposed_block<Block> >
{
  typedef typename Lvalue_factory_type<Block>
    ::template Rebind<Transposed_block<Block> >::type type;
  template <typename OtherBlock>
  struct Rebind {
    typedef typename Lvalue_factory_type<Block>
      ::template Rebind<OtherBlock>::type type;
  };
};

template <typename BlockT>
struct Is_modifiable_block<Transposed_block<BlockT> >
  : Is_modifiable_block<BlockT>
{};



// Take transpose of dimension-order.
template <typename T>
struct Transpose_order;

template <dimension_type Dim0,
	  dimension_type Dim1,
	  dimension_type Dim2>
struct Transpose_order<tuple<Dim0, Dim1, Dim2> >
{
  typedef tuple<Dim1, Dim0, Dim2> type;
};



template <typename Block>
struct Block_layout<Transposed_block<Block> >
{
  // Dimension: Same
  // Access   : Same
  // Order    : Reversed
  // Stride   :
  //    Stride_unit      -> Stride_unit
  //    Stride_unit_dense -> Stride_unit_dense
  //    Stride_unit_align -> Stride_unit
  //    Stride_unknown   -> Stride_unknown
  // Cmplx    : Same

  static dimension_type const dim = Block::dim;

  typedef typename Block_layout<Block>::access_type  access_type;
  typedef typename Transpose_order<
                      typename Block_layout<Block>::order_type>::type
					            order_type;
  typedef Stride_unknown                             pack_type;
  typedef typename Block_layout<Block>::complex_type complex_type;

  typedef Layout<dim, order_type, pack_type, complex_type> layout_type;
};



/// The Permutor policy class provides functions which do the actual
/// argument reordering.  There is no general version, but below are
/// partial specializations for all permutations of <0,1,2>.
/// Block is the underlying block, Ordering is a tuple.  This class
/// is not used directly.

template <typename Block, typename Ordering> struct Permutor;

/// The Permuted_block class reorders the indices to a 3-dimensional
/// block, and the dimensions visible via 2-argument size().  The
/// permutation is specified as a tuple (see [support]).

template <typename Block, typename Ordering>
class Permuted_block
    : impl::Compile_time_assert<Block::dim == 3>,
      public impl::Non_assignable
{
protected:
  // Policy class.
  typedef Permutor<Block, Ordering> perm_type;

public:
  // Compile-time values and types.
  static dimension_type const dim = Block::dim;
  typedef typename Block::value_type value_type;
  typedef value_type&       reference_type;
  typedef value_type const& const_reference_type;
  typedef typename Block::map_type map_type;

  // Constructors and destructors.
  Permuted_block(Block &blk) VSIP_NOTHROW
    : blk_ (&blk)
    {}
  Permuted_block(Permuted_block const& pb) VSIP_NOTHROW
    : blk_ (&*pb.blk_)
    {}
  ~Permuted_block() VSIP_NOTHROW
    {}

  // Accessors.
  length_type size() const VSIP_NOTHROW
    { return blk_->size(); }
  length_type size(dimension_type block_d, dimension_type d) const VSIP_NOTHROW
    { return blk_->size(block_d, perm_type::dimension_order(d)); }
  // These are noops as Transposed_block is held by-value.
  void increment_count() const VSIP_NOTHROW {}
  void decrement_count() const VSIP_NOTHROW {}
  map_type const& map() const { return blk_->map(); }

  // Data accessors.
  value_type get(index_type i, index_type j, index_type k) const VSIP_NOTHROW
    { return perm_type::get(*blk_, i, j, k); }

  void put(index_type i, index_type j, index_type k, value_type val) VSIP_NOTHROW
    { perm_type::put(*blk_, i, j, k, val); }

  reference_type impl_ref(index_type i, index_type j, index_type k) VSIP_NOTHROW
    { return perm_type::impl_ref(*blk_, i, j, k); }

  // Support Direct_data interface.
public:
  typedef impl::Storage<typename Block_layout<Block>::complex_type, value_type>
		storage_type;
  typedef typename storage_type::type       data_type;
  typedef typename storage_type::const_type const_data_type;

  data_type       impl_data()       VSIP_NOTHROW
  { return blk_->impl_data(); }

  const_data_type impl_data() const VSIP_NOTHROW
  { return blk_->impl_data(); }

  stride_type impl_stride(dimension_type Dim, dimension_type d)
     const VSIP_NOTHROW
  {
    assert(Dim == dim && d<dim);
    return blk_->impl_stride(dim,  perm_type::dimension_order(d));
  }

 private:
  // Data members.
  typename View_block_storage<Block>::type blk_;
};


/// Since all Permutor partial specializations differ only in the
/// permutation chosen, we use a macro to carry all the repeated text.
#define VSIP_IMPL_PERMUTOR_SPECIALIZATION(n0,n1,n2)                     \
template <typename Block>                                               \
struct Permutor<Block, tuple<n0, n1, n2> >                              \
{                                                                       \
  static dimension_type dimension_order(dimension_type d) VSIP_NOTHROW  \
    {                                                                   \
      static const dimension_type permutation[3] = { n0, n1, n2 };      \
      return permutation[d];                                            \
    }                                                                   \
  static typename Block::value_type                                     \
  get(Block const& blk, index_type i##n0, index_type i##n1,             \
      index_type i##n2) VSIP_NOTHROW                                    \
    { return blk.get(i0, i1, i2); }                                     \
  static void                                                           \
  put(Block& blk, index_type i##n0, index_type i##n1, index_type i##n2, \
      typename Block::value_type val) VSIP_NOTHROW                      \
    { blk.put(i0, i1, i2, val); }                                       \
  static typename Block::reference_type                                 \
  impl_ref(Block& blk, index_type i##n0, index_type i##n1,              \
      index_type i##n2) VSIP_NOTHROW                                    \
    { return blk.impl_ref(i0, i1, i2); }                                \
}

VSIP_IMPL_PERMUTOR_SPECIALIZATION(0,1,2);
VSIP_IMPL_PERMUTOR_SPECIALIZATION(1,0,2);
VSIP_IMPL_PERMUTOR_SPECIALIZATION(1,2,0);
VSIP_IMPL_PERMUTOR_SPECIALIZATION(2,1,0);
VSIP_IMPL_PERMUTOR_SPECIALIZATION(2,0,1);
VSIP_IMPL_PERMUTOR_SPECIALIZATION(0,2,1);

#undef VSIP_IMPL_PERMUTOR_SPECIALIZATION



// Store Permuted_block by-value.
template <typename Block, typename Ordering>
struct View_block_storage<Permuted_block<Block, Ordering> >
  : By_value_block_storage<Permuted_block<Block, Ordering> >
{};

// Permuted_block has impl_ref if the underlying block has impl_ref.
template <typename Block, typename Ordering>
struct Lvalue_factory_type<Permuted_block<Block, Ordering> >
{
  typedef typename Lvalue_factory_type<Block>
    ::template Rebind<Permuted_block<Block, Ordering> >::type type;
  template <typename OtherBlock>
  struct Rebind {
    typedef typename Lvalue_factory_type<Block>
      ::template Rebind<OtherBlock>::type type;
  };
};

template <typename BlockT, typename Ordering>
struct Is_modifiable_block<Permuted_block<BlockT, Ordering> >
  : Is_modifiable_block<BlockT>
{};


// Take permutation of dimension-order.
template <typename PermutionT,		// Tuple expressing permutation
	  typename OrderT>		// Original dim-order
struct Permute_order;

template <dimension_type Dim0, dimension_type Dim1, dimension_type Dim2>
struct Permute_order<tuple<0, 1, 2>, tuple<Dim0, Dim1, Dim2> >
{ typedef tuple<Dim0, Dim1, Dim2> type; };

template <dimension_type Dim0, dimension_type Dim1, dimension_type Dim2>
struct Permute_order<tuple<1, 0, 2>, tuple<Dim0, Dim1, Dim2> >
{ typedef tuple<Dim1, Dim0, Dim2> type; };

template <dimension_type Dim0, dimension_type Dim1, dimension_type Dim2>
struct Permute_order<tuple<0, 2, 1>, tuple<Dim0, Dim1, Dim2> >
{ typedef tuple<Dim0, Dim2, Dim1> type; };

template <dimension_type Dim0, dimension_type Dim1, dimension_type Dim2>
struct Permute_order<tuple<1, 2, 0>, tuple<Dim0, Dim1, Dim2> >
{ typedef tuple<Dim1, Dim2, Dim0> type; };

template <dimension_type Dim0, dimension_type Dim1, dimension_type Dim2>
struct Permute_order<tuple<2, 1, 0>, tuple<Dim0, Dim1, Dim2> >
{ typedef tuple<Dim2, Dim1, Dim0> type; };

template <dimension_type Dim0, dimension_type Dim1, dimension_type Dim2>
struct Permute_order<tuple<2, 0, 1>, tuple<Dim0, Dim1, Dim2> >
{ typedef tuple<Dim2, Dim0, Dim1> type; };

template <typename BlockT, typename PermutionT>
struct Block_layout<Permuted_block<BlockT, PermutionT> >
{
  // Dimension: Same
  // Access   : Same
  // Order    : permuted
  // Stride   : Stride_unknown
  // Cmplx    : Same

  static dimension_type const dim = BlockT::dim;

  typedef typename Block_layout<BlockT>::access_type  access_type;
  typedef typename Permute_order<PermutionT, 
                      typename Block_layout<BlockT>::order_type>::type
					            order_type;
  typedef Stride_unknown                             pack_type;
  typedef typename Block_layout<BlockT>::complex_type complex_type;

  typedef Layout<dim, order_type, pack_type, complex_type> layout_type;
};


template <typename MapT, dimension_type SubDim>
struct Sliced_block_map {};

template <typename MapT, dimension_type SubDim1, dimension_type SubDim2>
struct Sliced2_block_map {};

template <dimension_type Dim, dimension_type SubDim>
struct Sliced_block_map<Global_map<Dim>, SubDim>
{
  typedef Global_map<Dim - 1> type;
  static type convert_map(Global_map<Dim> const&, index_type) { return type();}
}; 

template <dimension_type Dim, dimension_type SubDim>
struct Sliced_block_map<Local_or_global_map<Dim>, SubDim>
{
  typedef Local_or_global_map<Dim - 1> type;
  static type convert_map(Local_or_global_map<Dim> const&, index_type)
    { return type();}
}; 

template <dimension_type SubDim>
struct Sliced_block_map<Local_map, SubDim>
{
  typedef Local_map type;
  static type convert_map(Local_map const&, index_type) { return type();}
}; 

template <dimension_type Dim, dimension_type SubDim>
struct Sliced_block_map<Scalar_block_map<Dim>, SubDim>
{
  typedef Scalar_block_map<Dim - 1> type;
  static type convert_map(Scalar_block_map<Dim> const&, index_type)
    { return type();}
}; 

template <typename Dist0,
          typename Dist1,
          typename Dist2,
	  dimension_type D>
struct Sliced_block_map<Map<Dist0, Dist1, Dist2>, D>
{
  typedef typename Map_project_1<D, Map<Dist0, Dist1, Dist2> >::type type;

  static type convert_map(Map<Dist0, Dist1, Dist2> const& map, index_type i)
  {
    return Map_project_1<D, Map<Dist0, Dist1, Dist2> >::project(map, i);
  }

  static index_type parent_subblock(Map<Dist0, Dist1, Dist2> const& map,
				    index_type i,
				    index_type sb)
  {
    return Map_project_1<D, Map<Dist0, Dist1, Dist2> >::parent_subblock(map, i, sb);
  }
}; 

template <dimension_type Dim,
	  dimension_type SubDim1,
	  dimension_type SubDim2>
struct Sliced2_block_map<Global_map<Dim>, SubDim1, SubDim2>
{
  typedef Global_map<Dim - 2> type;
  static type convert_map(Global_map<Dim> const&, index_type, index_type) { return type();}
}; 

template <dimension_type SubDim1,
	  dimension_type SubDim2>
struct Sliced2_block_map<Local_map, SubDim1, SubDim2>
{
  typedef Local_map type;
  static type convert_map(Local_map const&, index_type, index_type) { return type();}
}; 

template <typename       Dist0,
          typename       Dist1,
          typename       Dist2,
	  dimension_type SubDim1,
	  dimension_type SubDim2>
struct Sliced2_block_map<Map<Dist0, Dist1, Dist2>, SubDim1, SubDim2>
{
  typedef Map_project_2<SubDim1, SubDim2, Map<Dist0, Dist1, Dist2> > project_t;
  typedef typename project_t::type type;

  static type convert_map(Map<Dist0, Dist1, Dist2> const& map,
			  index_type idx0,
			  index_type idx1)
    { return project_t::project(map, idx0, idx1); }

  static index_type parent_subblock(Map<Dist0, Dist1, Dist2> const& map,
				    index_type idx0,
				    index_type idx1,
				    index_type sb)
    { return project_t::parent_subblock(map, idx0, idx1, sb); }
}; 

/// The Sliced_block class binds one of the indices of the underlying
/// N-dimensional block to a constant, producing an N-1-dimensional
/// block.  N must be 2 or 3, and the index chosen must be allowed for
/// the underlying block.

template <typename Block, dimension_type D> class Sliced_block;

// Store Sliced_block by-value.
template <typename Block, dimension_type D>
struct View_block_storage<Sliced_block<Block, D> >
  : By_value_block_storage<Sliced_block<Block, D> >
{};

// Sliced_block has impl_ref if the underlying block has impl_ref.
template <typename Block, dimension_type D>
struct Lvalue_factory_type<Sliced_block<Block, D> >
{
  typedef typename Lvalue_factory_type<Block>
    ::template Rebind<Sliced_block<Block, D> >::type type;
  template <typename OtherBlock>
  struct Rebind {
    typedef typename Lvalue_factory_type<Block>
      ::template Rebind<OtherBlock>::type type;
  };
};

template <typename BlockT, dimension_type D>
struct Is_modifiable_block<Sliced_block<BlockT, D> >
  : Is_modifiable_block<BlockT>
{};

template <typename Block, dimension_type D> 
class Sliced_block_base : public impl::Compile_time_assert<(Block::dim >= 2)>,
			  public impl::Non_assignable
{
public:
  // Compile-time values and types.
  static dimension_type const dim = Block::dim - 1;
  typedef typename Block::value_type value_type;
  typedef value_type&                reference_type;
  typedef value_type const&          const_reference_type;
  typedef typename Sliced_block_map<typename Block::map_type,
                                    D>::type map_type;

  // Constructors and destructors.
  Sliced_block_base(Sliced_block_base const& sb) VSIP_NOTHROW
    : map_(sb.map_), blk_(&*sb.blk_), index_(sb.index_)
  { map_.impl_apply(block_domain<dim>(*this)); }
  Sliced_block_base(Block &blk, index_type i) VSIP_NOTHROW
    : map_(Sliced_block_map<typename Block::map_type,
	                    D>::convert_map(blk.map(), i)),
      blk_(&blk),
      index_(i)
  { map_.impl_apply(block_domain<dim>(*this)); }
  ~Sliced_block_base() VSIP_NOTHROW {}

  map_type const& map() const { return map_;}

  // Accessors.
  // The total size of a sliced block is the total size of the underlying
  // block, divided by the size of the bound index.
  length_type size() const VSIP_NOTHROW
  { return index_ == no_index ? 0 : blk_->size() / blk_->size(Block::dim, D);}
  length_type size(dimension_type block_d, dimension_type d) const VSIP_NOTHROW
  { return index_ == no_index ? 0 :
      blk_->size(block_d + 1, Compare<dimension_type, D>() > d ? d : d + 1);
  }
  // These are noops as Sliced_block is helt by-value.
  void increment_count() const VSIP_NOTHROW {}
  void decrement_count() const VSIP_NOTHROW {}

  Block const& impl_block() const { return *this->blk_; }
  index_type   impl_index() const { return this->index_; }

  // Support Direct_data interface.
public:
  typedef impl::Storage<typename Block_layout<Block>::complex_type, value_type>
		storage_type;
  typedef typename storage_type::type       data_type;
  typedef typename storage_type::const_type const_data_type;

  par_ll_pbuf_type impl_ll_pbuf() VSIP_NOTHROW
  { return blk_->impl_ll_pbuf(); }

  stride_type impl_offset() VSIP_NOTHROW
  {
    return blk_->impl_offset() + index_*blk_->impl_stride(Block::dim, D);
  }

  data_type       impl_data()       VSIP_NOTHROW
  {
    return storage_type::offset(blk_->impl_data(),
				index_*blk_->impl_stride(Block::dim, D));
  }
  const_data_type impl_data() const VSIP_NOTHROW
  {
    return storage_type::offset(blk_->impl_data(),
				index_*blk_->impl_stride(Block::dim, D));
  }
  stride_type impl_stride(dimension_type Dim, dimension_type d)
     const VSIP_NOTHROW
  {
    assert(Dim == dim && d<dim);
    return blk_->impl_stride(Block::dim,
			     Compare<dimension_type, D>() > d ? d : d+1);
  }

protected:
  // Data members.
  map_type                                 map_;
  typename View_block_storage<Block>::type blk_;
  index_type const                         index_;
};

template <typename Block>
class Sliced_block<Block, 0> : public Sliced_block_base<Block, 0>
{
  typedef Sliced_block_base<Block, 0> Base;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::reference_type reference_type;

  Sliced_block(Sliced_block const& sb) VSIP_NOTHROW : Base(sb) {}
  Sliced_block(Block &blk, index_type i) VSIP_NOTHROW : Base(blk, i) {}

  // Data accessors.
  value_type get(index_type i) const VSIP_NOTHROW 
  { return this->blk_->get(this->index_, i);}
  value_type get(index_type i, index_type j) const VSIP_NOTHROW
  { return this->blk_->get(this->index_, i, j);}

  void put(index_type i, value_type val) VSIP_NOTHROW
  { this->blk_->put(this->index_, i, val);}
  void put(index_type i, index_type j, value_type val) VSIP_NOTHROW
  { this->blk_->put(this->index_, i, j, val);}

  reference_type impl_ref(index_type i) VSIP_NOTHROW
  { return this->blk_->impl_ref(this->index_, i); }
  reference_type impl_ref(index_type i, index_type j) VSIP_NOTHROW
  { return this->blk_->impl_ref(this->index_, i, j); }
};

template <typename Block>
class Sliced_block<Block, 1> : public Sliced_block_base<Block, 1>
{
  typedef Sliced_block_base<Block, 1> Base;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::reference_type reference_type;

  Sliced_block(Sliced_block const& sb) VSIP_NOTHROW : Base(sb) {}
  Sliced_block(Block &blk, index_type i) VSIP_NOTHROW : Base(blk, i) {}

  // Data accessors.
  value_type get(index_type i) const VSIP_NOTHROW 
  { return this->blk_->get(i, this->index_);}
  value_type get(index_type i, index_type j) const VSIP_NOTHROW
  { return this->blk_->get(i, this->index_, j);}

  void put(index_type i, value_type val) VSIP_NOTHROW
  { this->blk_->put(i, this->index_, val);}
  void put(index_type i, index_type j, value_type val) VSIP_NOTHROW
  { this->blk_->put(i, this->index_, j, val);}

  reference_type impl_ref(index_type i) const VSIP_NOTHROW 
  { return this->blk_->impl_ref(i, this->index_);}
  reference_type impl_ref(index_type i, index_type j) const VSIP_NOTHROW
  { return this->blk_->impl_ref(i, this->index_, j);}
};

template <typename Block>
class Sliced_block<Block, 2> : public Sliced_block_base<Block, 2>
{
  typedef Sliced_block_base<Block, 2> Base;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::reference_type reference_type;

  Sliced_block(Sliced_block const& sb) VSIP_NOTHROW : Base(sb) {}
  Sliced_block(Block &blk, index_type i) VSIP_NOTHROW : Base(blk, i) {}

  // Data accessors.
  value_type get(index_type i, index_type j) const VSIP_NOTHROW
  { return this->blk_->get(i, j, this->index_);}

  void put(index_type i, index_type j, value_type val) VSIP_NOTHROW
  { this->blk_->put(i, j, this->index_, val);}

  reference_type impl_ref(index_type i, index_type j) const VSIP_NOTHROW
  { return this->blk_->impl_ref(i, j, this->index_);}
};


/// The Sliced2_block class binds two of the indices of the underlying
/// N-dimensional block to a constant, producing an N-2-dimensional
/// block.  N must be >= 3, and the index chosen must be allowed for
/// the underlying block.

template <typename Block, dimension_type D1, dimension_type D2> 
class Sliced2_block;

// Store Sliced2_block by-value.
template <typename Block, dimension_type D1, dimension_type D2>
struct View_block_storage<Sliced2_block<Block, D1, D2> >
  : By_value_block_storage<Sliced2_block<Block, D1, D2> >
{};

// Sliced2_block has impl_ref if the underlying block has impl_ref.
template <typename Block, dimension_type D1, dimension_type D2>
struct Lvalue_factory_type<Sliced2_block<Block, D1, D2> >
{
  typedef typename Lvalue_factory_type<Block>
    ::template Rebind<Sliced2_block<Block, D1, D2> >::type type;
  template <typename OtherBlock>
  struct Rebind {
    typedef typename Lvalue_factory_type<Block>
      ::template Rebind<OtherBlock>::type type;
  };
};

template <typename BlockT, dimension_type D1, dimension_type D2>
struct Is_modifiable_block<Sliced2_block<BlockT, D1, D2> >
  : Is_modifiable_block<BlockT>
{};

template <typename Block, dimension_type D1, dimension_type D2>
class Sliced2_block_base 
  : public impl::Compile_time_assert<(Block::dim > D2 && D2 > D1)>,
    public impl::Non_assignable
{
public:
  // Compile-time values and types.
  static dimension_type const dim = Block::dim - 2;
  typedef typename Block::value_type value_type;
  typedef value_type&       reference_type;
  typedef value_type const& const_reference_type;
  typedef typename Sliced2_block_map<typename Block::map_type,
                                     D1,
                                     D2>::type map_type;

  // Constructors and destructors.
  Sliced2_block_base(Sliced2_block_base const& sb) VSIP_NOTHROW
    : map_(sb.map_), blk_(&*sb.blk_), index1_(sb.index1_), index2_(sb.index2_)
  { map_.impl_apply(block_domain<dim>(*this)); }
  Sliced2_block_base(Block &blk, index_type i, index_type j) VSIP_NOTHROW
    : map_(Sliced2_block_map<typename Block::map_type,
	                     D1,
	                     D2>::convert_map(blk.map(), i, j)),
      blk_(&blk), index1_(i), index2_(j)
  { map_.impl_apply(block_domain<dim>(*this)); }
  ~Sliced2_block_base() VSIP_NOTHROW {}

  map_type const& map() const { return map_;}

  // Accessors.
  // The total size of a sliced block is the total size of the underlying
  // block, divided by the size of the bound index.
  length_type size() const VSIP_NOTHROW
  { return blk_->size() / blk_->size(Block::dim, D1) / blk_->size(Block::dim, D2);}
  length_type size(dimension_type block_d, dimension_type d) const VSIP_NOTHROW
  { return blk_->size(block_d + 2,
		      Compare<dimension_type, D1>() > d     ? d   :
		      Compare<dimension_type, D2>() > (d+1) ? d+1 : d+2);
  }
  // These are noops as Sliced2_block is helt by-value.
  void increment_count() const VSIP_NOTHROW {}
  void decrement_count() const VSIP_NOTHROW {}

  Block const& impl_block()  const { return *this->blk_; }
  index_type   impl_index1() const { return this->index1_; }
  index_type   impl_index2() const { return this->index2_; }

  // Support Direct_data interface.
public:
  typedef impl::Storage<typename Block_layout<Block>::complex_type, value_type>
		storage_type;
  typedef typename storage_type::type       data_type;
  typedef typename storage_type::const_type const_data_type;

  par_ll_pbuf_type impl_ll_pbuf() VSIP_NOTHROW
  { return blk_->impl_ll_pbuf(); }

  stride_type impl_offset() VSIP_NOTHROW
  {
    return blk_->impl_offset()
	 + index1_*blk_->impl_stride(Block::dim, D1)
	 + index2_*blk_->impl_stride(Block::dim, D2);
  }

  data_type       impl_data()       VSIP_NOTHROW
  {
    return storage_type::offset(blk_->impl_data(),
				+ index1_*blk_->impl_stride(Block::dim, D1)
				+ index2_*blk_->impl_stride(Block::dim, D2));
  }

  const_data_type impl_data() const VSIP_NOTHROW
  {
    return storage_type::offset(blk_->impl_data(),
				+ index1_*blk_->impl_stride(Block::dim, D1)
				+ index2_*blk_->impl_stride(Block::dim, D2));
  }

  stride_type impl_stride(dimension_type Dim, dimension_type d)
     const VSIP_NOTHROW
  {
    assert(Dim == dim && d<dim);
    return blk_->impl_stride(Block::dim,
		      Compare<dimension_type, D1>() > d     ? d   :
		      Compare<dimension_type, D2>() > (d+1) ? d+1 : d+2);
  }

protected:
  // Data members.
  map_type                                 map_;
  typename View_block_storage<Block>::type blk_;
  index_type const                         index1_;
  index_type const                         index2_;
};

template <typename Block>
class Sliced2_block<Block, 0, 1> : public Sliced2_block_base<Block, 0, 1>
{
  typedef Sliced2_block_base<Block, 0, 1> Base;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::reference_type reference_type;

  Sliced2_block(Sliced2_block const& sb) VSIP_NOTHROW : Base(sb) {}
  Sliced2_block(Block &blk, index_type i, index_type j) VSIP_NOTHROW 
    : Base(blk, i, j)
  {}

  // Data accessors.
  value_type get(index_type i) const VSIP_NOTHROW 
  { return this->blk_->get(this->index1_, this->index2_, i);}

  void put(index_type i, value_type val) VSIP_NOTHROW
  { this->blk_->put(this->index1_, this->index2_, i, val);}

  reference_type impl_ref(index_type i) const VSIP_NOTHROW 
  { return this->blk_->impl_ref(this->index1_, this->index2_, i);}
};

template <typename Block>
class Sliced2_block<Block, 0, 2> : public Sliced2_block_base<Block, 0, 2>
{
  typedef Sliced2_block_base<Block, 0, 2> Base;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::reference_type reference_type;

  Sliced2_block(Sliced2_block const& sb) VSIP_NOTHROW : Base(sb) {}
  Sliced2_block(Block &blk, index_type i, index_type j) VSIP_NOTHROW 
    : Base(blk, i, j)
  {}

  // Data accessors.
  value_type get(index_type i) const VSIP_NOTHROW 
  { return this->blk_->get(this->index1_, i, this->index2_);}

  void put(index_type i, value_type val) VSIP_NOTHROW
  { this->blk_->put(this->index1_, i, this->index2_, val);}

  reference_type impl_ref(index_type i) const VSIP_NOTHROW 
  { return this->blk_->impl_ref(this->index1_, i, this->index2_);}
};

template <typename Block>
class Sliced2_block<Block, 1, 2> : public Sliced2_block_base<Block, 1, 2>
{
  typedef Sliced2_block_base<Block, 1, 2> Base;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::reference_type reference_type;

  Sliced2_block(Sliced2_block const& sb) VSIP_NOTHROW : Base(sb) {}
  Sliced2_block(Block &blk, index_type i, index_type j) VSIP_NOTHROW 
    : Base(blk, i, j)
  {}

  // Data accessors.
  value_type get(index_type i) const VSIP_NOTHROW 
  { return this->blk_->get(i, this->index1_, this->index2_);}

  void put(index_type i, value_type val) VSIP_NOTHROW
  { this->blk_->put(i, this->index1_, this->index2_, val);}

  reference_type impl_ref(index_type i) const VSIP_NOTHROW 
  { return this->blk_->impl_ref(i, this->index1_, this->index2_);}
};



template <typename       Tuple,
	  dimension_type NumDim,
	  dimension_type FixedDim>
struct Sliced_block_order;

template <dimension_type Dim0,
	  dimension_type Dim1,
	  dimension_type Dim2,
	  dimension_type FixedDim>
struct Sliced_block_order<tuple<Dim0, Dim1, Dim2>, 2, FixedDim>
{
  typedef row1_type type;
  static bool const unit_stride_preserved = (FixedDim != Dim1);
};

template <dimension_type Dim0,
	  dimension_type Dim1,
	  dimension_type Dim2,
	  dimension_type FixedDim>
struct Sliced_block_order<tuple<Dim0, Dim1, Dim2>, 3, FixedDim>
{
  typedef typename
  ITE_Type<FixedDim == Dim0,
	   ITE_Type<(Dim2 > Dim1), As_type<row2_type>, As_type<col2_type> >,
  ITE_Type<FixedDim == Dim1,
	   ITE_Type<(Dim2 > Dim0), As_type<row2_type>, As_type<col2_type> >,
           ITE_Type<(Dim1 > Dim0), As_type<row2_type>, As_type<col2_type> >
          > >::type
		type;
  static bool const unit_stride_preserved = (FixedDim != Dim2);
};



template <typename       BlockT,
	  dimension_type Dim>
struct Block_root<Sliced_block<BlockT, Dim> >
{
  typedef typename Block_root<BlockT>::type type;
};

template <typename       BlockT,
	  dimension_type Dim>
typename Block_root<Sliced_block<BlockT, Dim> >::type const&
block_root(Sliced_block<BlockT, Dim> const& block)
{
  return block_root(block.impl_block());
}



template <typename       Block,
	  dimension_type Dim>
struct Block_layout<Sliced_block<Block, Dim> >
{
  // Dimension: reduced by 1
  // Access   : Same
  // Order    : preserve non-fixed order
  // Stride   : Stride_unit if parent Stride_unit* and FixeDim != 0,
  //            Stride_unknown otherwise
  // Cmplx    : Same
private:
  typedef Sliced_block_order<typename Block_layout<Block>::order_type,
			   Block::dim,
			   Dim> sbo_type;

public:

  static dimension_type const dim = Block::dim-1;

  typedef typename Block_layout<Block>::access_type access_type;
  typedef typename sbo_type::type                   order_type;
  typedef typename ITE_Type<
    Block_layout<Block>::pack_type::is_ct_unit_stride &&
    sbo_type::unit_stride_preserved,
    As_type<Stride_unit>, As_type<Stride_unknown> >::type pack_type;
  typedef typename Block_layout<Block>::complex_type complex_type;

  typedef Layout<dim, order_type, pack_type, complex_type> layout_type;
};



template <typename       BlockT,
	  dimension_type D1,
	  dimension_type D2> 
struct Block_root<Sliced2_block<BlockT, D1, D2> >
{
  typedef typename Block_root<BlockT>::type type;
};

template <typename       BlockT,
	  dimension_type D1,
	  dimension_type D2> 
typename Block_root<Sliced2_block<BlockT, D1, D2> >::type const&
block_root(Sliced2_block<BlockT, D1, D2> const& block)
{
  return block_root(block.impl_block());
}



template <typename       Block,
	  dimension_type D1,
	  dimension_type D2> 
struct Block_layout<Sliced2_block<Block, D1, D2> >
{
  // Dimension: reduced by 2
  // Access   : Same
  // Order    : preserve non-fixed order
  // Stride   : Stride_unit if parent Stride_unit* and 
  //                           D1 != lowest dim and D2 != lowest dim
  //            Stride_unknown otherwise
  // Cmplx    : Same
private:
  typedef typename Block_layout<Block>::order_type par_order_type;

public:
  static dimension_type const dim = Block::dim-2;

  typedef typename Block_layout<Block>::access_type access_type;
  typedef row1_type                                 order_type;

  typedef typename ITE_Type<
    Block_layout<Block>::pack_type::is_ct_unit_stride &&
    D1 != par_order_type::impl_dim2 && D2 != par_order_type::impl_dim2,
    As_type<Stride_unit>, As_type<Stride_unknown> >::type pack_type;
  typedef typename Block_layout<Block>::complex_type complex_type;

  typedef Layout<dim, order_type, pack_type, complex_type> layout_type;
};



template <typename Block>
struct Distributed_local_block<Subset_block<Block> >
{
  typedef Subset_block<typename Distributed_local_block<Block>::type> type;
  typedef Subset_block<typename Distributed_local_block<Block>::proxy_type> proxy_type;
};



template <typename       Block,
	  dimension_type D>
struct Distributed_local_block<Sliced_block<Block, D> >
{
  typedef Sliced_block<typename Distributed_local_block<Block>::type, D> type;
  typedef Sliced_block<typename Distributed_local_block<Block>::proxy_type, D> proxy_type;
};



template <typename       Block,
	  dimension_type D1,
	  dimension_type D2> 
struct Distributed_local_block<Sliced2_block<Block, D1, D2> >
{
  typedef Sliced2_block<typename Distributed_local_block<Block>::type, D1, D2>
		type;
  typedef Sliced2_block<typename Distributed_local_block<Block>::proxy_type, D1, D2>
		proxy_type;
};



template <typename Block>
struct Distributed_local_block<Transposed_block<Block> >
{
  typedef Transposed_block<typename Distributed_local_block<Block>::type> type;
  typedef Transposed_block<typename Distributed_local_block<Block>::proxy_type>
		proxy_type;
};



template <typename Block,
          template <typename> class Extractor>
struct Distributed_local_block<Component_block<Block, Extractor> >
{
  typedef Component_block<typename Distributed_local_block<Block>::type,
			  Extractor> type;
  typedef Component_block<typename Distributed_local_block<Block>::proxy_type,
			  Extractor> proxy_type;
};



// Helper class to translate a distributed subset into a local subset.
//
// In general case, ask the parent block what the local subset should be.

template <typename BlockT,
	  typename MapT = typename BlockT::map_type>
struct Subset_parent_local_domain
{
  static dimension_type const dim = BlockT::dim;

  static Domain<dim> parent_local_domain(BlockT const& block, index_type sb)
  {
    return block.impl_block().map().impl_local_from_global_domain(sb,
					block.impl_domain());
  }
};



// Specialize for Subset_map, where the local subset is currently larger than
// what the parent thinks it should be.

template <typename       BlockT,
	  dimension_type Dim>
struct Subset_parent_local_domain<BlockT, Subset_map<Dim> >
{
  static dimension_type const dim = BlockT::dim;

  static Domain<dim> parent_local_domain(BlockT const& block, index_type sb)
  {
#if 0
    // consider asking Subset_map about parent's local block,
    // rather than asking the parent to recompute it.
    return block.map().template impl_parent_local_domain<dim>(sb);
#else
    index_type parent_sb = block.map().impl_parent_subblock(sb);
    return block.impl_block().map().impl_local_from_global_domain(parent_sb,
					block.impl_domain());
#endif
  }
};



template <typename Block>
Subset_block<typename Distributed_local_block<Block>::type>
get_local_block(
  Subset_block<Block> const& block)
{
  typedef typename Distributed_local_block<Block>::type super_type;
  typedef Subset_block<super_type>                      local_block_type;

  dimension_type const dim = Subset_block<Block>::dim;

  index_type sb = block.map().impl_rank_from_proc(local_processor());
  Domain<dim> dom;

  if (sb != no_subblock)
    dom = Subset_parent_local_domain<Subset_block<Block> >::
      parent_local_domain(block, sb);
  else
    dom = empty_domain<dim>();

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_block(block.impl_block());

  return local_block_type(dom, super_block);
}



template <typename Block>
Subset_block<typename Distributed_local_block<Block>::proxy_type>
get_local_proxy(
  Subset_block<Block> const& block,
  index_type                    sb)
{
  static dimension_type const dim = Block::dim;

  typedef typename Distributed_local_block<Block>::proxy_type super_type;
  typedef Subset_block<super_type>                            local_proxy_type;

  index_type super_sb = Subset_block_map<dim, typename Block::map_type>::
    parent_subblock(block.impl_block().map(), block.impl_domain(), sb);

  Domain<dim> l_dom = block.impl_block().map().
    impl_local_from_global_domain(sb,
				  block.impl_domain());

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_proxy(block.impl_block(), super_sb);

  return local_proxy_type(l_dom, super_block);
}



template <typename       Block,
	  dimension_type D>
Sliced_block<typename Distributed_local_block<Block>::type, D>
get_local_block(
  Sliced_block<Block, D> const& block)
{
  typedef Sliced_block<typename Distributed_local_block<Block>::type, D>
	local_block_type;

  // This conversion is only valid if the local processor holds
  // the subblock containing the slice.

  index_type idx;
  if (block.map().subblock() != no_subblock)
    idx = block.impl_block().map().
      impl_local_from_global_index(D, block.impl_index());
  else
    idx = no_index;

  return local_block_type(get_local_block(block.impl_block()), idx);
}



template <typename       Block,
	  dimension_type D>
Sliced_block<typename Distributed_local_block<Block>::proxy_type, D>
get_local_proxy(
  Sliced_block<Block, D> const& block,
  index_type                    sb)
{
  typedef typename Distributed_local_block<Block>::proxy_type super_type;
  typedef Sliced_block<super_type, D>                         local_proxy_type;

  index_type super_sb = Sliced_block_map<typename Block::map_type, D>::
    parent_subblock(block.impl_block().map(), block.impl_index(), sb);

  index_type l_idx = block.impl_block().map().
      impl_local_from_global_index(D, block.impl_index());

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_proxy(block.impl_block(), super_sb);
  return local_proxy_type(super_block, l_idx);
}



template <typename       Block,
	  dimension_type D1,
	  dimension_type D2>
Sliced2_block<typename Distributed_local_block<Block>::type, D1, D2>
get_local_block(
  Sliced2_block<Block, D1, D2> const& block)
{
  typedef Sliced2_block<typename Distributed_local_block<Block>::type, D1, D2>
	local_block_type;

  index_type idx1 = block.impl_block().map().
    impl_local_from_global_index(D1, block.impl_index1());
  index_type idx2 = block.impl_block().map().
    impl_local_from_global_index(D2, block.impl_index2());

  return local_block_type(get_local_block(block.impl_block()), idx1, idx2);
}



template <typename       Block,
	  dimension_type D1,
	  dimension_type D2>
Sliced2_block<typename Distributed_local_block<Block>::proxy_type, D1, D2>
get_local_proxy(
  Sliced2_block<Block, D1, D2> const& block,
  index_type                          sb)
{
  typedef typename Distributed_local_block<Block>::proxy_type super_type;
  typedef Sliced2_block<super_type, D1, D2>                   local_block_type;

  index_type l_idx1 = block.impl_block().map().
    impl_local_from_global_index(D1, block.impl_index1());
  index_type l_idx2 = block.impl_block().map().
    impl_local_from_global_index(D2, block.impl_index2());

  index_type super_sb = Sliced2_block_map<typename Block::map_type, D1, D2>::
    parent_subblock(block.impl_block().map(),
		    block.impl_index1(), block.impl_index2(), sb);

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_proxy(block.impl_block(), super_sb);

  return local_block_type(get_local_block(block.impl_block()), l_idx1, l_idx2);
}



template <typename Block>
Transposed_block<typename Distributed_local_block<Block>::type>
get_local_block(
  Transposed_block<Block> const& block)
{
  typedef typename Distributed_local_block<Block>::type super_type;
  typedef Transposed_block<super_type>                  local_block_type;

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_block(block.impl_block());

  return local_block_type(super_block);
}



template <typename Block>
Transposed_block<typename Distributed_local_block<Block>::proxy_type>
get_local_proxy(
  Transposed_block<Block> const& block,
  index_type                     sb)
{
  typedef typename Distributed_local_block<Block>::proxy_type super_type;
  typedef Transposed_block<super_type>                        local_proxy_type;

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_proxy(block.impl_block(), sb);

  return local_proxy_type(super_block);
}



template <typename Block,
          template <typename> class Extractor>
Component_block<typename Distributed_local_block<Block>::type, Extractor>
get_local_block(
  Component_block<Block, Extractor> const& block)
{
  typedef typename Distributed_local_block<Block>::type super_type;
  typedef Component_block<super_type, Extractor>        local_block_type;

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_block(block.impl_block());

  return local_block_type(super_block);
}



template <typename Block,
          template <typename> class Extractor>
Component_block<typename Distributed_local_block<Block>::proxy_type, Extractor>
get_local_proxy(
  Component_block<Block, Extractor> const& block,
  index_type                               sb)
{
  typedef typename Distributed_local_block<Block>::proxy_type super_type;
  typedef Component_block<super_type, Extractor>              local_proxy_type;

  typename View_block_storage<super_type>::plain_type
    super_block = get_local_proxy(block.impl_block(), sb);

  return local_proxy_type(super_block);
}






template <typename Block>
void
assert_local(
  Subset_block<Block> const& /*block*/,
  index_type                 /*sb*/)
{
}



template <typename       Block,
	  dimension_type D>
void
assert_local(
  Sliced_block<Block, D> const& /*block*/,
  index_type                    /*sb*/)
{
}



template <typename       Block,
	  dimension_type D1,
	  dimension_type D2>
void
assert_local(
  Sliced2_block<Block, D1, D2> const& /*block*/,
  index_type                          /*sb*/)
{
}



template <typename Block>
void
assert_local(
  Transposed_block<Block> const& block,
  index_type                     sb)
{
  assert_local(block.impl_block(), sb);
}



#if VSIP_IMPL_USE_GENERIC_VISITOR_TEMPLATES==0

/// Specialize Combine_return_type for Subset_block leaves.

template <typename       CombineT,
	  typename       Block>
struct Combine_return_type<CombineT, Subset_block<Block> >
{
  typedef Subset_block<Block> block_type;
  typedef typename CombineT::template return_type<block_type>::type
		type;
  typedef typename CombineT::template tree_type<block_type>::type
		tree_type;
};



/// Specialize apply_combine for Subset_block leaves.

template <typename       CombineT,
	  typename       Block>
typename Combine_return_type<CombineT, Subset_block<Block> >::type
apply_combine(
  CombineT const&               combine,
  Subset_block<Block> const& block)
{
  return combine.apply(block);
}



/// Specialize Combine_return_type for Sliced_block leaves.

template <typename       CombineT,
	  typename       Block,
	  dimension_type D>
struct Combine_return_type<CombineT, Sliced_block<Block, D> >
{
  typedef Sliced_block<Block, D> block_type;
  typedef typename CombineT::template return_type<block_type>::type
		type;
  typedef typename CombineT::template tree_type<block_type>::type
		tree_type;
};



/// Specialize apply_combine for Sliced_block leaves.

template <typename       CombineT,
	  typename       Block,
	  dimension_type D>
typename Combine_return_type<CombineT, Sliced_block<Block, D> >::type
apply_combine(
  CombineT const&               combine,
  Sliced_block<Block, D> const& block)
{
  return combine.apply(block);
}



/// Specialize Combine_return_type for Sliced2_block leaves.

template <typename       CombineT,
	  typename       Block,
	  dimension_type D1,
	  dimension_type D2>
struct Combine_return_type<CombineT, Sliced2_block<Block, D1, D2> >
{
  typedef Sliced2_block<Block, D1, D2> block_type;
  typedef typename CombineT::template return_type<block_type>::type
		type;
  typedef typename CombineT::template tree_type<block_type>::type
		tree_type;
};



/// Specialize apply_combine for Sliced_block leaves.

template <typename       CombineT,
	  typename       Block,
	  dimension_type D1,
	  dimension_type D2>
typename Combine_return_type<CombineT, Sliced2_block<Block, D1, D2> >::type
apply_combine(
  CombineT const&                     combine,
  Sliced2_block<Block, D1, D2> const& block)
{
  return combine.apply(block);
}



/// Specialize Combine_return_type for Transposed_block leaves.

template <typename       CombineT,
	  typename       Block>
struct Combine_return_type<CombineT, Transposed_block<Block> >
{
  typedef Transposed_block<Block> block_type;
  typedef typename CombineT::template return_type<block_type>::type
		type;
  typedef typename CombineT::template tree_type<block_type>::type
		tree_type;
};



/// Specialize apply_combine for Transposed_block leaves.

template <typename       CombineT,
	  typename       Block>
typename Combine_return_type<CombineT, Transposed_block<Block> >::type
apply_combine(
  CombineT const&                combine,
  Transposed_block<Block> const& block)
{
  return combine.apply(block);
}
#endif





/// The Diag_block class is similar to the Sliced_block class, 
/// producing diagonal slices based on an offset.  The center diagonal
/// is obtained with offset zero.   Positive values refer to diagonals
/// above the center and negative values to diagonals below.
/// Note the length of the resultant vector is affected by the size of
/// the original matrix as well as the offset from the main diagonal.

template <typename Block> 
class Diag_block;


// Store Diag_block by-value.
template <typename Block>
struct View_block_storage<Diag_block<Block> >
  : By_value_block_storage<Diag_block<Block> >
{};

template <typename Block>
struct Block_layout<Diag_block<Block> >
{
  // Dimension: Same
  // Access   : Same
  // Order    : Same
  // Stride   : Stride_unknown
  // Cmplx    : Same

  static dimension_type const dim = 1;

  typedef typename Block_layout<Block>::access_type  access_type;
  typedef row1_type                                  order_type;
  typedef Stride_unknown                             pack_type;
  typedef typename Block_layout<Block>::complex_type complex_type;

  typedef Layout<dim, order_type, pack_type, complex_type> layout_type;
};


// Diag_block has impl_ref if the underlying block has impl_ref.
template <typename BlockT>
struct Lvalue_factory_type<Diag_block<BlockT> >
{
  typedef typename Lvalue_factory_type<BlockT>
    ::template Rebind<Diag_block<BlockT> >::type type;
  template <typename OtherBlock>
  struct Rebind {
    typedef typename Lvalue_factory_type<BlockT>
      ::template Rebind<OtherBlock>::type type;
  };
};

template <typename BlockT>
struct Is_modifiable_block<Diag_block<BlockT> >
  : Is_modifiable_block<BlockT>
{};

template <typename Block> 
class Diag_block 
    : public impl::Compile_time_assert<(Block::dim == 2)>,
      public impl::Non_assignable
{
 public:
  // Compile-time values and types.
  static dimension_type const dim = Block::dim - 1;
  typedef typename Block::value_type value_type;
  typedef value_type&       reference_type;
  typedef value_type const& const_reference_type;
  typedef typename Block::map_type map_type;


  // Constructors and destructors.
  Diag_block(Diag_block const& sb) VSIP_NOTHROW
    : blk_(&*sb.blk_), offset_(sb.offset_)
    { }
  Diag_block(Block &blk, index_difference_type offset) VSIP_NOTHROW
    : blk_(&blk), offset_(offset)
    { }
  ~Diag_block() VSIP_NOTHROW {}


  // Accessors.
  // The total size of a diagonal is...
  length_type size() const VSIP_NOTHROW
    { 
      assert(dim == 1);
      length_type size = 0;
      length_type limit = std::min(this->blk_->size(2, 0), this->blk_->size(2, 1));
      if ( this->offset_ >= 0 )
        size = std::min( limit, this->blk_->size(2, 1) - this->offset_ );
      else
        size = std::min( limit, this->blk_->size(2, 0) + this->offset_ );

      return size;
    }
  length_type size(dimension_type block_d, dimension_type d) const VSIP_NOTHROW
    { 
      assert(block_d == 1 && dim == 1 && d == 0);
      return this->size();
    }

  // These are noops as Diag_block is held by-value.
  void increment_count() const VSIP_NOTHROW {}
  void decrement_count() const VSIP_NOTHROW {}
  map_type const& map() const { return this->blk_->map(); }

  // Data accessors.
  value_type get(index_type i) const VSIP_NOTHROW
    { 
      if ( this->offset_ >= 0 )
        return this->blk_->get(i, i + this->offset_);
      else 
        return this->blk_->get(i - this->offset_, i);
    }

  void put(index_type i, value_type val) VSIP_NOTHROW
    {
      if ( this->offset_ >= 0 )
        return this->blk_->put(i, i + this->offset_, val);
      else 
        return this->blk_->put(i - this->offset_, i, val);
    }
  
  reference_type impl_ref(index_type i) VSIP_NOTHROW
    {
      if ( this->offset_ >= 0 )
        return this->blk_->impl_ref(i, i + this->offset_);
      else
        return this->blk_->impl_ref(i - this->offset_, i);
    }
  
  // Support Direct_data interface.
 public:
  typedef impl::Storage<typename Block_layout<Block>::complex_type, value_type>
    storage_type;
  typedef typename storage_type::type       data_type;
  typedef typename storage_type::const_type const_data_type;

  data_type       impl_data()       VSIP_NOTHROW
    { 
      if ( this->offset_ >= 0 )
        return storage_type::offset(
		this->blk_->impl_data(),
		+ this->offset_ * this->blk_->impl_stride(2, 1));
      else
        return storage_type::offset(
		this->blk_->impl_data(),
		- this->offset_ * this->blk_->impl_stride(2, 0));
    }

  const_data_type impl_data() const VSIP_NOTHROW
    { 
      if ( this->offset_ >= 0 )
        return this->blk_->impl_data() + this->offset_ * this->blk_->impl_stride(2, 1);
      else
        return this->blk_->impl_data() - this->offset_ * this->blk_->impl_stride(2, 0);
    }

  stride_type impl_stride(dimension_type Dim, dimension_type d)
    const VSIP_NOTHROW
    {
      assert(Dim == dim && d<dim);
      return this->blk_->impl_stride(2, 0) + this->blk_->impl_stride(2, 1);
    }
  
 private:
  // Data members.
  typename View_block_storage<Block>::type blk_;
  index_difference_type const              offset_;
};




} // namespace vsip::impl
} // namespace vsip

#endif // vsip/core/subblock.hpp


