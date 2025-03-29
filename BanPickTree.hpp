#include<bits/stdc++.h>
using namespace std;
struct node {
    int low, high, take, height = 1;
    node *left, *right;
    bool operator < (const node &n) const {
        return high < n.low;
    }
    node(int low, int high) : low(low), high(high), take(high - low + 1), left(NULL), right(NULL) {}
    void update(){
        take = high - low + 1 + (left ? left->take : 0) + (right ? right->take : 0);
        height = max(left ? left->height : 0, right ? right->height : 0) + 1;
        return;
    }
};
// implement Banned interval tree with AVL tree
class BanPickTree {
    private:
        random_device rd;
        mt19937 gen = mt19937(rd());
        node *root = NULL;
        int H = 0;
        void printSubTree(node* v){
            if(v == NULL) return;
            printSubTree(v->left);
            cout << v->low << " " << v->high << " " << v->take << endl;
            printSubTree(v->right);
        }
        void rotateLeft(node* &v){
            node* u = v->right;
            v->right = u->left;
            u->left = v;
            v->update();
            u->update();
            v = u;
            return;
        }
        void rotateRight(node* &v){
            node* u = v->left;
            v->left = u->right;
            u->right = v;
            v->update();
            u->update();
            v = u;
            return;
        }
        void insertSubTree(node* &v, node* nv){
            if(v == NULL){
                v = nv;
                return;
            }
            if(*nv < *v){
                insertSubTree(v->left, nv);
                if(v->left->height - (v->right ? v->right->height : 0) == 2){
                    if(*nv < *v->left){
                        rotateRight(v);
                    }
                    else{
                        rotateLeft(v->left);
                        rotateRight(v);
                    }
                }
            }
            else{
                insertSubTree(v->right, nv);
                if(v->right->height - (v->left ? v->left->height : 0) == 2){
                    if(*v->right < *nv){
                        rotateLeft(v);
                    }
                    else{
                        rotateRight(v->right);
                        rotateLeft(v);
                    }
                }
            }
            v->update();
            return;
        }

        int G(){
            node *u = root;
            uniform_int_distribution<int> udist(1, H - (u ? u->take : 0));
            int y = udist(gen);
            int b = 0, temp = 0;
            while(u != NULL){
                temp = u->left ? u->left->take : 0;
                if(y + b + temp < u->low){
                    u = u->left;
                }
                else{
                    b = b + temp + (u->high - u->low + 1);
                    u = u->right;
                }
            }
            return y + b;
        }
    public:
        BanPickTree(){}
        
        BanPickTree(int H): H(H) {}
        
        int getTotal(){
            return H;
        }
        double getPercentage(){
            return 1.0 - 1.0 * remaining() / H;
        }
        void ban(int low, int high){
            // H = max(H, high);
            if(root == NULL){
                root = new node(low, high);
                return;
            }
            insertSubTree(root, new node(low, high));
            return;
        }
        int pick(){
            return remaining() ? G() : 0;
        }
        int remaining(){
            return H - (root ? root->take : 0);
        }
        

        bool available(int x){
            node *u = root;
            while(u){
                if(x >= u->low && x <= u->high) return false;
                if(x < u->low)u = u->left;
                else u = u->right;
            }
            return true;
        }
        
        bool available(int low, int high){
            node *u = root;
            while(u){
                if(low <= u->high && high >= u->low) return false;
                if(high < u->low)u = u->left;
                else u = u->right;
            }
            return true;
        }
        void print(){
            printSubTree(root);
        }
};
