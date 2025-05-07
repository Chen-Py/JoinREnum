#include <bits/stdc++.h>
using namespace std;

struct node {
    int low, high, take, height;
    int left = -1, right = -1;
    node() : low(0), high(0), take(0), height(1) {}
    node(int low, int high) : low(low), high(high), take(high - low + 1), height(1) {}

    void update(vector<node>& pool) {
        take = high - low + 1;
        if (left != -1) take += pool[left].take;
        if (right != -1) take += pool[right].take;
        height = max(left != -1 ? pool[left].height : 0, right != -1 ? pool[right].height : 0) + 1;
    }

    bool operator<(const node& n) const {
        return high < n.low;
    }
};

class BanPickTree {
private:
    vector<node> pool;
    int root = -1;
    int H = 0;

    void printSubTree(int v) {
        if (v == -1) return;
        printSubTree(pool[v].left);
        cout << pool[v].low << " " << pool[v].high << " " << pool[v].take << endl;
        printSubTree(pool[v].right);
    }

    void rotateLeft(int& v) {
        int u = pool[v].right;
        pool[v].right = pool[u].left;
        pool[u].left = v;
        pool[v].update(pool);
        pool[u].update(pool);
        v = u;
    }

    void rotateRight(int& v) {
        int u = pool[v].left;
        pool[v].left = pool[u].right;
        pool[u].right = v;
        pool[v].update(pool);
        pool[u].update(pool);
        v = u;
    }

    void insertSubTree(int& v, int nv) {
        if (v == -1) {
            v = nv;
            return;
        }
        if (pool[nv] < pool[v]) {
            insertSubTree(pool[v].left, nv);
            if (pool[v].left != -1 && pool[pool[v].left].height - (pool[v].right != -1 ? pool[pool[v].right].height : 0) == 2) {
                if (pool[nv] < pool[pool[v].left]) {
                    rotateRight(v);
                } else {
                    rotateLeft(pool[v].left);
                    rotateRight(v);
                }
            }
        } else {
            insertSubTree(pool[v].right, nv);
            if (pool[v].right != -1 && pool[pool[v].right].height - (pool[v].left != -1 ? pool[pool[v].left].height : 0) == 2) {
                if (pool[pool[v].right] < pool[nv]) {
                    rotateLeft(v);
                } else {
                    rotateRight(pool[v].right);
                    rotateLeft(v);
                }
            }
        }
        pool[v].update(pool);
    }

    int G() {
        int u = root;
        uniform_int_distribution<int> udist(1, H - (u != -1 ? pool[u].take : 0));
        int y = udist(gen);
        int b = 0, temp = 0;
        while (u != -1) {
            temp = pool[u].left != -1 ? pool[pool[u].left].take : 0;
            if (y + b + temp < pool[u].low) {
                u = pool[u].left;
            } else {
                b = b + temp + (pool[u].high - pool[u].low + 1);
                u = pool[u].right;
            }
        }
        return y + b;
    }

public:
    mt19937 gen;

    BanPickTree() : gen(random_device{}()) {}

    BanPickTree(int H) : H(H), gen(random_device{}()) {}

    int getTotal() {
        return H;
    }

    double getPercentage() {
        return 1.0 - 1.0 * remaining() / H;
    }

    void ban(int low, int high) {
        if (root == -1) {
            pool.emplace_back(low, high);
            root = pool.size() - 1;
            return;
        }
        pool.emplace_back(low, high);
        insertSubTree(root, pool.size() - 1);
    }

    int pick() {
        return remaining() ? G() : 0;
    }

    int remaining() {
        return H - (root != -1 ? pool[root].take : 0);
    }

    bool available(int x) {
        int u = root;
        while (u != -1) {
            if (x >= pool[u].low && x <= pool[u].high) return false;
            if (x < pool[u].low)
                u = pool[u].left;
            else
                u = pool[u].right;
        }
        return true;
    }

    bool available(int low, int high) {
        int u = root;
        while (u != -1) {
            if (low <= pool[u].high && high >= pool[u].low) return false;
            if (high < pool[u].low)
                u = pool[u].left;
            else
                u = pool[u].right;
        }
        return true;
    }

    void print() {
        printSubTree(root);
    }
};