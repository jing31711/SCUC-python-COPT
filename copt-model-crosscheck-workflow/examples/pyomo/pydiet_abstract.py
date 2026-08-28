# The code is adopted from:
#
# https://github.com/Pyomo/pyomo/blob/master/examples/pyomo/amplbook2/diet.py
#
# with some modification by developer of the Cardinal Optimizer

from pyomo.core import *

model = AbstractModel()

model.NUTR = Set()
model.FOOD = Set()

model.cost  = Param(model.FOOD, within=NonNegativeReals)
model.f_min = Param(model.FOOD, within=NonNegativeReals)

model.f_max = Param(model.FOOD)
model.n_min = Param(model.NUTR, within=NonNegativeReals)
model.n_max = Param(model.NUTR)
model.amt   = Param(model.NUTR, model.FOOD, within=NonNegativeReals)

def Buy_bounds(model, i):
  return (model.f_min[i], model.f_max[i])
model.Buy = Var(model.FOOD, bounds=Buy_bounds)

def Objective_rule(model):
  return sum_product(model.cost, model.Buy)
model.totalcost = Objective(rule=Objective_rule, sense=minimize)

def Diet_rule(model, i):
  expr = 0

  for j in model.FOOD:
    expr = expr + model.amt[i, j] * model.Buy[j]

  return (model.n_min[i], expr, model.n_max[i])
model.Diet = Constraint(model.NUTR, rule=Diet_rule)
