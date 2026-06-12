#include <catch2/catch_test_macros.hpp>

#include <gmpxx.h>
#include <random>
#include <cmath>

#include "pain.h"

static std::mt19937_64 rng(1337);

double rand_uniform() {
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    return dist(rng);
}

double rand_log_uniform() {
    std::uniform_int_distribution<int> expd(-300, 300);
    std::uniform_real_distribution<double> mant(0.5, 1.0);

    double s = (rng() & 1) ? 1 : -1;
    return s * std::ldexp(mant(rng), expd(rng));
}

std::pair<double,double> rand_cancelling_pair() {
    double x = rand_log_uniform();
    int ulps = std::uniform_int_distribution<int>(0, 16)(rng);
    double y = std::nextafter(x, INFINITY);
    for (int i=ulps; i--;)
        y = std::nextafter(y, INFINITY);
    return {x, -y};
}

double rand_subnormal() {
    uint64_t bits =
        std::uniform_int_distribution<uint64_t>(
            1,
            (1ull<<52)-1
        )(rng);
    return std::bit_cast<double>(bits);
}

double rand_stress_double() {
    int mode = std::uniform_int_distribution<int>(0,99)(rng);

    if(mode < 50)
        return rand_log_uniform();

    if(mode < 80)
        return rand_uniform();

    if(mode < 95)
        return rand_subnormal();

    return std::numeric_limits<double>::epsilon();
}

template<int N> Expansion<N> random_expansion() {
    Expansion<N> e;
    for(int i=0;i<N;i++)
        e.push_back(rand_log_uniform());
    std::sort(e.begin(), e.end(), [](double a,double b) { return std::abs(a) < std::abs(b); });
    return e;
}

TEST_CASE("sum(double,double)") {
    for (int i = 0; i < 10000; i++) {
        double a = rand_stress_double();
        double b = rand_stress_double();
        mpq_class R = mpq_class(a) + mpq_class(b);
        Expansion<2> e = sum(a, b);
        e.validate();
        mpq_class S = 0;
        for (double v : e)
            S += mpq_class(v);
        REQUIRE( R == S );
    }
}

TEST_CASE("sum(expansion,expansion)") {
    for(int t=0;t<50000;t++) {
        auto a = random_expansion<8>();
        auto b = random_expansion<8>();

        mpq_class R = 0;

        for(double v : a) R += mpq_class(v);
        for(double v : b) R += mpq_class(v);

        auto c = sum(a,b);
        mpq_class S = 0;
        for(double v : c)
            S += mpq_class(v);

        REQUIRE(R == S);

        c.validate();
    }
}

