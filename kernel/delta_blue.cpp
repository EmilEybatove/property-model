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
	const Variable& get_variable_const(size_t index) {
		return variables[index];
	}

	Variable& get_variable(size_t index) {
		return variables[index];
	}

	const std::unique_ptr<Constraint>& get_constraint_const(size_t index) {
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

	static void up_stay(size_t constraint_position, ConstraintGraph& GC) {
		size_t priority = GC.constraints[constraint_position]->priority;
		for (std::unique_ptr<Constraint>& current_constraint : GC.constraints) {
			if (current_constraint->IsStay) {
				priority = std::min(current_constraint->priority, priority);
				++current_constraint->priority;
			}
		}
		GC.get_constraint(constraint_position)->priority = priority;
		if (GC.get_constraint(constraint_position)->choosen == nullptr) {
			add_constraint(GC.get_constraint(constraint_position), GC);
		}
		recalc_force(GC);
	}

	static void enable_constraint(size_t constraint_position,
								  ConstraintGraph& GC) {
		if (!GC.constraints[constraint_position]->enabled) {
			for (size_t input : GC.get_constraint_const(constraint_position)
									->choosen->inputs) {
				GC.variables[input].output.emplace(constraint_position);
			}
		}
		GC.get_constraint(constraint_position)->enabled = true;
		GC.get_constraint(constraint_position)->choosen = nullptr;
	}

  private:
	static void add_constraint(std::unique_ptr<Constraint>& constraint,
							   ConstraintGraph& GC) {
		constraint->enabled = false;

		if (pacanski_razvorot(constraint->position, GC)) {
			std::vector<int> colors(GC.constraints.size(), 0);
			if (cycle_checker(constraint->position, GC, colors)) {
				exit(1);
			}
			add_propogate(constraint->choosen->output, GC, nullptr);
		}
	}

	static bool pacanski_razvorot(size_t constraint_index,
								  ConstraintGraph& GC) {
		size_t force = 0;
		size_t variable_position = 0;
		for (Method& method : GC.constraints[constraint_index]->methods) {
			if (GC.get_variable_const(method.output).force > force) {
				variable_position = method.output;
				GC.get_constraint(constraint_index)->choosen = &method;
				force = GC.get_variable_const(variable_position).force;
			}
		}
		if (force == 0) {
			exit(1); // это ужасно
		}

		if (force <= GC.constraints[constraint_index]->priority) {
			return false;
		}

		size_t blocked = variable_position;
		for (size_t input :
			 GC.get_constraint_const(constraint_index)->choosen->inputs) {
			GC.variables[input].output.insert(constraint_index);
		}
		std::swap(constraint_index,
				  GC.get_variable(variable_position).determined_by);

		while (GC.get_constraint_const(constraint_index)->priority != force) {
			for (const Method& method :
				 GC.get_constraint_const(constraint_index)->methods) {
				if (method.output != blocked &&
					GC.get_variable_const(method.output).force == force) {

					variable_position = method.output;
					GC.get_variable(variable_position)
						.output.insert(
							GC.get_variable(variable_position).determined_by);
					GC.get_variable(variable_position)
						.output.erase(constraint_index);

					std::swap(constraint_index,
							  GC.get_variable(variable_position).determined_by);
				}
			}
		}
		for (size_t input :
			 GC.get_constraint_const(constraint_index)->choosen->inputs) {
			GC.variables[input].output.emplace(constraint_index);
		}
		GC.get_constraint_const(constraint_index)->choosen = nullptr;
		return true;
	}

	static bool cycle_checker(size_t constraint_index, ConstraintGraph& GC,
							  std::vector<int>& constraint_colors) {
		constraint_colors[constraint_index] = 1;
		size_t variable_index =
			GC.constraints[constraint_index]->choosen->output;
		for (size_t neighbor_constraint_index :
			 GC.variables[variable_index].output) {
			if (constraint_colors[neighbor_constraint_index] == 0) {
				if (cycle_checker(neighbor_constraint_index, GC,
								  constraint_colors)) {
					return true;
				}
			} else if (constraint_colors[neighbor_constraint_index] == 1) {
				return true;
			}
		}
		constraint_colors[constraint_index] = 2;
		return false;
	}

	static void add_propogate(size_t variable_index, ConstraintGraph& GC,
							  std::vector<bool>* visited) {
		if (visited != nullptr) {
			if ((*visited)[variable_index]) {
				return;
			}
			(*visited)[variable_index] = true;
		}
		size_t constraint_index =
			GC.get_variable_const(variable_index).determined_by;
		size_t force =
			GC.get_constraint_const(
				  GC.get_variable_const(variable_index).determined_by)
				->priority;
		for (const Method& method :
			 GC.get_constraint_const(constraint_index)->methods) {
			if (&method != GC.get_constraint_const(constraint_index)->choosen) {
				force =
					std::max(force, GC.get_variable_const(method.output).force);
			}
		}
		GC.variables[variable_index].force = force;

		for (size_t constraint_index : GC.variables[variable_index].output) {
			add_propogate(
				GC.get_constraint_const(constraint_index)->choosen->output, GC,
				visited);
		}
	}

	static void recalc_force(ConstraintGraph& GC) {
		std::vector<bool> visited(GC.variables.size(), false);
		for (size_t i = 0; i < visited.size(); ++i) {
			if (!visited[i]) {
				add_propogate(i, GC, &visited);
			}
		}
	}
};
}; // namespace Kernel
