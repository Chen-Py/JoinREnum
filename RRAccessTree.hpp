#include "Index.hpp"


/**
 * @class RRAccessTreeNode
 * @brief Represents a node in an Relaxed Random Access Tree structure.
 * 
 * This class encapsulates the properties and behavior of a node in an RRAccess Tree.
 * Each node contains a bucket, a collection of child buckets, and pointers to its child nodes.
 * It also calculates and stores the empty size of the node based on the difference between
 * the AGM of its bucket and the sum of the AGMs of its child buckets.
 * 
 * @details
 * The RRAccessTreeNode class provides a constructor to initialize the node with a bucket,
 * child buckets, and child pointers. It also includes a method to print the node's details.
 * 
 * @note The `emptySize` is computed during construction and represents the remaining size
 * after accounting for the AGMs of the child buckets.
 * 
 * @var emptySize
 * The remaining size of the node after subtracting the sum of the AGMs of its child buckets
 * from the AGM of its own bucket.
 * 
 * @var B
 * The bucket associated with this node.
 * 
 * @var children_buckets
 * A vector containing the buckets of the child nodes.
 * 
 * @var children_pointers
 * A vector of pointers to the child nodes of this node.
 * 
 * @fn RRAccessTreeNode(Bucket B, vector<Bucket> children_buckets, vector<RRAccessTreeNode*> children_pointers)
 * @brief Constructs an RRAccessTreeNode object with the specified parameters.
 * 
 * @param B The bucket associated with this node.
 * @param children_buckets A vector of buckets representing the child nodes.
 * @param children_pointers A vector of pointers to the child nodes of this node.
 * 
 * @fn void print()
 * @brief Prints the details of the node, including its AGM, size, and bucket information.
 */
class RRAccessTreeNode {

public:
    int emptySize;
    Bucket* B;
    vector<Bucket> children_buckets;
    vector<RRAccessTreeNode*> children_pointers;

    /**
     * @brief Constructs an RRAccessTreeNode object with the specified parameters.
     * 
     * @param B The bucket associated with this node.
     * @param children_buckets A vector of buckets.
     * @param children_pointers A vector of pointers to the child nodes of this node.
     */
    RRAccessTreeNode(Bucket* B, vector<Bucket> && children_buckets) : B(B), children_buckets(move(children_buckets)) {
        int sumChildrenAGM = 0;
        for(int i = 0; i < this->children_buckets.size(); i++) {
            sumChildrenAGM += this->children_buckets[i].AGM;
        }
        children_pointers = vector<RRAccessTreeNode*>(this->children_buckets.size(), NULL);
        emptySize = B->AGM - sumChildrenAGM;
    }

    void print() {
        cout << "AGM: " << B->AGM << ", size: " << children_buckets.size() << ", ";
        B->print();
    }
};

class RRAccessTree {
private:


    
    /**
     * @brief Recursively calculates the empty size of the rightmost subtree in an RRAccessTree.
     *
     * This function determines the "empty right" size of a given bucket in the RRAccessTree.
     * It initializes the AGM (Aggregate Measure) and iterators for the bucket if not already set,
     * splits the bucket into child buckets if the node is null, and recursively processes the
     * rightmost child bucket to compute the empty size.
     *
     * @param B The bucket to process, containing data and metadata for the tree node.
     * @param node A reference to the pointer of the current RRAccessTreeNode. If null, a new node
     *             is created and initialized with child buckets and pointers.
     * @return The total empty size of the rightmost subtree, including the current node's empty size.
     */
    int getEmptyRight(Bucket &B, RRAccessTreeNode* &node) {
        if (B.AGM < 0) idx.setAGMandIters(B);
        if (B.getSplitDim() == B.getDim()) return 1 - B.AGM;
        if (!node) {
            // vector<Bucket> children = idx.Split(B);
            // auto startSplit = chrono::high_resolution_clock::now();
            node = new RRAccessTreeNode(&B, idx.Split(B));
            // auto endSplit = chrono::high_resolution_clock::now();
            // chrono::duration<double> elapsedSplit = endSplit - startSplit;
            // idx.totalSplitTime += elapsedSplit.count();
        }
        int emptyright = node->children_buckets.size() > 0 ? getEmptyRight(node->children_buckets[node->children_buckets.size() - 1], node->children_pointers[node->children_pointers.size() - 1]) : 0;

        return node->emptySize + emptyright;
    }

