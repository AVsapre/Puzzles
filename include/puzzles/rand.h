#pragma once
#ifndef GENERATE_H
#define GENERATE_H
#include <random>
using namespace std;
inline random_device rd;
inline  mt19937 rng(rd());
int rand_int(int max);
#endif 
