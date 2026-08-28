/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example solves the following QP model:
 *
 *   Minimize:
 *      x0^2 + 2 x0 * x1 + 2 x1^2 + 2 x1 * x2 + 2 x2^2 + 2 x2 * x3 + 2 x3^2 +
 *      2 x3 * x4 + 2 x4^2 + 2 x4 * x5 + 2 x5^2 + 2 x5 * x6 + 2 x6^2 + 2 x7^2 +
 *      2 x6 * x7 + 2 x7 * x8 + 2 x8^2 + 2 x8 * x9 + x9^2
 *
 *   Subject To:
 *      R0: x0 + 2 x1 + 3 x2  = 1
 *      R1: x1 + 2 x2 + 3 x3  = 1
 *      R2: x2 + 2 x3 + 3 x4  = 1
 *      R3: x3 + 2 x4 + 3 x5  = 1
 *      R4: x4 + 2 x5 + 3 x6  = 1
 *      R5: x5 + 2 x6 + 3 x7  = 1
 *      R6: x6 + 2 x7 + 3 x8  = 1
 *      R7: x7 + 2 x8 + 3 x9  = 1
 *
 *   Where:
 *       x0, x1, x2, x3, x4, x5, x6, x7, x8, x9 are free
 *
 */

#include "coptcpp_pch.h"

using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("qp_ex1");

    // Add variables
    Var x0 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x0");
    Var x1 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x1");
    Var x2 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x2");
    Var x3 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x3");
    Var x4 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x4");
    Var x5 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x5");
    Var x6 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x6");
    Var x7 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x7");
    Var x8 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x8");
    Var x9 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x9");

    // Add constraints
    model.AddConstr(x0 + 2 * x1 + 3 * x2 == 1, "R0");
    model.AddConstr(x1 + 2 * x2 + 3 * x3 == 1, "R1");
    model.AddConstr(x2 + 2 * x3 + 3 * x4 == 1, "R2");
    model.AddConstr(x3 + 2 * x4 + 3 * x5 == 1, "R3");
    model.AddConstr(x4 + 2 * x5 + 3 * x6 == 1, "R4");
    model.AddConstr(x5 + 2 * x6 + 3 * x7 == 1, "R5");
    model.AddConstr(x6 + 2 * x7 + 3 * x8 == 1, "R6");
    model.AddConstr(x7 + 2 * x8 + 3 * x9 == 1, "R7");

    // Set quadratic objective
    QuadExpr obj = x0 * x0 + x9 * x9;
    obj += 2 * x1 * x1 + 2 * x2 * x2 + 2 * x3 * x3 + 2 * x4 * x4;
    obj += 2 * x5 * x5 + 2 * x6 * x6 + 2 * x7 * x7 + 2 * x8 * x8;
    obj += 2 * x0 * x1 + 2 * x1 * x2 + 2 * x2 * x3 + 2 * x3 * x4 + 2 * x4 * x5;
    obj += 2 * x5 * x6 + 2 * x6 * x7 + 2 * x7 * x8 + 2 * x8 * x9;
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
