#pragma once

#include "electrical/ElectronicComponent.hpp"

#include <optional>
#include <variant>
#include <vector>

namespace ecim {
    struct Pagination {
        int itemsPerPage = 20;
        int pageNumber = 1; // Pages start at 1

        Pagination() {}
        Pagination(int pageNumber, int itemsPerPage) :
        pageNumber(pageNumber),
        itemsPerPage(itemsPerPage) {}

        bool operator==(const Pagination& other) const;
        bool operator!=(const Pagination& other) const;
        std::size_t hash() const;
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
            Contains, // Strings
            StartsWith, // Strings
            EndsWith, // Strings
            InRange,
            NotInRange
        };

        ComponentProperty property;
        Operation operation;
        std::variant<
            std::string,
            double,
            size_t,
            std::pair<double, double>, // For InRange operations
            std::pair<size_t, size_t>  // For InRange operations
        > value;

        bool operator==(const Filter& other) const;
        bool operator!=(const Filter& other) const;
        bool isValid() const;
        std::size_t hash() const;
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
        /// @note When set, the intent is to only to retrieve statistics; when
        /// aquisition of data is not nessesary.
        bool statisticsOnly = false;

        /// @brief Check if two mass queries have the same configuration
        /// to each other but NOT THIER VALUES. For example, two configs are
        /// not equal to each other if they have different sort orders, but
        /// they are equal if pagination is enabled on both even with different
        /// page numbers and/or page sizes.
        ///
        /// @note This is primarily used to cache a dynamically compiled
        /// prepared DB statements by using the query as the key
        bool operator==(const MassQueryConfig& other) const;
        bool operator!=(const MassQueryConfig& other) const;

        /// @brief Calculate hash of mass query
        ///
        /// @note This is primarily used to cache a dynamically compiled
        /// prepared DB statements by using the query as the key
        std::size_t hash() const;
    };
}

// This templated structure allows MassQueryConfig to be used in a HashMap
template<>
struct std::hash<ecim::MassQueryConfig> {
    std::size_t operator()(const ecim::MassQueryConfig& config) const {
        return config.hash();
    }
};
