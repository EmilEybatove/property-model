#include "kernel/delta_blue.cpp"
#include "library/binding.h"
#include "library/indexing.h"
#include <functional>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace PropertyModel {
using Method = Kernel::Method;
using Contraint = Kernel::Constraint;
using ConstraintGraph = Kernel::ConstraintGraph;
template<typename... Args>
using DataTypes = std::tuple<Args...>;

template<typename... Args>
using ValueTypes = std::tuple<Args...>;

template<typename... Args>
using OutputTypes = std::tuple<Args...>;

template<typename A, typename B, typename C>
class PropertyModelImpl;

template<typename A, typename B, typename C>
class Builder;

template<typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class PropertyModelImpl<DataTypes<DataArgs...>, ValueTypes<ValueArgs...>,
						OutputTypes<OutputArgs...>> {
  public:
	friend class Builder<DataTypes<DataArgs...>, ValueTypes<ValueArgs...>,
						 OutputTypes<OutputArgs...>>;
	using PMImpl =
		PropertyModelImpl<DataTypes<DataArgs...>, ValueTypes<ValueArgs...>,
						  OutputTypes<OutputArgs...>>;
	friend std::unique_ptr<PMImpl> std::make_unique<PMImpl>();
	using DataTuple = DataTypes<DataArgs...>;
	using ValueTuple = ValueTypes<ValueArgs...>;
	using OutputTuple = OutputTypes<OutputArgs...>;

  private:
	PropertyModelImpl() = default;
	PropertyModelImpl(DataArgs... dataArgs, ValueArgs... valueArgs,
					  OutputArgs... outputArgs)
		: data_(std::move(dataArgs...)), value_(std::move(valueArgs...)),
		  output_(std::move(outputArgs...)),
		  constraints_(std::vector<Constraint>()) {
	}

	void setConstraint(Constraint&& c) {
		constraints_.push_back(std::move(c));
	}

	template<typename OutVar, typename... InVars>
	std::function<void()> build_method(
		Library::Function<DataTuple, ValueTuple, OutputTuple, OutVar, InVars...>
			func) {
		auto bind = [this, func]() {
			Library::Getter<OutVar>::get(data_, value_, output_) = std::invoke(
				func, Library::Getter<InVars>::get(data_, value_, output_)...);
		};

		return bind;
	}

	DataTuple data_;
	ValueTuple value_;
	OutputTuple output_;
	std::vector<Constraint> constraints_;
};

template<typename A, typename B, typename C>
class PropertyModel;

template<typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class PropertyModel<DataTypes<DataArgs...>, ValueTypes<ValueArgs...>,
					OutputTypes<OutputArgs...>> {
  public:
	friend class Builder<DataTypes<DataArgs...>, ValueTypes<ValueArgs...>,
						 OutputTypes<OutputArgs...>>;
	using PMImpl =
		PropertyModelImpl<DataTypes<DataArgs...>, ValueTypes<ValueArgs...>,
						  OutputTypes<OutputArgs...>>;

	const PMImpl* operator->() const {
		return pm_.get();
	}

	PMImpl* operator->() {
		return pm_.get();
	}

  private:
	std::unique_ptr<PMImpl> pm_;

	explicit PropertyModel(std::unique_ptr<PMImpl> pm) : pm_(std::move(pm)) {
	}
};

template<typename... DataArgs, typename... ValueArgs, typename... OutputArgs>
class Builder<DataTypes<DataArgs...>, ValueTypes<ValueArgs...>,
			  OutputTypes<OutputArgs...>> {
  public:
	using DataTuple = DataTypes<DataArgs...>;
	using ValueTuple = ValueTypes<ValueArgs...>;
	using OutputTuple = OutputTypes<OutputArgs...>;
	using PMImpl = PropertyModelImpl<DataTuple, ValueTuple, OutputTuple>;
	using PM = PropertyModel<DataTuple, ValueTuple, OutputTuple>;

	Builder() : pm_(std::make_unique<PMImpl>()) {
	}

	template<class R, class T>
	void set(T&& value) {
		Library::Getter<R>::get(pm_->data_, pm_->value_, pm_->output_) =
			std::move(value);
	}

	void new_constraint(size_t priority) {
		if (!current_constraint_.methods.empty()) {
			pm_->setConstraint(std::move(current_constraint_));
		}
		current_constraint_.methods.clear();
		current_constraint_.priority = priority;
		if (priority > max_priority_) {
			max_priority_ = priority;
		}
	}

	template<typename OutVar, typename... InVars>
	void add_method(
		Library::Function<DataTuple, ValueTuple, OutputTuple, OutVar, InVars...>
			func) {
		auto bind = pm_->template build_method<OutVar, InVars...>(func);
		Method m = {std::move(bind)};
		(m.inputs.push_back(Library::GetIndex<InVars>::get(
			 pm_->data_, pm_->value_, pm_->output_)),
		 ...);
		m.output = Library::GetIndex<OutVar>::get(pm_->data_, pm_->value_,
												  pm_->output_);
		current_constraint_.methods.push_back(m);
	}

	PM get() {
		make_all_stays<0>(pm_->data_);
		make_all_stays<1>(pm_->value_);
		make_all_stays<2>(pm_->output_);
		new_constraint(0);

		return std::move(pm_);
	}

  private:
	template<int mode, size_t Ind, typename... TupleArgs>
	void make_stay(std::tuple<TupleArgs...>& variables) {
		new_constraint(++max_priority_);
		auto& val = std::get<Ind>(variables);

		switch (mode) {
		case 0:
			add_method<Library::Data<Ind>>([&val]() { return val; });
			break;
		case 1:
			add_method<Library::Value<Ind>>([&val]() { return val; });
		case 2:
			add_method<Library::Output<Ind>>([&val]() { return val; });
		default:
			break;
		}
	}

	template<int mode, typename... TupleArgs, size_t... Inds>
	void make_stays_from_seq(std::tuple<TupleArgs...>& variables,
							 std::integer_sequence<size_t, Inds...>) {
		(make_stay<mode, Inds>(variables), ...);
	}

	template<int mode, typename... TupleArgs>
	void make_all_stays(std::tuple<TupleArgs...>& variables) {
		make_stays_from_seq<mode>(
			variables, std::make_index_sequence<
						   std::tuple_size<std::tuple<TupleArgs...>>{}>{});
	}

	PM pm_;
	Constraint current_constraint_;
	size_t max_priority_ = 0;
};
} // namespace PropertyModel

size_t m1(int a, double b) {
	return a * b;
}

using namespace PropertyModel;

int main() {
	Builder<DataTypes<int, double>, ValueTypes<int, double>,
			OutputTypes<int, double>>
		builder;
	builder.set<Library::Data<0>>(2);
	builder.set<Library::Data<1>>(4.6);
	builder.set<Library::Value<0>>(4);
	builder.set<Library::Value<1>>(4.65);
	builder.set<Library::Output<0>>(123456);
	builder.set<Library::Output<1>>(1);

	builder.new_constraint(1);
	builder.add_method<Library::Output<0>, Library::Data<0>, Library::Data<1>>(
		m1);

	auto pm = builder.get();

	return 0;
}
