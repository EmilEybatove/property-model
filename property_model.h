#pragma once

#include "kernel/delta_blue.h"
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
using Constraint = Kernel::Constraint;
using ConstraintGraph = Kernel::ConstraintGraph;
using DeltaBlue = Kernel::DeltaBlue;
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
	using DataTuple = DataTypes<DataArgs...>;
	using ValueTuple = ValueTypes<ValueArgs...>;
	using OutputTuple = OutputTypes<OutputArgs...>;

  public:
	friend class Builder<DataTuple, ValueTuple, OutputTuple>;
	using PMImpl = PropertyModelImpl<DataTuple, ValueTuple, OutputTuple>;
	friend std::unique_ptr<PMImpl> std::make_unique<PMImpl>();

	template<typename R>
	void update_variable(
		Library::Type<R, DataTuple, ValueTuple, OutputTuple>&& value) {
		Library::Getter<R>::get(data_, value_, output_) = std::move(value);
		Kernel::DeltaBlue::changed_value(
			Library::GetIndex<R>::get(data_, value_, output_), GC_);
	}

	void disable_constraint(size_t constraint_position) {
		Kernel::DeltaBlue::disable_constraint(
			GC_.get_constraint(constraint_position), GC_);
	}

	void enable_constraint(size_t constraint_position) {
		Kernel::DeltaBlue::add_constraint(
			GC_.get_constraint(constraint_position), GC_);
	}

	template<typename R>
	Library::Type<R, DataTuple, ValueTuple, OutputTuple>& get() {
		return Library::Getter<R>::get(data_, value_, output_);
	}

  private:
	PropertyModelImpl() = default;
	PropertyModelImpl(DataArgs... dataArgs, ValueArgs... valueArgs,
					  OutputArgs... outputArgs)
		: data_(std::move(dataArgs...)), value_(std::move(valueArgs...)),
		  output_(std::move(outputArgs...)) {
	}

	void setConstraint(std::unique_ptr<Constraint>& c) {
		GC_.new_constraint(c);
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
	ConstraintGraph GC_;
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

	Builder()
		: pm_(std::make_unique<PMImpl>()),
		  current_constraint_(std::make_unique<Constraint>()) {
	}

	template<class R, class T>
	void set(T&& value) {
		Library::Getter<R>::get(pm_->data_, pm_->value_, pm_->output_) =
			std::move(value);
	}

	void new_constraint(size_t priority, bool disabled = false) {
		if (!current_constraint_->methods.empty()) {
			pm_->setConstraint(current_constraint_);
		}
		current_constraint_ = std::make_unique<Constraint>();
		current_constraint_->priority = priority;
		current_constraint_->disabled = disabled;

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
		current_constraint_->methods.push_back(m);
	}

	PM get() {
		make_all_stays<0>(pm_->data_);
		make_all_stays<1>(pm_->value_);
		make_all_stays<2>(pm_->output_);
		new_constraint(0);

		DeltaBlue::build_solution(pm_->GC_);

		return std::move(pm_);
	}

  private:
	template<int mode, size_t Ind, typename... TupleArgs>
	void make_stay(std::tuple<TupleArgs...>& variables) {
		new_constraint(++max_priority_);
		current_constraint_->IsStay = true;
		auto& val = std::get<Ind>(variables);

		if constexpr (mode == 0) {
			add_method<Library::Data<Ind>>([&val]() { return val; });
		} else if constexpr (mode == 1) {
			add_method<Library::Value<Ind>>([&val]() { return val; });
		} else if constexpr (mode == 2) {
			add_method<Library::Output<Ind>>([&val]() { return val; });
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
	std::unique_ptr<Constraint> current_constraint_;
	size_t max_priority_ = 0;
};
} // namespace PropertyModel
