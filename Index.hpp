#include "Table.h"
#include "AGM.hpp"
#include "Parcel.h"
#include "SplitBucket.hpp"
using namespace std;

class Index {
    private:
        Query q;
        vector<Table<Parcel> > tables;
        Bucket FB;
        
    public:
        Index() {};
        Index(Query q) : q(q) {};
        void preProcessing(const unordered_map<string, vector<string> >& relations, const unordered_map<string, string>& filenames, const unordered_map<string, int>& numLines) {
            for(int i = 0; i < q.getRelNames().size(); i++) {
                string relName = q.getRelNames()[i];
                Table<Parcel> tbl;
                vector<int> columns;
                for(int j = 0; j < q.getRelVars()[i].size(); j++) {
                    for(int k = 0; k < relations.at(relName).size(); k++) {
                        if(q.getRelVars()[i][j] == relations.at(relName)[k]) {
                            columns.push_back(k);
                            break;
                        }
                    }
                }
                tbl.loadFromFile(filenames.at(relName), numLines.at(relName), columns);
                tables.push_back(tbl);
            }
        }

        int AGM() {
            return AGMforBucket(getFullBucket());
        }

        Bucket getFullBucket() {
            int varnum = q.getVarNumber();
            vector<int> lowerBound(varnum, 2147483647);
            vector<int> upperBound(varnum, -2147483648);
            for(int i = 0; i < q.getRelations().size(); i++) {
                for(int j = 0; j < q.getRelations()[i].size(); j++) {
                    lowerBound[q.getRelations()[i][j]] = min(lowerBound[q.getRelations()[i][j]], tables[i].getLowerBounds()[j]);
                    upperBound[q.getRelations()[i][j]] = max(upperBound[q.getRelations()[i][j]], tables[i].getUpperBounds()[j]);
                }
            }
            return {lowerBound, upperBound};
        }

        int AGMforBucket(Bucket B) {
            int relnum = q.getRelations().size();
            vector<int> cardinalities;
            for(int i = 0; i < relnum; i++) {
                vector<int> lower_bound = {};
                vector<int> upper_bound = {};
                for(int j = 0; j < q.getRelations()[i].size(); j++) {
                    lower_bound.push_back(B.getLowerBound()[q.getRelations()[i][j]]);
                    upper_bound.push_back(B.getUpperBound()[q.getRelations()[i][j]]);
                }
                cardinalities.push_back(tables[i].count(lower_bound, upper_bound));
            }
            return int(q.AGM(cardinalities));
        }

        vector<pair<Bucket, int> > split(Bucket B){
            int splitDim = B.getSplitDim();
            int AGM = AGMforBucket(B);
            if(AGM == 0)return {};
            // cout <<"SPLIT:";
            // B.print();
            // cout << "SPLITDIM: " << splitDim << endl;
            
            long long l = B.getLowerBound()[splitDim], r = B.getUpperBound()[splitDim], mid;
            // cout <<"OL: " << l << " OR: " << r << endl;
            int splitPos = l;
            while(l <= r){
                mid = (l + r) >> 1;
                Bucket Bleft = B.replace(B.getLowerBound()[splitDim], mid - 1);
                int AGMleft = AGMforBucket(Bleft);
                // cout <<"QQQ " << l <<", " << r << ", " << mid << endl;
                // Bleft.print();
                // cout << AGMleft << " / " << AGM << endl;
                if(AGMleft <= (AGM >> 1))splitPos = mid, l = mid + 1;
                else r = mid - 1;
            }
            
            vector<pair<Bucket, int> > result = {};
            
            Bucket Bleft = B.replace(B.getLowerBound()[splitDim], splitPos - 1);
            int AGMleft = AGMforBucket(Bleft);
            if(splitPos - 1 >= B.getLowerBound()[splitDim] && AGMleft > 0)result.push_back(make_pair(Bleft, AGMleft));
            
            Bucket Bmid = B.replace(splitPos, splitPos);
            // cout <<"MIDBUCKET:";
            // Bmid.print();
            // cout <<"DIM AND DIM "<< splitDim << " " << Bmid.getDim() << endl;
            if(splitDim == B.getDim() - 1){
                int AGMmid = AGMforBucket(Bmid);
                if(AGMmid > 0)result.push_back(make_pair(Bmid, AGMforBucket(Bmid)));
            }
            else{
                vector<pair<Bucket, int> > temp = split(Bmid);
                // for(int kk = 0; kk < temp.size(); kk++){
                //     cout <<"[@@@" << endl;
                //     temp[kk].first.print();
                //     cout <<"@@@]" << endl;
                // }
                result.insert(result.end(), temp.begin(), temp.end());
            }

            Bucket Bright = B.replace(splitPos + 1, B.getUpperBound()[splitDim]);
            int AGMright = AGMforBucket(Bright);
            if(splitPos + 1 <= B.getUpperBound()[splitDim] && AGMright > 0)result.push_back(make_pair(Bright, AGMright));
            return result;
        }

