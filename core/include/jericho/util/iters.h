#ifndef JERICHO_UTIL_ITERS_H_
#define JERICHO_UTIL_ITERS_H_

#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <iostream>
#include <bits/stdc++.h>

template<typename T>
bool contains(std::vector<T> vec, T el) {
    for (auto v : vec) {
        if (v == el) return true;
    }
    return false;
}

// template<typename T>

template<typename K, typename V>
bool containsKey(std::unordered_map<K, V> map, K el) {
    for (auto m : map) {
        if (m.first == el) return true;
    }
    return false;
}

template<typename K, typename V>
bool containsVal(std::unordered_map<K, V> map, V el) {
    for (auto m : map) {
        if (m.second == el) return true;
    }
    return false;
}

template<typename K, typename V>
bool containsItem(std::unordered_map<K, V> map, K el, V el2) {
    for (auto m : map) {
        if (m.first == el && m.second == el2) return true;
    }
    return false;
}

template<typename T>
void print(std::vector<T> vec) {
    for (auto v : vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template<typename T>
void print(std::set<T> vec) {
    for (auto v : vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template<typename K, typename V>
std::set<K> keys(std::map<K, V> map) {
    std::set<K> result = {};
    for (auto kv : map) {
        result.insert(kv.first);
    }
    return result;
}

template<typename K, typename V>
std::set<V> values(std::map<K, V> map) {
    std::set<V> result = {};
    for (auto kv : map) {
        result.insert(kv.second);
    }
    return result;
}

template<typename K, typename V>
std::set<K> keys(std::unordered_map<K, V> map) {
    std::set<K> result = {};
    for (auto kv : map) {
        result.insert(kv.first);
    }
    return result;
}

template<typename K, typename V>
std::set<V> values(std::unordered_map<K, V> map) {
    std::set<V> result = {};
    for (auto kv : map) {
        result.insert(kv.second);
    }
    return result;
}

template<typename T>
bool subset(std::set<T> set_one, std::set<T> set_two) {
    return (std::includes(set_two.begin(), set_two.end(),
            set_one.begin(), set_one.end()));
}

#endif