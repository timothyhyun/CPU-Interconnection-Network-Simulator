#include "interconnect_internal.h"
#include "stacks.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool dfs(ic_network_t *graph, int start, int end) {
  stack_t *v_stack = new_stack();
  bool *visited = calloc(sizeof(bool), graph->size);
  if (!graph->nodes[start].busy){
    push(v_stack, start);
  }
  while (!empty(v_stack)) {
    int vertex = pop(v_stack);
    if(!visited[vertex]) {
      visited[vertex] = true;
      ic_node_t vtx = graph->nodes[vertex];
      for (int i = 0; i < vtx.num_neighbors; i++) {
        int curr = vtx.connected[i];
        if (!graph->nodes[curr].busy) {
          push(v_stack, vtx.connected[i]);
        }
      }
    }
  }
}

void route(ic_network_t *graph) {
  
}