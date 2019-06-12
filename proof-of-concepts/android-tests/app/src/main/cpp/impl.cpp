//
// Created by KolhiziN on 12.06.2019.
//
#include <xtensor/xtensor.hpp>
#include <xtensor/xrandom.hpp>

int getRandom()
{
    auto res = xt::random::randint<int>({1});
    return res[0];
}