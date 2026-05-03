#pragma once

#include "electrical/ElectronicComponent.hpp"

#include <optional>
#include <variant>
#include <vector>

namespace ecim {
    struct Pagination {
        int pageNumber = 1; // Pages start at 1
        int itemsPerPage = 20;

        Pagination() {}
        Pagination(int pageNumber, int itemsPerPage) :
        pageNumber(pageNumber),
        itemsPerPage(itemsPerPage) {
            if(pageNumber <= 0)
                throw std::invalid_argument("Page number cannot be equal or less than 0");

            if(itemsPerPage <= 0)
                throw std::invalid_argument("Items per page cannot be equal or less than 0");
        }

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

    struct FilterNode {
        enum class Type {
            Filter,
            And,
            Or,
            Not
        };

        Type type;

        // Valid if type == Filter
        std::optional<Filter> filter;

        // Valid if type == And/Or: children.size() >= 2
        // Valid if type == Not: children.size() == 1
        std::vector<FilterNode> children;

        // Convenience Constructors
        FilterNode(Filter f) : type(Type::Filter), filter(std::move(f)) {}
        FilterNode(std::vector<FilterNode> c, Type t = Type::And);
        FilterNode(std::vector<Filter> filters, Type t = Type::And);

        bool operator==(const FilterNode& other) const;
        bool operator!=(const FilterNode& other) const;
        bool isValid() const;
        void throwIfNotValid() const;
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
        /// by default.
        std::optional<Pagination> pagination;

        /// @brief Set the result as ascending, descending, any order.
        ///
        /// @note Items may not be sorted if no order is set.
        std::optional<SortOrder> order;

        // @brief Set by which property the items will be sorted by.
        //
        // @note Items is sorted by ID if no property is set.
        std::optional<ComponentProperty> sortBy;

        /// @brief A filter tree to narrow down a search.
        ///
        /// A filter tree is configured like a boolean equation that allows for
        /// more comprehensive searches. Instead of just having a series of
        /// filters narrowing down the results by ANDing each condition, it can
        /// be configured as, for example, (A || B) && C. This allows us to not
        /// only narrow down a search, but also be inclusive of other
        /// conditions.
        ///
        /// For instance, we want to find resistors from manufacturer 'RE'
        /// that have resistances of 220 or 330 ohms. You'd do:
        /// (Resistance == 220 || Resistance == 330) && Manufacturer == 'RE'
        /// So the tree would be structered as:
        ///           AND
        ///         /      \
        ///        OR        \
        ///      /    \        \
        ///    /        \        \
        /// r = 220   r = 330  m = 'RE'
        ///
        /// @note No filters are applied by default.
        std::optional<FilterNode> filters;

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
