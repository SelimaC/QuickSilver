#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;
    est = nullptr; // estimator not attached by default
}

void SimpleEvaluator::attachEstimator(std::shared_ptr<SimpleEstimator> &e) {
    est = e;
}

void SimpleEvaluator::prepare() {

    // if attached, prepare the estimator
    if(est != nullptr) est->prepare();

    // prepare other things here.., if necessary

}


std::vector<std::pair<uint32_t,uint32_t>> selectSource(std::string &s, std::vector<std::pair<uint32_t,uint32_t>> &in) {

    std::vector<std::pair<uint32_t,uint32_t>> out;
    for(auto tuple: in) {
        if(tuple.first ==std::stoi(s)){

            out.emplace_back(tuple.first, tuple.second);
        }
    }

    return out;
}

std::vector<std::pair<uint32_t,uint32_t>> selectTarget(std::string &t, std::vector<std::pair<uint32_t,uint32_t>> &in) {

    std::vector<std::pair<uint32_t,uint32_t>> out;
    for(auto tuple: in) {
        if(tuple.second ==std::stoi(t)){

            out.emplace_back(tuple.first, (uint32_t) std::stoi(t));
        }
    }

    return out;
}

cardStat SimpleEvaluator::computeStats(std::vector<std::pair<uint32_t,uint32_t>> &g) {

    cardStat stats {};

    stats.noPaths =  g.size();

    return stats;
}

bool compare( std::pair <uint32_t,uint32_t> p1,std::pair<uint32_t,uint32_t> p2){
    return p1.second<p2.second;
}

std::vector<std::pair<uint32_t,uint32_t>> SimpleEvaluator::join( std::vector<std::pair<uint32_t,uint32_t>> &left,  std::vector<std::pair<uint32_t,uint32_t>> &right, uint32_t left_size, uint32_t join_type) {

    std::vector<std::pair<uint32_t,uint32_t>> out;

    uint32_t index = 0;
    uint32_t start_index = 0;

    uint32_t curr_node = 0;
    uint32_t prev_node = 0;


    if(join_type == 0) {
        sort(left.begin(),left.end(),compare);
        std::sort(right.begin(),right.end());
        for (int i = 0; i < left.size(); i++) {
            uint32_t leftTarget = left[i].second;
            curr_node = leftTarget;
            if (curr_node == prev_node) {
                index = start_index;
            }

            start_index = index;

            while (index < right.size() && right[index].first < leftTarget) {

                index++;
            }
            while (index < right.size() && right[index].first == leftTarget) {

                auto rightTarget = right[index].second;
                out.emplace_back(left[i].first, rightTarget);
                index++;

            }

            prev_node = leftTarget;
        }
    } else{

        sort(left.begin(),left.end(),compare);
        std::sort(right.begin(),right.end());
        for (int i = 0; i < right.size(); i++) {
            uint32_t rightSource= right[i].first;
            curr_node = rightSource;
            if (curr_node == prev_node) {
                index = start_index;
            }

            start_index = index;

            while (index < left.size() && left[index].second < rightSource) {

                index++;
            }
            while (index < left.size() && left[index].second == rightSource) {

                auto rightTarget = right[i].second;
                out.emplace_back(left[index].first, rightTarget);
                index++;

            }

            prev_node = rightSource;
        }
    }


    std::sort(out.begin(),out.end());
    out.erase(unique(out.begin(), out.end()), out.end());

    return out;
}



std::vector<std::pair<int,int>> SimpleEvaluator::findBest(std::vector<std::string> q) {
    std::pair<int,int> g = std::make_pair(-1,-1);
    std::vector<std::pair<int,int>> result; 
    std::vector<int> vals;
    for ( int i = 1; i < q.size(); i++) {
        vals.clear();
        for(int j = 0; j < q.size() - i; j++) {
            std::vector<std::string>::const_iterator first = q.begin() + j;
            std::vector<std::string>::const_iterator last = q.begin() + j + i;
            std::vector<std::string> newVec(first,last);
            vals.push_back(est->estimate_vec(newVec).noPaths);
        }
        int min = 2147483647;
        int min_ind = -1;
        for(int k = 0; k < vals.size(); k++) {
            if (vals[k] < min) {
                min_ind = k;
                min = vals[k];
            }
        }
        result.push_back(std::make_pair(min_ind,min_ind+i));
    }
    return result;
}

