#pragma once

#include <algorithm>
#include <cstdio>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

namespace PropertyModel {

namespace Kernel {
struct Variable {
	size_t force;
	size_t determined_by;
	std::unordered_set<size_t> output;
	size_t position;
};

struct Method {
	std::function<void()> func;
	std::vector<size_t> inputs;
	size_t output;
};

struct Constraint {
	std::vector<Method> methods;
	size_t priority;
	bool IsStay;
	size_t position;
	Method* choosen;
	bool disabled;
};

class ConstraintGraph {
  public:
	const Variable& get_variable_const(size_t index) const;

	Variable& get_variable(size_t index);

	const std::unique_ptr<Constraint>& get_constraint_const(size_t index) const;

	std::unique_ptr<Constraint>& get_constraint(size_t index);

	void new_constraint(std::unique_ptr<Constraint>& constraint);

	std::vector<Variable> variables;
	std::vector<std::unique_ptr<Constraint>> constraints;
};

class DeltaBlue {
  public:
	static void build_solution(ConstraintGraph& GC) {

		for (std::unique_ptr<Constraint>& constraint : GC.constraints) {
			if (!constraint->IsStay && !constraint->disabled) {
				add_constraint(constraint, GC);
			}
		}
	}

	static void changed_value(size_t variable, ConstraintGraph& GC) {
		for (std::unique_ptr<Constraint>& constraint : GC.constraints) {
			if (constraint->IsStay &&
				constraint->methods[0].output == variable) {
				up_stay(constraint, GC);
				break;
			}
		}
		exec_plan(GC);
	}

	static void
	disable_constraint(const std::unique_ptr<Constraint>& constraint,
					   ConstraintGraph& GC) {
		if (!constraint->disabled && !constraint->IsStay) {
			size_t variable_position = constraint->choosen->output;
			for (size_t input : constraint->choosen->inputs) {
				GC.get_variable(input).output.emplace(constraint->position);
			}
			constraint->disabled = true;
			constraint->choosen = nullptr;

			Variable& variable = GC.get_variable(variable_position);

			for (std::unique_ptr<Constraint>& current_constraint :
				 GC.constraints) {
				if (current_constraint->IsStay &&
					current_constraint->methods[0].output ==
						variable_position) {
					current_constraint->choosen =
						&current_constraint->methods[0];
					variable.determined_by = current_constraint->position;
					add_propogate(variable, GC);
					break;
				}
			}

			std::unique_ptr<Constraint>* min_blocked_constraint = nullptr;

			for (std::unique_ptr<Constraint>& current_constraint :
				 GC.constraints) {
				if (is_blocked(current_constraint, GC)) {
					if (min_blocked_constraint == nullptr) {
						min_blocked_constraint = &current_constraint;
					} else if ((*min_blocked_constraint)->priority >
							   current_constraint->priority) {
						min_blocked_constraint = &current_constraint;
					}
				}
			}

			add_constraint(*min_blocked_constraint, GC);
		}
	}

  private:
	static void exec_plan(ConstraintGraph& GC) {
		std::vector<size_t> order;
		topsort_constraint(GC, order);
		for (size_t constraint_position : order) {
			GC.get_constraint_const(constraint_position)->choosen->func();
		}
	}

	static void up_stay(std::unique_ptr<Constraint>& constraint,
						ConstraintGraph& GC) {
		size_t priority = constraint->priority;
		for (std::unique_ptr<Constraint>& current_constraint : GC.constraints) {
			if (current_constraint->IsStay &&
				current_constraint->priority < constraint->priority) {
				priority = std::min(current_constraint->priority, priority);
				++current_constraint->priority;
			}
		}
		constraint->priority = priority;
		recalc_force(GC);
		if (constraint->choosen == nullptr) {
			add_constraint(constraint, GC);
		}
		recalc_force(GC);
	}

	static void add_constraint(std::unique_ptr<Constraint>& constraint,
							   ConstraintGraph& GC) {
		if (constraint->choosen != nullptr) {
			return;
		}
		constraint->disabled = false;

		size_t position = constraint->position;

		if (reverse_path(&constraint, GC)) {
			std::vector<int> colors(GC.constraints.size(), 0);
			if (cycle_checker(constraint, GC, colors)) {
				exit(1);
			}
			add_propogate(GC.get_variable(constraint->choosen->output), GC);
		}
	}

