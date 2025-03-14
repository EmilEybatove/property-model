#include <cstdio>
#include <functional>
#include <vector>
// #include <variant>

struct Variable {
	size_t force;
	size_t determined_by;
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
};

class ConstraintGraph {
	friend DeltaBlue;

  private:
	void new_constraint(Constraint constraint) {};

	std::vector<Variable> variables;
};

class SolutionGraph {
	friend DeltaBlue;

  private:
	void init_stay(Constraint constraint) {};
};

class DeltaBlue {
  public:
	static void add_constraint(Constraint constraint, ConstraintGraph& GC,
							   SolutionGraph& GS) {
		GC.new_constraint(constraint);

		for (Method& method : constraint.methods) {
		}
	}

	static void build_solution(std::vector<Constraint>& all_constraints,
							   ConstraintGraph& GC, SolutionGraph& GS) {
		init(all_constraints, GC, GS);
		for (Constraint& constraint : all_constraints) {
			add_constraint(constraint, GC, GS);
		}
	}

  private:
	static void init(std::vector<Constraint>& all_constraints,
					 ConstraintGraph& GC, SolutionGraph& GS) {
		for (Constraint& constraint : all_constraints) {
			if (constraint.IsStay) {
				GC.new_constraint(constraint);
				GS.init_stay(constraint);
			}
		}
	}

	static bool pacanski_razvorot(Constraint& constraint, ConstraintGraph& GC,
								  SolutionGraph& GS) {
		size_t force = 0;
		Variable& variable = GC.variables[0];
		for (Method& method : constraint.methods) {
			if (GC.variables[method.output].force > force) {
				variable = GC.variables[method.output];
				force = variable.force;
			}
		}
		if (force == 0) {
			exit(1); // это ужасно
		}

		if (force <= constraint.priority) {
			return false;
		}

		while (constraint.priority != force) {
		}

		for (Method& method : constraint.methods) {
			if (GC.variables[method.output].force == method.)
		}
	}
};
