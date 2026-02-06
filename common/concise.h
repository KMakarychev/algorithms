#ifndef ALG_CONCISE_H
#define ALG_CONCISE_H

#include <algorithm>
#include <vector>
#include <cstddef>

namespace alg {

///////////////////////////////////////////////////////////////////////////////
// Several convenient functions for sorting data.
// Starting with C++20, you can use std::ranges::sort
// instead of these functions.
///////////////////////////////////////////////////////////////////////////////

// Sort data in increasing order
template<class T>
void sort(T& data)
{
    std::sort(data.begin(), data.end());
}

// Sort data using a custom comparator
template<class T, class Compare>
void sort(T& data, Compare cmp)
{
    std::sort(data.begin(), data.end(), cmp);
}

///////////////////////////////////////////////////////////////////////////////
// Helper class order_by for sorting data by a field.
//
// Examples:
//   sort(jobs, order_by(&Job::finish));
//   sort(jobs, order_by(&Job::start).ascending());
//   sort(jobs, order_by(&Job::start).descending());
//
// Note: small runtime overhead due to indirect access.
///////////////////////////////////////////////////////////////////////////////

template<typename MemberPtr>
class order_by
{
public:
    explicit order_by(MemberPtr field)
        : field_(field), ascending_(true) {}

    order_by& ascending()
    {
        ascending_ = true;
        return *this;
    }

    order_by& descending()
    {
        ascending_ = false;
        return *this;
    }

    template<class T>
    bool operator()(const T& a, const T& b) const
    {
        return ascending_
            ? (a.*field_ < b.*field_)
            : (a.*field_ > b.*field_);
    }

private:
    MemberPtr field_;
    bool ascending_;
};

///////////////////////////////////////////////////////////////////////////////
// sort_by: compile-time field-based sorting
//
// Example:
//   alg::sort_by<&Job::start>(jobs);
//   alg::sort_by<&Job::finish>(jobs, false);
///////////////////////////////////////////////////////////////////////////////

template<auto Field, class Container>
void sort_by(Container& data, bool ascending = true)
{
    auto cmp = [&](const auto& a, const auto& b) {
        return ascending
            ? (a.*Field < b.*Field)
            : (a.*Field > b.*Field);
    };
    std::sort(data.begin(), data.end(), cmp);
}

template<auto Field, class Iter>
void sort_by(Iter begin, Iter end, bool ascending = true)
{
    auto cmp = [&](const auto& a, const auto& b) {
        return ascending
            ? (a.*Field < b.*Field)
            : (a.*Field > b.*Field);
    };
    std::sort(begin, end, cmp);
}

///////////////////////////////////////////////////////////////////////////////
// Helper class order_by_expr for sorting by an expression.
//
// Examples:
//   sort(jobs, order_by_expr([](const Job& j){ return j.finish; }));
//   sort(jobs, order_by_expr([](const Job& j){ return j.finish - j.start; }));
///////////////////////////////////////////////////////////////////////////////

template<typename Expr>
class order_by_expr
{
public:
    explicit order_by_expr(Expr expr)
        : expr_(expr), ascending_(true) {}

    order_by_expr& ascending()
    {
        ascending_ = true;
        return *this;
    }

    order_by_expr& descending()
    {
        ascending_ = false;
        return *this;
    }

    template<class T>
    bool operator()(const T& a, const T& b) const
    {
        return ascending_
            ? (expr_(a) < expr_(b))
            : (expr_(a) > expr_(b));
    }

private:
    Expr expr_;
    bool ascending_;
};

///////////////////////////////////////////////////////////////////////////////
// create_table: multidimensional table creation (useful for DP)
//
// Example:
//   auto matrix = create_table(5, 10, -1); // 5x10 matrix filled with -1
///////////////////////////////////////////////////////////////////////////////

template<typename T>
auto create_table(std::size_t size, const T& value)
{
    return std::vector<T>(size, value);
}

template<typename... Args>
auto create_table(std::size_t size, Args&&... args)
{
    auto slice = create_table(std::forward<Args>(args)...);
    return std::vector<decltype(slice)>(size, slice);
}

} // namespace alg

#endif // ALG_CONCISE_H
