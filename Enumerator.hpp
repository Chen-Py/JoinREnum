#include "BanPickTree.hpp"
#include "RRAccessTree.hpp"
#include <ctime>
using namespace std;

class Enumerator {

private:
public:
    RRAccessTree access_tree;
    BanPickTree bp;

    Enumerator(
        unordered_map<string, vector<string> > relations,
        unordered_map<string, string> filenames,
        unordered_map<string, int> numlines) :
        access_tree(relations, filenames, numlines),
        // bp(min(access_tree.AGM, access_tree.idx.jt.treeUpp(access_tree.idx.FB))) {}   
        bp(access_tree.AGM) {}
    void random_enumerate() {
        double totalRRAccessTime = 0;
        int cntsuccess = 0, cnt = 0, step = 20;
        clock_t start = clock();
        clock_t end;
        double elapsed = 0;
        double last_percentage = 0;
        while(bp.remaining()){
            cnt++;
            int s = bp.pick();
            // auto startRRAccess = std::chrono::high_resolution_clock::now();
            bool res = access_tree.RRAccess(s);
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
                if(cntsuccess == 77610){
                end = clock();
                elapsed = double(end - start) / CLOCKS_PER_SEC;
                cout << cntsuccess << ", " << cnt << ", " << bp.remaining() << ", " << bp.getPercentage() << ", " << elapsed << endl;
                }
            }
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