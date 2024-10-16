#include <iostream>
#include <random>
#include <vector>
#include "Table.h"
#include "Parcel.h"
#include "Index.hpp"
#include "ReadConfig.hpp"
#include "BanPickTree.hpp"
using namespace std;

int main() {
    // Table<Parcel> tbl;
    unordered_map<string, string> filenames = readFilenames("db/filenames.txt");
    unordered_map<string, int> numlines = readNumLines("db/numlines.txt");
    unordered_map<string, vector<string> > relations = readRelations("db/relations.txt");
    
    Query q({"R", "S", "T"}, {{"x", "y"}, {"y", "z"}, {"z", "x"}});
    Index idx(q);
    idx.preProcessing(relations, filenames, numlines);
    cout << "Variables: ";
    for(int i = 0; i < q.getVarNames().size(); i++) {
        cout << q.getVarNames()[i] << " ";
    }
    cout << endl;
    // idx.getFullBucket().print();
    // for(int i = 1; i < 20; i++) {
    //     vector<int> res = idx.sampleUntilSuccess();
    //     cout << "Sample " << i << ": ";
    //     for(int j = 0; j < res.size(); j++) {
    //         cout << res[j] << " ";
    //     }
    //     cout << endl;
    // }


    int cntsuccess = 0, cntfail = 0;
    // for(int i = 1; i <= idx.AGM(); i++) {
    //         pair<bool, vector<int> > res = idx.randomAccess(idx.getFullBucket(), i);
    //         cout << i << ": ";
    //         cout << res.first << "::";
    //         for(int j = 0; j < res.second.size(); j++) {
    //             cout << res.second[j] << ",";
    //         }
    //         cout << endl;
    //         if(!res.first) {
    //             cntfail++;
    //             i = res.second[1];
    //         }
    //         else cntsuccess++;
    // }

    BanPickTree bp(idx.AGM());
    while(bp.remaining()){
        int s = bp.pick();
        pair<bool, vector<int> > res = idx.randomAccess(idx.getFullBucket(), s);
        if(res.first){
            cout << s << ": ";
            cout << res.first << "::";
            for(int j = 0; j < res.second.size(); j++) {
                cout << res.second[j] << ",";
            }
            cout << endl;
        }
        else cout << "Fail" << endl;
        if(res.first) bp.ban(s,s), cntsuccess++;
        else bp.ban(res.second[0], res.second[1]), cntfail++;
    }
    cout << "Success: " << cntsuccess << " Fail: " << cntfail << endl;
    
    return 0;
}
