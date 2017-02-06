#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch.hpp>
#include <vector>
#include <string>
#include <numeric>
#include <boost/hana.hpp>

#include "composition.hpp"

namespace hana = boost::hana;

auto addThree (const std::vector<int>& v) -> std::vector<int> {
    std::vector<int> w(v.size());
    std::transform(v.begin(), v.end(), w.begin(),
                   [](const auto val) { return val + 3; });
    return w;
}

auto sum(const std::vector<int>& v) -> int {
    return std::accumulate(v.begin(), v.end(), 0);
};

auto toString (const int & value) -> std::string {
    return std::to_string(value);
};

TEST_CASE( "Composition of Functions", "[composition]" ) {

    auto pipeline = compose(toString, sum, addThree);
    std::vector<int> n = { 3, 4, 5 };
    auto x = pipeline(n); 

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

struct stdoptional_tag {};

template <typename... T> struct hana::tag_of<std::optional<T...>> {
  using type = stdoptional_tag;
};

template <> struct hana::transform_impl<stdoptional_tag> {

    template <typename F, typename Fs>
    static constexpr auto apply(std::optional<Fs> const &b, F &&f) {
        if (b) {
            return std::optional<Fs>(f(b.value()));
        } else {
            return b;
        }
    }
};

// Applicative
template <> struct hana::lift_impl<stdoptional_tag> {
  template <typename X> static constexpr auto apply(X &&x) {
      return std::optional<X>(std::forward<X>(x));
  }
};

template <> struct hana::ap_impl<stdoptional_tag> {
  template <typename F, typename X>
  static constexpr decltype(auto) apply(F &&f, X &&x) {
    if (f) {
        return hana::transform(x, f.value());
    } else {
        return x;
    }
  }
};

TEST_CASE( "Functor Functions", "[functors]" ) {

    std::optional<int> value = 20;
    std::optional<int> empty;
    
    auto plus3 = [](auto val) -> decltype(val) {
        return val + 3; 
    };

    REQUIRE( plus3(3) == 6 );
    REQUIRE( hana::Functor<decltype(value)>::value );
    REQUIRE( hana::Applicative<decltype(value)>::value );

    // plus3(value); // compile error!
    // std::optional<int> x = plus3(value);
    auto x = hana::transform(value, plus3);
    REQUIRE ( x == 23 );
    
    std::optional<decltype(plus3)> f_wrapped = plus3;
    auto y = hana::map(value, f_wrapped);
    REQUIRE ( y == 23 );
    
}



