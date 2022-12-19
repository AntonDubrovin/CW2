#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include "Node.h"
#include "Graph.h"
#include <sys/time.h>

using namespace std;

int cube_size;

double wctime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + 1E-6 * tv.tv_usec;
}

vector<vector<vector<int>>> bfs(Node *start) {
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


int main() {
    parlay::sequence < vector<vector<vector<int>>> seq_ress;
    Node *start = new Node(0, 0, 0);
    for (int i = 0; i < 5; i++) {
        double start_time = wctime();
        vector<vector<vector<int>>> seq_res = bfs(start);
        double end_time = wctime();
        cout << "Seq Time:" << end_time - start_time << endl;
        seq_ress.push_back(seq_res);
    }
}