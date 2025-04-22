#include "BanPickTree.hpp"
#include "RRAccessTree.hpp"
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
        
        int cntsuccess = 0, cnt = 0, step = 20;
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double last_percentage = 0;
        while(bp.remaining()){
            cnt++;
            int s = bp.pick();
            pair<bool, vector<int> > res = access_tree.RRAccess(s);
            if(res.first){
                // cout << "(";
                // for(int i = 0; i < res.second.size(); i++) {
                //     cout << res.second[i] << ",";
                // }
                // cout << ")" << endl;
                
                cntsuccess++;
                if(cntsuccess < step || cntsuccess % step == 0){
                end = std::chrono::high_resolution_clock::now();
                elapsed = end - start;
                cout << cntsuccess << ", " << cnt << ", " << bp.remaining() << ", " << bp.getPercentage() << ", " << elapsed.count() << endl;
                }
            }
            if(res.first) bp.ban(s,s);
            else bp.ban(res.second[0], res.second[1]);
        }
        end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        cout << cntsuccess << ", " << cnt << ", " << bp.remaining() << ", " << bp.getPercentage() << ", " << elapsed.count() << endl;

    }

};