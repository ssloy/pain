#undef NDEBUG
#include <cassert>
#include <cmath>
#include <array>
#include <limits>
#include <ranges>


// Weak expansion system with exact arithmetic + lazy normalization.
// The expansion must satisfy:
// 1) exact representation: sum e_i = exact value
// 2) ordering: |e_i| <= |e_{i+1}|

template<int N> struct Expansion {
    std::array<double, N> e = {};
    int n = 0;

    auto begin() const noexcept { return e.begin();     }
    auto end()   const noexcept { return e.begin() + n; }
    auto begin()       noexcept { return e.begin();     }
    auto end()         noexcept { return e.begin() + n; }

    constexpr double operator[](int i) const noexcept {
        assert(i>=0 && i<n);
        return e[i];
    }

    constexpr double& operator[](int i) noexcept {
        assert(i>=0 && i<n);
        return e[i];
    }

    constexpr void push_back(double x) noexcept {
        assert(n < N-1);
        e[n++] = x;
    }

    void normalize();

    void validate() {
        static_assert(std::numeric_limits<double>::radix == 2,   "Only base-2 floating point supported"   );
        static_assert(std::numeric_limits<double>::digits == 53, "Only IEEE-754 binary64 supported"       );
        static_assert(std::numeric_limits<double>::is_iec559,    "IEEE-754 required for robust predicates");
#ifndef NDEBUG
        for (double v : *this)
            assert(std::isfinite(v));
        for (int i = 0; i < n-1; ++i)
            assert(std::abs(e[i]) <= std::abs(e[i+1]));
#endif
    }
};

inline Expansion<2> sum(double a, double b) {
    double hi = a + b;
    double bb = hi - a;
    double aa = hi - bb;
    double lo = (a - aa) + (b - bb);
    return {{lo, hi}, 2};
}

template<int N> Expansion<N+1> sum(const Expansion<N>& e, double b) {
    Expansion<N+1> out;
    double q = b;
    for (double v : e) {
        Expansion<2> t = sum(v, q);
        out.push_back(t[0]);
        q = t[1];
    }
    out.push_back(q);
    return out;
}

template<int A, int B> Expansion<A + B> sum(const Expansion<A>& e, const Expansion<B>& f) {
    Expansion<A + B> out;
    double q = 0;

    auto add = [&](double x) {
        Expansion<2> t = sum(q, x);
        if (t[0]) out.push_back(t[0]);
        q = t[1];
    };

    int i = 0, j = 0;
    while (i < e.n && j < f.n) {
        if (std::abs(e[i]) < std::abs(f[j]))
            add(e[i++]);
        else
            add(f[j++]);
    }
    while (i < e.n) add(e[i++]);
    while (j < f.n) add(f[j++]);

    if (q) out.push_back(q);
    return out;
}

template <int N> void Expansion<N>::normalize() {
    Expansion<N> tmp;
    double q = 0;

    for (double &v : *this | std::views::reverse) {
        Expansion<2> t = sum(q, v);
        if (t[0]) tmp.push_back(t[0]);
        q = t[1];
    }

    n = 0;
    for (double &v : tmp | std::views::reverse) {
        Expansion<2> t = sum(q, v);
        if (t[0]) push_back(t[0]);
        q = t[1];
    }

    if (q) push_back(q);
}


template<int N> Expansion<N> negate(const Expansion<N>& e) {
    Expansion<N> r = e;
    for (double &v : r)
        v = -v;
    return r;
}

inline Expansion<2> diff(double a, double b) {
    return sum(a, -b);
}

template<int N> Expansion<N+1> diff(const Expansion<N>& e, double b) {
    return sum(e, -b);
}

template<int A, int B> Expansion<A + B> diff(const Expansion<A>& e, const Expansion<B>& f) {
    return sum(e, negate(f));
}

inline Expansion<2> product(double a, double b) {
    double hi = a * b;
    double lo = std::fma(a, b, -hi);
    return {{lo, hi}, 2};
}

