#include "UnitTests.cpp"

int main()
{
    betterpresser::tests::BetterPresserTestSuite suite;
    suite.runAllTests();

    for (const auto& r : suite.results)
    {
        if (!r.passed)
            return 1;
    }
    return 0;
}
