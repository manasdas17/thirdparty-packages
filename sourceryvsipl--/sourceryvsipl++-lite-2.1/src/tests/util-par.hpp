/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/util-par.hpp
    @author  Jules Bergmann
    @date    2005-05-10
    @brief   VSIPL++ Library: Common utilities for parallel/distributed
			      test cases.
*/

#ifndef VSIP_TESTS_UTIL_PAR_HPP
#define VSIP_TESTS_UTIL_PAR_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>
#include <string>

#include <vsip/vector.hpp>
#include <vsip/matrix.hpp>
#include <vsip/parallel.hpp>
#include <vsip/domain.hpp>

#include <vsip_csl/output.hpp>
#include "extdata-output.hpp"



/***********************************************************************
  Definitions
***********************************************************************/

/// Organize processors as a 2-D grid (roughly)

void
get_np_square(
  vsip::length_type& np,
  vsip::length_type& nr,
  vsip::length_type& nc)
{
  np = vsip::num_processors();
  nr = (vsip::length_type)floor(sqrt((double)np));
  nc = (vsip::length_type)floor((double)np/nr);

  assert(nr*nc <= np);
}



void
get_np_cube(
  vsip::length_type& np,
  vsip::length_type& n1,
  vsip::length_type& n2,
  vsip::length_type& n3)
{
  np = vsip::num_processors();
  // cbrt() may not be available, so do it manually.
  n1 = (vsip::length_type)floor(exp(log((double)np)/3));
  n2 = (vsip::length_type)floor((double)np/(n1*n1));
  n3 = (vsip::length_type)floor((double)np/(n1*n2));

  assert(n1*n2*n3 <= np);
}



/// Divide the number of processors in half (roughly)

void
get_np_half(
  vsip::length_type& np,
  vsip::length_type& nr,
  vsip::length_type& nc)
{
  np = vsip::num_processors();

  if (np >= 2)
  {
    nr = (vsip::length_type)floor((double)np/2);
    nc = 2;
  }
  else
  {
    nr = np;
    nc = 1;
  }
  assert(nr*nc <= np);
  assert(nr*nc >  0);
}



// Check validity of local view.

template <vsip::dimension_type                Dim,
	  template <typename, typename> class ViewT,
	  typename                            T,
	  typename                            BlockT>
inline void
check_local_view(
  ViewT<T, BlockT> view)
{
  typedef typename vsip::impl::Distributed_local_block<BlockT>::type
          local_block_t;

  typename BlockT::map_type const& map = view.block().map();
  typename ViewT<T, BlockT>::local_type lview = view.local();

  vsip::index_type sb = map.subblock();

  vsip::Domain<Dim> dom = subblock_domain(view, sb);
  test_assert(lview.size() == vsip::impl::size(dom));
  for (vsip::dimension_type d=0; d<Dim; ++d)
    test_assert(lview.size(d) == dom[d].size());

  if (sb == vsip::no_subblock)
    test_assert(lview.size() == 0);
}



// Syncronize processors and print message to screen.

template <typename Map>
void
msg(Map& map, std::string str)
{
  vsip::impl::Communicator comm = map.impl_comm();
  comm.barrier();
  if (comm.rank() == 0) std::cout << str;
  comm.barrier();
}



// Print a distributed vector view to cout.

template <typename T,
	  typename Block>
