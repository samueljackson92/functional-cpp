#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch.hpp>
#include <vector>
#include <string>
#include <numeric>
#include <functional>
#include <range/v3/all.hpp>
#include "functions.hpp"

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

TEST_CASE( "Higher Order", "[High]") {
    auto applyThrice = [](auto f, auto x) {
        return f(f(f(x)));
    };

    auto value = applyThrice([](auto x) { return x*3; }, 3);
    REQUIRE( value == 81 );

}

TEST_CASE( "Composition of Functions", "[composition]" ) {

    auto pipeline = hana::compose(toString, sum, addThree);
    std::vector<int> n = { 3, 4, 5 };
    auto x = pipeline(std::move(n)); 

    REQUIRE( x == "21" );
}

TEST_CASE( "Slow Composition", "[Lazy]" ) {
    std::vector<int> n = { 3, 4, 5 };
    auto f = hana::compose(allToString, timeTwo, addThree);
    auto xs = f(n);
    REQUIRE( xs[0] == "12" );
    REQUIRE( xs[1] == "14" );
    REQUIRE( xs[2] == "16" );
}

TEST_CASE( "Lazy Composition", "[Lazy]" ) {
    std::vector<int> n = { 3, 4, 5 };

    auto xs = n 
        | ranges::view::transform([](auto x) { return x+3; })
        | ranges::view::transform([](auto x) { return x*2; })
        | ranges::view::transform([](auto x) { return std::to_string(x); });

    // roughly equivilent to
    // for (auto & val : values) {
    // h(g(f(x)));
    // }

    REQUIRE( xs[0] == "12" );
    REQUIRE( xs[1] == "14" );
    REQUIRE( xs[2] == "16" );
}

TEST_CASE( "Error handling", "[Error]") {
    
    auto foo = [](int x) {
        if ( x < 5 ) {
            return std::make_optional(x);
        } else {
            return std::optional<int>();
        }
    };

    REQUIRE( foo(3).value() == 3 );
    REQUIRE( !foo(5) );
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

TEST_CASE( "Simple Monad", "[Monad]") {
    auto foo = [](auto x) {
        if ( x < 5 ) {
            return std::make_optional(x*2);
        } else {
            return std::optional<decltype(x)>();
        }
    };

    auto bar = hana::monadic_compose(foo, foo);
    REQUIRE( bar(1).value() == 4 );
    REQUIRE( bar(2).value() == 8 );
    REQUIRE( !bar(3) );
}

TEST_CASE( "Monadic Functions", "[HKL]" ) {
    std::vector<double> vec1 = { 1.02, 1.05, 1.01 };
    std::vector<double> vec2 = { 1.5, 1.5, 1.5 };
    std::vector<double> vec3 = { 0, 0, 0 };
    std::vector<double> empty;

    auto curryTransform = hana::curry<2>([](auto f, auto v) {
      return hana::transform(v, f);
    });

    // curry a function prior to composition
    auto cubicDSpacingCurried = hana::curry<2>(cubicDSpacing);
    auto pyriteDSpacing = cubicDSpacingCurried(5.47);

    // swap args and curry so we can compose
    // monadically compose the two functions together
    auto p1 = hana::monadic_compose(convertProtoToInteger, createProtoHKL);
    // compose non-monadic function with pipeline
    auto pipeline = hana::compose(curryTransform(pyriteDSpacing), p1);
    auto x = pipeline(vec1);

    REQUIRE( x.has_value() );
    REQUIRE( x.value() == Approx(3.15).epsilon(0.01) );

    REQUIRE( !pipeline(vec2));   // fails to convert to IntegerHKL
    REQUIRE( !pipeline(vec3));   // fails to convert to IntegerHKL
    REQUIRE( !pipeline(empty));  // fails to convert to ProtoHKL
}


