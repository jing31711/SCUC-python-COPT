/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

#include <cmath>
#include "coptcpp_pch.h"

using namespace std;

/*
 * This example solves the following SOCP model with regular cone:
 *
 *   Minimize: z
 *
 *   Subject To:
 *      R0: 3 x + y >= 1
 *      C0: z^2 >= x^2 + 2 y^2
 *
 *   Where:
 *      x, y, z are non-negative
 *
 *   Note that C0 is equivalent to
 *      R1: t = sqrt(2.0) y
 *      C1: z^2 >= x^2 + t^2    (regular cone)
 */
void solve_soc()
{
  try
  {
    // Create COPT environment
    Envr env = Envr();

    // Create COPT model
    Model model = env.CreateModel("solve_soc");

    // Add variables
    Var x = model.AddVar(-COPT_INFINITY, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x");
    Var y = model.AddVar(-COPT_INFINITY, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "y");
    Var z = model.AddVar(0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "z");
    Var t = model.AddVar(-COPT_INFINITY, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "t");

    // Set objective
    model.SetObjective(z, COPT_MINIMIZE);

    // Add two constraints
    model.AddConstr(3.0 * x + y >= 1.0, "R0");
    model.AddConstr(sqrt(2.0) * y - t == 0, "R1");

    // Add a regular cone
    VarArray vars;
    vars.PushBack(z);
    vars.PushBack(x);
    vars.PushBack(t);
    model.AddCone(vars, COPT_CONE_QUAD);

    // Set parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 10);

    // Solve the model
    model.Solve();

    // Analyze solution
    if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_OPTIMAL)
    {
      cout << "\nOptimal objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;

      VarArray vars = model.GetVars();
      cout << "Solution of variables:" << endl;
      for (int i = 0; i < vars.Size(); ++i)
      {
        Var var = vars.GetVar(i);
        cout << "  " << var.GetName() << " = " << var.Get(COPT_DBLINFO_VALUE) << endl;
      }
      cout << endl;
    }
  }
  catch (CoptException e)
  {
    cout << "Error code = " << e.GetCode() << endl;
    cout << e.what() << endl;
  }
  catch (...)
  {
    cout << "Exception during optimization" << endl;
  }
}

/*
 * This example solves the following SOCP model with rotated cone:
 *
 *   Minimize: 1.5 x - 2 y + z
 *
 *   Subject To:
 *      R0: 2 x +   y >= 2
 *      R1:  -x + 2 y <= 6
 *      R2: r = 1
 *      R3: 2.8284271247 x + 0.7071067811 y - s = 0
 *      R4: 3.0822070014 y - t = 0
 *      C0: 2 z * r >= s^2 + t^2    (rotated cone)
 *
 *   Where:
 *      0 <= x <= 20
 *      y, z, r are non-negative
 *      s, t are free
 */
void solve_rsoc()
{
  try
  {
    // Create COPT environment
    Envr env = Envr();

    // Create COPT model
    Model model = env.CreateModel("solve_rsoc");

    // Add variables
    Var x = model.AddVar(0.0, 20.0, 0.0, COPT_CONTINUOUS, "x");
    Var y = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "y");
    Var z = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "z");
    Var r = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "r");
    Var s = model.AddVar(-COPT_INFINITY, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "s");
    Var t = model.AddVar(-COPT_INFINITY, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "t");

    // Set objective
    model.SetObjective(1.5 * x - 2 * y + z, COPT_MINIMIZE);

    // Add five constraints
    model.AddConstr(2 * x + y >= 2, "R0");
    model.AddConstr(-x + 2 * y <= 6, "R1");
    model.AddConstr(r == 1, "R2");
    model.AddConstr(2.8284271247 * x + 0.7071067811 * y - s == 0, "R3");
    model.AddConstr(3.0822070014 * y - t == 0, "R4");

    // Add a rotated cone
    VarArray vars;
    vars.PushBack(z);
    vars.PushBack(r);
    vars.PushBack(s);
    vars.PushBack(t);
    model.AddCone(vars, COPT_CONE_RQUAD);

    // Set parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 10);

    // Solve the model
    model.Solve();

    // Analyze solution
    if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_OPTIMAL)
    {
      cout << "\nOptimal objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;

      VarArray vars = model.GetVars();
      cout << "Solution of variables:" << endl;
      for (int i = 0; i < vars.Size(); ++i)
      {
        Var var = vars.GetVar(i);
        cout << "  " << var.GetName() << " = " << var.Get(COPT_DBLINFO_VALUE) << endl;
      }
      cout << endl;
    }
  }
  catch (CoptException e)
  {
    cout << "Error code = " << e.GetCode() << endl;
    cout << e.what() << endl;
  }
  catch (...)
  {
    cout << "Exception during optimization" << endl;
  }
}

/*
 * This example reads problem from a MPS file and solve the SOCP model:
 *
 */
void solve_mps(const char* szFileName)
{
  try
  {
    // Create COPT environment
    Envr env = Envr();

    // Create COPT model
    Model model = env.CreateModel("solve_mps");

    // Read SOCP from MPS file
    model.ReadMps(szFileName);

    // Set parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 10);

    // Solve the model
    model.Solve();
  }
  catch (CoptException e)
  {
    cout << "Error code = " << e.GetCode() << endl;
    cout << e.what() << endl;
  }
  catch (...)
  {
    cout << "Exception during optimization" << endl;
  }
}

int main(int argc, char* argv[])
{
  // Solve SOCP model with regular cone
  solve_soc();

  // Solve SOCP model with rotated cone
  solve_rsoc();

  // Solve SOCP model from file
  if (argc >= 2)
  {
    solve_mps(argv[1]);
  }

  return 0;
}
