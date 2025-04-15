#include "Index.hpp"


/**
 * @class RRAccessTreeNode
 * @brief Represents a node in an RR Access Tree structure.
 * 
 * This class encapsulates the properties and behavior of a node in an RR Access Tree.
 * Each node contains a bucket, an AGM value, a list of child buckets
 * paired with their associated integer values, and pointers to its child nodes.
 * 
 * @details
 * The class provides a constructor to initialize the node with its bucket, AGM value,
 * child buckets, and child pointers. It also calculates the `emptySize` of the node,
 * which is the difference between the AGM value and the sum of the AGM values of its children.
 * Additionally, the class includes a `print` method to display the node's information.
 * 
 * @note
 * The `Bucket` class and its `print` method are assumed to be defined elsewhere.
 * 
 * @var RRAccessTreeNode::AGM
 * The AGM value for this node.
 * 
 * @var RRAccessTreeNode::emptySize
 * The difference between the AGM value and the sum of the AGM values of the child buckets.
 * 
 * @var RRAccessTreeNode::B
 * The bucket associated with this node.
 * 
 * @var RRAccessTreeNode::children_buckets
 * A vector of pairs, where each pair consists of a child bucket and its associated integer value.
 * 
 * @var RRAccessTreeNode::children_pointers
 * A vector of pointers to the child nodes of this node.
 */
class RRAccessTreeNode {

public:
    int AGM, emptySize;
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
    RRAccessTreeNode(Bucket B, int AGM, vector<pair<Bucket, int> > children_buckets, vector<RRAccessTreeNode*> children_pointers) : B(B), AGM(AGM), children_buckets(children_buckets), children_pointers(children_pointers) {
        int sumChildrenAGM = 0;
        for(int i = 0; i < children_buckets.size(); i++) {
            sumChildrenAGM += children_buckets[i].second;
        }
        emptySize = AGM - sumChildrenAGM;
    }

    void print() {
        cout << "AGM: " << AGM << ", size: " << children_buckets.size() << ", ";
        B.print();
    }
};

class RRAccessTree {
private:


    
    /**
     * @brief Recursively computes the sum empty sizes of the right side of a given bucket 
     * and its right-most child in an RRAccessTree.
     *
     * This function recursively calculates the empty size on the right side of a bucket
     * by traversing the RRAccessTree structure. If the node corresponding to the bucket
     * does not exist, it creates the node and its children based on the bucket's split.
     *
     * @param B The bucket for which the empty right size is to be calculated.
     * @param node A reference to the pointer of the RRAccessTreeNode corresponding to the bucket.
     *             If the node does not exist, it will be created.
     * @param AGM (Optional) The AGM (Aggregate Measure) value for the bucket. If not provided
     *            (default is -1), it will be computed using idx.AGMforBucket(B).
     * @return The total empty size on the right side of the given bucket.
     */
    int getEmptyRight(Bucket B, RRAccessTreeNode* &node, int AGM = -1) {
        if (AGM < 0) AGM = idx.AGMforBucket(B);
        if (B.getSplitDim() == B.getDim()) return 1 - AGM;
        if (!node) {
            vector<pair<Bucket, int> > children = idx.split(B, AGM);
            node = new RRAccessTreeNode(B, AGM, children, vector<RRAccessTreeNode*>(children.size(), NULL));
        }
        int emptyright = node->children_buckets.size() > 0 ? getEmptyRight(node->children_buckets[node->children_buckets.size() - 1].first, node->children_pointers[node->children_pointers.size() - 1], node->children_buckets[node->children_buckets.size() - 1].second) : 0;

        return node->emptySize + emptyright;
    }

    pair<bool, vector<int> > RRAccess(int k, Bucket B, RRAccessTreeNode* &node, int offset = 0, int AGM = -1) {
        if(B.getSplitDim() == B.getDim()) return make_pair(true, B.getLowerBound());
        if(AGM < 0) AGM = idx.AGMforBucket(B);
        if (!node) {
            vector<pair<Bucket, int> > children = idx.split(B, AGM);
            node = new RRAccessTreeNode(B, AGM, children, vector<RRAccessTreeNode*>(children.size(), NULL));
        }
        if(offset + AGM - node->emptySize < k)
            return make_pair(false, vector<int>({offset + AGM - getEmptyRight(B, node, AGM) + 1, offset + AGM}));
        int childAGM, temp = 0;
        for(int i = 0; i < node->children_buckets.size() - 1; i++) {
            childAGM = node->children_buckets[i].second;
            if(offset + temp + childAGM >= k){
                return RRAccess(k, node->children_buckets[i].first, node->children_pointers[i], offset + temp, childAGM);
            }
            else temp += childAGM;
        }
        int last = node->children_buckets.size() - 1;
        pair<bool, vector<int> > res = RRAccess(k, node->children_buckets[last].first, node->children_pointers[last], offset + temp, node->children_buckets[last].second);
        
        if(!res.first && res.second[1] == offset + AGM - node->emptySize)return make_pair(false, vector<int>({res.second[0], offset + AGM}));
        else return res;
    }

    
    

public:
    int AGM;
    RRAccessTreeNode* root = NULL;
    Index idx;

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