#include <cstdio>
#include <tuple>
#include <functional>
#include "utils.h"

namespace Library
{

    template <class A>
    struct GetIndex;

    template <size_t Ind>
    struct GetIndex<Data<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        static size_t get(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return Ind;
        }
    };

    template <size_t Ind>
    struct GetIndex<Value<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        static size_t get(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return Ind + std::tuple_size<std::tuple<DataTypes...>>{};
        }
    };

    template <size_t Ind>
    struct GetIndex<Output<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        static size_t get(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return Ind + std::tuple_size<std::tuple<DataTypes...>>{} + std::tuple_size<std::tuple<ValueTypes...>>{};
        }
    };
}