    bool RRAccess_opt(int k, Bucket &B, RRAccessTreeNode* &node, int offset = 0) {
        if(B.getSplitDim() == B.getDim()){
            result = B.getLowerBound();
            return true;
        }
        if(B.AGM < 0) idx.setAGMandIters(B);
        if (!node) {
            // auto startSplit = chrono::high_resolution_clock::now();
            node = new RRAccessTreeNode(&B, idx.Split(B));
            // auto endSplit = chrono::high_resolution_clock::now();
            // chrono::duration<double> elapsedSplit = endSplit - startSplit;
            // idx.totalSplitTime += elapsedSplit.count();
            trivialIntervals[numti].first = offset + B.AGM - getEmptyRight(B, node) + 1;
            trivialIntervals[numti++].second = offset + B.AGM;
        }
        int childAGM, temp = 0;
        for(int i = 0; i < node->children_buckets.size(); i++) {
            childAGM = node->children_buckets[i].AGM;
            if(offset + temp + childAGM >= k){
                return RRAccess_opt(k, node->children_buckets[i], node->children_pointers[i], offset + temp);
            }
            else temp += childAGM;
        }
        return false;
    }


    bool RRAccess(int k, Bucket &B, RRAccessTreeNode* &node, int offset = 0) {
        if(B.getSplitDim() == B.getDim()){
            result = B.getLowerBound();
            return true;
        }
        if(B.AGM < 0) idx.setAGMandIters(B);
        if (!node) {
            // vector<Bucket> children = idx.Split(B);
            // auto startSplit = chrono::high_resolution_clock::now();
            node = new RRAccessTreeNode(&B, idx.Split(B));
            // auto endSplit = chrono::high_resolution_clock::now();
            // chrono::duration<double> elapsedSplit = endSplit - startSplit;
            // idx.totalSplitTime += elapsedSplit.count();
        }
        if(offset + B.AGM - node->emptySize < k){
            trivialInterval.first = offset + B.AGM - getEmptyRight(B, node) + 1;
            trivialInterval.second = offset + B.AGM;
            return false;
        }
            // return make_pair(false, vector<int>({offset + B.AGM - getEmptyRight(B, node) + 1, offset + B.AGM}));
        int childAGM, temp = 0;
        for(int i = 0; i < node->children_buckets.size() - 1; i++) {
            childAGM = node->children_buckets[i].AGM;
            if(offset + temp + childAGM >= k){
                return RRAccess(k, node->children_buckets[i], node->children_pointers[i], offset + temp);
            }
            else temp += childAGM;
        }
        int last = node->children_buckets.size() - 1;
        bool res = RRAccess(k, node->children_buckets[last], node->children_pointers[last], offset + temp);
        
        if(!res && trivialInterval.second == offset + B.AGM - node->emptySize){
            trivialInterval.second = offset + B.AGM;
            // return make_pair(false, vector<int>({res.second[0], offset + B.AGM}));
        }
        return res;
    }

    
    

public:
    int AGM;
    RRAccessTreeNode* root = NULL;
    Index idx;
    vector<int> result;
    pair<int, int> trivialInterval;
    vector<pair<int, int> > trivialIntervals = vector<pair<int, int> >(50);
    int numti = 0;

    RRAccessTree() {}

    /**
     * @brief Constructs an RRAccessTree object.
     *
     * This constructor initializes the RRAccessTree by creating an Index object
     * using the provided relations, filenames, and numlines. It also retrieves
     * the AGM (AGM bound) from the Index object.
     *
     * @param relations A map where the key is a string representing a relation name,
     *                  and the value is a vector of strings representing its related variables.
     * @param filenames A map where the key is a string representing a relation name,
     *                  and the value is a string representing the corresponding filename.
     * @param numlines  A map where the key is a string representing a relation name,
     *                  and the value is an integer representing the number of lines in the file.
     */
    RRAccessTree(
        const unordered_map<string, vector<string> > &relations,
        const unordered_map<string, string> &filenames,
        const unordered_map<string, int> &numlines) {
        vector<string> q_relations;
        vector<vector<string> > q_variables;
        for(unordered_map<string, vector<string> >::const_iterator it = relations.begin(); it != relations.end(); it++) {
            q_relations.push_back(it->first);
            // q_variables.push_back(it->second);
        }
        sort(q_relations.begin(), q_relations.end());
        for(int i = 0; i < q_relations.size(); i++) {
            q_variables.push_back(relations.at(q_relations[i]));
        }
        Query q(q_relations, q_variables);
        idx = Index(q);
        idx.preProcessing(relations, filenames, numlines);
        AGM = idx.AGM();
    }

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

    Bucket getFullBucket() {
        return idx.getFullBucket();
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
    bool RRAccess(int k) {
        numti = 0;
        return RRAccess_opt(k, idx.FB, root, 0);
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