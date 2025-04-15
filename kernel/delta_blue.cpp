#include "delta_blue.h"

namespace PropertyModel {

namespace Kernel {

const Variable& ConstraintGraph::get_variable_const(size_t index) const {
	return variables[index];
}

Variable& ConstraintGraph::get_variable(size_t index) {
	return variables[index];
}

const std::unique_ptr<Constraint>&
ConstraintGraph::get_constraint_const(size_t index) const {
	return constraints[index];
}

std::unique_ptr<Constraint>& ConstraintGraph::get_constraint(size_t index) {
	return constraints[index];
}

void ConstraintGraph::new_constraint(std::unique_ptr<Constraint>& constraint) {
	constraint->position = constraints.size();
	constraint->choosen = nullptr;
	if (constraint->IsStay) {
		while (constraint->methods[0].output >= variables.size()) {
			variables.push_back(Variable());
			variables[variables.size() - 1].position = variables.size() - 1;
		}
		variables[constraint->methods[0].output].determined_by =
			constraint->position;
		variables[constraint->methods[0].output].force = constraint->priority;
		constraint->choosen = &constraint->methods[0];
	}
	constraints.push_back(std::move(constraint));
};

}; // namespace Kernel
}; // namespace PropertyModel
