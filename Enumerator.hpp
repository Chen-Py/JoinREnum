#include "BanPickTree.hpp"
#include "RRAccessTree.hpp"
#include <ctime>
using namespace std;

class Enumerator {

private:
public:
    int option = 3; // 0: REnum, 1: REnum_L, 2: REnum_M, 3: REnum_B
    bool treeflag = false; // true: use TU-S
    RRAccessTree access_tree;
    BanPickTree bp;

    Enumerator(
        unordered_map<string, vector<string> > relations,
        unordered_map<string, string> filenames,
        unordered_map<string, int> numlines) :
        access_tree(relations, filenames, numlines),
        // bp(min(access_tree.AGM, access_tree.idx.jt.treeUpp(access_tree.idx.FB))) {}   
        bp(access_tree.AGM) {access_tree.idx.treeflag = treeflag;}
    void random_enumerate() {
        double totalRRAccessTime = 0;
        int cntsuccess = 0, cnt = 0, step = 20;
        clock_t start = clock();
        clock_t end;
        double elapsed = 0;
        double last_percentage = 0;
        long long s;
        bool res;
        while(bp.remaining()){
            cnt++;
            s = bp.pick();
            // auto startRRAccess = std::chrono::high_resolution_clock::now();
            switch(option) {
                case 0: res = access_tree.RRAccess(s); break;
                case 1: res = access_tree.RRAccess_LTI(s); break;
                case 2: res = access_tree.RRAccess_MTI(s); break;
                case 3: res = access_tree.RRAccess_BTI(s); break;
                default: res = access_tree.RRAccess_BTI(s); break;
            }
            // res = access_tree.RRAccess_BTI(s);
            // auto endRRAccess = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double> elapsedRRAccess = endRRAccess - startRRAccess;
            // totalRRAccessTime += elapsedRRAccess.count();
            if(res){
                // cout << "(";
                // for(int i = 0; i < res.second.size(); i++) {
                //     cout << res.second[i] << ",";
                // }
                // cout << ")" << endl;
                
                cntsuccess++;
                // if(cntsuccess == 77610){
                if(cntsuccess <= 20 || cntsuccess % 10000 == 0){
                end = clock();
                elapsed = double(end - start) / CLOCKS_PER_SEC;
                cout << cntsuccess << ", " << cnt << ", " << bp.remaining() << ", " << bp.getPercentage() << ", " << elapsed << endl;
                }
            }
            // if(cnt % 100 == 0){
            //     end = clock();
            //     elapsed = double(end - start) / CLOCKS_PER_SEC;
            //     cout << cntsuccess << ", " << cnt << ", " << bp.remaining() << ", " << bp.getPercentage() << ", " << elapsed << endl;
            //     }
            if(res) bp.ban(s,s);
            for(int i = 0; i < access_tree.numti; i++) {
                bp.ban(access_tree.trivialIntervals[i].first, access_tree.trivialIntervals[i].second);
            }
            // else bp.ban(access_tree.trivialInterval.first, access_tree.trivialInterval.second);
        }
        end = clock();
        elapsed = double(end - start) / CLOCKS_PER_SEC;
        cout << cntsuccess << ", " << cnt << ", " << bp.remaining() << ", " << bp.getPercentage() << ", " << elapsed << endl;
        cout << "Total RRAccess Time: " << totalRRAccessTime << endl;
    }

};