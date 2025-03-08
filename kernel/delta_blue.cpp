#include <cstdio>
#include <vector>
#include <functional>

struct Variable
{
    size_t force;
    size_t determined_by;
};

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
    bool IsStay;
};

struct SolutionGraph
{
    std::vector<Variable> variables;
    std::vector<std::reference_wrapper<Constraint>> constraints;
    std::vector<size_t> StayPositions;
    std::vector<int> choosenMethods;
};

class DeltaBlue
{
private:
    static SolutionGraph init(std::vector<Constraint> &all_constraints)
    {
        std::vector<Variable> variables;
        std::vector<std::reference_wrapper<Constraint>> constraints;
        std::vector<size_t> StayPositions;
        std::vector<int> choosenMethods;

        for (size_t i = 0; i < all_constraints.size(); ++i)
        {
            if (all_constraints[i].IsStay)
            {
                variables.push_back({all_constraints[i].priority, i});
                constraints.push_back(all_constraints[i]);
                StayPositions.push_back(i);
                choosenMethods.push_back(0);
            }
        }

        return {variables, constraints, StayPositions, choosenMethods};
    }
};