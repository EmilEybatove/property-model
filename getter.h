#include <tuple>
#include <functional>
#include "utils.h"

namespace Getter
{

    template <class A>
    struct GetPtr;

    template <size_t Ind>
    struct GetPtr<Data<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        auto &operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return std::get<Ind>(data);
        }
    };

    template <size_t Ind>
    struct GetPtr<Value<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        auto &operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return std::get<Ind>(value);
        }
    };

    template <size_t Ind>
    struct GetPtr<Output<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        auto &operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return std::get<Ind>(output);
        }
    };

    template <class A>
    struct GetIndex;

    template <size_t Ind>
    struct GetIndex<Data<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        size_t operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return Ind;
        }
    };

    template <size_t Ind>
    struct GetIndex<Value<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        size_t operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return Ind + std::tuple_size<std::tuple<DataTypes...>>{};
        }
    };

    template <size_t Ind>
    struct GetIndex<Output<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        size_t operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return Ind + std::tuple_size<std::tuple<DataTypes...>>{} + std::tuple_size<std::tuple<ValueTypes...>>{};
        }
    };

    template <typename A, typename B, typename C>
    class Getter;

    template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
    class Getter<std::tuple<DataTypes...>, std::tuple<ValueTypes...>, std::tuple<OutputTypes...>>
    {
    public:
        Getter(std::tuple<DataTypes...> &dataPtr, std::tuple<ValueTypes...> &valuePtr, std::tuple<OutputTypes...> &outPtr) : data(dataPtr), value(valuePtr), output(outPtr) {};

        template <class R>
        auto &getPtr() const
        {
            return GetPtr<R>()(data, value, output);
        }

        template <class R>
        size_t getIndex() const
        {
            return GetIndex<R>()(data, value, output);
        }

    private:
        std::tuple<DataTypes...> &data;
        std::tuple<ValueTypes...> &value;
        std::tuple<OutputTypes...> &output;
    };

}
