#include<iostream>
#include<vector>
using namespace std;

class Bucket {
    private:
        vector<int> lowerBound;
        vector<int> upperBound;
        int splitDim = 0;
        vector<pair<Bucket*, int> > children = {};

    public:
        Bucket(){}

        Bucket(vector<int> lowerBound, vector<int> upperBound){
            this->lowerBound = lowerBound;
            this->upperBound = upperBound;
            while(splitDim < lowerBound.size() && lowerBound[splitDim] == upperBound[splitDim])splitDim++;
        }

        const vector<int>& getLowerBound() const {
            return lowerBound;
        }

        const vector<int>& getUpperBound() const {
            return upperBound;
        }

        int getDim() const {
            return lowerBound.size();
        }

        int getSplitDim() const {
            return splitDim;
        }

        void replaceSelf(int lower, int upper){
            lowerBound[splitDim] = lower;
            upperBound[splitDim] = upper;
            while(splitDim < lowerBound.size() && lowerBound[splitDim] == upperBound[splitDim])splitDim++;
            return;
        }

        Bucket replace(int lower, int upper){
            Bucket newBucket = Bucket(lowerBound, upperBound);
            newBucket.replaceSelf(lower, upper);
            return newBucket;
        }

        void print(){
            cout << "Bucket: ";
            for(int i = 0; i < lowerBound.size(); i++){
                cout << "[" << lowerBound[i] << ", " << upperBound[i] << "] ";
            }
            cout << endl;
        }

        const vector<pair<Bucket*, int> >& getChildren() const {
            return children;
        }

        void addChild(Bucket* child, int agm){
            children.push_back(make_pair(child, agm));
        }
};

// vector<Bucket> split(Bucket B){
//     int splitDim = B.getSplitDim();
//     int splitPos = 0;
//     vector<Bucket> result = {};
//     if(splitPos - 1 >= B.getLowerBound()[splitDim])result.push_back(B.replace(B.getLowerBound()[splitDim], splitPos - 1));
//     Bucket Bmid = B.replace(splitPos, splitPos);
//     if(splitDim == B.getDim() - 1)result.push_back(Bmid);
//     else{
//         vector<Bucket> temp = split(Bmid);
//         result.insert(result.end(), temp.begin(), temp.end());
//     }
//     if(splitPos + 1 <= B.getUpperBound()[splitDim])result.push_back(B.replace(splitPos + 1, B.getUpperBound()[splitDim]));
//     return result;
// }