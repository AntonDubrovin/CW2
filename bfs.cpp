#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include "Node.h"
#include "Graph.h"
#include <sys/time.h>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>

using namespace std;

int cube_size = 500;

vector<vector<vector<int>>> bfs_seq(Node *start) {
    queue<Node> q;
    q.push(*start);

    vector<vector<vector<bool>>> visited(cube_size + 1, vector<vector<bool>>(cube_size + 1, vector<bool>(false)));
    vector<vector<vector<int>>> dist(cube_size + 1, vector<vector<int>>(cube_size + 1, vector<int>(cube_size + 1, 0)));

    visited[start->x][start->y][start->z] = true;

    int count = 0;
    while (!q.empty()) {
        Node vert = q.front();
        q.pop();
        count++;

        parlay::sequence <Node> neighbours = Graph::neighbours(vert, cube_size);
        for (auto to: neighbours) {
            if (!visited[to.x][to.y][to.z]) {
                visited[to.x][to.y][to.z] = true;
                q.push(to);
                dist[to.x][to.y][to.z] = dist[vert.x][vert.y][vert.z] + 1;
            }
        }
    }

    return dist;
}

parlay::sequence<parlay::sequence<Node>> bfs_par(const Node *start) {
    parlay::sequence<parlay::sequence<parlay::sequence<atomic_flag>>> visited =
            parlay::tabulate<parlay::sequence<parlay::sequence<atomic_flag> >>(
            cube_size + 1,
            [&](const int x) {
                return parlay::tabulate< parlay::sequence<atomic_flag>>(
                        cube_size + 1,
                        [&](const int y) {
                            return parlay::tabulate<atomic_flag>(
                                cube_size + 1,
                                [&](const int z) {
                                    return start->x == x && start->y == y && start->z == z;
                                });
                        });
            });

    parlay::sequence<Node> frontier(1, *start);
    parlay::sequence<parlay::sequence<Node>> frontiers;

    while (!frontier.empty()) {
        frontiers.push_back(frontier);

        const parlay::sequence<unsigned long long> deg = parlay::map(
                frontier, [&](Node v) { return Graph::neighbours(v).size(); });

        const auto deg_scan = parlay::scan(deg);

        const parlay::sequence<unsigned long long> deg_scan_inclusive = deg_scan.first;

        const unsigned long last = deg_scan.second;

        parlay::sequence<Node> new_frontier =
                parlay::tabulate(last, [&](const int i) { return Node(-1, -1, -1); });

        parlay::parallel_for(0, frontier.size(), [&](size_t i) {
            const Node node = frontier[i];
            const parlay::sequence<Node> neighbours = Graph::neighbours(node);
            parlay::parallel_for(0, neighbours.size(), [&](size_t u) {
                const auto to = neighbours[u];

                if (!visited[to.x][to.y][to.z].test_and_set(memory_order_relaxed)) {
                    new_frontier[deg_scan_inclusive[i] + u] = to;
                }
            });
        });

        frontier = parlay::filter(new_frontier, [](Node node) {
            return node.x != -1 && node.y != -1 && node.z != -1;
        });
    }

    return frontiers;
}

bool is_correct_par(const parlay::sequence<parlay::sequence<Node>> &frontiers) {
    bool is_correct = true;

    for (int i = 0; i < frontiers.size(); i++) {
        const parlay::sequence<Node> &frontier = frontiers[i];

        for (auto node: frontier) {
            is_correct &= (node.x + node.y + node.z == i);
        }
    }

    return is_correct;
}

bool is_correct_seq(vector<vector<vector<int>>> &dists) {
    bool is_correct = true;

    for (int x = 0; x < dists.size(); x++) {
        for (int y = 0; y < dists[x].size(); y++) {
            for (int z = 0; z < dists[x][y].size(); z++) {
                is_correct &= (dists[x][y][z] == x + y + z);
            }
        }
    }
    return is_correct;
}



int main() {
    parlay::sequence<parlay::sequence<parlay::sequence<Node>>> parallel_res;
    parlay::sequence<vector<vector<vector<int>>> sequential_res;

    const Node *start = new Node(0, 0, 0);

    parlay::internal::timer par_timer("par bfs");
    for (int i = 0; i < 5; i++) {
        parlay::sequence<parlay::sequence<Node>> par_res = bfs_par(start);
        parallel_res.push_back(par_res);
        par_timer.next("parallel");
    }
    cout << "Parallel bfs " << par_timer.total_time() / 5 << endl;

    parlay::internal::timer seq_timer("seq bfs");
    for (int i = 0; i < 5; i++) {
        vector<vector<Node>> seq_res = bfs_seq(start);
        sequential_res.push_back(seq_res);
        seq_timer.next("Sequential");
    }
    cout << "Sequential bfs " << seq_timer.total_time() / 5 << endl;
    
    for (int i = 0; i < parallel_res.size(); i++) {
        if (!is_correct_par(parallel_res[i])) {
            cout << "par bfs is not correct " << i << endl;
        }
    }
    
    for (int i = 0; i < sequential_res.size(); i++) {
        if (!is_correct_seq(sequential_res[i])) {
            cout << "seq bfs is not correct " << i << endl;
        }
    }

}