std::vector<std::pair<uint32_t,uint32_t>> SimpleEvaluator::evaluatePath(std::vector<std::string> q, std::string s, std::string t) {
    std::regex directLabel (R"((\d+)\+)");
    std::regex inverseLabel (R"((\d+)\-)");

    uint32_t label1;
    bool inverse1;
    uint32_t label2;
    bool inverse2;
    std::smatch matches;

    bool start = est->table_card[std::stoi(q[0])] < est->table_card[std::stoi(q[q.size()-1])];

    if (q.size()==1) {
        if(std::regex_search(q[0], matches, directLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = false;
        } else if(std::regex_search(q[0], matches, inverseLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
        }
        if(!inverse1){
            if(s!="*")
                return selectSource(s,graph->edge_pairs[label1]);
            if(t!="*")
                return selectTarget(t,graph->edge_pairs[label1]);
            return graph->edge_pairs[label1];
        }else{
            std::vector<std::pair<uint32_t,uint32_t>> out;
            for(auto tuple: graph->edge_pairs[label1]){
                out.emplace_back(tuple.second, tuple.first);
            }
            if(s!="*")
                return selectSource(s,out);
            if(t!="*")
                return selectTarget(t,out);
            return out;
        }
    }

    if(t!="*") {
        if(std::regex_search(q[q.size()-1], matches, directLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = false;
        } else if(std::regex_search(q[q.size()-1], matches, inverseLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
        }
        std::vector<std::pair<uint32_t,uint32_t>> leftGraph;
        if(!inverse1) {
            leftGraph = selectTarget(t,graph->edge_pairs[label1]);
        } else{
            std::vector<std::pair<uint32_t,uint32_t>> out;
            for(auto tuple: graph->edge_pairs[label1]){
                out.emplace_back(tuple.second, tuple.first);
            }
            leftGraph = selectSource(t,out);
        }

        for (int i = q.size()-2; i >=0; i--) {

            std::smatch matches2;
            if (std::regex_search(q[i], matches2, directLabel)) {
                label2 = (uint32_t) std::stoul(matches2[1]);
                inverse2 = false;
            } else if (std::regex_search(q[i], matches2, inverseLabel)) {
                label2 = (uint32_t) std::stoul(matches2[1]);
                inverse2 = true;
            } else {
                std::cerr << "Label parsing failed!" << std::endl;
            }

            std::vector<std::pair<uint32_t,uint32_t>> rightGraph;
            if(!inverse2) {
                rightGraph = graph->edge_pairs[label2];
            } else{
                std::vector<std::pair<uint32_t,uint32_t>> out;
                for(auto tuple: graph->edge_pairs[label2]){
                    out.emplace_back(tuple.second, tuple.first);
                }
                rightGraph = out;
            }

            // join left with right
            leftGraph = SimpleEvaluator::join(rightGraph, leftGraph,rightGraph.size(), 0);

        }
        return leftGraph;
    }

    std::vector<std::pair<int,int>> pred_result = findBest(q);
    std::vector<int> order;
    int min_order = pred_result[0].first;
    int max_order = pred_result[0].second;

    order.push_back(pred_result[0].first);
    order.push_back(pred_result[0].second);
    for ( int i = 1; i < pred_result.size(); i++) {
        int first = pred_result[i].first;
        int second = pred_result[i].second;
        if (std::find(order.begin(), order.end(),first)==order.end()) {
            if (first < min_order){
                for ( int m = min_order-1; m >= first; m--)
                    order.push_back(m);
                min_order = first;
            }
            else {
                for ( int m = max_order+1; m <= first; m++)
                    order.push_back(m);
                max_order = first;
            }
        }
        if (std::find(order.begin(), order.end(),second)==order.end()) {
            if (second < min_order) {
                for ( int m = min_order-1; m >= second; m--)
                    order.push_back(m);
                min_order = second;
            }
            else {
                for ( int m = max_order+1; m <= second; m++)
                    order.push_back(m);
                max_order = second;
            }
        }
        
    }

    //for(auto c: order)
    //    std::cout << c << std::endl;
    if(s!="*") {
        if(std::regex_search(q[0], matches, directLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = false;
        } else if(std::regex_search(q[0], matches, inverseLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
        }
        std::vector<std::pair<uint32_t,uint32_t>> leftGraph;
        if(!inverse1) {
            leftGraph = selectSource(s,graph->edge_pairs[label1]);
        } else{
            std::vector<std::pair<uint32_t,uint32_t>> out;
            for(auto tuple: graph->edge_pairs[label1]){
                out.emplace_back(tuple.second, tuple.first);
            }
            leftGraph = selectTarget(s,out);

        }

        if (q.size()==1)
            return leftGraph;
        for (int i = 1; i < q.size(); i++) {

            std::smatch matches2;
            if (std::regex_search(q[i], matches2, directLabel)) {
                label2 = (uint32_t) std::stoul(matches2[1]);
                inverse2 = false;
            } else if (std::regex_search(q[i], matches2, inverseLabel)) {
                label2 = (uint32_t) std::stoul(matches2[1]);
                inverse2 = true;
            } else {
                std::cerr << "Label parsing failed!" << std::endl;
            }

            std::vector<std::pair<uint32_t,uint32_t>> rightGraph;
            if(!inverse2) {
                rightGraph = graph->edge_pairs[label2];
            } else{
                std::vector<std::pair<uint32_t,uint32_t>> out;
                for(auto tuple: graph->edge_pairs[label2]){
                    out.emplace_back(tuple.second, tuple.first);
                }
                rightGraph = out;
            }

            // join left with right
            leftGraph = SimpleEvaluator::join(leftGraph, rightGraph,leftGraph.size(), 0);

        }
        return leftGraph;
    } 

    else {

        std::shared_ptr<SimpleGraph> joinGraph;
        if(std::regex_search(q[order[0]], matches, directLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = false;
        } else if(std::regex_search(q[order[0]], matches, inverseLabel)) {
            label1 = (uint32_t) std::stoul(matches[1]);
            inverse1 = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
        }
        auto currGraph = graph->edge_pairs[label1];
        if (q.size()==1)
            return currGraph;

        for (int i = 1; i < q.size(); i++) {
            std::smatch matches2;
            //std::cout << order[i] << std::endl;
            if (std::regex_search(q[order[i]], matches2, directLabel)) {
                label2 = (uint32_t) std::stoul(matches2[1]);
                inverse2 = false;
            } else if (std::regex_search(q[order[i]], matches2, inverseLabel)) {
                label2 = (uint32_t) std::stoul(matches2[1]);
                inverse2 = true;
            } else {
                std::cerr << "Label parsing failed!" << std::endl;
            }

            std::vector<std::pair<uint32_t,uint32_t>> joinGraph;
            if(!inverse2) {
                joinGraph = graph->edge_pairs[label2];
            } else{
                std::vector<std::pair<uint32_t,uint32_t>> out;
                for(auto tuple: graph->edge_pairs[label2]){
                    out.emplace_back(tuple.second, tuple.first);
                }
                joinGraph = out;
            }

            if(order[i] > order[i-1]) {
                // new graph i.e. joinGraph should be the right graph
                currGraph = SimpleEvaluator::join(currGraph, joinGraph,currGraph.size(), 0);
            }
            if(order[i] < order[i-1]) {
                    currGraph = SimpleEvaluator::join(joinGraph, currGraph,joinGraph.size(), 0);
            }
        }

        return currGraph;
    }

}



cardStat SimpleEvaluator::evaluate(PathQuery *query) {

    auto query_string = pathTreeToString(query->path);
    if (query_string.empty())
        return {0,0,0};


    std::string s = query->s;

    std::string t = query->t;

    auto res = evaluatePath(query_string,s,t);

    //if(query->t != "*") res = selectTarget(query->t, res);
    return SimpleEvaluator::computeStats(res);
}

std::vector<std::string> SimpleEvaluator::pathTreeToString(PathTree *query) {
    if(query->isLeaf()) {
        std::vector<std::string> array;
        array.push_back(query->data);
        return array;
    } else {
        auto left = pathTreeToString(query->left);
        auto right = pathTreeToString(query->right);
        std::vector<std::string> results;
        results.reserve(left.size() + right.size());
        results.insert(results.end(), left.begin(), left.end());
        results.insert(results.end(), right.begin(), right.end());
        return results;
    }
}

SimpleEvaluator::~SimpleEvaluator() {
    
}
