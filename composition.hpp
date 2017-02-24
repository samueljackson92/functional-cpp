#include <boost/hana.hpp>
#include <type_traits>

namespace hana = boost::hana;

template<typename F>
auto compose(F f) {
    return [=](auto x) { return f(x); };
}

template<typename F, typename ...Fs>
auto compose(F f, Fs ...fs) {
    return [=](auto x) { return f(compose(fs...)(x)); };
}

////////////////////////////////////////////////////////////////////////////////
// std::optional typeclass implementations
////////////////////////////////////////////////////////////////////////////////

struct stdoptional_tag {};

template <typename... T> struct hana::tag_of<std::optional<T...>> {
  using type = stdoptional_tag;
};

//------------------------------------------------------------------------------
// Functor
//------------------------------------------------------------------------------

template <> struct hana::transform_impl<stdoptional_tag> {
    template <typename F, typename Fs>
    static constexpr auto apply(std::optional<Fs> const &b, F &&f){
        if (b) {
            auto v = f(b.value());
            return std::optional<decltype(f(b.value()))>(v);
        } else {
            return std::optional<decltype(f(b.value()))>();
        }
    }
};

//------------------------------------------------------------------------------
// Applicative
//------------------------------------------------------------------------------

template <> struct hana::lift_impl<stdoptional_tag> {
  template <typename X> static constexpr auto apply(X &&x) {
      return std::optional<X>(std::forward<X>(x));
  }
};

template <> struct hana::ap_impl<stdoptional_tag> {
  template <typename F, typename X>
    static constexpr auto apply(X &&x, F &&fs) {
        return hana::chain(fs, [&](auto f) {
                return hana::transform(x, f);
                });
    }
    /* if (f && x) { */
    /*     auto func = f.value(); */
    /*     auto value = x.value(); */
    /*     return hana::lift<stdoptional_tag>(func(value)); */
    /* } else { */
    /*     return std::optional<decltype(f.value()(x.value()))>(); */
    /* } */
};

//------------------------------------------------------------------------------
// Monad
//------------------------------------------------------------------------------

template <> struct hana::flatten_impl<stdoptional_tag> {
    template <typename X>
    static constexpr auto apply(X &&x)  {
        if(x.has_value()) {
           return x.value();
        } else {
           return std::decay_t<decltype(x.value())>();
        }
    }
};
