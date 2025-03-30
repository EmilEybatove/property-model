#include <cstdio>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

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
	bool enabled;
};

class ConstraintGraph {
  public:
	const Variable& get_variable_const(size_t index) const {
		return variables[index];
	}

	Variable& get_variable(size_t index) {
		return variables[index];
	}

	const std::unique_ptr<Constraint>&
	get_constraint_const(size_t index) const {
		return constraints[index];
	}

	std::unique_ptr<Constraint>& get_constraint(size_t index) {
		return constraints[index];
	}

	void new_constraint(std::unique_ptr<Constraint>& constraint) {
		constraint->position = constraints.size();
		constraint->choosen = nullptr;
		if (constraint->IsStay) {
			while (constraint->methods[0].output >= variables.size()) {
				variables.push_back(Variable());
				variables[variables.size() - 1].position = variables.size() - 1;
			}
			variables[constraint->methods[0].output].determined_by =
				constraint->position;
			variables[constraint->methods[0].output].force =
				constraint->priority;
			constraint->choosen = &constraint->methods[0];
		}
		constraints.push_back(std::move(constraint));
	};

	std::vector<Variable> variables;
	std::vector<std::unique_ptr<Constraint>> constraints;
};

class DeltaBlue {
  public:
	static void build_solution(ConstraintGraph& GC) {

		for (std::unique_ptr<Constraint>& constraint : GC.constraints) {
			if (!constraint->IsStay) {
				add_constraint(constraint, GC);
			}
		}
	}

	static void up_stay(std::unique_ptr<Constraint>& constraint,
						ConstraintGraph& GC) {
		size_t priority = constraint->priority;
		for (std::unique_ptr<Constraint>& current_constraint : GC.constraints) {
			if (current_constraint->IsStay) {
				priority = std::min(current_constraint->priority, priority);
				++current_constraint->priority;
			}
		}
		constraint->priority = priority;
		if (constraint->choosen == nullptr) {
			add_constraint(constraint, GC);
		}
		recalc_force(GC);
	}

	static void enable_constraint(const std::unique_ptr<Constraint>& constraint,
								  ConstraintGraph& GC) {
		if (!constraint->enabled) {
			for (size_t input : constraint->choosen->inputs) {
				GC.get_variable(input).output.emplace(constraint->position);
			}
			constraint->enabled = true;
			constraint->choosen = nullptr;
		}
	}

  private:
	static void add_constraint(std::unique_ptr<Constraint>& constraint,
							   ConstraintGraph& GC) {
		constraint->enabled = false;

		if (reverse_path(&constraint, GC)) {
			std::vector<int> colors(GC.constraints.size(), 0);
			if (cycle_checker(constraint, GC, colors)) {
				exit(1);
			}
			add_propogate(GC.get_variable(constraint->choosen->output), GC,
						  nullptr);
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

		while ((*constraint)->priority != force) {
			for (const Method& method : (*constraint)->methods) {
				if (method.output != blocked && variable->force == force) {

					variable = &GC.get_variable(method.output);
					variable->output.insert(variable->determined_by);
					variable->output.erase((*constraint)->position);

					size_t tmp = variable->determined_by;
					variable->determined_by = (*constraint)->position;
					constraint = &GC.get_constraint(tmp);
				}
			}
		}
		for (size_t input : (*constraint)->choosen->inputs) {
			GC.get_variable(input).output.emplace((*constraint)->position);
		}
		(*constraint)->choosen = nullptr;
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

	static void add_propogate(Variable& variable, ConstraintGraph& GC,
							  std::vector<bool>* visited) {
		if (visited != nullptr) {
			if ((*visited)[variable.position]) {
				return;
			}
			(*visited)[variable.position] = true;
		}
		const std::unique_ptr<Constraint>& constraint =
			GC.get_constraint_const(variable.determined_by);
		size_t force = constraint->priority;
		for (const Method& method : constraint->methods) {
			if (&method != constraint->choosen) {
				force =
					std::max(force, GC.get_variable_const(method.output).force);
			}
		}
		variable.force = force;

		for (size_t constraint_index : variable.output) {
			add_propogate(GC.get_variable(constraint->choosen->output), GC,
						  visited);
		}
	}

	static void recalc_force(ConstraintGraph& GC) {
		std::vector<bool> visited(GC.variables.size(), false);
		for (size_t i = 0; i < visited.size(); ++i) {
			if (!visited[i]) {
				add_propogate(GC.get_variable(i), GC, &visited);
			}
		}
	}
};
}; // namespace Kernel
