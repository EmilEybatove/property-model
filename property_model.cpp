#include <tuple>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>
#include "getter.h"
#include "binding.h"

struct Method
{
    std::function<void()> func;
    std::vector<size_t> inputs;
    size_t output;
};

struct Constraint
{
    std::vector<Method> methods;
    size_t priority;
};

template <typename A, typename B, typename C>
class PropertyModelImpl;

template <typename A, typename B, typename C>
class Builder;

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>
{
public:
    friend class Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;
    using PMImpl = PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;
    friend std::unique_ptr<PMImpl> std::make_unique<PMImpl>();

private:
    PropertyModelImpl() = default;
    PropertyModelImpl(DataArgs... dataArgs, ValueArgs... valueArgs, OutputArgs... outputArgs) : data_(std::make_tuple(dataArgs...)), value_(std::make_tuple(valueArgs...)), output_(std::make_tuple(outputArgs...)), constraints_(std::vector<Constraint>()) {}

    void setConstraint(Constraint &&c)
    {
        constraints_.push_back(std::move(c));
    }

    std::tuple<DataArgs...> data_;
    std::tuple<ValueArgs...> value_;
    std::tuple<OutputArgs...> output_;
    std::vector<Constraint> constraints_;
};

template <typename A, typename B, typename C>
class PropertyModel;

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class PropertyModel<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>
{
public:
    friend class Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;
    using PMImpl = PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>;

    const PMImpl *operator->() const
    {
        return pm_.get();
    }

    PMImpl *operator->()
    {
        return pm_.get();
    }

private:
    std::unique_ptr<PMImpl> pm_;

    explicit PropertyModel(std::unique_ptr<PMImpl> pm) : pm_(std::move(pm)) {}
};

template <typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>, std::tuple<OutputArgs...>>
{
public:
    using DataTuple = std::tuple<DataArgs...>;
    using ValueTuple = std::tuple<ValueArgs...>;
    using OutputTuple = std::tuple<OutputArgs...>;
    using Getter = Getter::Getter<DataTuple, ValueTuple, OutputTuple>;
    using PMImpl = PropertyModelImpl<DataTuple, ValueTuple, OutputTuple>;
    using PM = PropertyModel<DataTuple, ValueTuple, OutputTuple>;

    Builder() : pm_(std::make_unique<PMImpl>()), getter_(pm_->data_, pm_->value_, pm_->output_) {}

    template <class R, class T>
    void set(T &&value)
    {
        getter_.template getPtr<R>() = std::move(value);
    }

    void new_constraint(size_t priority)
    {
        if (!current_constraint_.methods.empty())
        {
            pm_->setConstraint(std::move(current_constraint_));
        }
        current_constraint_.methods.clear();
        current_constraint_.methods.shrink_to_fit();
        current_constraint_.priority = priority;
        if (priority > max_priority_)
        {
            max_priority_ = priority;
        }
    }

    template <typename OutVar, typename... InVars>
    void add_method(Binding::Function<DataTuple, ValueTuple, OutputTuple, OutVar, InVars...> func)
    {
        Getter &current_getter = getter_;

        auto bind = [this, &current_getter, func]()
        {
            getter_.template getPtr<OutVar>() = std::invoke(func, getter_.template getPtr<InVars>()...);
        };

        std::vector<size_t> inputs = {getter_.template getIndex<InVars>()...};
        Method m(std::move(bind),
                 inputs,
                 getter_.template getIndex<OutVar>());

        current_constraint_.methods.push_back(m);
    }

    PM get()
    {
        make_stay_data<0>();
        make_stay_value<0>();
        make_stay_output<0>();
        new_constraint(0);

        return std::move(pm_);
    }

private:
    template <size_t Ind>
    void make_stay_data()
    {
        new_constraint(++max_priority_);
        auto &val = getter_.template getPtr<Data<Ind>>();
        add_method<Data<Ind>>([&val]()
                              { return val; });
        if constexpr (Ind + 1 < std::tuple_size<DataTuple>{})
        {
            make_stay_data<Ind + 1>();
        }
    }

    template <size_t Ind>
    void make_stay_value()
    {
        new_constraint(++max_priority_);
        auto &val = getter_.template getPtr<Value<Ind>>();
        add_method<Value<Ind>>([&val]()
                               { return val; });
        if constexpr (Ind + 1 < std::tuple_size<ValueTuple>{})
        {
            make_stay_data<Ind + 1>();
        }
    }

    template <size_t Ind>
    void make_stay_output()
    {
        new_constraint(++max_priority_);
        auto &val = getter_.template getPtr<Output<Ind>>();
        add_method<Output<Ind>>([&val]()
                                { return val; });
        if constexpr (Ind + 1 < std::tuple_size<OutputTuple>{})
        {
            make_stay_data<Ind + 1>();
        }
    }

    PM pm_;
    Getter getter_;
    Constraint current_constraint_;
    size_t max_priority_ = 0;
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
    Builder<std::tuple<int, double>, std::tuple<int, double>, std::tuple<int, double>> builder;
    builder.set<Data<0>>(2);
    builder.set<Data<1>>(4.6);
    builder.set<Value<0>>(4);
    builder.set<Value<1>>(4.65);
    builder.set<Output<0>>(123456);
    builder.set<Output<1>>(1);

    builder.new_constraint(1);
    builder.add_method<Output<0>, Data<0>, Data<1>>(m1);

    auto pm = builder.get();

    return 0;
}
