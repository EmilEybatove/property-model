#include <functional>
#include <iostream>
#include <tuple>
#include <string>

namespace GetSignatureTest
{

    template <size_t Index>
    struct Data;

    template <size_t Index>
    struct Value;

    template <size_t Index>
    struct Output;

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

    template <size_t Index, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct TypeH<Data<Index>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = GetH<std::tuple<TupleTupes1...>, Index>::type;
    };

    template <size_t Index, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct TypeH<Value<Index>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = GetH<std::tuple<TupleTupes2...>, Index>::type;
    };

    template <size_t Index, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct TypeH<Output<Index>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = GetH<std::tuple<TupleTupes3...>, Index>::type;
    };

    template <typename... A>
    struct Type;

    template <size_t Ind, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct Type<Data<Ind>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = typename TypeH<Data<Ind>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>::type;
    };

    template <typename... A>
    struct FunctionH;

    template <typename T, typename... Ts, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct FunctionH<std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>, T, Ts...>
    {
        using type = std::function<typename Type<T, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>::type(typename Type<Ts, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>::type...)>;
    };

    // template <size_t> class... Rs, size_t... Inds
    template <typename Tuple1, typename Tuple2, typename Tuple3, typename... Ts>
    using Function = typename FunctionH<Tuple1, Tuple2, Tuple3, Ts...>::type;

    using DataTuple = std::tuple<int, double, int>;

    DataTuple cur_data = std::make_tuple(5, 1.65, 56);

    // template <template <size_t> class... Rs, size_t... Inds>
    // template <class... Rs>
    // auto method(Function<DataTuple, Rs...> func)
    // {
    //     return func(5, 1.65);
    // }

    // int func(int a, double b)
    // {
    //     return a * b;
    // }

    // void test()
    // {

    //     std::cout << method<Data<2>, Data<0>, Data<1>>(func);
    // }

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
    // GetSignatureTest::test();
    return 0;
}