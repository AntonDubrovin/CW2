//
// Created by Антон Дубровин on 19.12.2022.
//

#ifndef CW2_GRAPH_H
#define CW2_GRAPH_H

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>

#include "Node.h"

class Graph {
public:
    static parlay::sequence <Node> neighbours(Node &node, int cube_size) {
        parlay::sequence <Node> result = parlay::sequence<Node>();
        if (node.x < cube_size) {
            result.emplace_back(node.x + 1, node.y, node.z);
        }

        if (node.y < cube_size) {
            result.emplace_back(node.x, node.y + 1, node.z);
        }

        if (node.z < cube_size) {
            result.emplace_back(node.x, node.y, node.z + 1);
        }

        return result;
    }
};


#endif //CW2_GRAPH_H
