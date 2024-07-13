#include "Table.h"
#include "AGM.hpp"
#include "Parcel.h"
#include "SplitBucket.hpp"
using namespace std;

class Index {
    private:
        Query q;
        vector<Table<Parcel> > tables;
        
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
            return ceil(q.AGM(cardinalities));
        }

        vector<pair<Bucket, int> > split(Bucket B){
            int splitDim = B.getSplitDim();
            int AGM = AGMforBucket(B);
            if(AGM == 0)return {};
            
            long long l = B.getLowerBound()[splitDim] + 1, r = B.getUpperBound()[splitDim], mid;
            while(l < r){
                mid = (l + r) >> 1;
                Bucket Bleft = B.replace(B.getLowerBound()[splitDim], mid - 1);
                int AGMleft = AGMforBucket(Bleft);
                if(AGMleft <= (AGM >> 1))l = mid + 1;
                else r = mid;
            }
            int splitPos = l - 1;
            
            vector<pair<Bucket, int> > result = {};
            
            Bucket Bleft = B.replace(B.getLowerBound()[splitDim], splitPos - 1);
            int AGMleft = AGMforBucket(Bleft);
            if(splitPos - 1 >= B.getLowerBound()[splitDim] && AGMleft > 0)result.push_back(make_pair(Bleft, AGMleft));
            
            Bucket Bmid = B.replace(splitPos, splitPos);
            if(splitDim == B.getDim() - 1)result.push_back(make_pair(Bmid, AGMforBucket(Bmid)));
            else{
                vector<pair<Bucket, int> > temp = split(Bmid);
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

        pair<bool, vector<int> > randomAccess(Bucket B, int k, int AGM = -1){
            if(AGM < 0)AGM = AGMforBucket(B);
            if(B.getSplitDim() == B.getDim())return make_pair(true, B.getLowerBound());
            vector<pair<Bucket, int> > sons = split(B);
            for(int i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                if(k <= son.second)return randomAccess(son.first, k, son.second);
                k -= son.second;
                AGM -= son.second;
            }
            return make_pair(false, vector<int> {AGM - k});
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