// dfa.hpp
// Copyright (c) 2014-2023 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PARSERTL_DFA_HPP
#define PARSERTL_DFA_HPP

#include <deque>
#include <vector>

namespace parsertl
{
    struct cursor
    {
        std::size_t _id;
        std::size_t _index;

        cursor() :
            _id(0),
            _index(0)
        {
        }

        cursor(const std::size_t id_, const std::size_t index_) :
            _id(id_),
            _index(index_)
        {
        }

        bool operator==(const cursor& rhs_) const
        {
            return _id == rhs_._id && _index == rhs_._index;
        }

        bool operator<(const cursor& rhs_) const
        {
            return _id < rhs_._id ||
                _id == rhs_._id && _index < rhs_._index;
        }
    };

    using cursor_vector = std::vector<cursor>;

    struct dfa_state
    {
        cursor_vector _basis;
        cursor_vector _closure;
        cursor_vector _transitions;
    };

    // Must be deque due to iterator usage in basic_generator::build_dfa().
    using dfa = std::deque<dfa_state>;
}

#endif