void
dump_view(
  char*                  name,
  vsip::Vector<T, Block> view)
{
  using vsip::index_type;
  using vsip::no_subblock;
  using vsip::impl::Distributed_local_block;

  typedef typename Block::map_type map_t;
  typedef typename Distributed_local_block<Block>::type local_block_t;

  map_t const& am     = view.block().map();

  msg(am, std::string(name) + " ------------------------------------------\n");
  std::cout << "(" << vsip::local_processor() << "): dump_view(" << name
	    << ")\n";
  std::cout << "(" << vsip::local_processor() << "):    map   "
	    << Type_name<map_t>::name() << "\n";
  std::cout << "(" << vsip::local_processor() << "):    block "
	    << Type_name<Block>::name() << "\n";

  index_type sb = am.subblock();
  if (sb != no_subblock)
  {
    vsip::Vector<T, local_block_t> local_view = view.local();

    for (index_type p=0; p<am.num_patches(sb); ++p)
    {
      std::cout << "  subblock: " << sb
		<< "  patch: " << p << std::endl;
      vsip::Domain<1> ldom = am.template local_domain<1>(sb, p);
      vsip::Domain<1> gdom = am.template global_domain<1>(sb, p);

      for (index_type i=0; i<ldom.length(); ++i) 
      {
	index_type li = ldom.impl_nth(i);
	index_type gi = gdom.impl_nth(i);
	std::cout << "    [" << li << ":" << gi << "] = " << local_view.get(li)
		  << std::endl;
      }
    }
  }
  else
  {
    std::cout << "  no_subblock" << std::endl;
  }

  msg(am, " ------------------------------------------\n");
}



// Print a distributed matrix view to cout.

template <typename T,
	  typename Block>
void
dump_view(
  char*                        name,
  vsip::const_Matrix<T, Block> view)
{
  using vsip::index_type;
  using vsip::dimension_type;
  using vsip::processor_type;
  using vsip::no_subblock;
  using vsip::impl::Distributed_local_block;

  dimension_type const dim = 2;
  typedef typename Block::map_type map_t;
  typedef typename Distributed_local_block<Block>::type local_block_t;

  map_t const& am    = view.block().map();
  vsip::impl::Communicator comm = am.impl_comm();
  vsip::Vector<processor_type> pset = vsip::processor_set();

  msg(am, std::string(name) + " ------------------------------------------\n");

  for (index_type i=0; i<pset.size(); i++)
  {
    comm.barrier();
    if (vsip::local_processor() == pset.get(i))
    {

      index_type sb = am.subblock();
      if (sb != no_subblock)
      {
	vsip::Matrix<T, local_block_t> local_view = view.local();

	for (index_type p=0; p<num_patches(view, sb); ++p)
	{
	  char str[256];
	  sprintf(str, "  lblock: %08lx",
		  (unsigned long)&(local_view.block()));

	  std::cout << "(" << vsip::local_processor() << "): dump_view(Matrix "
		    << name << ") "
		    << "  subblock: " << sb
		    << "  patch: " << p
		    << str
		    << std::endl;
	  vsip::Domain<dim> ldom = local_domain(view, sb, p);
	  vsip::Domain<dim> gdom = global_domain(view, sb, p);

	  for (index_type r=0; r<ldom[0].length(); ++r) 
	    for (index_type c=0; c<ldom[1].length(); ++c) 
	    {
	      index_type lr = ldom[0].impl_nth(r);
	      index_type lc = ldom[1].impl_nth(c);
	      
	      index_type gr = gdom[0].impl_nth(r);
	      index_type gc = gdom[1].impl_nth(c);
	      
	      std::cout << "(" << vsip::local_processor() << ") "
			<< sb << "/" << p
			<< "    ["
			<< lr << "," << lc << ":"
			<< gr << "," << gc << "] = "
			<< local_view.get(lr, lc)
			<< std::endl;
	    }
	}
      }
      else
      {
	std::cout << "(" << vsip::local_processor() << "): dump_view(Matrix "
		  << name << ") no subblock\n";
      }
    }
  }

  msg(am, " ------------------------------------------\n");
}



template <vsip::dimension_type Dim,
	  typename             MapT>
void
dump_map(MapT const& map)
{
  typedef typename MapT::processor_iterator p_iter_t;
  vsip::processor_type rank = vsip::local_processor();

  std::ostringstream s;
  s << map.impl_proc_from_rank(0);
  for (vsip::index_type i=1; i<map.num_processors(); ++i)
    s << "," << map.impl_proc_from_rank(i);

  std::cout << rank << ": " << Type_name<MapT>::name()
	    << " [" << s.str() << "]"
	    << std::endl;

  for (vsip::index_type sb=0; sb<map.num_subblocks(); ++sb)
  {
    std::cout << "  sub " << sb << ": ";
    // if (map.impl_is_applied())
    std::cout << map.template impl_global_domain<Dim>(sb, 0);
    std::cout << " [";

    for (p_iter_t p=map.processor_begin(sb); p != map.processor_end(sb); ++p)
    {
      std::cout << *p << " ";
    }
    std::cout << "]" << std::endl;
  }
}



