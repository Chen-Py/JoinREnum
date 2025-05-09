using namespace std;


void getpos(const vector<pair<vector<Point<int> >::iterator, vector<Point<int> >::iterator> > &iters, const vector<int> &d, const int target, vector<int> &pos) {
    int dim;
    for (int i = 0; i < iters.size(); i++) {
        dim = d[i];
        if(dim == -1) pos[i] = iters[i].second - iters[i].first;
        else pos[i] = lower_bound(iters[i].first, iters[i].second, target,
            [dim](const Point<int> &a, const int &t){
                return a[dim] < t;
            }) - iters[i].first;
    }
}


int MultiHeadBinarySearch(const vector<pair<vector<Point<int> >::iterator, vector<Point<int> >::iterator> > &iters, const vector<int> &d,  const int target, Query &q) {
    vector<pair<vector<Point<int> >::iterator, vector<Point<int> >::iterator> > bounds = iters;
    vector<vector<Point<int> >::iterator> itermid(iters.size());
    vector<int> pos(iters.size());
    vector<int> tmppos(iters.size());
    int mini, maxi, cnt = 0, upp;
    double ans;
    for(int i = 0; i < iters.size(); i++) {
        if(d[i] < 0) {
            pos[i] = iters[i].second - iters[i].first;
            cnt++;
            continue;
        }
        itermid[i] = iters[i].first + (iters[i].second - iters[i].first) / 2;
        pos[i] = itermid[i] - iters[i].first;
    }
    while(cnt < iters.size()) {
        mini = -1, maxi = -1;
        for(int i = 0; i < iters.size(); i++){
            if(d[i] < 0 || bounds[i].second - bounds[i].first <= 1) continue;
            if(mini == -1 || (*itermid[i])[d[i]] < (*itermid[mini])[d[i]]) mini = i;
            if(maxi == -1 || (*itermid[i])[d[i]] > (*itermid[maxi])[d[i]]) maxi = i;
        }
        ans = q.AGM(pos);
        upp = ceil(ans) - ans < 1e-5 ? ceil(ans) : int(ans);
        if(upp <= target) {
            bounds[mini].first = itermid[mini];
            if(bounds[mini].second - bounds[mini].first <= 1) {
                getpos(iters, d, (*bounds[mini].first)[d[mini]] + 1, tmppos);
                ans = q.AGM(tmppos);
                upp = ceil(ans) - ans < 1e-5 ? ceil(ans) : int(ans);
                if(upp > target) return (*bounds[mini].first)[d[mini]];
                else pos[mini] = bounds[mini].second - iters[mini].first;
                cnt++;
            }
            else {
                itermid[mini] = bounds[mini].first + (bounds[mini].second - bounds[mini].first) / 2;
                pos[mini] = itermid[mini] - iters[mini].first;
            }
        }
        else {
            bounds[maxi].second = itermid[maxi];
            if(bounds[maxi].second - bounds[maxi].first <= 1) {
                getpos(iters, d, (*bounds[maxi].first)[d[maxi]] + 1, tmppos);
                ans = q.AGM(tmppos);
                upp = ceil(ans) - ans < 1e-5 ? ceil(ans) : int(ans);
                if(upp > target) return (*bounds[maxi].first)[d[maxi]];
                else pos[maxi] = bounds[maxi].second - iters[maxi].first;
                cnt++;
            }
            else {
                itermid[maxi] = bounds[maxi].first + (bounds[maxi].second - bounds[maxi].first) / 2;
                pos[maxi] = itermid[maxi] - iters[maxi].first;
            }
        }
    }
    int res = 2147483647;
    for(int i = 0; i < iters.size(); i++) {
        if(bounds[i].second != iters[i].second) res = min(res, (*bounds[i].second)[d[i]]);
    }
    return res;
}

