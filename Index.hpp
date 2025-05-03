#include "Table.h"
#include "AGM.hpp"
#include "Parcel.h"
#include "Bucket.hpp"
#include "JoinTree.hpp"
#include "MHBS.hpp"
using namespace std;

class Index {
    private:
        map<Bucket, vector<pair<Bucket, int> > > bucketSplitCache = map<Bucket, vector<pair<Bucket, int> > >();
        
    public:
        Query q;
        JoinTree jt;
        Bucket FB;
        vector<vector<int> > R;
        vector<Table<Parcel> > tables;
        vector<vector<vector<int> > > data;
        vector<vector<int> > varPos; // varPos[i][j] = the position of the j-th variable in the i-th relation, -1 if not found
        vector<vector<bool> > mask;
        vector<int> attVal;
        // vector<vector<Point<int> >::iterator> beginIters;
        int cntCacheHit = 0;
        int cntTotalCall = 0;
        int cntAGMCall = 0;
        int cntSplitCall = 0;
        int cntBSCall = 0;
        double totalAGMTime = 0;
        double totalCountOracleTime = 0;
        double totalSplitTime = 0;
        double totalCacheHitTime = 0;
        double totalBoundPrepareTime = 0;

        Index() {};

        Index(Query q) : q(q) {};

        // Index(
        //     const unordered_map<string, vector<string> >& relations,
        //     const unordered_map<string, string>& filenames,
        //     const unordered_map<string, int>& numLines) {
        //     // parse vector<string> relationNames, vector<vector<string> > relations from relations
        //     vector<string> relationNames;
        //     vector<vector<string> > relationVars;
        //     for(unordered_map<string, vector<string> >::const_iterator it = relations.begin(); it != relations.end(); it++) {
        //         relationNames.push_back(it->first);
        //         relationVars.push_back(it->second);
        //     }
        //     q = Query(relationNames, relationVars);
        //     preProcessing(relations, filenames, numLines);
        // }

        void preProcessing(const unordered_map<string, vector<string> >& relations, const unordered_map<string, string>& filenames, const unordered_map<string, int>& numLines) {
            for(size_t i = 0; i < q.getRelNames().size(); i++) {
                string relName = q.getRelNames()[i];
                Table<Parcel> tbl;
                vector<int> columns;
                for(size_t j = 0; j < q.getRelVars()[i].size(); j++) {
                    for(size_t k = 0; k < relations.at(relName).size(); k++) {
                        if(q.getRelVars()[i][j] == relations.at(relName)[k]) {
                            columns.push_back(k);
                            break;
                        }
                    }
                }
                tbl.loadFromFile(filenames.at(relName), numLines.at(relName), columns);
                tables.push_back(tbl);
            }
            jt = JoinTree(q, getCountOracles());
            R = q.getRelations();
            int varnum = q.getVarNumber();
            vector<int> lowerBound(varnum, 2147483647);
            vector<int> upperBound(varnum, -2147483648);
            for(size_t i = 0; i < R.size(); i++) {
                for(size_t j = 0; j < R[i].size(); j++) {
                    lowerBound[R[i][j]] = min(lowerBound[R[i][j]], tables[i].getLowerBounds()[j]);
                    upperBound[R[i][j]] = max(upperBound[R[i][j]], tables[i].getUpperBounds()[j]);
                }
            }
            FB = {lowerBound, upperBound};
            // for(size_t i = 0; i < tables.size(); i++) {
            //     beginIters.push_back(tables[i].rt.points.begin());
            // }
            set<int> attValSet;
            for(size_t i = 0; i < tables[0].rt.points.size(); i++) {
                attValSet.insert(tables[0].rt.points[i][0]);
            }
            for(auto it = attValSet.begin(); it != attValSet.end(); it++) {
                attVal.push_back(*it);
            }
            // store the points in column
            data.resize(tables.size());
            varPos.resize(tables.size(), vector<int>(q.getVarNumber(), -1));
            mask.resize(q.getVarNumber(), vector<bool>(tables.size(), false));
            for(size_t i = 0; i < data.size(); i++) {
                data[i].resize(q.getRelations()[i].size());
                for(size_t j = 0; j < data[i].size(); j++) {
                    varPos[i][q.getRelations()[i][j]] = j;
                    mask[q.getRelations()[i][j]][i] = true;
                    data[i][j].resize(tables[i].rt.points.size());
                    for(size_t k = 0; k < data[i][j].size(); k++) {
                        data[i][j][k] = tables[i].rt.points[k][j];
                    }
                }
            }
            cout << "VarPos: " << endl;
            for(size_t i = 0; i < varPos.size(); i++) {
                cout << "Relation " << i << ": ";
                for(size_t j = 0; j < varPos[i].size(); j++) {
                    cout << varPos[i][j] << ", ";
                }
                cout << endl;
            }
            cout << "Mask: " << endl;
            for(size_t i = 0; i < mask.size(); i++) {
                cout << "Variable " << i << ": ";
                for(size_t j = 0; j < mask[i].size(); j++) {
                    cout << mask[i][j] << ", ";
                }
                cout << endl;
            }
            cout << "ATTVAL: " << attVal.size() << endl;
            q.print();
            cout << "TreeUpperBound: " << jt.treeUpp(FB) << endl;
            jt.print();
            cout << "CountRels: " << endl;
            for(size_t i = 0; i < jt.countRels.size(); i++) {
                cout << "SplitDim=" << i  << ": ";
                for(size_t j = 0; j < jt.countRels[i].size(); j++) {
                    cout << "R[" << jt.countRels[i][j] << "], ";
                }
                cout << endl;

            }
            setAGMandIters(FB);
        }

