#define _FILE_OFFSET_BITS 64
#define _THREAD_SAFE

#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace boost;

#include <stdio.h>
#include <stdint.h>

extern "C" {
#include "../compat.h"
#include "../graph500.h"
}

typedef adjacency_list<vecS, vecS, undirectedS> Graph;
static Graph *g;

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
  // how many vertices do we have?
  int64_t num_vertices = 0;
  for( int64_t k = 0; k < nedge*2; k++ )
    {
      if( IJ_in[k] > num_vertices )
	{num_vertices = IJ_in[k];}
    }

  g = new Graph(num_vertices);
  for( int64_t k = 0; k < nedge; k++ )
    {
      add_edge(IJ_in[k*2], IJ_in[k*2+1], *g);
    }
  return err;
}

int
make_bfs_tree (int64_t *bfs_tree_out, int64_t *max_vtx_out,
	       int64_t srcvtx)
{
  int err = 0;

  *max_vtx_out = boost::num_vertices(*g) - 1;
  typedef Graph::vertex_descriptor Vertex;
  std::vector<Vertex> p(boost::num_vertices(*g));
  typedef std::vector<Vertex>::value_type* Piter;

  for( int64_t i = 0; i < boost::num_vertices(*g); i++ )
    {p[i] = -1;}
  p[srcvtx] = srcvtx;

  boost::breadth_first_search
    (*g, srcvtx, 
     boost::visitor(boost::make_bfs_visitor
		    (boost::record_predecessors(&p[0],
						boost::on_tree_edge()) )));
  for( int64_t i = 0; i < boost::num_vertices(*g); i++ )
    {bfs_tree_out[i] = p[i];}
  /*
  printf("source %" PRId64 ": [ ", srcvtx);
  for( int64_t i = 0; i < boost::num_vertices(*g); i++ )
    {printf("%" PRId64 " ", p[i]);}
  printf("]\n");
  */
  return err;
}

void
destroy_graph (void)
{
}
