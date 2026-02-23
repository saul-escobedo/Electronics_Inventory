#pragma once

#include "electrical/ElectronicComponent.hpp"

#include <optional>
#include <variant>
#include <vector>

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
        Decending,
        Any
    };

    struct Filter {
        enum class Operation {
            Equals,
            NotEquals,
            LessThan,
            GreaterThan,
            LessThanOrEqual,
            GreaterThanOrEqual,
            Contains,
            StartsWith,
            EndsWith,
            InRange
        };

        ComponentProperty property;
        Operation operation;
        std::variant<
            std::string,
            double,
            size_t,
            std::pair<double, double>, // For InRange
            std::pair<size_t, size_t>  // For InRange
        > value;
    };

    /// @brief Mass query settings.
    ///
    /// This struct is used to specify additional settings for mass queries,
    /// such as pagination and sorting, when retrieving components from the database.
    struct MassQueryConfig {
        /// @brief Establish pagination parameters like page size and current
        /// page.
        ///
        /// @note If no pagination setting is set, all found items are returned
        ///  by default.
        std::optional<Pagination> pagination;

        /// @breif Set the result as ascending, descending, any order.
        ///
        /// @note Items may not be sorted if no order is set.
        std::optional<SortOrder> order;

        // @brief Set by which property the items will be sorted by.
        //
        // @note Items is sorted by ID if no property is set.
        std::optional<ComponentProperty> sortBy;

        /// @brief Set a list of filters to apply to items.
        ///
        /// @note No filters are applied by default.
        std::vector<Filter> filters;

        /// @brief If set to true, the result will not return any items.
        ///
        /// @note It is set to only to retrieve statistics; it is when
        /// aquisition of data is not nessesary.
        bool statisticsOnly = false;
    };
}
