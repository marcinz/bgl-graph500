
/* Author: Brian Makin (merimus@gmail.com) */

#define _FILE_OFFSET_BITS 64
#define _THREAD_SAFE

#define BOOST_GRAPH_USE_NEW_CSR_INTERFACE
#define BOOST_NO_CXX11_NULLPTR

#include <utility>
#include <vector>
#include <iostream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/graphviz.hpp>

using namespace boost;

#include <stdio.h>
#include <stdint.h>

extern "C" {
#include "../compat.h"
#include "../graph500.h"
}

template<typename T, typename Sequence = std::vector<T> >
class vector_queue {
public:
  vector_queue() : c(), _bottom(0), _top(0) {}
  vector_queue(size_t sz) : c(sz), _bottom(0), _top(0) {}
  explicit vector_queue(Sequence&& __c) : c(std::forward<Sequence>(__c)), _bottom(0), _top(0) {}
  
  void push(T&& v) { c[_top++] = std::forward<T>(v); }
  void push(T& v) { c[_top++] = v; }
  T& top() { return c[_bottom]; }
  void pop() { ++_bottom; }
  bool empty() { return _bottom == _top; }
private:
  Sequence c;
  size_t _bottom, _top;
};

// template<typename Sequence, typename IndexMap, typename Value, Value v_white>
// class pred_to_color {
// public:
//   pred_to_color(const Sequence &seq, const IndexMap& index = IndexMap()) : seq(seq), index(index) {}

//   typedef typename property_traits<IndexMap>::key_type key_type;
//   typedef one_bit_color_type value_type;
//   typedef void reference;
//   typedef read_write_property_map_tag category;

//   template<typename T>
//   friend void put(const pred_to_color& map, key_type k, value_type v) { }
//   friend one_bit_color_type get(const pred_to_color& map, key_type k) {
//     if(map.seq[get(map.index, k)] == v_white)
//       return one_bit_white;
//     else
//       return one_bit_not_white;
//   }
// private:
//   const Sequence &seq;
//   const IndexMap index;
// };

// template<typename Sequence, typename IndexMap, typename Value>
// pred_to_color<Sequence, IndexMap, Value, -1>
// make_pred_to_color(const Sequence& seq, IndexMap index, Value v) {
//   return pred_to_color<Sequence, IndexMap, Value, -1>(seq, index);
// }

template<typename PredMap, typename boost::property_traits<PredMap>::value_type white = -1>
class pred_to_color {
public:
  typedef one_bit_color_type value_type;
  typedef void reference;
  typedef typename boost::property_traits<PredMap>::key_type key_type;
  typedef read_write_property_map_tag category;

  friend void put(const pred_to_color& map, key_type k, value_type v) { }
  friend one_bit_color_type get(const pred_to_color& map, key_type k) {
    if(get(map.pmap, k) == white)
      return one_bit_white;
    else
      return one_bit_not_white;
  }         

  pred_to_color(const PredMap pmap) : pmap(pmap) {}
private:
  const PredMap pmap;
};

template<typename PredMap>
pred_to_color<PredMap>
make_pred_to_color(PredMap pmap) { return pred_to_color<PredMap>(pmap); }

typedef compressed_sparse_row_graph<directedS, no_property, no_property, no_property, int64_t> Graph;
static Graph *g;

typedef std::pair<int64_t, int64_t> edge_t;

int 
create_graph_from_edgelist (int64_t *IJ_in, int64_t nedge)
{
  int err = 0;

  /*  
  {
    printf("[ ");
    for( int64_t i = 0; i < nedge; i++ )
      {
	printf("<%" PRId64 ",%" PRId64 "> ", IJ_in[2*i], IJ_in[2*i+1]);
      }
    printf(" ]\n");
  }
  */

  int64_t num_vertices = 0;
  for( int64_t k = 0; k < nedge*2; k++ )
    {
      if( IJ_in[k] > num_vertices )
	{num_vertices = IJ_in[k];}
    }

  std::vector<edge_t> edges;
  for( int64_t k = 0; k < nedge; k++ )
    {
      edges.push_back(edge_t(IJ_in[k*2], IJ_in[k*2+1]));
      edges.push_back(edge_t(IJ_in[k*2+1], IJ_in[k*2]));
    }

  g = new Graph(edges_are_unsorted_multi_pass, edges.begin(), edges.end(), num_vertices+1);

  //  write_graphviz(std::cout, *g);

  return err;
}

int
make_bfs_tree (int64_t *bfs_tree_out, int64_t *max_vtx_out,
	       int64_t srcvtx)
{
  int err = 0;
  const Graph &graph = *g;

  int64_t * restrict p = bfs_tree_out;
  *max_vtx_out = boost::num_vertices(graph) - 1;
  typedef Graph::vertex_descriptor Vertex;
  //std::vector<Vertex> p(boost::num_vertices(graph));
  typedef std::vector<Vertex>::value_type* Piter;

//    for( int64_t i = 0; i < boost::num_vertices(graph); i++ )
//      {p[i] = -1;}
//    p[srcvtx] = srcvtx;

  for (int64_t k1 = 0; k1 < boost::num_vertices(graph); ++k1)
    p[k1] = -1;
  p[srcvtx] = srcvtx;

  vector_queue<Vertex> buffer(boost::num_vertices(graph));
  boost::breadth_first_search
    (graph, srcvtx, 
     boost::visitor(boost::make_bfs_visitor
                   (boost::record_predecessors(p,
                                               boost::on_tree_edge()) ))
     .buffer(buffer)
     .color_map(make_pred_to_color(p))
     );
//    for( int64_t i = 0; i < boost::num_vertices(graph); i++ )
//      {bfs_tree_out[i] = p[i];}

  /*
  printf("source %" PRId64 ": [ ", srcvtx);
  for( int64_t i = 0; i < boost::num_vertices(graph); i++ )
    {printf("%" PRId64 " ", p[i]);}
  printf("]\n");
  */

  return err;
}

void
destroy_graph (void)
{
  delete g;
}