        vector<CountOracle<int>* > getCountOracles() {
            vector<CountOracle<int>* > CO;
            for(size_t i = 0; i < tables.size(); i++) {
                CO.push_back(&tables[i].rt);
            }
            return CO;
        }

        int AGM() {
            return AGMforBucket(getFullBucket());
        }

        Bucket getFullBucket() {
            return FB;
        }

        void setAGM(Bucket &B) {
            // cout << "SET AGM of: ";
            // B.print();
            // B.printIters(beginIters);
            vector<int> cardinalities(B.iters.size());
            for(size_t i = 0; i < cardinalities.size(); i++) {
                cardinalities[i] = B.iters[i].second - B.iters[i].first;
            }
            double ans = q.AGM(cardinalities);
            B.AGM = ceil(ans) - ans < 1e-5 ? ceil(ans) : int(ans);
            // cout << "BEFORE TREEUPP" << endl;
            // B.AGM = min(B.AGM, jt.treeUpp(B.splitDim, B.iters));
            return;
        }

        vector<pair<vector<int>::iterator, vector<int>::iterator> > transformIters(const vector<pair<vector<Point<int> >::iterator, vector<Point<int> >::iterator> > &iters, const int splitDim) {
            vector<pair<vector<int>::iterator, vector<int>::iterator> > result(tables.size());
            // cout << "Transform iters: " << endl;
            for(size_t i = 0; i < iters.size(); i++) {
                // cout << iters[i].first - tables[i].rt.points.begin() << ", " << iters[i].second - tables[i].rt.points.begin() << endl;
                result[i].first = int(iters[i].first - tables[i].rt.points.begin()) + data[i][max(0, varPos[i][splitDim])].begin();
                result[i].second = int(iters[i].second - tables[i].rt.points.begin()) + data[i][max(0, varPos[i][splitDim])].begin();
            }
            // for(int i = 0; i < result.size(); i++) {
            //     cout << "Relation " << i << ": ";
            //     for(vector<int>::iterator it = result[i].first; it != result[i].second; it++)
            //         cout << *it << ", ";
            //     cout << endl;
            // }
            // cout << "Mask: ";
            // for(int i = 0; i < mask.size(); i++) {
            //     cout << mask[i][splitDim] << ", ";
            // }
            // cout << endl;
            return result;
        }

        
        void setAGMandIters(Bucket &B, const vector<pair<vector<Point<int> >::iterator, vector<Point<int> >::iterator> >& iters = {}) {
            int relnum = R.size();
            B.iters = vector<pair<vector<Point<int> >::iterator, vector<Point<int> >::iterator> >(relnum);
            vector<int> cardinalities(relnum, 0);
            vector<int> lower_bound = {};
            vector<int> upper_bound = {};
            for(size_t i = 0; i < relnum; i++) {
                
                lower_bound = vector<int>(R[i].size(), 0);
                upper_bound = vector<int>(R[i].size(), 0);
                
                for(size_t j = 0; j < R[i].size(); j++) {
                    lower_bound[j] = B.lowerBound[R[i][j]];
                    upper_bound[j] = B.upperBound[R[i][j]];
                }

                if(iters.size() > 0) B.iters[i] = tables[i].rt.getRange(lower_bound, upper_bound, iters[i].first, iters[i].second);
                else B.iters[i] = tables[i].rt.getRange(lower_bound, upper_bound);
                cardinalities[i] = B.iters[i].second - B.iters[i].first;
            }
            double ans = q.AGM(cardinalities);
            B.AGM = ceil(ans) - ans < 1e-5 ? ceil(ans) : int(ans);
            // B.AGM = min(B.AGM, jt.treeUpp(B.splitDim, B.iters));
            return;
        }

