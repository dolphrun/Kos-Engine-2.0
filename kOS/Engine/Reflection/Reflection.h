/********************************************************************/
/*!
\file      Reflection.h
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Sep 11, 2025
\brief     This header file provides macros for compile-time reflection, enabling
           operations on class members without explicit enumeration.
           - REFLECTABLE: Macro to enable reflection on specified class members.
           - CLASSTOSTRING: Macro to retrieve the class name as a string.
           - FOR_EACH: Macro to apply another macro to each argument in a list.
           - COUNT_ARGS: Counts the number of arguments provided to a macro.
           - ApplyFunction: Template function that applies a provided function to
             each member of a class using reflection.

This file allows users to perform compile-time reflection in C++, enabling
dynamic access to class members for serialization, inspection, or function
application without needing runtime type information.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#ifndef REFLECT_H
#define REFLECT_H

#include <array>
#include <string>
#include <tuple>
#include <functional>
#include <type_traits>
#include <vector>

//David Mazières implementation of for each
/************************************************************************/
#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

// The FOR_EACH macro recursively applies a macro to each argument in __VA_ARGS__
#define FOR_EACH(macro, ...)                                    \
  __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                         \
  macro(a1)                                                     \
  __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

/************************************************************************/
 
// Count the number of arguments (this helps with reflection of members)
#define COUNT_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, count, ...) count
#define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_ARGS__,40,39,38,37,36,35,34,33,32, 31, 30, 29, 28 ,27, 26, 25, 24, 23, 22, 21, 20, 19, 18 ,17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Simple stringification for each member
#define TOSTRING(ARG) #ARG,  // Removed the extra comma to simplify usage

// Creates class to string
#define CLASSTOSTRING(CLASS) \
    inline static constexpr const char* classname() { \
        return #CLASS; \
    }

#define REFLECTABLE(CLASSNAME, ...) \
    CLASSTOSTRING(CLASSNAME) \
    inline auto membercount() const { \
        return COUNT_ARGS(__VA_ARGS__); \
    } \
    inline auto member()  { \
        return std::tie(__VA_ARGS__); \
    } \
    inline auto member() const { \
        return std::tie(__VA_ARGS__); \
    } \
    inline static const std::array<std::string, COUNT_ARGS(__VA_ARGS__)> Names() { \
        return {FOR_EACH(TOSTRING, __VA_ARGS__)}; \
    } \
    inline static const std::vector<std::string> NamesV() { \
        return {FOR_EACH(TOSTRING, __VA_ARGS__)}; \
    } \
    template <typename T> \
    void ApplyFunction(T&& function) { \
        std::tuple members = member(); \
        std::apply([&function](auto&... args) { ((function(args)), ...); }, members); \
    }\
    template <typename T, typename OtherType> \
    void ApplyFunctionPairwise(T&& function, OtherType& other) { \
        auto membersA = member(); \
        auto membersB = other.member(); \
        constexpr std::size_t N = std::tuple_size<decltype(membersA)>::value; \
        ApplyFunctionPairwiseImpl(std::forward<T>(function), membersA, membersB, std::make_index_sequence<N>{}); \
    } \
    template <typename T, typename TupleA, typename TupleB, std::size_t... I> \
    static void ApplyFunctionPairwiseImpl(T&& function, TupleA& membersA, TupleB& membersB, std::index_sequence<I...>) { \
        ((function(std::get<I>(membersA), std::get<I>(membersB))), ...); \
    }



#endif