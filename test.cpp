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
    
    // Query q({"R1", "R2", "R3"}, {{"x1", "x2"}, {"x2", "x3"}, {"x1", "x3"}});
    // R1(P,Q,R)
    // R2(Q,S,T)
    // R3(R,T,U)
    // R4(P,S,V)
    // R5(U,V,W)
    // R6(W,P,Q)
    Query q({"R1", "R2", "R3", "R4", "R5", "R6"}, {{"P", "Q", "R"}, {"Q", "S", "T"}, {"R", "T", "U"}, {"P", "S", "V"}, {"U", "V", "W"}, {"W", "P", "Q"}});
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
    // idx.printBucketInfo(idx.getFullBucket());
    // idx.printBucketTree(idx.getFullBucket());


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
    // vector<int> cars = idx.getCar(idx.getFullBucket());
    cout << endl;
    while(bp.remaining()){
        int s = bp.pick();
        pair<bool, vector<int> > res = idx.randomAccess_opt(idx.getFullBucket(), s);
        if(res.first){
            cout << s << ": ";
            cout << res.first << "::";
            for(int j = 0; j < res.second.size(); j++) {
                cout << res.second[j] << ",";
            }
            cout << " Percentage: " << bp.getPercentage() << endl;
            cout << endl;
        }
        // else{
        //     cout << s << ": " << "Fail" << endl;
        //     cout <<"[ " << res.second[0] << ", " << res.second[1] << " ]" << endl;
        // } 
        
        if(res.first) bp.ban(s,s), cntsuccess++;
        else bp.ban(res.second[0], res.second[1]), cntfail++;
    }
    cout << "Success: " << cntsuccess << " Fail: " << cntfail << endl;
    cout << "Total: " << bp.getTotal() << endl;
    return 0;
}
