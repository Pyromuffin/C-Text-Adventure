//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

typedef unsigned int uint;
typedef uint IndexType;
typedef IndexType ReferentHandle;
typedef uint Hash;

bool IsWhiteSpace(char c);
void TrimSelf(char* input);



template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }