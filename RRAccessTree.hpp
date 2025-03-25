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
        cout << "AGM: " << AGM << ", ";
        B.print();
    }
};

class RRAccessTree {
private:
    RRAccessTreeNode* root = NULL;
    Index idx;
public:
    int AGM;
    RRAccessTree(Query q, unordered_map<string, string> filenames, unordered_map<string, int> numlines, unordered_map<string, vector<string> > relations) {
        idx = Index(q);
        idx.preProcessing(relations, filenames, numlines);
        AGM = idx.AGM();
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

    pair<bool, vector<int> > RRAccess(int k) {
        return RRAccess(k, idx.getFullBucket(), root);
    }

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