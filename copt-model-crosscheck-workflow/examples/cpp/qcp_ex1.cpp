/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example solves the following QCP model:
 *
 *   Minimize:
 *      2.1 x1 - 1.2 x2 + 3.2 x3 + x4 + x5 + x6 + 2 x7 + 0.5 x2^2
 *
 *   Subject To:
 *      R1:   x1 +   2 x2  = 6
 *      R2: 2 x1 +     x3 >= 5
 *      R3:   x6 +   2 x7 <= 7
 *      R4:  -x1 + 1.2 x7 >= -2.3
 *      Q1: -1.8 x1^2 + x2^2 <= 0
 *      Q2: 4.25 x3^2 - 2 x3 * x4 + 4.25 x4^2 - 2 x4 * x5 + 4 x5^2 + 2 x1 + 3 x3 <= 9.9
 *      Q3:      x6^2 - 2.2 x7^2 >= 5
 *
 *   Where:
 *      0.2 <= x1 <= 3.8
 *      0.1 <= x3 <= 0.7
 *      x2, x4, x5, x7 are free
 *
 */

#include "coptcpp_pch.h"

using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("qcp_ex1");

    // Add variables
    Var x1 = model.AddVar(0.2, 3.8, 0.0, COPT_CONTINUOUS, "x1");
    Var x2 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x2");
    Var x3 = model.AddVar(0.1, 0.7, 0.0, COPT_CONTINUOUS, "x3");
    Var x4 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x4");
    Var x5 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x5");
    Var x6 = model.AddVar(0, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x6");
    Var x7 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x7");

    // Add linear constraints
    model.AddConstr(x1 + 2 * x2 == 6, "R1");
    model.AddConstr(2 * x1 + x3 >= 5, "R2");
    model.AddConstr(x6 + 2 * x7 <= 7, "R3");
    model.AddConstr(-x1 + 1.2 * x7 >= -2.3, "R4");

    // Add quadratic constraints
    model.AddQConstr(-1.8 * x1 * x1 + x2 * x2 <= 0, "Q1");
    model.AddQConstr(4.25 * x3 * x3 - 2 * x3 * x4 + 4.25 * x4 * x4 - 2 * x4 * x5 + 4 * x5 * x5 + 2 * x1 + 3 * x3 <= 9.9,
      "Q2");
    model.AddQConstr(x6 * x6 - 2.2 * x7 * x7 >= 5, "Q3");

    // Set quadratic objective
    QuadExpr obj = 2.1 * x1 - 1.2 * x2 + 3.2 * x3 + x4 + x5 + x6 + 2 * x7 + 0.5 * x2 * x2;
    model.SetQuadObjective(obj, COPT_MINIMIZE);

    // Set parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 60);

    // Solve the problem
    model.Solve();

    // Output solution
    if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_OPTIMAL)
    {
      cout << "\nOptimal objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;

      cout << "Solution of variables: " << endl;
      VarArray vars = model.GetVars();
      for (int iCol = 0; iCol < vars.Size(); ++iCol)
      {
        Var var = vars.GetVar(iCol);
        cout << "  " << var.GetName() << " = " << var.Get(COPT_DBLINFO_VALUE) << endl;
      }
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
