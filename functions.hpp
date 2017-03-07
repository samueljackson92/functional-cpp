#include <boost/hana.hpp>
#include <type_traits>

namespace hana = boost::hana;

//------------------------------------------------------------------------------
// Generic Functions for testing
//------------------------------------------------------------------------------

auto addThree (std::vector<int> v) -> std::vector<int> {
    std::transform(v.begin(), v.end(), v.begin(),
                   [](const auto val) { return val + 3; });
    return v;
}

auto timeTwo (std::vector<int> v) -> std::vector<int> {
    std::transform(v.begin(), v.end(), v.begin(),
                   [](const auto val) { return val * 2; });
    return v;
}

auto allToString (std::vector<int> v) -> std::vector<std::string> {
    std::vector<std::string> s;
    std::transform(v.begin(), v.end(), std::back_inserter(s),
                   [](auto i) { return std::to_string(i); });
    return s;
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
