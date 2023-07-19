#include <iostream>
#include <sstream>
#include <toml++/toml.h>
using namespace std::string_view_literals;

// Example program shows how to use toml++ to:
// 1. Read and combine sub-sections distributed across toml files
// 2. Override previously set key-value pairs

// Recursively merges two tables
void mergeTables(toml::table& dest, const toml::table& src) {
    for (const auto& [key, value] : src) {
        if (value.is_table()) {
            if (!dest.contains(key)) {
                dest.insert_or_assign(key, toml::table{});
            }
            mergeTables(*dest[key].as_table(), *value.as_table());
        } else {
            dest.insert_or_assign(key, value);
        }
    }
}

int main()
{
    static constexpr std::string_view some_toml = R"(
        test_domain=1

        [library.first_entry]
        name = "toml++"
        authors = ["Mark Gillard <mark.gillard@outlook.com.au>"]
        cpp = 17
    )"sv;

    static constexpr std::string_view another_toml = R"(
        test_domain=2            # override previously set value

        [library.second_entry]   # extend previously defined table
        name = "vilas++"
        authors = ["Vilas Chitrakaran <cvilas@gmail.com>"]
        cpp = 17
    )"sv;

    try
    {
        toml::table combined_tbl;
        toml::table tbl1 = toml::parse(some_toml);
        toml::table tbl2 = toml::parse(another_toml);
        mergeTables(combined_tbl, tbl1);
        mergeTables(combined_tbl, tbl2);
        std::cout << combined_tbl << "\n";
    }
    catch (const toml::parse_error& err)
    {
        std::cerr << "Parsing failed:\n" << err << "\n";
        return 1;
    }

    return 0;
}
