#include <bits/stdc++.h>
#include "Index.hpp"
#include "ReadConfig.hpp"

using namespace std;
int main() {
    
    unordered_map<string, vector<string> > relations = readRelations("db/relations.txt");
    unordered_map<string, string> filenames = readFilenames("db/filenames.txt");
    unordered_map<string, int> numlines = readNumLines("db/numlines.txt");

    Query q({"R1", "R2", "R3"}, {{"A", "B"}, {"B", "C"}, {"A", "C"}});

    Index idx(q);

    idx.preProcessing(relations, filenames, numlines);

    Bucket B = idx.getFullBucket();
    B.print();
    idx.setAGMandIters(B);
    B.print();
    vector<vector<Point<int> >::iterator> begins;
    for(int i = 0; i < q.getRelations().size(); i++) {
        begins.push_back(idx.tables[i].rt.points.begin());
    }
    vector<Bucket> sons = idx.splitBucket(B);
    for(size_t i = 0; i < sons.size(); i++){
        sons[i].print();
        sons[i].printIters(begins);
    }
    // idx.printBucketTree(idx.getFullBucket());

    return 0;
}