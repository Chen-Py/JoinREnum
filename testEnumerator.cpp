#include <bits/stdc++.h>
#include "ReadConfig.hpp"
#include "Enumerator.hpp"
using namespace std;

int main() {
    if(freopen("res/result.txt", "w", stdout) == NULL)cout << "WRITEERR" << endl;

    unordered_map<string, string> filenames = readFilenames("db/filenames.txt");
    unordered_map<string, int> numlines = readNumLines("db/numlines.txt");
    unordered_map<string, vector<string> > relations = readRelations("db/relations.txt");

    Enumerator enumerator(relations, filenames, numlines);
    enumerator.random_enumerate();
    return 0;
}