// iterator.hpp
// Copyright (c) 2018 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PARSERTL_SEARCH_ITERATOR_HPP
#define PARSERTL_SEARCH_ITERATOR_HPP

#include "match_results.hpp"
#include "search.hpp"

namespace parsertl
{
    template<typename iter, typename lsm_type, typename gsm_type,
        typename id_type = std::uint16_t>
        class search_iterator
    {
    public:
        using results = std::vector<std::vector<std::pair<iter, iter>>>;
        using value_type = results;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        search_iterator() :
            _end(),
            _lsm(nullptr),
            _gsm(nullptr)
        {
        }

        search_iterator(const iter& first_, const iter& second_, const lsm_type& lsm_,
            const gsm_type& gsm_) :
            _end(second_),
            _lsm(&lsm_),
            _gsm(&gsm_)
        {
            _captures.push_back(std::vector<std::pair<iter, iter>>());
            _captures.back().push_back(std::make_pair(first_, first_));
            lookup();
        }

        search_iterator& operator ++()
        {
            lookup();
            return *this;
        }

        search_iterator operator ++(int)
        {
            search_iterator iter_ = *this;

            lookup();
            return iter_;
        }

        const value_type& operator *() const
        {
            return _captures;
        }

        const value_type* operator ->() const
        {
            return &_captures;
        }

        bool operator ==(const search_iterator& rhs_) const
        {
            return _lsm == rhs_._lsm && _gsm == rhs_._gsm &&
                (_gsm == nullptr ? true :
                    _captures == rhs_._captures);
        }

        bool operator !=(const search_iterator& rhs_) const
        {
            return !(*this == rhs_);
        }

    private:
        iter _end;
        results _captures;
        const lsm_type* _lsm;
        const gsm_type* _gsm;

        void lookup()
        {
            const auto pair_ = _captures[0].back();

            _captures.clear();

            if (!search(pair_.second, _end, _captures, *_lsm, *_gsm))
            {
                _lsm = nullptr;
                _gsm = nullptr;
            }
        }
    };

    using ssearch_iterator = search_iterator<std::string::const_iterator,
        lexertl::state_machine, state_machine>;
    using csearch_iterator = search_iterator<const char*, lexertl::state_machine,
        state_machine>;
    using wssearch_iterator = search_iterator<std::wstring::const_iterator,
        lexertl::state_machine, state_machine>;
    using wcsearch_iterator = search_iterator<const wchar_t*, lexertl::state_machine,
        state_machine>;
}

#endif
