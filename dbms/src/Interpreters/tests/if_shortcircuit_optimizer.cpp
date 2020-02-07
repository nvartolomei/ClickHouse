#include <Parsers/ASTSelectQuery.h>
#include <Parsers/ParserSelectQuery.h>
#include <Parsers/parseQuery.h>
#include <Parsers/queryToString.h>
#include <Parsers/ASTFunction.h>
#include <Core/Settings.h>
#include <Common/typeid_cast.h>

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <algorithm>


namespace
{

struct TestEntry
{
    std::string input;
    std::string expected_output;
    UInt64 limit;
};

using TestEntries = std::vector<TestEntry>;
using TestResult = std::pair<bool, std::string>;

void run();
void performTests(const TestEntries & entries);
TestResult check(const TestEntry & entry);
bool parse(DB::ASTPtr & ast, const std::string & query);
bool equals(const DB::ASTPtr & lhs, const DB::ASTPtr & rhs);
void reorder(DB::IAST * ast);


void run()
{
    /// NOTE: Queries are not always realistic, but we are only interested in the syntax.
    TestEntries entries =
    {
        {
            "SELECT 1",
            "SELECT 1",
            3
        },
        {
            "SELECT count() FROM constCondOptimization WHERE if(1 = 0, 1, n = 1000) AND time between now() - 300 AND now();",
            "SELECT count() FROM constCondOptimization WHERE n = 1000 AND time between now() - 300 AND now();",
            4
        },
    };

    performTests(entries);
}

void performTests(const TestEntries & entries)
{
    unsigned int count = 0;
    unsigned int i = 1;

    for (const auto & entry : entries)
    {
        auto res = check(entry);
        if (res.first)
        {
            ++count;
            std::cout << "Test " << i << " passed.\n";
        }
        else
            std::cout << "Test " << i << " failed. Expected: " << entry.expected_output << ". Received: " << res.second << "\n";

        ++i;
    }
    std::cout << count << " out of " << entries.size() << " test(s) passed.\n";
}

TestResult check(const TestEntry & entry)
{
    try
    {
        /// Parse and optimize the incoming query.
        DB::ASTPtr ast_input;
        if (!parse(ast_input, entry.input))
            return TestResult(false, "parse error");

        auto select_query = typeid_cast<DB::ASTSelectQuery *>(&*ast_input);
        UNUSED(select_query);

        auto where_expr = select_query->where();
        if (where_expr)
        {
            for (const auto & child : where_expr->children)
            {
                if (auto el = child->as<DB::ASTExpressionList>())
                {
                    for (const auto & child2 : el->children)
                    {
                        if (auto f = child2->as<DB::ASTFunction>())
                        {
                            if (f->name == "if")
                            {
                                child2->dumpTree(std::cerr);
                            }
                        }
                    }
                }
            }
        }

        /// Parse the expected result.
        DB::ASTPtr ast_expected;
        if (!parse(ast_expected, entry.expected_output))
            return TestResult(false, "parse error");

        /// Compare the optimized query and the expected result.
        bool res = equals(ast_input, ast_expected);
        std::string output = DB::queryToString(ast_input);

        return TestResult(res, output);
    }
    catch (DB::Exception & e)
    {
        return TestResult(false, e.displayText());
    }
}

bool parse(DB::ASTPtr & ast, const std::string & query)
{
    DB::ParserSelectQuery parser;
    std::string message;
    auto begin = query.data();
    auto end = begin + query.size();
    ast = DB::tryParseQuery(parser, begin, end, message, false, "", false, 0);
    return ast != nullptr;
}

bool equals(const DB::ASTPtr & lhs, const DB::ASTPtr & rhs)
{
    DB::ASTPtr lhs_reordered = lhs->clone();
    reorder(&*lhs_reordered);

    DB::ASTPtr rhs_reordered = rhs->clone();
    reorder(&*rhs_reordered);

    return lhs_reordered->getTreeHash() == rhs_reordered->getTreeHash();
}

void reorderImpl(DB::IAST * ast)
{
    if (ast == nullptr)
        return;

    auto & children = ast->children;
    if (children.empty())
        return;

    for (auto & child : children)
        reorderImpl(&*child);

    std::sort(children.begin(), children.end(), [](const DB::ASTPtr & lhs, const DB::ASTPtr & rhs)
    {
        return lhs->getTreeHash() < rhs->getTreeHash();
    });
}

void reorder(DB::IAST * ast)
{
    if (ast == nullptr)
        return;

    auto select_query = typeid_cast<DB::ASTSelectQuery *>(ast);
    if (select_query == nullptr)
        return;

    reorderImpl(select_query->where().get());
    reorderImpl(select_query->prewhere().get());
    reorderImpl(select_query->having().get());
}

}

int main()
{
    run();
    return 0;
}
