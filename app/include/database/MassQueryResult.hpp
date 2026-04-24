#pragma once

#include <memory>
#include <vector>

#include "electrical/ElectronicComponent.hpp"

namespace ecim {
    /// @brief The MassQueryResult struct represents the results of a mass
    /// query executed on the database.
    ///
    /// Each instance contains a list of electronic components matching the
    /// criteria specified by the query, as well as metadata such as the total
    /// number of items returned, and the total number of pages that could be
    /// obtained if pagination is enabled.
    struct MassQueryResult {
        std::vector<std::unique_ptr<ElectronicComponent>> items;
        size_t numItems;
        size_t numPages;
        size_t totalNumItems;
        size_t currentPage;
    };
}