	static bool reverse_path(std::unique_ptr<Constraint>* constraint,
							 ConstraintGraph& GC) {
		size_t force = 0;
		Variable* variable = nullptr;
		for (Method& method : (*constraint)->methods) {
			variable = &GC.get_variable(method.output);
			if (variable->force > force) {
				(*constraint)->choosen = &method;
				force = variable->force;
			}
		}
		if (force == 0) {
			exit(1); // это ужасно
		}

		if (force <= (*constraint)->priority) {
			return false;
		}

		size_t blocked = variable->position;
		for (size_t input : (*constraint)->choosen->inputs) {
			GC.get_variable(input).output.insert((*constraint)->position);
		}
		size_t tmp = variable->determined_by;
		variable->determined_by = (*constraint)->position;
		constraint = &GC.get_constraint(tmp);
		(*constraint)->choosen = nullptr;

		while ((*constraint)->priority != force) {
			for (Method& method : (*constraint)->methods) {
				if (method.output != blocked && variable->force == force) {

					variable = &GC.get_variable(method.output);
					variable->output.insert(variable->determined_by);
					variable->output.erase((*constraint)->position);

					blocked = variable->position;

					size_t tmp = variable->determined_by;
					variable->determined_by = (*constraint)->position;
					(*constraint)->choosen = &method;
					constraint = &GC.get_constraint(tmp);
					(*constraint)->choosen = nullptr;
					break;
				}
			}
		}

		for (size_t input : (*constraint)->methods[0].inputs) {
			GC.get_variable(input).output.erase((*constraint)->position);
		}

		GC.get_variable((*constraint)->methods[0].output)
			.output.erase((*constraint)->position);

		return true;
	}

	static bool cycle_checker(const std::unique_ptr<Constraint>& constraint,
							  ConstraintGraph& GC,
							  std::vector<int>& constraint_colors) {
		constraint_colors[constraint->position] = 1;
		const Variable& variable =
			GC.get_variable_const(constraint->choosen->output);
		for (size_t neighbor_constraint_index : variable.output) {
			if (constraint_colors[neighbor_constraint_index] == 0) {
				if (cycle_checker(
						GC.get_constraint_const(neighbor_constraint_index), GC,
						constraint_colors)) {
					return true;
				}
			} else if (constraint_colors[neighbor_constraint_index] == 1) {
				return true;
			}
		}
		constraint_colors[constraint->position] = 2;
		return false;
	}

	static void add_propogate(Variable& variable, ConstraintGraph& GC) {
		const std::unique_ptr<Constraint>& constraint =
			GC.get_constraint_const(variable.determined_by);
		add_propogate_step(variable, constraint, GC);

		for (size_t constraint_index : variable.output) {
			add_propogate(GC.get_variable(constraint->choosen->output), GC);
		}
	}

	static void
	add_propogate_step(Variable& variable,
					   const std::unique_ptr<Constraint>& constraint,
					   ConstraintGraph& GC) {
		size_t force = constraint->priority;
		for (const Method& method : constraint->methods) {
			if (&method != constraint->choosen) {
				force =
					std::max(force, GC.get_variable_const(method.output).force);
			}
		}
		variable.force = force;
	}

	static void recalc_force(ConstraintGraph& GC) {
		std::vector<size_t> order;
		topsort_constraint(GC, order);
		for (size_t constraint_position : order) {
			const std::unique_ptr<Constraint>& constraint =
				GC.get_constraint_const(constraint_position);
			Variable& variable = GC.get_variable(constraint->choosen->output);
			add_propogate_step(variable, constraint, GC);
		}
	}

	static void topsort_constraint(ConstraintGraph& GC,
								   std::vector<size_t>& order) {
		std::vector<bool> visited(GC.constraints.size());
		order.clear();

		for (const std::unique_ptr<Constraint>& constraint : GC.constraints) {
			topsort_constraint(constraint, GC, order, visited);
		}

		std::reverse(order.begin(), order.end());
	}

	static void
	topsort_constraint(const std::unique_ptr<Constraint>& constraint,
					   ConstraintGraph& GC, std::vector<size_t>& order,
					   std::vector<bool>& visited) {
		if (visited[constraint->position] || constraint->choosen == nullptr) {
			return;
		}
		order.push_back(constraint->position);
		visited[constraint->position] = true;
		for (size_t variable_position : constraint->choosen->inputs) {
			topsort_constraint(
				GC.get_constraint_const(
					GC.get_variable_const(variable_position).determined_by),
				GC, order, visited);
		}
	}

	static bool is_blocked(const std::unique_ptr<Constraint>& constraint,
						   ConstraintGraph& GC) {
		for (Method& method : constraint->methods) {
			if (GC.get_variable_const(method.output).force >
				constraint->priority) {
				return true;
			}
		}
		return false;
	}
};
}; // namespace Kernel
}; // namespace PropertyModel
