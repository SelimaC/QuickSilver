#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleGraph.h"

struct table_calc {
    float prev_table_card;
    float prev_col_from;
    float prev_col_to;
} ;


class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;


public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator();

    uint32_t *table_card;
    std::pair<uint32_t,uint32_t> *column_card;

    void prepare() override ;
    double cardest(PathTree *p);
    double colcarfrom(PathTree *p);
    double colcarto(PathTree *p);
    cardStat estimate(PathQuery *q);
    cardStat estimate_vec(std::vector<std::string> labels_path);
    struct table_calc recur(PathTree *q, uint32_t first_nodeid);

};

#endif //QS_SIMPLEESTIMATOR_H
