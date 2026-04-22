#include "database/MassQueryConfig.hpp"
#include <stdexcept>

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

template<>
struct std::hash<FilterNode> {
    std::size_t operator()(const FilterNode& node) {
        return node.hash();
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

// --------------- FilterNode --------------- //

FilterNode::FilterNode(std::vector<FilterNode> c, Type t) {
    if(!c.size())
        throw std::invalid_argument("To construct a FilterNode from a list of other nodes, it must have at least one node");

    if(c.size() == 1 && t == Type::Not) {
        type = t;
        children = std::move(c);
        return;
    } else if(c.size() == 1)
        throw std::invalid_argument("If there is only one FilterNode in the list, you must set the type to NOT");

    if(t != Type::And && t != Type::Or)
        throw std::invalid_argument("To a construct a FilterNode from a list of node, the type must be equal to And or Or");

    type = t;
    children = std::move(c);
}

FilterNode::FilterNode(std::vector<Filter> filters, Type t) {
    if(!filters.size())
        throw std::invalid_argument("To construct a FilterNode from a list of filters, it must have at least one filter");

    if(filters.size() == 1 && t == Type::Not) {
        type = t;
        children.push_back(FilterNode(filters[0]));
        return;
    } else if(filters.size() == 1) {
        type = Type::Filter;
        filter = std::move(filters[0]);
        return;
    }

    if(t != Type::And && t != Type::Or)
        throw std::invalid_argument("To a construct a FilterNode from a list of filters, the type must be equal to And or Or");

    type = t;

    children.reserve(filters.size());
    for(auto& filter : filters)
        children.push_back(FilterNode(filter));
}

static void s_checkIfFilterNodeIsValidOrThrow(const FilterNode& node) {
    if(node.type == FilterNode::Type::Filter && !node.filter.has_value())
        throw std::invalid_argument("Filter node is type Filter, but no filter is assigned");

    if((node.type == FilterNode::Type::And || node.type == FilterNode::Type::Or) && node.children.size() < 2)
        throw std::invalid_argument("Filter node is AND or OR type, but there is not enough children");

    if(node.type == FilterNode::Type::Not && node.children.size() != 1)
        throw std::invalid_argument("Filter node is a NOT type, but it does not have exactly 1 child");
}

bool FilterNode::operator==(const FilterNode& other) const {
    // Throw if not valid, or else infinite recursion might occur
    s_checkIfFilterNodeIsValidOrThrow(*this);

    if(type != other.type)
        return false;

    if(type == Type::Filter)
        return filter.value() == other.filter.value();

    if(children.size() != other.children.size())
        return false;

    for(int i = 0; i < children.size(); i++)
        if(children[i] != other.children[i])
            return false;

    return true;
}

bool FilterNode::operator!=(const FilterNode& other) const {
    return !(*this == other);
}

bool FilterNode::isValid() const {
    if(type == Type::Filter && !filter.has_value())
        return false;

    if((type == Type::And || type == Type::Or) && children.size() < 2)
        return false;

    if(type == Type::Not && children.size() != 1)
        return false;

    return true;
}

void FilterNode::throwIfNotValid() const {
    s_checkIfFilterNodeIsValidOrThrow(*this);
}

std::size_t FilterNode::hash() const {
    std::size_t hash = 0;

    // Throw if not valid, or else infinite recursion might occur
    s_checkIfFilterNodeIsValidOrThrow(*this);

    if(type == Type::Filter)
        return filter.value().hash();

    s_combineHash(hash, type);

    for(auto& child : children)
        s_combineHash(hash, child.hash());

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

    if(filters != other.filters)
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

    s_combineHash(hash, filters);

    s_combineHash(hash, statisticsOnly);

    return hash;
}
