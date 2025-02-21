#include <functional>
#include <iostream>
#include <tuple>
#include <string>

namespace GetSignatureTest
{

    template <size_t Index>
    struct Data;

    template <class List, size_t Index>
    struct GetH;

    template <class T, class... Ts>
    struct GetH<std::tuple<T, Ts...>, 0>
    {
        using type = T;
    };

    template <class T, class... Ts, size_t ind>
    struct GetH<std::tuple<T, Ts...>, ind>
    {
        using type = GetH<std::tuple<Ts...>, ind - 1>::type;
    };

    template <class Index, typename... TupleTupes>
    struct TypeH;

    template <size_t Index, typename... TupleTupes>
    struct TypeH<Data<Index>, std::tuple<TupleTupes...>>
    {
        using type = GetH<std::tuple<TupleTupes...>, Index>::type;
    };

    struct Penis
    {
    };

    template <typename... A>
    struct Type
    {
        using type = Penis;
    };

    // using Type = TypeH<Data<Index>, std::tuple<TupleTupes...>>::type;

    // template <size_t Ind, typename... TupleTupes>
    // struct Type<Data<Ind>, std::tuple<TupleTupes...>>
    // {
    //     using type = TypeH<Data<Ind>, std::tuple<TupleTupes...>>::type;
    // };

    template <typename... A>
    struct FunctionH;

    template <typename T, typename... Ts, typename... TupleTupes>
    struct FunctionH<std::tuple<TupleTupes...>, T, Ts...>
    {
        using type = std::function<Type<T, std::tuple<TupleTupes...>>()>;
    };

    template <template <size_t> typename R, size_t Ind, class... TupleTupes>
    struct FunctionH<std::tuple<TupleTupes...>, R<Ind>>
    {
        // using type = Type<TupleTupes...>::type (*hui)(void);
        // using type = std::function<Type<TupleTupes...>::type()>;

        using tmp = Type<TupleTupes...>::type;
        using type = std::function<tmp()>;
    };

    // template <size_t> class... Rs, size_t... Inds

    // template <typename... A>
    // struct Function;

    // template <typename... Ts, typename... TupleTupes>
    // struct Function<std::tuple<TupleTupes...>, Ts...>
    // {
    //     using f = FunctionH<std::tuple<TupleTupes...>, Ts...>::type;
    // };

}

namespace GetTest
{
    template <size_t Index>
    struct Data
    {
    };

    template <size_t Index>
    struct Value
    {
    };

    class Pukpuk
    {
    public:
        std::tuple<int, double, std::string> data = std::make_tuple(1, 4.5, "lol");
        std::tuple<double, std::string, size_t> value = std::make_tuple(5.4, "pupupu", 123456);
    };

    template <typename... A>
    struct Get;

    template <size_t Ind>
    struct Get<Data<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes>
        auto operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value)
        {
            return std::get<Ind>(data);
        }
    };

    template <size_t Ind>
    struct Get<Value<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes>
        auto operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value)
        {
            return std::get<Ind>(value);
        }
    };

    template <typename A, typename B>
    class Getter;

    template <typename... DataTypes, typename... ValueTypes>
    class Getter<std::tuple<DataTypes...>, std::tuple<ValueTypes...>>
    {
    public:
        Getter(std::tuple<DataTypes...> &dataPtr, std::tuple<ValueTypes...> &valuePtr) : data(dataPtr), value(valuePtr) {};

        template <template <size_t> typename R, size_t Ind>
        auto operator()(R<Ind>)
        {
            return Get<R<Ind>>()(data, value);
        }

    private:
        std::tuple<DataTypes...> &data;
        std::tuple<ValueTypes...> &value;
    };
}

int main()
{

    return 0;
}