#include <tuple>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include "indexing.h"
#include "binding.h"

namespace PropertyModel
{
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
        using DataTuple = std::tuple<DataArgs...>;
        using ValueTuple = std::tuple<ValueArgs...>;
        using OutputTuple = std::tuple<OutputArgs...>;

    private:
        PropertyModelImpl() = default;
        PropertyModelImpl(DataArgs... dataArgs, ValueArgs... valueArgs, OutputArgs... outputArgs) : data_(std::move(dataArgs...)), value_(std::move(valueArgs...)), output_(std::move(outputArgs...)), constraints_(std::vector<Constraint>()) {}

        void setConstraint(Constraint &&c)
        {
            constraints_.push_back(std::move(c));
        }

        template <typename OutVar, typename... InVars>
        std::function<void()> build_method(Library::Function<DataTuple, ValueTuple, OutputTuple, OutVar, InVars...> func)
        {
            auto bind = [this, func]()
            {
                Library::Getter<OutVar>::get(data_, value_, output_) = std::invoke(func, Library::Getter<InVars>::get(data_, value_, output_)...);
            };

            return bind;
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
        using PMImpl = PropertyModelImpl<DataTuple, ValueTuple, OutputTuple>;
        using PM = PropertyModel<DataTuple, ValueTuple, OutputTuple>;

        Builder() : pm_(std::make_unique<PMImpl>()) {}

        template <class R, class T>
        void set(T &&value)
        {
            Library::Getter<R>::get(pm_->data_, pm_->value_, pm_->output_) = std::move(value);
        }

        void new_constraint(size_t priority)
        {
            if (!current_constraint_.methods.empty())
            {
                pm_->setConstraint(std::move(current_constraint_));
            }
            current_constraint_.methods.clear();
            current_constraint_.priority = priority;
            if (priority > max_priority_)
            {
                max_priority_ = priority;
            }
        }

        template <typename OutVar, typename... InVars>
        void add_method(Library::Function<DataTuple, ValueTuple, OutputTuple, OutVar, InVars...> func)
        {
            auto bind = pm_->template build_method<OutVar, InVars...>(func);
            std::vector<size_t> inputs = {Library::GetIndex<InVars>::get(pm_->data_, pm_->value_, pm_->output_)...};
            Method m(std::move(bind),
                     inputs,
                     Library::GetIndex<OutVar>::get(pm_->data_, pm_->value_, pm_->output_));
            current_constraint_.methods.push_back(m);
        }

        PM get()
        {
            make_all_stays(pm_->data_);
            make_all_stays(pm_->value_);
            make_all_stays(pm_->output_);
            new_constraint(0);

            return std::move(pm_);
        }

    private:
        template <size_t Ind, typename... TupleArgs>
        void make_stay(std::tuple<TupleArgs...> &variables)
        {
            new_constraint(++max_priority_);
            auto &val = std::get<Ind>(variables);
            add_method<Data<Ind>>([&val]()
                                  { return val; });
        }

        template <typename... TupleArgs, size_t... Inds>
        void make_stays_from_seq(std::tuple<TupleArgs...> &variables, std::integer_sequence<size_t, Inds...>)
        {
            (make_stay<Inds>(variables), ...);
        }

        template <typename... TupleArgs>
        void make_all_stays(std::tuple<TupleArgs...> &variables)
        {
            make_stays_from_seq(variables, std::make_index_sequence<std::tuple_size<std::tuple<TupleArgs...>>{}>{});
        }

        PM pm_;
        Constraint current_constraint_;
        size_t max_priority_ = 0;
    };
}

size_t m1(int a, double b)
{
    return a * b;
}

using namespace PropertyModel;

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
