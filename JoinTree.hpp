#include <bits/stdc++.h>
#include "AGM.hpp"
#include "CountOracle.hpp"
using namespace std;

class JoinTree {
private:
    vector<vector<int> > cnt;
public:
    int root;
    vector<vector<int> > children; // children[i] = list of children of node i
    vector<int> parent; // parent[i] = parent of node i, -1 if root
    vector<vector<vector<int> > > joinPos; // joinPos[i][j] = the join positions of the edge between i and j

    JoinTree(){}

    JoinTree(Query q) {
        queue<int> que;
        root = 0;
        children = vector<vector<int> >(q.getRelNames().size(), vector<int>());
        joinPos = vector<vector<vector<int > > >(q.getRelNames().size(), vector<vector<int> >());
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
        for(int rel = 0; rel < children.size(); rel++) {
            for(int i = 0; i < children[rel].size(); i++) {
                joinPos[rel].push_back(vector<int>());
                int child = children[rel][i], cnt = 0;
                for(int j = 0; j < q.getRelations()[rel].size(); j++) {
                    if(q.getRelations()[rel][j] == q.getRelations()[child][cnt]){
                        joinPos[rel][i].push_back(j);
                        cnt++;
                    }
                }
            }
        }
    }

    void preprocess(int node, const CountOracle<int> &CO) {
        if(node < 0 || node >= (int)children.size()) return;
        for(int i = 0; i < children[node].size(); i++) preprocess(children[node][i], CO);

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
            cout << "R" << i << " has children: ";
            for(int j = 0; j < children[i].size(); j++) {
                cout << "R" << children[i][j] << "("; // print the children of the current node
                for(int k = 0; k < joinPos[i][j].size() - 1; k++) {
                    cout << joinPos[i][j][k] << ", ";
                }
                cout << joinPos[i][j][joinPos[i][j].size() - 1] << "); ";
            }
            cout << endl; // new line for the next node
        }
    }

    void print(){
        printTree(root);
    }
};