        vector<int> sample(Bucket B, int AGM = -1){
            if(AGM < 0)AGM = AGMforBucket(B);
            if(AGM == 0)return {};
            if(B.getSplitDim() == B.getDim())return B.getLowerBound();
            vector<pair<Bucket, int> > sons = split(B);
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> distr(1, AGM);
            int p = distr(gen);
            for(int i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                if(p <= son.second)return sample(son.first, son.second);
                p -= son.second;
            }
            return {};
        }

        vector<int> sampleUntilSuccess(){
            vector<int> sample = {};
            while(sample.size() == 0)sample = this->sample(getFullBucket());
            return sample;
        }

        // pair<bool, vector<int> > randomAccess(Bucket B, int k, int AGM = -1){
        //     if(AGM < 0)AGM = AGMforBucket(B);
        //     if(B.getSplitDim() == B.getDim())return make_pair(true, B.getLowerBound());
        //     vector<pair<Bucket, int> > sons = split(B);
        //     for(int i = 0; i < sons.size(); i++){
        //         pair<Bucket, int> son = sons[i];
        //         if(k <= son.second)return randomAccess(son.first, k, son.second);
        //         k -= son.second;
        //         AGM -= son.second;
        //     }
        //     return make_pair(false, vector<int> {AGM - k});
        // }

        ///////////////////////////// TBD: BETTER TRIVAL INTERVAL
        int getEmptyRight(Bucket B, int AGM = -1){
            // B.print();
            if(AGM < 0)AGM = AGMforBucket(B);
            if(B.getSplitDim() == B.getDim()){
                return 1 - AGM;
            }
            vector<pair<Bucket, int> > sons = split(B);
            int temp = 0;
            for(int i = 0; i < sons.size(); i++){
                // cout << "SON::" << i <<": ";
                // sons[i].first.print();
                temp += sons[i].second;
            }
            // cout <<"RE NOT HERE0" << endl;
            int emptyright = sons.size() > 0 ? getEmptyRight(sons[sons.size() - 1].first, sons[sons.size() - 1].second) : 0;
            
            // cout <<"RE NOT HERE1" << endl;
            return AGM - temp + emptyright;
        }

