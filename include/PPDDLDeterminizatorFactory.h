//
// Created by gcanal on 21/03/18.
//

#ifndef PPDDL_PARSER_PPDDLDETERMINIZATORFACTORY_H
#define PPDDL_PARSER_PPDDLDETERMINIZATORFACTORY_H

#include "Strategies/MLODeterminizator.h"
#include "Strategies/AODeterminizator.h"
#include "Strategies/TLDeterminizator.h"

class PPDDLDeterminizatorFactory {
public:
    enum Strategies {
        ALL_OUTCOME,
        MOST_LIKELY_OUTCOME,
        TRANSITION_LIKELIHOOD
    };
    static std::unique_ptr<PPDDLDeterminizator> createStrategy(Strategies strat, const std::vector<double>& params = std::vector<double>());
};


#endif //PPDDL_PARSER_PPDDLDETERMINIZATORFACTORY_H