        int AGMforBucket(Bucket B) {
            
            // auto startAGM = chrono::high_resolution_clock::now();
            // cntAGMCall++;
            int relnum = R.size();
            vector<int> cardinalities(relnum, 0);
            // vector<pair<vector<int>, vector<int> > > bounds;
            vector<int> lower_bound = {};
            vector<int> upper_bound = {};
            for(size_t i = 0; i < relnum; i++) {
                // auto startCountOracle = chrono::high_resolution_clock::now();
                lower_bound = vector<int>(R[i].size(), 0);
                upper_bound = vector<int>(R[i].size(), 0);
                for(size_t j = 0; j < R[i].size(); j++) {
                    lower_bound[j] = B.lowerBound[R[i][j]];
                    upper_bound[j] = B.upperBound[R[i][j]];
                }
                // auto endCountOracle = chrono::high_resolution_clock::now();
                // chrono::duration<double> elapsedCountOracle = endCountOracle - startCountOracle;
                // totalBoundPrepareTime += elapsedCountOracle.count();
                // bounds.push_back({lower_bound, upper_bound});
                // startCountOracle = chrono::high_resolution_clock::now();
                cardinalities[i] = tables[i].count(lower_bound, upper_bound);
                // endCountOracle = chrono::high_resolution_clock::now();
                // elapsedCountOracle = endCountOracle - startCountOracle;
                // totalCountOracleTime += elapsedCountOracle.count();
            }
            
            double ans = q.AGM(cardinalities);
            // auto endAGM = chrono::high_resolution_clock::now();
            // chrono::duration<double> elapsedAGM = endAGM - startAGM;
            // totalAGMTime += elapsedAGM.count();
            // ans = min(ans, (double)jt.treeUpp(B.getSplitDim(), bounds));
            return ceil(ans)-ans < 1e-5 ? ceil(ans) : int(ans);
        }

