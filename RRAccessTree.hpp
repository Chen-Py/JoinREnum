#include "SplitBucket.hpp"
#include "Index.hpp"
class RRAccessTreeNode {

public:
    int AGM;
    Bucket B;
    vector<pair<Bucket, int> > children_buckets;
    vector<RRAccessTreeNode*> children_pointers;

    RRAccessTreeNode(Bucket B, int AGM, vector<pair<Bucket, int> > children_buckets, vector<RRAccessTreeNode*> children_pointers) : B(B), AGM(AGM), children_buckets(children_buckets), children_pointers(children_pointers) {}

};

class RRAccessTree {
private:
    RRAccessTreeNode* root = NULL;
    Index idx;
public:
    RRAccessTree(Query q, unordered_map<string, string> filenames, unordered_map<string, int> numlines, unordered_map<string, vector<string> > relations) {
        idx = Index(q);
        idx.preProcessing(relations, filenames, numlines);
    }

    pair<bool, vector<int> > RRAccess(int k, const Bucket &B, RRAccessTreeNode* &node) {
        if (!node) {
            if(B.getSplitDim() == B.getDim()) return make_pair(true, B.getLowerBound());
            int AGM = idx.AGMforBucket(B);
            vector<pair<Bucket, int> > children = idx.split(B);
            node = new RRAccessTreeNode(B, AGM, children, vector<RRAccessTreeNode*>(children.size(), NULL));
        }
        int pos;
        for(pos = 0; pos < node->children_buckets.size(); pos++) {
            if(node->children_buckets[pos].second >= k) break;
            else k -= node->children_buckets[pos].second;
        }
        if(pos == node->children_buckets.size()) return make_pair(false, vector<int>());
        else return RRAccess(k, node->children_buckets[pos].first, node->children_pointers[pos]);
    }
};