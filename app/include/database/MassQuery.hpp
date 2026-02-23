#pragma once

#include <optional>

namespace ecim {
    struct Pagination {
        int itemsPerPage = 20;
        int pageNumber = 1; // Pages start at 1

        Pagination(int pageNumber, int itemsPerPage) :
        pageNumber(pageNumber),
        itemsPerPage(itemsPerPage) {}
    };

    enum class SortOrder {
        Acending,
        Decending
    };

    struct MassQuery {
        /// @brief Establish pagination parameters like page size and current
        /// page.
        std::optional<Pagination> pagination;

        /// @breif Set the result as ascending or descending
        std::optional<SortOrder> order;

        /// @brief If set to true, the result will not return any items.
        ///
        /// @note It is typcially used only to retrieve statistics, and
        /// aquisition of data is not nessesary.
        bool statisticsOnly = false;
    };
}