        vector<pair<Bucket, int> > split(Bucket B, int AGM = -1){
            cntSplitCall++;
            if(AGM < 0)AGM = AGMforBucket(B);
            int splitDim = B.getSplitDim();
            // int AGM = AGMforBucket(B);
            if(AGM == 0)return {};
            
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
            
            // int AGMmid = AGMforBucket(Bmid);
            // if(splitDim == B.getDim() - 1 || AGMmid <= AGM >> 1){
            //     if(AGMmid > 0)result.push_back(make_pair(Bmid, AGMforBucket(Bmid)));
            // }
            // else{
            //     vector<pair<Bucket, int> > temp = split(Bmid);
            //     result.insert(result.end(), temp.begin(), temp.end());
            // }

            // 
            if(splitDim == B.getDim() - 1){
                int AGMmid = AGMforBucket(Bmid);
                if(AGMmid > 0)result.push_back(make_pair(Bmid, AGMforBucket(Bmid)));
            }
            else{
                vector<pair<Bucket, int> > temp = split(Bmid);
                result.insert(result.end(), temp.begin(), temp.end());
            }

            Bucket Bright = B.replace(splitPos + 1, B.getUpperBound()[splitDim]);
            int AGMright = AGMforBucket(Bright);
            if(splitPos + 1 <= B.getUpperBound()[splitDim] && AGMright > 0)result.push_back(make_pair(Bright, AGMright));
            return result;
        }

