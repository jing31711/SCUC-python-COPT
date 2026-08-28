# 
# This file is part of the Cardinal Optimizer, all rights reserved.
# 
from __future__ import print_function, division

import pyomo.environ as pyo
import pyomo.opt as pyopt

# You can comment this line if use 'coptampl'
from copt_pyomo import *

# Create model
model = pyo.ConcreteModel()
model.name = "sos2"

# X set
I = range(4)

model.I = pyo.Set(initialize=I)

# Create variables
def x_bounds(model, i):
    return (0.0, 3.0)
model.x = pyo.Var(model.I, bounds=x_bounds, within=pyo.NonNegativeIntegers)

# Set objective
def Objective_rule(model):
    return model.x[0] + 2 * model.x[1] + 0.5 * model.x[2] + 3 * model.x[3]
model.obj = pyo.Objective(rule=Objective_rule, sense=pyo.maximize)

# Add linear constraint
def Linear_constr_rule(model):
    return pyo.quicksum(model.x[i] for i in model.I) <= 5
model.linear_constr = pyo.Constraint(rule=Linear_constr_rule)

# Add SOS2 constraint
model.sos2 = pyo.SOSConstraint(var=model.x, weights=(1, 2, 3, 4), sos=2)

# Use 'coptampl' to solve the problem
# solver = pyopt.SolverFactory('coptampl')
# solver.options['outlev'] = 1

# Use 'copt_direct' solver to solve the problem
solver = pyopt.SolverFactory('copt_direct')

# Use 'copt_persistent' solver to solve the problem
# solver = pyopt.SolverFactory('copt_persistent')
# solver.set_instance(model)

# solve the model
results = solver.solve(model, tee=True)

# Display solution
# Check result
print("")
if results.solver.status == pyopt.SolverStatus.ok and \
        results.solver.termination_condition == pyopt.TerminationCondition.optimal:
    print("Optimal solution found")
else:
    print("Something unexpected happened: ", str(results.solver))

for i in model.I:
    print('x%d %g' % (i, pyo.value(model.x[i])))

print('Obj: %g' % pyo.value(model.obj))
