#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H


#include <memory>
#include <cmath>
#include "SimpleGraph.h"
#include "PathTree.h"
#include "Evaluator.h"
#include "Graph.h"

class SimpleEvaluator : public Evaluator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;
    std::vector<std::pair<std::string, std::string>> optimal_order;

public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator();

    void prepare() override ;
    cardStat evaluate(PathQuery *query) override ;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::vector<std::pair<uint32_t,uint32_t>> evaluatePath(std::vector<std::string> q, std::string s, std::string t);
    static std::shared_ptr<SimpleGraph> project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g, std::string s, std::string t);
    static std::vector<std::pair<uint32_t,uint32_t>> join( std::vector<std::pair<uint32_t,uint32_t>> &left,  std::vector<std::pair<uint32_t,uint32_t>> &right, uint32_t left_size, uint32_t join_type);
    std::vector<std::pair<int,int>> findBest(std::vector<std::string> q);
    static cardStat computeStats(std::vector<std::pair<uint32_t,uint32_t>> &g);

    std::vector<std::string> pathTreeToString(PathTree *q);

};


#endif //QS_SIMPLEEVALUATOR_H