        vector<Bucket> splitBucket(Bucket &B){
            
            // cout << "SPLITTING: ";
            // B.print();
            cntSplitCall++;
            // auto startSplit = chrono::high_resolution_clock::now();
            if(B.AGM < 0) setAGMandIters(B);
            if(B.AGM == 0)return {};
            int splitDim = B.getSplitDim();
            
            long long l = B.lowerBound[splitDim], r = B.upperBound[splitDim], mid;
            int splitPos = l, AGMleft, x;
            double ans;
            vector<int> cardinalities(B.iters.size(), 0);
            for(size_t i = 0; i < cardinalities.size(); i++) {
                cardinalities[i] = B.iters[i].second - B.iters[i].first;
            }
            // vector<bool> flag(cardinalities.size(), false);
            vector<int> rels = q.getRels(splitDim);
            vector<int> splitVarinRels(rels.size());
            vector<vector<int> > BleftUpperBounds(rels.size()), BmidUpperBounds(rels.size());
            for(size_t i = 0; i < rels.size(); i++) {
                // flag[rels[i]] = true;
                BleftUpperBounds[i] = vector<int>(R[rels[i]].size());
                BmidUpperBounds[i] = vector<int>(R[rels[i]].size());
                for(size_t j = 0; j < R[rels[i]].size(); j++) {
                    BleftUpperBounds[i][j] = B.upperBound[R[rels[i]][j]];
                    BmidUpperBounds[i][j] = B.upperBound[R[rels[i]][j]];
                    if(R[rels[i]][j] == splitDim) splitVarinRels[i] = j;
                }
            }
            vector<pair<vector<int>::iterator, vector<int>::iterator> > vecIters = transformIters(B.iters, splitDim);
            int pos = MultiHeadBinarySearch(vecIters, mask[splitDim], B.AGM >> 1, q);
            // cout << "POS: " << pos << endl;

            // vector<pair<vector<Point<int> >::iterator, vector<Point<int> >::iterator> > BleftIters = B.iters;
            while(l <= r){
                cntBSCall++;
                mid = (l + r) >> 1;
                for(size_t i = 0; i < rels.size(); i++) {
                    // vector<int> BleftUpb(R[rels[i]].size());
                    // for(size_t j = 0; j < R[rels[i]].size(); j++) {
                    //     if(R[rels[i]][j] == splitDim) BleftUpb[j] = mid - 1;
                    //     else BleftUpb[j] = B.getUpperBound()[R[rels[i]][j]];
                    // }
                    x = rels[i];
                    BleftUpperBounds[i][splitVarinRels[i]] = mid - 1;
                    // BleftIters[x].first = tables[x].rt.getUpperBoundIter(BleftUpperBounds[i], B.iters[x].first, B.iters[x].second);
                    cardinalities[x] = tables[x].rt.getUpperBoundIter(BleftUpperBounds[i], B.iters[x].first, B.iters[x].second) - B.iters[x].first;
                }
                ans = q.AGM(cardinalities);
                AGMleft = ceil(ans)-ans < 1e-5 ? ceil(ans) : int(ans);
                // AGMleft = min(AGMleft, jt.treeUpp(splitDim, BleftIters));
                if(AGMleft <= (B.AGM >> 1))splitPos = mid, l = mid + 1;
                else r = mid - 1;
            }
            // cout << "BINARY SEARCH DONE: " << splitPos << endl;
            if(pos != splitPos) cout << "ERROR: POS != SPLITPOS" << endl;
            vector<Bucket> result = {};
            
            Bucket Bleft = B, Bmid = B, Bright = B;
            Bleft.upperBound[splitDim] = splitPos - 1;
            Bmid.lowerBound[splitDim] = splitPos;
            Bmid.upperBound[splitDim] = splitPos;
            Bright.lowerBound[splitDim] = splitPos + 1;
            Bleft.updateSplitDim();
            Bmid.updateSplitDim();
            Bright.updateSplitDim();
            // cout << "UPDATE SPLITDIM DONE" << endl;
            vector<Point<int> >::iterator leftIter, rightIter;
            for(size_t i = 0; i < rels.size(); i++) {
                x = rels[i];
                BleftUpperBounds[i][splitVarinRels[i]] = splitPos - 1;
                BmidUpperBounds[i][splitVarinRels[i]] = splitPos;

                leftIter = tables[x].rt.getUpperBoundIter(BleftUpperBounds[i], B.iters[x].first, B.iters[x].second);
                rightIter = tables[x].rt.getUpperBoundIter(BmidUpperBounds[i], B.iters[x].first, B.iters[x].second);

                Bleft.iters[x] = make_pair(B.iters[x].first, leftIter);
                Bmid.iters[x] = make_pair(leftIter, rightIter);
                Bright.iters[x] = make_pair(rightIter, B.iters[x].second);
            }

            // cout << "SET ITERS DONE" << endl;
            // Bleft.print();
            // Bmid.print();
            // Bright.print();
            setAGM(Bleft);
            setAGM(Bmid);
            setAGM(Bright);

            
            // cout << "SET AGM DONE" << endl;

            
            if(Bmid.AGM > 0 && splitDim < B.getDim() - 1) {
                // vector<Bucket> temp = splitBucket(Bmid);
                // result.insert(result.end(), temp.begin(), temp.end());
                result = splitBucket(Bmid);
            }
            else if(Bmid.AGM > 0)result.push_back(Bmid);
            // if(splitDim == B.getDim() - 1) {
            //     if(Bmid.AGM > 0)result.push_back(Bmid);
            // }

            if(splitPos - 1 >= B.getLowerBound()[splitDim] && Bleft.AGM > 0)result.push_back(Bleft);

            if(splitPos + 1 <= B.getUpperBound()[splitDim] && Bright.AGM > 0)result.push_back(Bright);
            // auto endSplit = chrono::high_resolution_clock::now();
            // chrono::duration<double> elapsedSplit = endSplit - startSplit;
            // totalSplitTime += elapsedSplit.count();
            // cout << "DONE: ";
            // B.print();
            return result;
        }

