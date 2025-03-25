#include "Index.hpp"
class RRAccessTreeNode {

public:
    int AGM;
    Bucket B;
    vector<pair<Bucket, int> > children_buckets;
    vector<RRAccessTreeNode*> children_pointers;

    /**
     * @brief Constructs an RRAccessTreeNode object with the specified parameters.
     * 
     * @param B The bucket associated with this node.
     * @param AGM The AGM (Aggregate Measure) value for this node.
     * @param children_buckets A vector of pairs, where each pair consists of a child bucket and its associated integer value.
     * @param children_pointers A vector of pointers to the child nodes of this node.
     */
    RRAccessTreeNode(Bucket B, int AGM, vector<pair<Bucket, int> > children_buckets, vector<RRAccessTreeNode*> children_pointers) : B(B), AGM(AGM), children_buckets(children_buckets), children_pointers(children_pointers) {}

    void print() {
        cout << "AGM: " << AGM << ", size: " << children_buckets.size() << ", ";
        B.print();
    }
};

class RRAccessTree {
private:
    RRAccessTreeNode* root = NULL;
    Index idx;

    pair<bool, vector<int> > RRAccess(int k, Bucket B, RRAccessTreeNode* &node, int offset = 0, int AGM = -1) {
        if(B.getSplitDim() == B.getDim()) return make_pair(true, B.getLowerBound());
        if(AGM < 0) AGM = idx.AGMforBucket(B);
        if (!node) {
            vector<pair<Bucket, int> > children = idx.split(B);
            node = new RRAccessTreeNode(B, AGM, children, vector<RRAccessTreeNode*>(children.size(), NULL));
        }
        int sonAGM, temp = 0;
        for(int i = 0; i < node->children_buckets.size(); i++) {
            sonAGM = node->children_buckets[i].second;
            if(offset + temp + sonAGM >= k) return RRAccess(k, node->children_buckets[i].first, node->children_pointers[i], offset + temp, sonAGM);
            else temp += sonAGM;
        }
        return make_pair(false, vector<int>());
        // return make_pair(false, vector<int>({offset + temp - node->children_buckets[node->children_buckets.size() - 1].second + 1, offset + AGM}));
    }

    
    int getEmptyRight(Bucket B, RRAccessTreeNode* node, int AGM = -1) {
        if (AGM < 0) AGM = idx.AGMforBucket(B);
        if (B.getSplitDim() == B.getDim()) return 1 - AGM;
        if (!node) {
            vector<pair<Bucket, int> > children = idx.split(B);
            node = new RRAccessTreeNode(B, AGM, children, vector<RRAccessTreeNode*>(children.size(), NULL));
        }
        int temp = 0;
        for (int i = 0; i < node->children_buckets.size(); i++) {
            temp += node->children_buckets[i].second;
        }
        int emptyright = node->children_buckets.size() > 0 ? getEmptyRight(node->children_buckets[node->children_buckets.size() - 1].first, node->children_pointers[node->children_pointers.size() - 1], node->children_buckets[node->children_buckets.size() - 1].second) : 0;

        return AGM - temp + emptyright;
    }

public:
    int AGM;

    // /**
    //  * @brief Constructs an RRAccessTree object.
    //  *
    //  * This constructor initializes the RRAccessTree by creating an Index object
    //  * using the provided relations, filenames, and numlines. It also retrieves
    //  * the AGM (AGM bound) from the Index object.
    //  *
    //  * @param relations A map where the key is a string representing a relation name,
    //  *                  and the value is a vector of strings representing its related variables.
    //  * @param filenames A map where the key is a string representing a relation name,
    //  *                  and the value is a string representing the corresponding filename.
    //  * @param numlines  A map where the key is a string representing a relation name,
    //  *                  and the value is an integer representing the number of lines in the file.
    //  */
    // RRAccessTree(
    //     const unordered_map<string, vector<string> > &relations,
    //     const unordered_map<string, string> &filenames,
    //     const unordered_map<string, int> &numlines) {
    //     idx = Index(relations, filenames, numlines);
    //     AGM = idx.AGM();
    // }

    /**
     * @brief Constructs an RRAccessTree object and initializes its internal state.
     *
     * @param q The query object used to initialize the index.
     * @param relations A map where the key is a string representing a relation name,
     *                  and the value is a vector of strings representing its variable names.
     * @param filenames A map where the key is a string representing a relation name,
     *                  and the value is a string representing the corresponding file name.
     * @param numlines A map where the key is a string representing a relation name,
     *                 and the value is an integer representing the number of lines in the file.
     *
     * This constructor initializes the `idx` member by creating an Index object with the given query.
     * It then performs preprocessing on the index using the provided relations, filenames, and numlines.
     * Finally, it retrieves and stores the AGM bound from the index.
     */
    RRAccessTree(
        const Query &q,
        const unordered_map<string, vector<string> > &relations,
        const unordered_map<string, string> &filenames,
        const unordered_map<string, int> &numlines) {
        idx = Index(q);
        idx.preProcessing(relations, filenames, numlines);
        AGM = idx.AGM();
    }


    /**
     * @brief Performs a relaxed-random-access operation on the tree.
     * 
     * This function retrieves data from the tree based on the specified key `k`.
     * It internally calls an overloaded version of `RRAccess` with additional parameters
     * such as the full bucket index, the root node, the starting depth, and the AGM value.
     * 
     * @param k The key used to perform the relaxed-random-access.
     * @return A pair containing:
     *         - A boolean indicating the success or failure of the operation.
     *         - A vector of integers representing the retrieved data.
     *           - if the operation is successful, the vector is the retrieved join result.
     *           - if the operation fails, the vector is an trivial interval.
     */
    pair<bool, vector<int> > RRAccess(int k) {
        return RRAccess(k, idx.getFullBucket(), root, 0, AGM);
    }

    void print(RRAccessTreeNode* node, int depth = 0) {
        for (int i = 0; i < depth; i++) cout << "| ";
        if (!node){
            cout << "NULL" << endl;
            return;
        }
        node->print();
        for (int i = 0; i < node->children_pointers.size(); i++) {
            print(node->children_pointers[i], depth + 1);
        }
        return;
    }
    
    void print() {
        print(root);
    }
};