#include "database/MassQueryConfig.hpp"

using namespace ecim;

// --------------- Hashing Helpers --------------- //

template <class T>
inline static void s_combineHash(std::size_t& s, const T& v) {
    std::hash<T> hash;
    s ^= hash(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
}

template <class T, class U>
struct std::hash<std::pair<T, U>> {
    size_t operator()(const std::pair<T, U>& pair) const {
        std::size_t hash = 0;

        s_combineHash(hash, pair.first);
        s_combineHash(hash, pair.second);

        return hash;
    }
};

template<>
struct std::hash<Pagination> {
    std::size_t operator()(const Pagination& pagination) {
        return pagination.hash();
    }
};

template<>
struct std::hash<Filter> {
    std::size_t operator()(const Filter& filter) {
        return filter.hash();
    }
};

// --------------- Pagination --------------- //

bool Pagination::operator==(const Pagination& other) const {
    return (itemsPerPage == other.itemsPerPage) &&
        (pageNumber == other.pageNumber);
}

bool Pagination::operator!=(const Pagination& other) const {
    return !(*this == other);
}

std::size_t Pagination::hash() const {
    std::size_t hash = 0;

    s_combineHash(hash, itemsPerPage);
    s_combineHash(hash, pageNumber);

    return hash;
}

// --------------- Filter --------------- //

bool Filter::operator==(const Filter& other) const {
    if(property != other.property)
        return false;

    if(operation != other.operation)
        return false;

    return true;
}

bool Filter::operator!=(const Filter& other) const {
    return !(*this == other);
}

bool Filter::isValid() const {
    if((operation == Operation::Contains ||
        operation == Operation::StartsWith ||
        operation == Operation::EndsWith) && value.index() != 0) {
        printf("[Warning]: Filter is using a string operation but a string is not assigned\n");
        return false;
    }

    if((operation == Operation::InRange ||
        operation == Operation::NotInRange) &&
        (value.index() != 3 && value.index() != 4)) {
        printf("[Warning]: Filter is using an 'InRange' operation but the bounds are not assigned\n");
        return false;
    }

    return true;
}

std::size_t Filter::hash() const {
    std::size_t hash = 0;

    s_combineHash(hash, property);
    s_combineHash(hash, operation);

    return hash;
}

// --------------- MassQueryConfig --------------- //

bool MassQueryConfig::operator==(const MassQueryConfig& other) const {
    if(pagination.has_value() != other.pagination.has_value())
        return false;

    if(order != other.order)
        return false;

    if(sortBy != other.sortBy)
        return false;

    if(filters.size() != other.filters.size())
        return false;

    int i = 0;
    for(const Filter& filter : filters)
        if(filter != other.filters[i++])
            return false;

    if(statisticsOnly != other.statisticsOnly)
        return false;

    return true;
}

bool MassQueryConfig::operator!=(const MassQueryConfig& other) const {
    return !(*this == other);
}

std::size_t MassQueryConfig::hash() const {
    std::size_t hash = 0;

    s_combineHash(hash, pagination.has_value());
    s_combineHash(hash, order);
    s_combineHash(hash, sortBy);

    for(const Filter& filter : filters)
        s_combineHash(hash, filter);

    s_combineHash(hash, statisticsOnly);

    return hash;
}
