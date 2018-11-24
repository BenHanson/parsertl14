parsertl
========

parsertl: The Modular Parser Generator

As well as normal parsing, it is now possible to use the library as a more powerful regex.

### Match a string with captures

```cpp
#include "parsertl/generator.hpp"
#include <iostream>
#include "parsertl/match.hpp"

int main()
{
    try
    {
        parsertl::rules grules(parsertl::enable_captures);
        parsertl::state_machine gsm;
        lexertl::rules lrules;
        lexertl::state_machine lsm;

        grules.token("Int Name");
        grules.push("list", "(Name) "
            "| list ',' (Int) "
            "| list ',' (Name)");
        parsertl::generator::build(grules, gsm);
        lrules.push("[A-Z_a-z]\\w*", grules.token_id("Name"));
        lrules.push("\\d+", grules.token_id("Int"));
        lrules.push(",", grules.token_id("','"));
        lrules.push("\\s+", lrules.skip());
        lexertl::generator::build(lrules, lsm);

        std::string input = "One, 2, Three, Four";
        typedef std::vector<std::pair<const char *, const char *> > capture_vector;
        std::vector<capture_vector> captures;

        if (parsertl::match(input.c_str(), input.c_str() + input.size(), captures, lsm, gsm))
        {
            std::vector<capture_vector>::const_iterator cvi = captures.begin();
            std::vector<capture_vector>::const_iterator cve = captures.end();

            for (; cvi != cve ; ++cvi)
            {
                capture_vector::const_iterator vi = cvi->begin();
                capture_vector::const_iterator ve = cvi->end();

                for (; vi != ve; ++vi)
                {
                    std::cout << std::string(vi->first, vi->second) << '\n';
                }
            }
        }
        else
        {
            std::cout << "No match\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }

    return 0;
}
```
This outputs:
```cpp
One, 2, Three, Four
One
2
Three
Four
```

To search a string with captures, switch `match()` for `search()` above:
```cpp
parsertl::search(input.c_str(), input.c_str() + input.size(), captures, lsm, gsm)
```
The `captures` argument can be omitted if it is not required in both cases.

You can use an iterator instead of calling search:

```cpp
    parsertl::citerator iter(input.c_str(), input.c_str() + input.size(), lsm, gsm);
    parsertl::citerator end;

    for (; iter != end; ++iter)
    {
        for (const auto &vec : *iter)
        {
            for (const auto &pair : vec)
            {
                std::cout << std::string(pair.first, pair.second) << '\n';
            }
        }

        std::cout << '\n';
    }
```

## More examples and documentation

For documentation see http://www.benhanson.net/parsertl.html
