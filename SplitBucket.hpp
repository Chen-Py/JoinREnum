#include<iostream>
#include<vector>
using namespace std;

class Bucket {
    private:
        vector<int> lowerBound = {};
        vector<int> upperBound = {};
        int splitDim = 0;
        vector<pair<Bucket*, int> > children = {};

    public:
        Bucket(){}

        /**
         * @brief Constructor for the Bucket class.
         * 
         * Initializes a Bucket object with the specified lower and upper bounds.
         * The constructor also determines the first dimension (splitDim) where
         * the lower and upper bounds differ.
         * 
         * @param lowerBound A vector of integers representing the lower bounds of the bucket.
         * @param upperBound A vector of integers representing the upper bounds of the bucket.
         * 
         * @note If all dimensions of the lower and upper bounds are equal, splitDim
         *       will be set to the size of the lowerBound vector.
         */
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

        /**
         * @brief Retrieves the dimensionality of the bounds.
         * 
         * This function returns the number of dimensions represented
         * by the `lowerBound` vector, which corresponds to the size
         * of the `lowerBound` container.
         * 
         * @return int The number of dimensions (size of `lowerBound`).
         */
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

        // bool operator==(const Bucket& B) const {
        //     if(lowerBound.size() != B.getLowerBound().size() || upperBound.size() != B.getUpperBound().size()){
        //         cout << "Bucket size mismatch @ EQ" << endl;
        //         cout << lowerBound.size() << " != " << B.getLowerBound().size() << "||" << upperBound.size() << " != " << B.getUpperBound().size() << endl;
        //         return false;
        //     }
        //     return lowerBound == B.getLowerBound() && upperBound == B.getUpperBound();
        // }

        bool operator<(const Bucket& B) const {
            if (lowerBound.size() != B.getLowerBound().size() || upperBound.size() != B.getUpperBound().size()) {
                cout << "Bucket size mismatch @ lessEQ" << endl;
                cout << lowerBound.size() << " != " << B.getLowerBound().size() << "||" << upperBound.size() << " != " << B.getUpperBound().size() << endl;
                return false;
            }
            if (lowerBound < B.getLowerBound()) return true;
            if (lowerBound > B.getLowerBound()) return false;
            return upperBound < B.getUpperBound();
        }

        Bucket replace(int lower, int upper) const {
            Bucket newBucket = Bucket(lowerBound, upperBound);
            newBucket.replaceSelf(lower, upper);
            return newBucket;
        }

        void print() const {
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