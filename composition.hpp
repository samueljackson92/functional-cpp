
template<typename F>
auto compose(F f) {
    return [=](auto x) { return f(x); };
}

template<typename F, typename ...Fs>
auto compose(F f, Fs ...fs) {
    return [=](auto x) { return f(compose(fs...)(x)); };
}