// Function object to increment an element by a delta.  (Works with
// foreach_point)

template <vsip::dimension_type Dim,
	  typename             T>
class Increment
{
public:
  Increment(T delta) : delta_(delta) {}

  T operator()(T value,
	       vsip::Index<Dim> const&,
	       vsip::Index<Dim> const&)
    { return value + delta_; }

  // Member Data
private:
  T	delta_;
};



// Function object to set values to 'identity', a unique value
// computed from an elements global index.  (Works with foreach_point)

template <vsip::dimension_type Dim>
class Set_identity
{
public:

  Set_identity(vsip::Domain<Dim> const& dom, int k = 1, int o = 0)
    : dom_(dom), k_(k), o_(o) {}

  template <typename T>
  T operator()(T /*value*/,
	       vsip::Index<1> const& /*local*/,
	       vsip::Index<1> const& global)
    { return T(k_*global[0] + o_); }

  template <typename T>
  T operator()(T /*value*/,
	       vsip::Index<2> const& /*local*/,
	       vsip::Index<2> const& global)
  {
    vsip::index_type i = global[0]*dom_[1].length()+global[1];
    return T(k_*i+o_);
  }

  template <typename T>
  T operator()(T /*value*/,
	       vsip::Index<3> const& /*local*/,
	       vsip::Index<3> const& global)
  {
    vsip::index_type i = global[0]*dom_[1].length()*dom_[2].length()
                       + global[1]*dom_[2].length()
                       + global[2];
    return T(k_*i+o_);
  }

private:
  vsip::Domain<Dim> dom_;
  int         k_;
  int         o_;
};



// Function object to check values for 'identity', as set by the
// 'Set_identy' function object above.

template <vsip::dimension_type Dim>
class Check_identity
{
public:
  Check_identity(vsip::Domain<Dim> const& dom, int k = 1, int o = 0)
    : dom_(dom), k_(k), o_(o), good_(true) {}

  bool good() { return good_; }


  template <typename T>
  T operator()(T value,
	       vsip::Index<1> const& /*local*/,
	       vsip::Index<1> const& global)
  {
    int i = global[0];
    T expected = T(k_*i + o_);
    if (value != expected)
    {
      std::cout << "Check_identity: MISCOMPARE" << std::endl
		<< "  global   = " << global[0] << std::endl
		<< "  expected = " << expected << std::endl
		<< "  actual   = " << value << std::endl;
      good_ = false;
    }
    return value;
  }

  template <typename T>
  T operator()(T value,
	       vsip::Index<2> const& /*local*/,
	       vsip::Index<2> const& global)
  {
    int i = global[0]*dom_[1].length()+global[1];
    T expected = T(k_*i+o_);

    if (value != expected)
    {
      std::cout << "Check_identity: MISCOMPARE" << std::endl
		<< "  global   = " << global[0] << ", " << global[1] 
		<< std::endl
		<< "  expected = " << expected << std::endl
		<< "  actual   = " << value << std::endl;
      good_ = false;
    }
    return value;
  }

  template <typename T>
  T operator()(T value,
	       vsip::Index<3> const& /*local*/,
	       vsip::Index<3> const& global)
  {
    int i = global[0]*dom_[1].length()*dom_[2].length()
          + global[1]*dom_[2].length()
          + global[2];
    T expected = T(k_*i+o_);

    if (value != expected)
    {
      std::cout << "Check_identity: MISCOMPARE" << std::endl
		<< "  global   = " << global[0] << ", " << global[1] 
		<< std::endl
		<< "  expected = " << expected << std::endl
		<< "  actual   = " << value << std::endl;
      good_ = false;
    }
    return value;
  }

private:
  vsip::Domain<Dim> dom_;
  int         k_;
  int         o_;
  bool        good_;
};

#endif // VSIP_TESTS_UTIL_PAR_HPP