        vector<Bucket> Split(Bucket &B) {
            vector<Bucket> result = splitBucket(B);
            while(result.size() == 1 && result[0].splitDim != result[0].getDim()){
                result = splitBucket(result[0]);
            }
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
            for(size_t i = 0; i < sons.size(); i++){
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
        //     for(size_t i = 0; i < sons.size(); i++){
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
            cntTotalCall++;
            auto startCacheHit = chrono::high_resolution_clock::now();
            bool flag = bucketSplitCache.find(B) == bucketSplitCache.end();
            auto endCacheHit = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsedCacheHit = endCacheHit - startCacheHit;
            totalCacheHitTime += elapsedCacheHit.count();
            if(flag){
                auto start = chrono::high_resolution_clock::now();
                vector<pair<Bucket, int> > result = split(B);
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = end - start;
                totalSplitTime += elapsed.count();
                startCacheHit = chrono::high_resolution_clock::now();
                bucketSplitCache[B] = result;
                endCacheHit = chrono::high_resolution_clock::now();
                elapsedCacheHit = endCacheHit - startCacheHit;
                totalCacheHitTime += elapsedCacheHit.count();
                // cout << "cache success" << endl;
            }
            else cntCacheHit++;
            startCacheHit = chrono::high_resolution_clock::now();
            vector<pair<Bucket, int> > sons = bucketSplitCache[B];
            endCacheHit = chrono::high_resolution_clock::now();
            elapsedCacheHit = endCacheHit - startCacheHit;
            totalCacheHitTime += elapsedCacheHit.count();
            // vector<pair<Bucket, int> > sons = split(B);
            int temp = 0;
            for(size_t i = 0; i < sons.size(); i++){
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
            cntTotalCall++;
            // B.print();
            // cout << "ThisBucketInterval: " << offset + 1 << " " << offset + AGM << endl;
            if(B.getSplitDim() == B.getDim())return make_pair(true, B.getLowerBound());
            
            auto startCacheHit = chrono::high_resolution_clock::now();
            bool flag = bucketSplitCache.find(B) == bucketSplitCache.end();
            auto endCacheHit = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsedCacheHit = endCacheHit - startCacheHit;
            totalCacheHitTime += elapsedCacheHit.count();
            if(flag){
                auto start = chrono::high_resolution_clock::now();
                vector<pair<Bucket, int> > result = split(B);
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = end - start;
                totalSplitTime += elapsed.count();
                // B.print();
                // if(B.getLowerBound().size() != 3 || B.getUpperBound().size() != 3) cout << "ERROR: " << B.getLowerBound().size() << ", " << B.getUpperBound().size() << endl;
                // cout << "-----------------------v" << endl;
                // for (const auto& son : result) {
                //     son.first.print();
                //     cout << "AGM: " << son.second << endl;
                // }
                
                // cout << "-----------------------^" << endl;
                startCacheHit = chrono::high_resolution_clock::now();
                bucketSplitCache[B] = result;
                endCacheHit = chrono::high_resolution_clock::now();
                elapsedCacheHit = endCacheHit - startCacheHit;
                totalCacheHitTime += elapsedCacheHit.count();
                // cout << "cache success" << endl;
            }
            else cntCacheHit++;
            startCacheHit = chrono::high_resolution_clock::now();
            vector<pair<Bucket, int> > sons = bucketSplitCache[B];
            endCacheHit = chrono::high_resolution_clock::now();
            elapsedCacheHit = endCacheHit - startCacheHit;
            totalCacheHitTime += elapsedCacheHit.count();
            // vector<pair<Bucket, int> > sons = split(B);
            int temp = 0;
            for(size_t i = 0; i < sons.size(); i++){
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
            for(size_t i = 0; i < sons.size(); i++){
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
            for(size_t i = 0; i < relnum; i++) {
                vector<int> lower_bound = {};
                vector<int> upper_bound = {};
                for(size_t j = 0; j < q.getRelations()[i].size(); j++) {
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
            for(size_t i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                printBucketInfo(son.first, tempoffset, son.second);
                tempoffset += son.second;
            }
            for(size_t i = 0; i < sons.size(); i++){
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
                for(size_t i = 0; i < B.getDim() - 1; i++) {
                    cout << B.getLowerBound()[i] << ",";
                }
                cout << B.getLowerBound()[B.getDim() - 1] << ")"<< endl;
                return;
            }
            vector<pair<Bucket, int> > sons = split(B);
            for(size_t i = 0; i < sons.size(); i++){
                pair<Bucket, int> son = sons[i];
                enumeration(son.first, son.second);
            }
            return;
        }

        void print(){
            for(size_t i = 0; i < tables.size(); i++){
                cout << "Relation: " << i << endl;
                tables[i].print();
            }
        }
};