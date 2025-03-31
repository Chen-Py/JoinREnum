#include <bits/stdc++.h>
#include "AGM.hpp"
using namespace std;

class JoinTree {
private:
public:
    int root;
    vector<vector<int> > children; // children[i] = list of children of node i
    vector<int> parent; // parent[i] = parent of node i, -1 if root

    JoinTree(){}

    JoinTree(Query q) {
        queue<int> que;
        root = 0;
        children = vector<vector<int> >(q.getRelNames().size(), vector<int>());
        parent = vector<int>(q.getRelNames().size(), -1);
        vector<bool> visited(q.getRelNames().size(), false);
        visited[0] = true;
        que.push(0);
        while(!que.empty()){
            int rel = que.front(); // get the front of the queue
            que.pop();
            vector<int> neighbors = q.getNeighborRels(rel); // get the neighbors of the current relation
            for(int i = 0; i < neighbors.size(); i++) {
                int neighbor = neighbors[i];
                if(visited[neighbor]) continue; // if the neighbor has already been visited, skip it
                bool cyclic = false;
                vector<int> neineighbors = q.getNeighborRels(neighbor);
                for(int j = 0; j < neineighbors.size(); j++) {
                    int neineighbor = neineighbors[j];
                    if(neineighbor == rel) continue;
                    if(visited[neineighbor])cyclic = true;
                }
                if(!cyclic) {
                    children[rel].push_back(neighbor);
                    parent[neighbor] = rel;
                    visited[neighbor] = true;
                    que.push(neighbor); // add the neighbor to the queue for further exploration
                }
            }
        }
    }

    void printTree(int nodeID, int depth = 0) {
        // Print the current node with indentation based on depth
        for (int i = 0; i < depth; i++) {
            cout << "| "; // Indent for each level of depth
        }
        cout << "R" << nodeID << endl;

        // Recursively print children
        for (int childID : children[nodeID]) {
            printTree(childID, depth + 1);
        }

    }

    void printChildren() {
        for(int i = 0; i < children.size(); i++) {
            cout << "Node " << i << " has children: ";
            for(int j = 0; j < children[i].size(); j++) {
                cout << children[i][j] << " "; // print the children of the current node
            }
            cout << endl; // new line for the next node
        }
    }

    void print(){
        printTree(root);
    }
};