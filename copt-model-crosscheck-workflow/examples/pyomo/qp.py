# 
# This file is part of the Cardinal Optimizer, all rights reserved.
# 

"""
Minimize
 OBJ.FUNC: [ 2 X0 ^2 + 4 X0 * X1 + 4 X1 ^2
           + 4 X1 * X2 + 4 X2 ^2
           + 4 X2 * X3 + 4 X3 ^2
           + 4 X3 * X4 + 4 X4 ^2
           + 4 X4 * X5 + 4 X5 ^2
           + 4 X5 * X6 + 4 X6 ^2
           + 4 X6 * X7 + 4 X7 ^2
           + 4 X7 * X8 + 4 X8 ^2
           + 4 X8 * X9 + 2 X9 ^2 ] / 2
Subject To
 ROW0: X0 + 2 X1 + 3 X2  = 1
 ROW1: X1 + 2 X2 + 3 X3  = 1
 ROW2: X2 + 2 X3 + 3 X4  = 1
 ROW3: X3 + 2 X4 + 3 X5  = 1
 ROW4: X4 + 2 X5 + 3 X6  = 1
 ROW5: X5 + 2 X6 + 3 X7  = 1
 ROW6: X6 + 2 X7 + 3 X8  = 1
 ROW7: X7 + 2 X8 + 3 X9  = 1
Bounds
      X0 Free
      X1 Free
      X2 Free
      X3 Free
      X4 Free
      X5 Free
      X6 Free
      X7 Free
      X8 Free
      X9 Free
End
"""

from __future__ import print_function, division

import pyomo.environ as pyo
import pyomo.opt as pyopt

# You can comment this line if use 'coptampl'
from copt_pyomo import *

# Create COPT model
model = pyo.ConcreteModel()
model.name = "qp"

# Add variables
model.x0 = pyo.Var(bounds=(None, None))
model.x1 = pyo.Var(bounds=(None, None))
model.x2 = pyo.Var(bounds=(None, None))
model.x3 = pyo.Var(bounds=(None, None))
model.x4 = pyo.Var(bounds=(None, None))
model.x5 = pyo.Var(bounds=(None, None))
model.x6 = pyo.Var(bounds=(None, None))
model.x7 = pyo.Var(bounds=(None, None))
model.x8 = pyo.Var(bounds=(None, None))
model.x9 = pyo.Var(bounds=(None, None))
x = [model.x0, model.x1, model.x2, model.x3, model.x4,
     model.x5, model.x6, model.x7, model.x8, model.x9]

# Add constraints
model.c0 = pyo.Constraint(rule=lambda model: model.x0 + 2 * model.x1 + 3 * model.x2 == 1)
model.c1 = pyo.Constraint(rule=lambda model: model.x1 + 2 * model.x2 + 3 * model.x3 == 1)
model.c2 = pyo.Constraint(rule=lambda model: model.x2 + 2 * model.x3 + 3 * model.x4 == 1)
model.c3 = pyo.Constraint(rule=lambda model: model.x3 + 2 * model.x4 + 3 * model.x5 == 1)
model.c4 = pyo.Constraint(rule=lambda model: model.x4 + 2 * model.x5 + 3 * model.x6 == 1)
model.c5 = pyo.Constraint(rule=lambda model: model.x5 + 2 * model.x6 + 3 * model.x7 == 1)
model.c6 = pyo.Constraint(rule=lambda model: model.x6 + 2 * model.x7 + 3 * model.x8 == 1)
model.c7 = pyo.Constraint(rule=lambda model: model.x7 + 2 * model.x8 + 3 * model.x9 == 1)


# Set quadratic objective
def Objective_rule(model):
    obj = model.x0 * model.x0 + model.x9 * model.x9
    obj += 2 * model.x1 * model.x1 + 2 * model.x2 * model.x2 + 2 * model.x3 * model.x3 + 2 * model.x4 * model.x4
    obj += 2 * model.x5 * model.x5 + 2 * model.x6 * model.x6 + 2 * model.x7 * model.x7 + 2 * model.x8 * model.x8
    obj += 2 * model.x0 * model.x1 + 2 * model.x1 * model.x2 + 2 * model.x2 * model.x3 + 2 * model.x3 * model.x4 + 2 * model.x4 * model.x5
    obj += 2 * model.x5 * model.x6 + 2 * model.x6 * model.x7 + 2 * model.x7 * model.x8 + 2 * model.x8 * model.x9
    return obj
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