void getpos(const vector<pair<vector<int>::iterator, vector<int>::iterator> > &iters, const vector<pair<vector<int>::iterator, vector<int>::iterator> > &bounds, const vector<bool> &flag, const int target, vector<int> &pos) {
    for (int i = 0; i < iters.size(); i++) {
        if(!flag[i]) pos[i] = iters[i].second - iters[i].first;
        else pos[i] = lower_bound(bounds[i].first, bounds[i].second, target) - iters[i].first;
    }
}

int MultiHeadBinarySearch(const vector<pair<vector<int>::iterator, vector<int>::iterator> > &iters, const vector<bool> &flag, vector<int> &rels, const int target, Query &q) {
    vector<pair<vector<int>::iterator, vector<int>::iterator> > bounds = iters;
    vector<vector<int>::iterator> itermid(iters.size());
    vector<int> pos(iters.size());
    vector<int> tmppos(iters.size());
    int mini, maxi, cnt = 0, upp;
    double res;
    for(int i = 0; i < iters.size(); i++) {
        if(!flag[i]) {
            pos[i] = iters[i].second - iters[i].first;
            cnt++;
        }
        else if(iters[i].second - iters[i].first <= 1) {
            getpos(iters, bounds, flag, *iters[i].first + 1, tmppos);
            res = q.AGM(tmppos);
            upp = ceil(res) - res < 1e-5 ? ceil(res) : int(res);
            if(upp > target) return *iters[i].first;
            else pos[i] = iters[i].second - iters[i].first;
            cnt++;
        }
        else{
            itermid[i] = iters[i].first + (iters[i].second - iters[i].first) / 2;
            pos[i] = itermid[i] - iters[i].first;
        }
    }
    while(cnt < iters.size()) {
        mini = -1, maxi = -1;
        for(size_t i : rels){
            if(bounds[i].second - bounds[i].first <= 1) continue;
            if(mini == -1 || *itermid[i] < *itermid[mini]) mini = i;
            if(maxi == -1 || *itermid[i] > *itermid[maxi]) maxi = i;
        }
        res = q.AGM(pos);
        upp = ceil(res) - res < 1e-5 ? ceil(res) : int(res);
        if(upp <= target) {
            bounds[mini].first = itermid[mini];
            if(bounds[mini].second - bounds[mini].first <= 1) {
                if(*bounds[mini].first == *bounds[mini].second) return *bounds[mini].first;
                getpos(iters, bounds, flag, *bounds[mini].first + 1, tmppos);
                res = q.AGM(tmppos);
                upp = ceil(res) - res < 1e-5 ? ceil(res) : int(res);
                if(upp > target) return *bounds[mini].first;
                else pos[mini] = bounds[mini].second - iters[mini].first;
                cnt++;
            }
            else {
                itermid[mini] = bounds[mini].first + (bounds[mini].second - bounds[mini].first) / 2;
                pos[mini] = itermid[mini] - iters[mini].first;
            }
        }
        else {
            bounds[maxi].second = itermid[maxi];
            if(bounds[maxi].second - bounds[maxi].first <= 1) {
                if(*bounds[maxi].first == *bounds[maxi].second) return *bounds[maxi].first;
                getpos(iters, bounds, flag, *bounds[maxi].first + 1, tmppos);
                res = q.AGM(tmppos);
                upp = ceil(res) - res < 1e-5 ? ceil(res) : int(res);
                if(upp > target) return *bounds[maxi].first;
                else pos[maxi] = bounds[maxi].second - iters[maxi].first;
                cnt++;
            }
            else {
                itermid[maxi] = bounds[maxi].first + (bounds[maxi].second - bounds[maxi].first) / 2;
                pos[maxi] = itermid[maxi] - iters[maxi].first;
            }
        }
    }
    int ans = 2147483647;
    for(int i = 0; i < iters.size(); i++) {
        if(bounds[i].second != iters[i].second) ans = min(ans, *bounds[i].second);
    }
    return ans;
}