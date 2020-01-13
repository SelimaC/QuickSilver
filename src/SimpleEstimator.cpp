#include <set>
#include <cmath>
#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include "PathTree.h"

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
    table_card = new uint32_t[graph->getNoLabels()];
    column_card = new std::pair<uint32_t,uint32_t>[graph->getNoLabels()];
}


void SimpleEstimator::prepare() {
    uint32_t labels = graph->getNoLabels();

    std::set<int> *firsts = new std::set<int> [labels];

    std::set<int> *seconds = new std::set<int> [labels];

    for(int i = 0; i<labels; i++) {
        column_card[i] = std::make_pair(0,0);
        table_card[i] = 0;
    }
    bool bits[labels];
    for (int j = 0; j < labels; j++) {
        bits[j] = false;
    }

    for(int k =0; k< labels; k++){
        table_card[k] = 0;
    }

    for(int label = 0; label < graph->getNoLabels(); label++) {
        for(int i = 0; i < graph->edge_pairs[label].size(); i++) {
            table_card[label] += 1;
            firsts[label].insert(graph->edge_pairs[label][i].first);
            seconds[label].insert(graph->edge_pairs[label][i].second);
        }
    }

    for(int i = 0; i<labels; i++) {
        column_card[i].first= firsts[i].size();
        column_card[i].second= seconds[i].size();
    }

    /*for(int i = 0; i<labels; i++) {
        std::cout << "Label " << i << " " << table_card[i] << "\n";
    }*/




    uint32_t current_label;
    for(int i=0; i < adj_size; i++) {
        for( int j=0; j < graph->adj[i].size(); j++) {
            current_label = graph->adj[i][j].first;
            table_card[current_label] += 1;
            bits[current_label] = true;
        }
        for (int k = 0; k < labels; k++) {
            if (bits[k]) {
                column_card[current_label].first++;
                bits[k] = false;
            }
        }
    }
    for (int j = 0; j < labels; j++) {
        bits[j] = false;
    }
    for(int i=0; i < reverse_adj_size; i++) {
        uint32_t prev_label = -1;
        for( int j=0; j < graph->reverse_adj[i].size(); j++) {
            current_label = graph->reverse_adj[i][j].first;
            bits[current_label] = true;
        }
        for (int k = 0; k < labels; k++) {
            if (bits[k]) {
                column_card[current_label].second++;
                bits[k] = false;
            }
        }
    }
    double multip = (double)graph->getNoDistinctEdges() / graph->getNoEdges();
    for(int i = 0; i < labels; i++) {
        table_card[i] *= multip;
    }

    sampling = new double[2*labels];
    double num[2*labels];
    for(int i = 0; i < labels; i++) {
        sampling[i] = 0;
        num[i] = 0;
    }

    for (int j = 0; j < labels; j++) {
        bits[j] = false;
    }
    for ( int i = 0; i < vertices/2; i++) {
        int ind = std::rand() % vertices;
        for ( auto c: graph->adj[ind]) {
            sampling[c.first]++;
            bits[c.first] = true;
        }
        for ( int k = 0; k < labels; k++) {
            if (bits[k])
                num[k]++;
            bits[k] = false;
        }
        for( auto c: graph->reverse_adj[ind]) {
            sampling[labels+c.first]++;
            bits[c.first] = true;
        }
        for ( int k = 0; k < labels; k++) {
            if (bits[k])
                num[labels+k]++;
            bits[k] = false;
        }
    }

    for(int i = 0; i < labels; i++) {
        if (sampling[i] != 0)
            sampling[i] = sampling[i] / num[i];
        if (sampling[i+labels] != 0)
            sampling[i+labels] = sampling[i+labels] / num[i+labels];
        //std::cout << sampling[i] << std::endl;
    }

}




std::vector<std::string> queryToVector(PathQuery *q) {
    PathTree *path = q->path;

    PathTree *curr2 = path;


    std::vector<std::string> labels_path;
    while(!curr2->isLeaf()) {
        if (curr2->isConcat()) {
            labels_path.push_back(curr2->right->data);
        }
        curr2 = curr2->left;
    }
    labels_path.push_back(curr2->data);
    return labels_path;
}

