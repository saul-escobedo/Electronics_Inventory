#pragma once

#include <memory>
#include <vector>

#include "electrical/ElectronicComponent.hpp"

namespace ecim {
    struct MassQueryResult {
        std::vector<std::unique_ptr<ElectronicComponent>> items;
        int totalItems;
        int totalPages;
    };
}
