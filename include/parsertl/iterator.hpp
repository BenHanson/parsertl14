// iterator.hpp
// Copyright (c) 2022 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PARSERTL_ITERATOR_HPP
#define PARSERTL_ITERATOR_HPP

#include "lookup.hpp"
#include "match_results.hpp"
#include "token.hpp"

namespace parsertl
{
    template<typename iter, typename lsm_type, typename gsm_type,
        typename id_type = std::uint16_t>
        class iterator
    {
    public:
        using results = basic_match_results<gsm_type>;
        using value_type = results;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        using lex_iterator = lexertl::iterator<iter, lsm_type,
            lexertl::match_results<iter>>;
        // Qualify token to prevent arg dependant lookup
        using token = parsertl::token<lex_iterator>;
        using token_vector = typename token::token_vector;

        iterator() :
            _gsm(nullptr)
        {
        }

        iterator(const iter& first_, const iter& second_,
            const lsm_type& lsm_, const gsm_type& gsm_) :
            _iter(first_, second_, lsm_),
            _results(_iter->id, gsm_),
            _gsm(&gsm_)
        {
            lookup();
        }

        typename token_vector::value_type dollar(const std::size_t index_) const
        {
            return _results.dollar(*_gsm, index_, _productions);
        }

        iterator& operator ++()
        {
            lookup();
            return *this;
        }

        iterator operator ++(int)
        {
            iterator iter_ = *this;

            lookup();
            return iter_;
        }

        const value_type& operator *() const
        {
            return _results;
        }

        const value_type* operator ->() const
        {
            return &_results;
        }

        bool operator ==(const iterator& rhs_) const
        {
            return _gsm == rhs_._gsm &&
                (_gsm == nullptr ? true :
                    _results == rhs_._results);
        }

        bool operator !=(const iterator& rhs_) const
        {
            return !(*this == rhs_);
        }

    private:
        lex_iterator _iter;
        basic_match_results<gsm_type> _results;
        token_vector _productions;
        const gsm_type* _gsm;

        void lookup()
        {
            do
            {
                parsertl::lookup(*_gsm, _iter, _results, _productions);
            } while (_results.entry.action == action::shift ||
                _results.entry.action == action::go_to);

            switch (_results.entry.action)
            {
            case action::accept:
            case action::error:
                _gsm = nullptr;
                break;
            }
        }
    };

    using siterator = iterator<std::string::const_iterator,
        lexertl::state_machine, state_machine>;
    using citerator = iterator<const char*, lexertl::state_machine,
        state_machine>;
    using wsiterator = iterator<std::wstring::const_iterator,
        lexertl::wstate_machine, state_machine>;
    using wciterator = iterator<const wchar_t*, lexertl::wstate_machine,
        state_machine>;
}

#endif