        pair<bool, vector<int> > randomAccess_opt(Bucket B, int k, int offset = 0, int AGM = -1){
            if(AGM < 0)AGM = AGMforBucket(B);
            // B.print();
            // cout << "ThisBucketInterval: " << offset + 1 << " " << offset + AGM << endl;
            if(B.getSplitDim() == B.getDim())return make_pair(true, B.getLowerBound());
            vector<pair<Bucket, int> > sons = split(B);
            int temp = 0;
            for(int i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                if(k - offset - temp <= son.second){
                    pair<bool, vector<int> > res = randomAccess_opt(son.first, k, offset + temp, son.second);
                    // cout <<res.first << " " << res.second[0] << " " << res.second[1] << endl;
                    // cout <<"THISEMPTY: "<< offset + temp  +son.second << ", " << offset + AGM << endl;
                    if(res.first || i < sons.size() - 1 || res.second[1] < offset + temp + son.second) return res;
                    // cout << "->(" << res.second[0] << ", " << offset + AGM << ")" << endl;
                    return make_pair(false, vector<int> {res.second[0], offset + AGM});
                }
                // k -= son.second;
                temp += son.second;
            }
            // return make_pair(false, vector<int> {offset + temp + 1, offset + AGM});
            ///////////////////////////// TBD: BETTER TRIVAL INTERVAL
            int emptyright = sons.size() > 0 ? getEmptyRight(sons[sons.size() - 1].first, sons[sons.size() - 1].second) : 0;
            // cout << offset + temp + 1 << ", " << offset + AGM << endl;
            // cout << offset + temp - emptyright + 1 << ", " << offset + AGM << endl;
            return make_pair(false, vector<int> {offset + temp - emptyright + 1, offset + AGM});
        }

        pair<bool, vector<int> > randomAccess(Bucket B, int k, int offset = 0, int AGM = -1){
            if(AGM < 0)AGM = AGMforBucket(B);
            // cout << "BucketInterval: " << offset + 1 << " " << offset + AGM << endl;
            if(B.getSplitDim() == B.getDim())return make_pair(true, B.getLowerBound());
            vector<pair<Bucket, int> > sons = split(B);
            int temp = 0;
            for(int i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                if(k - offset - temp <= son.second)return randomAccess(son.first, k, offset + temp, son.second);
                // k -= son.second;
                temp += son.second;
            }
            return make_pair(false, vector<int> {offset + temp + 1, offset + AGM});
        }

        void printBucketInfo(Bucket B, int offset = 0, int AGM = -1){
            if(AGM < 0)AGM = AGMforBucket(B);
            cout << "------------------------------------" << endl;
            B.print();
            int relnum = q.getRelations().size();
            vector<int> cardinalities;
            for(int i = 0; i < relnum; i++) {
                vector<int> lower_bound = {};
                vector<int> upper_bound = {};
                for(int j = 0; j < q.getRelations()[i].size(); j++) {
                    lower_bound.push_back(B.getLowerBound()[q.getRelations()[i][j]]);
                    upper_bound.push_back(B.getUpperBound()[q.getRelations()[i][j]]);
                }
                cardinalities.push_back(tables[i].count(lower_bound, upper_bound));
            }
            for (int i = 0; i < cardinalities.size(); i++) {
                cout << "Cardinality of relation " << i << ": " << cardinalities[i] << endl;
            }
            cout << "AGM: " << AGM << endl;
            cout << "BucketInterval: [" << offset + 1 << " " << offset + AGM << "]" << endl;
        }

        void printBucketTree(Bucket B, int offset = 0, int AGM = -1){
            if(AGM < 0)AGM = AGMforBucket(B);
            if(B.getSplitDim() == B.getDim())return;
            vector<pair<Bucket, int> > sons = split(B);
            int tempoffset = offset;
            for(int i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                printBucketInfo(son.first, tempoffset, son.second);
                tempoffset += son.second;
            }
            for(int i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                printBucketTree(son.first, offset, son.second);
                offset += son.second;
            }
            return;
        }

        void enumeration(Bucket B, int AGM = -1){
            if(AGM < 0)AGM = AGMforBucket(B);
            if(B.getSplitDim() == B.getDim()){
                cout << "Res(";
                for(int i = 0; i < B.getDim() - 1; i++) {
                    cout << B.getLowerBound()[i] << ",";
                }
                cout << B.getLowerBound()[B.getDim() - 1] << ")"<< endl;
                return;
            }
            vector<pair<Bucket, int> > sons = split(B);
            for(int i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                enumeration(son.first, son.second);
            }
            return;
        }

        void print(){
            for(int i = 0; i < tables.size(); i++){
                cout << "Relation: " << i << endl;
                tables[i].print();
            }
        }
};