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
	const Variable& get_veriable_const(size_t index) {
		return variables[index];
	}

	Variable& get_veriable(size_t index) {
		return variables[index];
	}

	const std::unique_ptr<Constraint>& get_constraint_const(size_t index) {
		return constraints[index];
	}

	std::unique_ptr<Constraint>& get_constraint(size_t index) {
		return constraints[index];
	}

	void init_variables(size_t variable_count) {
		variables = std::vector<Variable>(variable_count);
	}

	void new_constraint(std::unique_ptr<Constraint>& constraint) {
		constraints.push_back(std::move(constraint));
		constraint->position = constraints.size() - 1;
		constraint->choosen = nullptr;
		if (constraint->IsStay) {
			variables[constraint->methods[0].output].determined_by =
				constraint->position;
			constraint->choosen = &constraint->methods[0];
		}
	};

	std::vector<Variable> variables;
	std::vector<std::unique_ptr<Constraint>> constraints;
};

class DeltaBlue {
  public:
	static void
	build_solution(std::vector<std::unique_ptr<Constraint>>& all_constraints,
				   size_t variable_count, ConstraintGraph& GC) {

		init(all_constraints, variable_count, GC);
		for (std::unique_ptr<Constraint>& constraint : all_constraints) {
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
	static void init(std::vector<std::unique_ptr<Constraint>>& all_constraints,
					 size_t variable_count, ConstraintGraph& GC) {
		GC.init_variables(variable_count);

		for (std::unique_ptr<Constraint>& constraint : all_constraints) {
			GC.new_constraint(constraint);
		}
	}

	static void add_constraint(std::unique_ptr<Constraint>& constraint,
							   ConstraintGraph& GC) {
		GC.new_constraint(constraint);

		if (pacanski_razvorot(GC.constraints.size() - 1, GC)) {
			std::vector<int> colors(GC.constraints.size(), 0);
			if (cycle_checker(GC.constraints.size() - 1, GC, colors)) {
				exit(1);
			}
			add_propogate(GC.constraints.size() - 1, GC, nullptr);
		}
	}

	static bool pacanski_razvorot(size_t constraint_index,
								  ConstraintGraph& GC) {
		size_t force = 0;
		size_t variable_position = 0;
		for (Method& method : GC.constraints[constraint_index]->methods) {
			if (GC.get_veriable_const(method.output).force > force) {
				variable_position = method.output;
				force = GC.get_veriable_const(variable_position).force;
			}
		}
		if (force == 0) {
			exit(1); // это ужасно
		}

		if (force <= GC.constraints[constraint_index]->priority) {
			return false;
		}

		size_t blocked = variable_position;
		GC.get_veriable(variable_position)
			.output.insert(GC.get_veriable(variable_position).determined_by);
		std::swap(constraint_index,
				  GC.get_veriable(variable_position).determined_by);

		while (GC.constraints[constraint_index]->priority != force) {
			for (const Method& method :
				 GC.get_constraint_const(constraint_index)->methods) {
				if (method.output != blocked &&
					GC.get_veriable_const(method.output).force == force) {

					variable_position = method.output;
					GC.get_veriable(variable_position)
						.output.insert(
							GC.get_veriable(variable_position).determined_by);
					GC.get_veriable(variable_position)
						.output.erase(constraint_index);

					std::swap(constraint_index,
							  GC.get_veriable(variable_position).determined_by);
				}
			}
		}
	}

	static bool cycle_checker(size_t constraint_index, ConstraintGraph& GC,
							  std::vector<int>& constraint_colors) {
		constraint_colors[constraint_index] = 1;
		size_t variable_index =
			GC.constraints[constraint_index]->choosen->output;
		for (size_t neighbor_constraint_index :
			 GC.variables[variable_index].output) {
			if (constraint_colors[neighbor_constraint_index] == 0) {
				if (!cycle_checker(neighbor_constraint_index, GC,
								   constraint_colors)) {
					return false;
				}
			} else if (constraint_colors[neighbor_constraint_index] == 1) {
				return false;
			}
		}
		constraint_colors[constraint_index] = 2;
	}

	static void add_propogate(size_t variable_index, ConstraintGraph& GC,
							  std::vector<bool>* visited) {
		if (visited != nullptr) {
			if ((*visited)[variable_index]) {
				return;
			}
			(*visited)[variable_index] = true;
		}
		size_t constraint_index = GC.variables[variable_index].determined_by;
		size_t force = GC.variables[variable_index].force;
		for (const Method& method :
			 GC.get_constraint_const(constraint_index)->methods) {
			force = std::max(force, GC.get_veriable_const(method.output).force);
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
