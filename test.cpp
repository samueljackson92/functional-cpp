#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch.hpp>
#include <vector>
#include <string>
#include <numeric>
#include <functional>
#include "composition.hpp"

//------------------------------------------------------------------------------
// Generic Functions for testing
//------------------------------------------------------------------------------

auto addThree (std::vector<int>& v) -> std::vector<int> {
    std::transform(v.begin(), v.end(), v.begin(),
                   [](const auto val) { return val + 3; });
    return v;
}

auto sum(const std::vector<int>& v) -> int {
    return std::accumulate(v.begin(), v.end(), 0);
};

auto toString (const int & value) -> std::string {
    return std::to_string(value);
};

auto plus3(int val) -> int { return val + 3; }

auto almostInteger(const double value, const double tol = 1e-1) -> bool {
    const double intVal = std::round(value);
    return std::abs(intVal - value) < tol;
}

//------------------------------------------------------------------------------
// HKL Functions for testing
//------------------------------------------------------------------------------

class IntegerHKL {
    public:
        IntegerHKL(int h, int k, int l) : m_hkl({h, k, l}) {}
        auto H() const -> int { return m_hkl[0]; }
        auto K() const -> int { return m_hkl[1]; }
        auto L() const -> int { return m_hkl[2]; }
    private:
        std::array<int, 3> m_hkl;
};

class ProtoHKL {
    public:
        ProtoHKL(double h, double k, double l) : m_hkl({h, k, l}) {}
        auto H() const -> double { return m_hkl[0]; }
        auto K() const -> double { return m_hkl[1]; }
        auto L() const -> double { return m_hkl[2]; }
    private:
        std::array<double, 3> m_hkl;
};

auto createProtoHKL(const std::vector<double>& hkl) -> decltype(auto) {
    if (hkl.size() != 3)
        return std::optional<ProtoHKL>();
    
    return std::make_optional(ProtoHKL(hkl[0], hkl[1], hkl[2]));
}

auto convertProtoToInteger(const ProtoHKL& proto) -> decltype(auto) {
    // check it's not all zero
    if (proto.H() == 0 && proto.K() == 0 && proto.L() == 0)
        return std::optional<IntegerHKL>();

    // check the value are close to zero
    if (almostInteger(proto.H()) && almostInteger(proto.K()) && almostInteger(proto.L())) {
        const int h = std::round(proto.H());
        const int k = std::round(proto.K());
        const int l = std::round(proto.L());
        return std::make_optional<IntegerHKL>(h, k, l);
    }

    return std::optional<IntegerHKL>();
}


auto cubicDSpacing(const double a, const IntegerHKL& hkl) {
    const auto div = std::pow(hkl.H(), 2) + std::pow(hkl.K(), 2) + std::pow(hkl.L(), 2);
    return a / std::sqrt(div);
}

auto identity(const IntegerHKL& hkl) {
    return hkl;
}


//------------------------------------------------------------------------------
// Test Cases
//------------------------------------------------------------------------------

TEST_CASE( "Composition of Functions", "[composition]" ) {

    auto pipeline = compose(toString, sum, addThree);
    std::vector<int> n = { 3, 4, 5 };
    auto x = pipeline(std::move(n)); 

    REQUIRE( x == "21" );
}

TEST_CASE( "Currying Functions", "[currying]" ) {
    auto add = hana::curry<2>([](int a, int b) {
        return a + b;
    });

    auto addFive = add(5);
    REQUIRE( addFive(5) == 10 );
    REQUIRE( addFive(3) == 8 );
}


TEST_CASE( "Functor Functions", "[functors]" ) {
    std::optional<int> value = 20;
    std::optional<int> empty;
    
    REQUIRE( plus3(5) == 8 );
    REQUIRE( hana::Functor<decltype(value)>::value );

    // plus3(value); // compile error!
    auto x = hana::transform(value, plus3);
    REQUIRE ( x == 23 );

    x = hana::transform(empty, plus3);
    REQUIRE ( !x.has_value() );
}

TEST_CASE( "Applicative Functions", "[applicatives]" ) {
    std::optional<int> value = 20;
    std::optional<int> empty;

    auto optionalPlus3 = std::make_optional(plus3);
    std::optional<std::function<int(int)>> emptyFunc;

    REQUIRE( hana::Applicative<decltype(optionalPlus3)>::value );
    REQUIRE( hana::Applicative<decltype(emptyFunc)>::value );

    auto x = hana::ap(value, optionalPlus3);
    REQUIRE ( x == 23 );
    x = hana::ap(empty, optionalPlus3);
    REQUIRE ( !x );

    x = hana::ap(value, emptyFunc);
    REQUIRE ( 20 );
    x = hana::ap(empty, emptyFunc);
    REQUIRE ( !x );
}

TEST_CASE( "Monadic Functions", "[HKL]" ) {
    std::vector<double> vec1 = { 1.02, 1.05, 1.01 };
    std::vector<double> vec2 = { 1.5, 1.5, 1.5 };
    std::vector<double> vec3 = { 0, 0, 0 };
    std::vector<double> empty;

    // curry a function prior to composition
    auto cubicDSpacingCurried = hana::curry<2>(cubicDSpacing);
    auto pyriteDSpacing = cubicDSpacingCurried(5.47);

    auto p1 = hana::monadic_compose(convertProtoToInteger, createProtoHKL);
    auto pipeline = hana::monadic_compose(pyriteDSpacing, p1);

    REQUIRE( pipeline(std::move(vec1))  == Approx(3.15).epsilon(0.01) );
    REQUIRE( !pipeline(std::move(vec2)));    // fails to convert to IntegerHKL
    REQUIRE( !pipeline(std::move(vec3)) );   // fails to convert to IntegerHKL
    REQUIRE( !pipeline(std::move(empty)) );  // fails to convert to ProtoHKL
}
