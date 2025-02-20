#include <tuple>
#include <vector>
#include <functional>
#include <iostream>

template <typename... Args>
struct Data
{
    std::tuple<Args...> values;
};

template <typename... Args>
struct Value
{
    std::tuple<Args...> values;

    Value(Args... args) : values(args...) {};
};

template <typename... Args>
struct Output
{
    std::tuple<Args...> values;

    Output(Args... args) : values(args...) {};
};

enum Variable
{
    data,
    value,
    output
};

template <typename A, typename B, typename C>
class PropertyModel;

template <typename A, typename B, typename C>
class Builder;

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class PropertyModel<Data<DataArgs...>, Value<ValueArgs...>, Output<OutputArgs...>>
{
public:
    using Constraint = std::vector<std::function<void()>>;
    friend class Builder<DataArgs..., ValueArgs..., OutputArgs...>;

private:
    Data<DataArgs...> data;
    Value<ValueArgs...> value;
    Output<OutputArgs...> result;
    std::vector<Constraint> constraints;

    constexpr PropertyModel(DataArgs... dataArgs, ValueArgs... valueArgs, OutputArgs... outputArgs) : data(std::make_tuple(dataArgs...)), value(std::make_tuple(valueArgs...)), result(std::make_tuple(outputArgs...)), constraints(std::vector<Constraint>()) {};

    void setConstraint(Constraint &&c)
    {
        constraints.push_back(std::move(c));
    }
};

struct FuncArg
{
    int64_t index;
    Variable var;
};

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class Builder<Data<DataArgs...>, Value<ValueArgs...>, Output<OutputArgs...>>
{
public:
    using Constraint = std::vector<std::function<void()>>;

    constexpr Builder(DataArgs... dataArgs, ValueArgs... valueArgs, OutputArgs... outputArgs) : pm(dataArgs..., valueArgs..., outputArgs...) {};

    void new_constraint()
    {
        current_constr.clear();
        current_constr.shrink_to_fit();
    }

    template <typename A, typename B>
    void add_method(std::function<A(B)>, std::vector<FuncArg>, std::vector<FuncArg>);

    template <typename... outTypes, typename... inTypes>
    void add_method(std::function<outTypes...(inTypes)> func, std::vector<FuncArg> inputArgs, std::vector<FuncArg> outputArgs)
    {
        std::tuple<inTypes> input;

        for (int64_t i = 0; i < inputArgs.size(); ++i)
        {
            std::get<i>
        }

        auto lambda[]
        {

            std::vector<FuncArg> out = func()
        }
    }

private:
    PropertyModel<DataArgs..., ValueArgs..., OutputArgs...> pm;
    Constraint current_constr;
};

int main()
{
    std::tuple<int, double> a = std::make_tuple<int, double>(1, 1.5);

    int n;
    std::cin >> n;
    const int m = n;
    return 0;
}