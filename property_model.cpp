#include <tuple>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>

template <size_t Index>
struct Data;

template <size_t Index>
struct Value;

template <size_t Index>
struct Output;

namespace Getter_
{
    template <class A>
    struct Get;

    template <size_t Ind>
    struct Get<Data<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        auto &operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return std::get<Ind>(data);
        }
    };

    template <size_t Ind>
    struct Get<Value<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        auto &operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return std::get<Ind>(value);
        }
    };

    template <size_t Ind>
    struct Get<Output<Ind>>
    {
        template <typename... DataTypes, typename... ValueTypes, typename... OutputTypes>
        auto &operator()(std::tuple<DataTypes...> &data, std::tuple<ValueTypes...> &value, std::tuple<OutputTypes...> &output)
        {
            return std::get<Ind>(output);
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
        auto &get() const
        {
            return Get<R>()(data, value, output);
        }

    private:
        std::tuple<DataTypes...> &data;
        std::tuple<ValueTypes...> &value;
        std::tuple<OutputTypes...> &output;
    };

}

namespace Binding
{
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
        using type = typename GetH<std::tuple<Ts...>, ind - 1>::type;
    };

    template <class Index, typename... TupleTupes>
    struct TypeH;

    template <size_t Index, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct TypeH<Data<Index>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = typename GetH<std::tuple<TupleTupes1...>, Index>::type;
    };

    template <size_t Index, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct TypeH<Value<Index>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = typename GetH<std::tuple<TupleTupes2...>, Index>::type;
    };

    template <size_t Index, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct TypeH<Output<Index>, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = typename GetH<std::tuple<TupleTupes3...>, Index>::type;
    };

    template <typename... A>
    struct Type;

    template <typename R, typename... TupleTupes1, typename... TupleTupes2, typename... TupleTupes3>
    struct Type<R, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>
    {
        using type = typename TypeH<R, std::tuple<TupleTupes1...>, std::tuple<TupleTupes2...>, std::tuple<TupleTupes3...>>::type;
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

}

template <typename A, typename B, typename C>
class PropertyModelImpl;

template <typename A, typename B, typename C>
class Builder;

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>
{
public:
    using Constraint = std::vector<std::function<void()>>;
    friend class Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;

private:
    std::tuple<DataArgs...> data;
    std::tuple<ValueArgs...> value;
    std::tuple<OutputArgs...> output;
    std::vector<Constraint> constraints;

    PropertyModelImpl(DataArgs... dataArgs, ValueArgs... valueArgs, OutputArgs... outputArgs) : data(std::make_tuple(dataArgs...)), value(std::make_tuple(valueArgs...)), output(std::make_tuple(outputArgs...)), constraints(std::vector<Constraint>()) {};
    PropertyModelImpl() = default;

    void setConstraint(Constraint &c)
    {
        constraints.push_back(std::move(c));
    }
};

template <typename A, typename B, typename C>
class PropertyModel;

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class PropertyModel<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>
{
public:
    using PMImpl = PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;

    PropertyModel(std::unique_ptr<PMImpl> pm) : pm(std::move(pm)) {};

    const PMImpl *operator->() const
    {
        return pm.get();
    }

    PMImpl *operator->()
    {
        return pm.get();
    }

private:
    std::unique_ptr<PMImpl> pm;
};

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>
{
public:
    using DataTuple = std::tuple<DataArgs...>;
    using ValueTuple = std::tuple<ValueArgs...>;
    using OutputTuple = std::tuple<OutputArgs...>;
    using Constraint = std::vector<std::function<void()>>;
    using Getter = Getter_::Getter<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;
    using PMImpl = PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;
    using PM = PropertyModel<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;

    Builder(DataArgs... dataArgs, ValueArgs... valueArgs, OutputArgs... outputArgs) : pm(std::unique_ptr<PMImpl>(new PMImpl(dataArgs..., valueArgs..., outputArgs...))),
                                                                                      getter(pm->data, pm->value, pm->output) {};
    Builder() : pm(std::unique_ptr<PMImpl>(new PMImpl())), getter(pm->data, pm->value, pm->output) {};

    template <class R, class T>
    void set(T &&value)
    {
        getter.template get<R>() = std::move(value);
    }

    void new_constraint()
    {
        pm->setConstraint(current_constr);
        current_constr.clear();
        current_constr.shrink_to_fit();
    }

    template <typename OutVar, typename... InVars>
    void add_method(Binding::Function<DataTuple, ValueTuple, OutputTuple, OutVar, InVars...> func)
    {
        Getter &current_getter = getter;

        auto bind = [this, &current_getter, func]()
        {
            getter.template get<OutVar>() = std::invoke(func, getter.template get<InVars>()...);
        };

        current_constr.push_back(std::function<void()>(bind));
    }

    auto get()
    {
        return std::move(pm);
    }

private:
    // PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>> pm;
    PM pm;

    Getter getter;
    Constraint current_constr;
};

size_t m1(int a, double b)
{
    return a * b;
}

char m2(std::string b)
{
    return b[1];
}

int main()
{
    // Builder<std::tuple<int, double>, std::tuple<std::string, char>, std::tuple<size_t, std::string>> builder(2, 4.6, "pupupu", 'n', 123456, "test");
    Builder<std::tuple<int, double>, std::tuple<std::string, char>, std::tuple<size_t, std::string>> builder;
    builder.set<Data<0>>(2);
    builder.set<Data<1>>(4.6);
    builder.set<Value<0>>("pupupu");
    builder.set<Value<1>>('n');
    builder.set<Output<0>>(123456);
    builder.set<Output<1>>("test");

    builder.add_method<Output<0>, Data<0>, Data<1>>(m1);
    builder.new_constraint();
    auto pm(std::move(builder.get()));

    return 0;
}