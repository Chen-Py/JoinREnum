#include "JoinTree.hpp"
using namespace std;
int main() {
    
    // Query q({"R1", "R2", "R3"}, {{"A", "B"}, {"B", "C"}, {"A", "C"}});
    Query q({"R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9"}, {{"x1", "x2"}, {"x2", "x3"}, {"x1", "x3"}, {"x3", "x4"}, {"x4", "x5"}, {"x5", "x6"}, {"x4", "x6"}, {"x1", "x5"}, {"x2", "x6"}});

    // Query q({"R1", "R2", "R3", "R4"}, {{"A", "B", "C", "D"}, {"B", "D", "E", "G"}, {"B", "C", "E", "F"}, {"C", "D", "F", "G"}});
    q.print();
    // for(int i = 0; i < q.getRelNames().size(); i++) {
    //     vector<int> neighbors = q.getNeighborRels(i);
    //     for(int j = 0; j < neighbors.size(); j++) {
    //         cout << "Relation " << i << " has neighbor: " << neighbors[j] << endl; // print the neighbors of the current relation
    //     }
    // }
    JoinTree tree(q);
    tree.print();
    return 0;
}