cardStat SimpleEstimator::estimate(PathQuery *q) {

    /*PathTree *path = q->path;

    PathTree *curr2 = path;


    std::vector<std::string> labels_path;
    while(!curr2->isLeaf()) {
        if (curr2->isConcat()) {
            labels_path.push_back(curr2->right->data);
        }
        curr2 = curr2->left;
    }
    labels_path.push_back(curr2->data);*/
    std::vector<std::string> labels_path = queryToVector(q);
    
    std::string leftmost=labels_path[0];
    std::string rightmost=labels_path[labels_path.size()-1];
    //bool left_bounded = q->s != "*";
    //bool right_bounded = q->t != "*";
    float result = table_card[std::stoi(rightmost)];

    for(int i =labels_path.size()-2;i>=0;i--){
        int max= std::max(column_card[std::stoi(labels_path[i])].first, column_card[std::stoi(labels_path[i])].second);
        result = (result*table_card[std::stoi(labels_path[i])])/(float)max;
    }


    /*if(left_bounded) {
        result=result/(float)column_card[std::stoi(rightmost)].first;
    }
    if(right_bounded) {
        result=result/(float)column_card[std::stoi(leftmost)].second;
    }*/

    return cardStat {0, (uint32_t)std::round(result), 0};

}

table_calc SimpleEstimator::recur(PathTree *q, uint32_t first_nodeid) {
    PathTree *curr = q;
    if(curr->isLeaf()) {
        uint32_t curr_label = std::stoi(curr->data);
        table_calc to_return;
        uint32_t num_of_corr = 1; // will be used only if source bounded.
        if(curr->data.back() == '+') {
            to_return.prev_col_from = column_card[curr_label].first;
            to_return.prev_table_card = table_card[curr_label];
            to_return.prev_col_to = column_card[curr_label].second;

        }
        else {

                to_return.prev_table_card = table_card[curr_label];
                to_return.prev_col_from = column_card[curr_label].second;
                to_return.prev_col_to = column_card[curr_label].first;

        }
        return to_return;
    }
    else {
        table_calc prev = SimpleEstimator::recur(curr->left, first_nodeid);
        if(prev.prev_table_card == 0 || prev.prev_col_from == 0 || prev.prev_col_to == 0)
            return table_calc{0,0,0};
        table_calc to_return;
        uint32_t curr_label = std::stoi(curr->right->data);
        if(curr->right->data.back() == '+') {
            //float max = column_card[curr_label].first > prev.prev_col_to ? column_card[curr_label].first : prev.prev_col_to;
            float min = column_card[curr_label].first < prev.prev_col_to ? column_card[curr_label].first : prev.prev_col_to;
            to_return.prev_table_card = (float)(prev.prev_table_card) / prev.prev_col_to * min * sampling[curr_label];
            to_return.prev_col_from = prev.prev_col_from * min / prev.prev_col_to;
            to_return.prev_col_to = column_card[curr_label].second * min / column_card[curr_label].first;
        }
        else {
            float max = column_card[curr_label].second > prev.prev_col_from ? column_card[curr_label].second : prev.prev_col_from;
            float min = column_card[curr_label].second < prev.prev_col_from ? column_card[curr_label].second : prev.prev_col_from;
            to_return.prev_table_card = (float)(prev.prev_table_card * table_card[curr_label]) / (float)(max);
            to_return.prev_col_from = (float)(prev.prev_col_to * min) / (float)(prev.prev_col_from);
            to_return.prev_col_to = column_card[curr_label].first * min / column_card[curr_label].second;
        }
        return to_return;
    }
}


cardStat SimpleEstimator::estimate_vec(std::vector<std::string> labels_path) {

    std::string query = "";
    for (int i = 0; i < labels_path.size() ; ++i) {
        query += labels_path[i];
    }
    PathTree *path = PathTree::strToTree(query);
    PathTree *curr = PathTree::strToTree(query);

    float result = 1;
    uint32_t min = 0xFFFFFFFF;
    uint32_t curr_label = -1;
    uint32_t curr_col_card = -1;
    uint32_t prev_col_card = -1;

    uint32_t first_item = -1;

    result = recur(curr,first_item).prev_table_card;
    return cardStat {0, (uint32_t)std::round(result), 0};
}

SimpleEstimator::~SimpleEstimator() {
    delete[] table_card;
    delete[] column_card;
}
