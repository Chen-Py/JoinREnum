#include<map>
#include<iostream>
#include<string>
#include<vector>
#include<glpk.h>
#include <cmath>
using namespace std;
class Query{
    private:
        vector<string> relationNames;
        vector<vector<string> > relationVars;
        map<string, int> variables;
        vector<string> variableNames;
        vector<vector<int> > relations;
        vector<int> cardinalities;
        vector<vector<int> > rels;
        // vector<string> variableNames;
        glp_prob *lp;
        
        void initRels(){
            rels = vector<vector<int> >(variables.size(), vector<int>());
            for(int i = 0; i < relations.size(); i++){
                for(int j = 0; j < relations[i].size(); j++){
                    rels[relations[i][j]].push_back(i);
                }
            }
        }
        
        void initLP(){
            lp = glp_create_prob();
            // glp_set_prob_name(lp, "relaxed_hypergraph_edge_cover");
            glp_set_obj_dir(lp, GLP_MIN);

            glp_add_cols(lp, relations.size());
            for (int i = 1; i <= relations.size(); i++) {
                glp_set_col_name(lp, i, ("r" + std::to_string(i)).c_str());
                glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0);
            }

            glp_add_rows(lp, variables.size());
            for(int i = 1; i <= variables.size(); i++){
                glp_set_row_name(lp, i, ("v" + std::to_string(i)).c_str());
                glp_set_row_bnds(lp, i, GLP_LO, 1.0, 0.0);
                vector<int> rs = rels[i - 1];
                int ind[rs.size() + 1] = {0};
                double val[rs.size() + 1] = {0};
                for(int j = 0; j < rs.size(); j++){
                    ind[j + 1] = rs[j] + 1;
                    val[j + 1] = 1.0;
                }
                glp_set_mat_row(lp, i, rs.size(), ind, val);
            }
        }

        void updateCars(vector<int> cars = {}){
            if(cars.size() == relations.size()){
                this->cardinalities = cars;
                for(int i = 1; i <= relations.size(); i++){
                    glp_set_obj_coef(lp, i, log2(cars[i - 1]));
                }
            }
        }
    public:
        Query(){}

        Query(vector<string> relationNames, vector<vector<string> > relations, vector<int> cardinalities = {}){
            this->relationNames = relationNames;
            this->relationVars = relations;
            this->cardinalities = cardinalities;
            int cnt = 0;
            for(int i = 0; i < relations.size(); i++){
                vector<int> relation;
                for(int j = 0; j < relations[i].size(); j++){
                    if(this->variables.find(relations[i][j]) == this->variables.end()){
                        this->variables[relations[i][j]] = cnt++;
                        this->variableNames.push_back(relations[i][j]);
                    }
                    relation.push_back(this->variables[relations[i][j]]);
                }
                this->relations.push_back(relation);
            }
            initRels();
            initLP();
            updateCars(cardinalities);
        }

        ~Query(){
            // glp_delete_prob(lp);
        }

        int getVarIndex(string var){
            return variables[var];
        }

        int getVarNumber(){
            return variables.size();
        }

        const vector<string>& getVarNames(){
            return variableNames;
        }

        const vector<string>& getRelNames(){
            return relationNames;
        }

        const vector<vector<string> >& getRelVars(){
            return relationVars;
        }

        const vector<vector<int> >& getRelations(){
            return relations;
        }

        const vector<int>& getRels(int i){
            return rels[i];
        }

        const vector<int>& getRels(string var){
            return rels[variables[var]];
        }

        int getCardinality(int i){
            return cardinalities[i];
        }

        void print(){
            cout << "Variables: " << endl;
            for(auto it = variables.begin(); it != variables.end(); it++){
                cout << it->first << ": " << it->second << endl;
            }
            cout << "Relations: " << endl;
            for(int i = 0; i < relations.size(); i++){
                cout << "R" << i+1 <<"(";
                for(int j = 0; j < relations[i].size(); j++){
                    cout << relations[i][j];
                    if(j + 1 < relations[i].size()) cout << ", ";
                }
                cout << ")" << endl;
            }
        
        }

        double AGM(vector<int> cars = {}){
            for (int i = 0; i < cars.size(); i++)if(cars[i] <= 0)return 0;
            initLP();
            updateCars(cars);

            glp_smcp params;
            glp_init_smcp(&params);
            params.msg_lev = GLP_MSG_OFF;
            glp_simplex(lp, &params);

            double res = glp_get_obj_val(lp);

            glp_delete_prob(lp);
            // std::cout << "Optimal objective value: " << res << std::endl;
            // for (int i = 1; i <= 3; ++i) {
            //     double xi = glp_get_col_prim(lp, i);
            //     std::cout << "x" << i << " = " << xi << std::endl;
            // }
            return pow(2, res);
        }
};