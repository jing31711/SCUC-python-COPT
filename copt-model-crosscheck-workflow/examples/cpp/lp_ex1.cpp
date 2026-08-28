/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

using namespace std;

/*
 * This example solves the following LP model:
 *
 *  Maximize:
 *    1.2 x + 1.8 y + 2.1 z
 *
 *  Subject to:
 *    R0: 1.5 x + 1.2 y + 1.8 z <= 2.6
 *    R1: 0.8 x + 0.6 y + 0.9 z >= 1.2
 *
 *  Where:
 *    0.1 <= x <= 0.6
 *    0.2 <= y <= 1.5
 *    0.3 <= z <= 2.8
 */
int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("lp_ex1");

    // Add variables
    Var x = model.AddVar(0.1, 0.6, 0.0, COPT_CONTINUOUS, "x");
    Var y = model.AddVar(0.2, 1.5, 0.0, COPT_CONTINUOUS, "y");
    Var z = model.AddVar(0.3, 2.8, 0.0, COPT_CONTINUOUS, "z");

    // Set objective
    model.SetObjective(1.2 * x + 1.8 * y + 2.1 * z, COPT_MAXIMIZE);

    // Add linear constraints using linear expression
    model.AddConstr(1.5 * x + 1.2 * y + 1.8 * z <= 2.6, "R0");

    Expr expr(x, 0.8);
    expr.AddTerm(y, 0.6);
    expr += 0.9 * z;
    model.AddConstr(expr >= 1.2, "R1");

    // Set parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 10);

    // Solve problem
    model.Solve();

    // Output solution
    if (model.GetIntAttr(COPT_INTATTR_HASLPSOL) != 0)
    {
      cout << "\nFound optimal solution:" << endl;
      VarArray vars = model.GetVars();
      for (int i = 0; i < vars.Size(); i++)
      {
        Var var = vars.GetVar(i);
        cout << "  " << var.GetName() << " = " << var.Get(COPT_DBLINFO_VALUE) << endl;
      }
      cout << "Obj = " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;
    }
  }
  catch (CoptException e)
  {
    cout << "Error Code = " << e.GetCode() << endl;
    cout << e.what() << endl;
  }
  catch (...)
  {
    cout << "Unknown exception occurs!";
  }
}
