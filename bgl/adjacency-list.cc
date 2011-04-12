#define _FILE_OFFSET_BITS 64
#define _THREAD_SAFE

#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
using namespace boost;

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

  return err;
}

void
destroy_graph (void)
{
}
