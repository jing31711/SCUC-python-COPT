# The code is adopted from:
#
# https://github.com/Pyomo/pyomo/blob/master/examples/pyomo/amplbook2/diet.py
#
# with some modification by developer of the Cardinal Optimizer

from __future__ import print_function, division

import pyomo.environ as pyo
import pyomo.opt as pyopt

from copt_pyomo import *

# Nutrition set
NUTR = ["A", "C", "B1", "B2"]
# Food set
FOOD = ["BEEF", "CHK", "FISH", "HAM", "MCH", "MTL", "SPG", "TUR"]

# Price of foods
cost = {"BEEF": 3.19, "CHK": 2.59, "FISH": 2.29, "HAM": 2.89, "MCH": 1.89,
        "MTL":  1.99, "SPG": 1.99, "TUR":  2.49}
# Nutrition of foods
amt = {"BEEF": {"A": 60, "C": 20, "B1": 10, "B2": 15},
       "CHK":  {"A": 8,  "C": 0,  "B1": 20, "B2": 20},
       "FISH": {"A": 8,  "C": 10, "B1": 15, "B2": 10},
       "HAM":  {"A": 40, "C": 40, "B1": 35, "B2": 10},
       "MCH":  {"A": 15, "C": 35, "B1": 15, "B2": 15},
       "MTL":  {"A": 70, "C": 30, "B1": 15, "B2": 15},
       "SPG":  {"A": 25, "C": 50, "B1": 25, "B2": 15},
       "TUR":  {"A": 60, "C": 20, "B1": 15, "B2": 10}}

# The "diet problem" using ConcreteModel
model = pyo.ConcreteModel()

model.NUTR = pyo.Set(initialize=NUTR)
model.FOOD = pyo.Set(initialize=FOOD)

model.cost = pyo.Param(model.FOOD, initialize=cost)

def amt_rule(model, i, j):
  return amt[i][j]
model.amt  = pyo.Param(model.FOOD, model.NUTR, initialize=amt_rule)

model.f_min = pyo.Param(model.FOOD, default=0)
model.f_max = pyo.Param(model.FOOD, default=100)

model.n_min = pyo.Param(model.NUTR, default=700)
model.n_max = pyo.Param(model.NUTR, default=10000)

def Buy_bounds(model, i):
  return (model.f_min[i], model.f_max[i])
model.buy = pyo.Var(model.FOOD, bounds=Buy_bounds)

def Objective_rule(model):
  return pyo.sum_product(model.cost, model.buy)
model.totalcost = pyo.Objective(rule=Objective_rule, sense=pyo.minimize)

def Diet_rule(model, j):
  expr = 0

  for i in model.FOOD:
    expr = expr + model.amt[i, j] * model.buy[i]

  return (model.n_min[j], expr, model.n_max[j])
model.Diet = pyo.Constraint(model.NUTR, rule=Diet_rule)

# Reduced costs of variables
model.rc = pyo.Suffix(direction=pyo.Suffix.IMPORT)

# Activities and duals of constraints
model.slack = pyo.Suffix(direction=pyo.Suffix.IMPORT)
model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)

# Use 'copt_direct' solver to solve the problem
solver = pyopt.SolverFactory('copt_direct')

# Use 'copt_persistent' solver to solve the problem
# solver = pyopt.SolverFactory('copt_persistent')
# solver.set_instance(model)

results = solver.solve(model, tee=True)

# Check result
print("")
if results.solver.status == pyopt.SolverStatus.ok and \
   results.solver.termination_condition == pyopt.TerminationCondition.optimal:
  print("Optimal solution found")
else:
  print("Something unexpected happened: ", str(results.solver))

print("")
print("Optimal objective value:")
print("  totalcost: {0:6f}".format(pyo.value(model.totalcost)))

print("")
print("Variables solution:")
for i in FOOD:
  print("  buy[{0:4s}] = {1:9.6f} (rc: {2:9.6f})".format(i, \
                                                  pyo.value(model.buy[i]), \
                                                  model.rc[model.buy[i]]))

print("")
print("Constraint solution:")
for i in NUTR:
  print("  diet[{0:2s}] = {1:12.6f} (dual: {2:9.6f})".format(i, \
                                                      model.slack[model.Diet[i]], \
                                                      model.dual[model.Diet[i]]))
