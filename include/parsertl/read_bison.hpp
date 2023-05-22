// read_bison.hpp
// Copyright (c) 2014-2023 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PARSERTL_READ_BISON_HPP
#define PARSERTL_READ_BISON_HPP

#include "generator.hpp"
#include "lookup.hpp"
#include "match_results.hpp"
#include "token.hpp"

namespace parsertl
{
    template<typename char_type, typename rules_type>
    void read_bison(const char_type* start_, const char_type* end_,
        rules_type& rules_)
    {
        using bison_lrules = lexertl::basic_rules<char, char_type>;
        using bison_lsm = lexertl::basic_state_machine<char_type>;
        using bison_crmatch =
            lexertl::recursive_match_results<const char_type*>;
        using bison_criterator =
            lexertl::iterator<const char_type*, bison_lsm, bison_crmatch>;
        using bison_lgenerator =
            lexertl::basic_generator<bison_lrules, bison_lsm>;
        using string = std::basic_string<char_type>;

        rules grules_;
        state_machine gsm_;
        bison_lrules lrules_;
        bison_lsm lsm_;

        grules_.token("LITERAL NAME");
        grules_.push("start", "list");
        grules_.push("list", "directives '%%' rules '%%'");
        grules_.push("directives", "%empty "
            "| directives directive");

        grules_.push("directive", "'%code' "
            "| '%define' "
            "| '%expect' "
            "| '%verbose' "
            "| '%initial-action'");
        const std::size_t token_index_ =
            grules_.push("directive", "'%token' tokens '\n'");
        const std::size_t left_index_ =
            grules_.push("directive", "'%left' tokens '\n'");
        const std::size_t right_index_ =
            grules_.push("directive", "'%right' tokens '\n'");
        const std::size_t nonassoc_index_ = grules_.push("directive",
            "'%nonassoc' tokens '\n'");
        const std::size_t precedence_index_ =
            grules_.push("directive", "'%precedence' tokens '\n'");
        const std::size_t start_index_ =
            grules_.push("directive", "'%start' NAME '\n'");

        grules_.push("directive", "'\n'");
        grules_.push("tokens", "tokens name "
            "| name");
        grules_.push("name", "LITERAL | NAME");
        grules_.push("rules", "rules rule "
            "| rule");

        const std::size_t prod_index_ =
            grules_.push("rule", "NAME ':' productions ';'");

        // Meh
        grules_.push("rule", "';'");
        grules_.push("productions", "productions '|' production prec "
            "| production prec");
        grules_.push("production", "'%empty' | prod_list");
        grules_.push("prod_list", "token "
            "| prod_list token");
        grules_.push("token", "LITERAL | NAME");
        grules_.push("prec", "%empty | '%prec' NAME");

        std::string warnings_;

        generator::build(grules_, gsm_, &warnings_);

        lrules_.push_state("CODE");
        lrules_.push_state("FINISH");
        lrules_.push_state("PRODUCTIONS");
        lrules_.push_state("PREC");

        lrules_.push("%code[^{]*", grules_.token_id("'%code'"));
        lrules_.push("%define.*", grules_.token_id("'%define'"));
        lrules_.push("%expect.*", grules_.token_id("'%expect'"));
        lrules_.push("%verbose", grules_.token_id("'%verbose'"));
        lrules_.push("%initial-action[^{]*",
            grules_.token_id("'%initial-action'"));
        lrules_.push("%left", grules_.token_id("'%left'"));
        lrules_.push("\n", grules_.token_id("'\n'"));
        lrules_.push("%nonassoc", grules_.token_id("'%nonassoc'"));
        lrules_.push("%precedence", grules_.token_id("'%precedence'"));
        lrules_.push("%right", grules_.token_id("'%right'"));
        lrules_.push("%start", grules_.token_id("'%start'"));
        lrules_.push("%token", grules_.token_id("'%token'"));
        lrules_.push("%union[^{]*[{](.|\n)*?[}]", lrules_.skip());
        lrules_.push("<[^>]+>", lrules_.skip());
        lrules_.push("%[{](.|\n)*?%[}]", lrules_.skip());
        lrules_.push("[ \t\r]+", lrules_.skip());

        lrules_.push("INITIAL,CODE,PRODUCTIONS", "[{]", ">CODE");
        lrules_.push("CODE", R"('(\\.|[^'])*')", ".");

        lrules_.push("CODE", R"(\"(\\.|[^"])*\")", ".");
        lrules_.push("CODE", "<%", ">CODE");
        lrules_.push("CODE", "%>", "<");
        lrules_.push("CODE", "[^}]", ".");
        lrules_.push("CODE", "[}]", lrules_.skip(), "<");

        lrules_.push("INITIAL", "%%", grules_.token_id("'%%'"), "PRODUCTIONS");
        lrules_.push("PRODUCTIONS", ":", grules_.token_id("':'"), ".");
        lrules_.push("PRODUCTIONS", ";", grules_.token_id("';'"), ".");
        lrules_.push("PRODUCTIONS", "[|]", grules_.token_id("'|'"),
            "PRODUCTIONS");
        lrules_.push("PRODUCTIONS", "%empty",
            grules_.token_id("'%empty'"), ".");
        lrules_.push("INITIAL,PRODUCTIONS",
            R"('(\\([^0-9cx]|[0-9]{1,3}|c[@a-zA-Z]|x\d+)|[^'])+'|)"
            R"(\"(\\([^0-9cx]|[0-9]{1,3}|c[@a-zA-Z]|x\d+)|[^"])+\")",
            grules_.token_id("LITERAL"), ".");
        lrules_.push("INITIAL,PRODUCTIONS",
            "[A-Za-z_.][-A-Za-z_.0-9]*", grules_.token_id("NAME"), ".");
        lrules_.push("PRODUCTIONS", "%%", grules_.token_id("'%%'"), "FINISH");
        lrules_.push("PRODUCTIONS", "%prec",
            grules_.token_id("'%prec'"), "PREC");
        lrules_.push("PREC", "[A-Za-z_.][-A-Za-z_.0-9]*",
            grules_.token_id("NAME"), "PRODUCTIONS");
        // Always skip comments
        lrules_.push("CODE,INITIAL,PREC,PRODUCTIONS",
            "[/][*](.|\n|\r\n)*?[*][/]|[/][/].*", lrules_.skip(), ".");
        // All whitespace in PRODUCTIONS mode is skipped.
        lrules_.push("PREC,PRODUCTIONS", "\\s+", lrules_.skip(), ".");
        lrules_.push("FINISH", "(.|\n)+", lrules_.skip(), "INITIAL");

        bison_lgenerator::build(lrules_, lsm_);

        bison_criterator iter_(start_, end_, lsm_);
        using token = token<bison_criterator>;
        typename token::token_vector productions_;
        match_results results_(iter_->id, gsm_);

        while (results_.entry.action != action::error &&
            results_.entry.action != action::accept)
        {
            if (results_.entry.action == action::reduce)
            {
                if (results_.entry.param == token_index_)
                {
                    const token& token_ =
                        results_.dollar(1, gsm_, productions_);
                    const string str_(token_.first, token_.second);

                    rules_.token(str_.c_str());
                }
                else if (results_.entry.param == left_index_)
                {
                    const token& token_ =
                        results_.dollar(1, gsm_, productions_);
                    const string str_(token_.first, token_.second);

                    rules_.left(str_.c_str());
                }
                else if (results_.entry.param == right_index_)
                {
                    const token& token_ =
                        results_.dollar(1, gsm_, productions_);
                    const string str_(token_.first, token_.second);

                    rules_.right(str_.c_str());
                }
                else if (results_.entry.param == nonassoc_index_)
                {
                    const token& token_ =
                        results_.dollar(1, gsm_, productions_);
                    const string str_(token_.first, token_.second);

                    rules_.nonassoc(str_.c_str());
                }
                else if (results_.entry.param == precedence_index_)
                {
                    const token& token_ =
                        results_.dollar(1, gsm_, productions_);
                    const string str_(token_.first, token_.second);

                    rules_.precedence(str_.c_str());
                }
                else if (results_.entry.param == start_index_)
                {
                    const token& name_ =
                        results_.dollar(1, gsm_, productions_);

                    rules_.start(string(name_.first,
                        name_.second).c_str());
                }
                else if (results_.entry.param == prod_index_)
                {
                    const token& lhs_ = results_.dollar(0, gsm_, productions_);
                    const token& rhs_ = results_.dollar(2, gsm_, productions_);
                    const string lhs_str_(lhs_.first, lhs_.second);
                    const string rhs_str_(rhs_.first, rhs_.second);

                    rules_.push(lhs_str_.c_str(), rhs_str_.c_str());
                }
            }

            lookup(iter_, gsm_, results_, productions_);
        }

        if (results_.entry.action == action::error)
        {
            std::ostringstream ss_;
            string token_ = iter_->str();

            ss_ << "Syntax error on line " <<
                std::count(start_, iter_->first, '\n') + 1 <<
                ": '";
            narrow(token_.c_str(), ss_);
            ss_ << '\'';
            throw runtime_error(ss_.str());
        }
    }
}

#endif
