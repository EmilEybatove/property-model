#include "property_model.h"
#include <cassert>

namespace {
using namespace PropertyModel;
using namespace Library;

void test_building_property_model() {

	Builder<DataTypes<int, int>, ValueTypes<int, int>, OutputTypes<int, int>>
		builder;
	builder.set<Data<0>>(1);
	builder.set<Data<1>>(2);
	builder.set<Value<0>>(3);
	builder.set<Value<1>>(4);
	builder.set<Output<0>>(5);
	builder.set<Output<1>>(6);

	builder.new_constraint(1);
	builder.add_method<Output<0>, Data<0>, Data<1>>(
		[](int a, double b) { return a * b; });

	auto pm = builder.get();

	const std::tuple<int, double> data_correct = {1, 2};
	const std::tuple<int, double> value_correct = {3, 4};
	const std::tuple<int, double> output_correct = {5, 6};

	const std::tuple<int, double> data = {pm->get<Data<0>>(),
										  pm->get<Data<1>>()};
	const std::tuple<int, double> value = {pm->get<Value<0>>(),
										   pm->get<Value<1>>()};
	const std::tuple<int, double> output = {pm->get<Output<0>>(),
											pm->get<Output<1>>()};

	assert(data_correct == data);
	assert(value_correct == value);
	assert(output_correct == output);
}

void test_simple_dialog() {
	Builder<DataTypes<int>, ValueTypes<int, int>, OutputTypes<>> builder;

	builder.set<Data<0>>(1000); // Initial Heigh

	builder.set<Value<0>>(1000); // Absolute Heigh
	builder.set<Value<1>>(100);	 // Relative Heigh

	builder.new_constraint(0); // absolute heigh relation

	builder.add_method<Value<0>, Value<1>, Data<0>>(
		[](int a, int b) { return a * b / 100; });

	builder.add_method<Value<1>, Value<0>, Data<0>>(
		[](int a, int b) { return a * 100 / b; });

	auto pm = builder.get();

	pm->update_variable<Value<0>>(2000);

	assert(pm->get<Value<0>>() == 2000 && pm->get<Value<1>>() == 200);

	pm->update_variable<Value<0>>(3000);

	assert(pm->get<Value<0>>() == 3000 && pm->get<Value<1>>() == 300);

	pm->update_variable<Value<1>>(50);

	assert(pm->get<Value<0>>() == 500 && pm->get<Value<1>>() == 50);

	return;
}

void test_dialog() {
	// Builder<DataTypes<int, int>, ValueTypes<int, int, int, int, bool>,
	// 		OutputTypes<int, int>>
	// 	builder;

	// builder.set<Data<0>>(1500); // Initial Heigh
	// builder.set<Data<1>>(2100); // Initial Width

	// builder.set<Value<0>>(1500); // Absolute Heigh
	// builder.set<Value<1>>(2100); // Absolute Width

	// builder.set<Value<2>>(100); // Relative Heigh
	// builder.set<Value<3>>(100); // Relative Width

	// builder.set<Value<4>>(true); // Preserve ration

	// builder.set<Output<0>>(1500); // Output Heigh
	// builder.set<Output<1>>(2100); // Output Width

	// builder.new_constraint(0); // absolute width relation

	// builder.add_method<Value<1>, Value<3>, Data<1>>(
	// 	[](int a, int b) { return a * b / 100; });

	// builder.add_method<Value<3>, Value<1>, Data<1>>(
	// 	[](int a, int b) { return a * 100 / b; });

	// builder.new_constraint(0); // absolute heigh relation

	// builder.add_method<Value<0>, Value<2>, Data<0>>(
	// 	[](int a, int b) { return a * b / 100; });

	// builder.add_method<Value<2>, Value<0>, Data<0>>(
	// 	[](int a, int b) { return a * 100 / b; });

	// builder.new_constraint(0); // ratio relation

	// builder.add_method<Value<2>, Value<3>>([](int a) { return a; });
	// builder.add_method<Value<3>, Value<2>>([](int a) { return a; });

	// builder.new_constraint(0); // output Heigh relation

	// builder.add_method<Output<0>, Value<0>>([](int a) { return a; });

	// builder.new_constraint(0); // output Width relation

	// builder.add_method<Output<1>, Value<1>>([](int a) { return a; });

	// auto pm = builder.get();

	// pm->update_variable<Value<0>>(2000);
	// return;
}

} // namespace

int main() {

	test_building_property_model();
	test_simple_dialog();

	return 0;
}
