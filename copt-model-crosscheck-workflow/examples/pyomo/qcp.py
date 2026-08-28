#
# This file is part of the Cardinal Optimizer, all rights reserved.
#

"""
Minimize
 obj: 2.1 x1 - 1.2 x2 + 3.2 x3 + x4 + x5 + x6 + 2 x7 + [ x2^2 ] / 2
Subject To
 r1: x1 + 2 x2 = 6
 r2: 2 x1 + x3 >= 5
 r3: x6 + 2 x7 <= 7
 r4: -x1 + 1.2 x7 >= -2.3
 q1: [ -1.8 x1^2 + x2^2 ] <= 0
 q2: [ 4.25 x3^2 - 2 x3 * x4 + 4.25 x4^2 - 2 x4 * x5 + 4 x5^2  ] + 2 x1 + 3 x3 <= 9.9
 q3: [ x6^2 - 2.2 x7^2 ] >= 5
Bounds
 0.2 <= x1 <= 3.8
 x2 Free
 0.1 <= x3 <= 0.7
 x4 Free
 x5 Free
 x7 Free
End
"""

from __future__ import print_function, division

import pyomo.environ as pyo
import pyomo.opt as pyopt

# You can comment this line if use 'coptampl'
from copt_pyomo import *

# Create COPT model
model = pyo.ConcreteModel()
model.name = "qcp"

# Add variables
model.x1 = pyo.Var(bounds=(0.2, 3.8))
model.x2 = pyo.Var(bounds=(None, None))
model.x3 = pyo.Var(bounds=(0.1, 0.7))
model.x4 = pyo.Var(bounds=(None, None))
model.x5 = pyo.Var(bounds=(None, None))
model.x6 = pyo.Var(bounds=(0, None))
model.x7 = pyo.Var(bounds=(None, None))
x = [model.x1, model.x2, model.x3, model.x4, model.x5, model.x6, model.x7]

# Add linear constraints
model.c1 = pyo.Constraint(rule=lambda model: model.x1 + 2 * model.x2 == 6)
model.c2 = pyo.Constraint(rule=lambda model: 2 * model.x1 + model.x3 >= 5)
model.c3 = pyo.Constraint(rule=lambda model: model.x6 + 2 * model.x7 <= 7)
model.c4 = pyo.Constraint(rule=lambda model: -model.x1 + 1.2 * model.x7 >= -2.3)

# Add quadratic constraints
model.qc1 = pyo.Constraint(rule=lambda model: -1.8 * model.x1 * model.x1 + model.x2 * model.x2 <= 0)
model.qc2 = pyo.Constraint(rule=lambda
    model: 4.25 * model.x3 * model.x3 - 2 * model.x3 * model.x4 + 4.25 * model.x4 * model.x4 - 2 * model.x4 * model.x5 + 4 * model.x5 * model.x5 + 2 * model.x1 + 3 * model.x3 <= 9.9)
model.qc3 = pyo.Constraint(rule=lambda model: model.x6 * model.x6 - 2.2 * model.x7 * model.x7 >= 5)

# Set quadratic objective
def Objective_rule(model):
    return 2.1 * model.x1 - 1.2 * model.x2 + 3.2 * model.x3 + model.x4 + model.x5 + model.x6 + 2 * model.x7 + 0.5 * model.x2 * model.x2
model.obj = pyo.Objective(rule=Objective_rule, sense=pyo.minimize)

# Use 'coptampl' to solve the problem
# solver = pyopt.SolverFactory('coptampl')
# solver.options['outlev'] = 1

# Use 'copt_direct' solver to solve the problem
solver = pyopt.SolverFactory('copt_direct')

# Use 'copt_persistent' solver to solve the problem
# solver = pyopt.SolverFactory('copt_persistent')
# solver.set_instance(model)

# Set parameters
# solve the model
results = solver.solve(model, tee=True, timelimit=60)

# Display solution
# Check result
print("")
if results.solver.status == pyopt.SolverStatus.ok and \
   results.solver.termination_condition == pyopt.TerminationCondition.optimal:
    print("Optimal solution found")
else:
    print("Something unexpected happened: ", str(results.solver))

print("\nOptimal objective value: {0:.9e}".format(pyo.value(model.obj)))

print("Variable solution:")
for i, x_i in enumerate(x):
    print(" x{0} = {1:.9e}".format(i, pyo.value(x_i)))
