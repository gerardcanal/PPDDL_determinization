//
// Created by gcanal on 21/03/18.
//

#include "PPDDLDeterminizatorFactory.h"

std::unique_ptr<PPDDLDeterminizator>
PPDDLDeterminizatorFactory::createStrategy(PPDDLDeterminizatorFactory::Strategies strat,
                                           const std::vector<double> &params) {

    if (strat == MOST_LIKELY_OUTCOME) return std::unique_ptr<PPDDLDeterminizator>(new MLODeterminizator());
    if (strat == ALL_OUTCOME) return std::unique_ptr<PPDDLDeterminizator>(new AODeterminizator());
    if (strat == TRANSITION_LIKELIHOOD) {
        if (params.size() < 2) {
            std::cerr << "Error: Not enough parameters specified for the transition-likelihood strategy" << std::endl;
            return nullptr;
        }
        return std::unique_ptr<PPDDLDeterminizator>(new TLDeterminizator(params[0], params[1]));
    }
    std::cerr << "Error: Undefined Determinization Strategy " << strat << std::endl;
    return nullptr;
}
