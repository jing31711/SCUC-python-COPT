/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

using namespace std;

/*
 * Solve the following nonlinear problem via nonlinear expresion input:
 *
 * min:    x1 * x4 * (sin(x1 + x2) + cos(x2 * x3) + tan(x3 / x4)) + x3
 * s.t.    x1 * x2 * x3 * x4 + x1 + x2 >= 35
 *         log(x1) + 2 * log(x2) + 3 * log(x3) + 4 * log(x4) + x3 + x4 >= 15
 *         x1^2 + x2^2 + x3^2 + x4^2 + x1 + x3 >= 50
 *
 * where:
 *         1 <= x1, x2, x3, x4 <= 5
 */
int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("nlp_ex1");

    // Add variables
    Var x1 = model.AddVar(1.0, 5.0, 0.0, COPT_CONTINUOUS, "x1");
    Var x2 = model.AddVar(1.0, 5.0, 0.0, COPT_CONTINUOUS, "x2");
    Var x3 = model.AddVar(1.0, 5.0, 0.0, COPT_CONTINUOUS, "x3");
    Var x4 = model.AddVar(1.0, 5.0, 0.0, COPT_CONTINUOUS, "x4");

    // Add non-linear constraints
    model.AddNlConstr(x1 * x2 * x3 * x4 + x1 + x2 >= 35, "nlrow1");
    model.AddNlConstr(NL::Log(x1) + 2 * NL::Log(x2) + 3 * NL::Log(x3) + 4 * NL::Log(x4) + x3 + x4 >= 15, "nlrow2");

    model.AddQConstr(x1 * x1 + x2 * x2 + x3 * x3 + x4 * x4 + x1 + x3 >= 50, "q1");

    // Set non-linear objective
    NlExpr obj = x1 * x4 * NL::Sum(NL::Sin(x1 + x2), NL::Cos(x2 * x3), NL::Tan(x3 / x4)) + x3;
    model.SetNlObjective(obj, COPT_MINIMIZE);

    // Set parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 60);

    // Solve the problem
    model.Solve();

    // Output solution
    if (model.GetIntAttr(COPT_INTATTR_HASLPSOL))
    {
      cout << "\nOptimal objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;

      cout << "Solution of variables: " << endl;
      VarArray vars = model.GetVars();
      for (int iCol = 0; iCol < vars.Size(); ++iCol)
      {
        Var x = vars.GetVar(iCol);
        cout << "  " << x.GetName() << " = " << x.Get(COPT_DBLINFO_VALUE) << endl;
      }
    }
    else
    {
      cout << "Error: no solution available!" << endl;
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
