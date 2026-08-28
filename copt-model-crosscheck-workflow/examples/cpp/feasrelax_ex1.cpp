/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example computes feasibility relaxation for the following infeasible
 * LP problem, which is from 'itest6' test case in netlib-infeas.
 *
 *   Minimize:
 *             x2 + x3 + x4
 *
 *   Subject To:
 *    R1:  0.8 x3 + x4  <= 10000
 *    R2:      x1       <= 90000
 *    R3:    2 x6 - x8  <= 10000
 *    R4:     -x2 + x3  >= 50000
 *    R5:     -x2 + x4  >= 87000
 *    R6:      x3       <= 50000
 *    R7:   -3 x5 + x7  >= 10000
 *    R8:  0.5 x5 + 0.6  x6           <= 300000
 *    R9:      x2 - 0.05 x3            = 5000
 *    R10:     x2 - 0.04 x3 - 0.05 x4  = 4500
 *    R11:     x2                     >= 80000
 *
 */
#include <iostream>
#include "coptcpp_pch.h"

using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("feasrelax_ex1");

    // Add variables
    Var x1 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x1");
    Var x2 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x2");
    Var x3 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x3");
    Var x4 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x4");
    Var x5 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x5");
    Var x6 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x6");
    Var x7 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x7");
    Var x8 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "x8");

    // Add constraints
    model.AddConstr(0.8 * x3 + x4 <= 10000, "R1");
    model.AddConstr(x1 <= 90000, "R2");
    model.AddConstr(2 * x6 - x8 <= 10000, "R3");
    model.AddConstr(-x2 + x3 >= 50000, "R4");
    model.AddConstr(-x2 + x4 >= 87000, "R5");
    model.AddConstr(x3 <= 50000, "R6");
    model.AddConstr(-3 * x5 + x7 >= 10000, "R7");
    model.AddConstr(0.5 * x5 + 0.6 * x6 <= 300000, "R8");
    model.AddConstr(x2 - 0.05 * x3 == 5000, "R9");
    model.AddConstr(x2 - 0.04 * x3 - 0.05 * x4 == 4500, "R10");
    model.AddConstr(x2 >= 80000, "R11");

    // Set objective
    model.SetObjective(x2 + x3 + x4, COPT_MINIMIZE);

    // Set parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 60);

    // Solve the problem
    model.Solve();

    // Compute feasibility relaxation if problem is infeasible
    if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_INFEASIBLE)
    {
      // Set feasibility relaxation mode: minimize sum of violations
      model.SetIntParam(COPT_INTPARAM_FEASRELAXMODE, 0);

      // Compute feasibility relaxation
      model.FeasRelax(1, 1);

      // Check if feasibility relaxation solution is available
      if (model.GetIntAttr(COPT_INTATTR_HASFEASRELAXSOL))
      {
        ConstrArray cons = model.GetConstrs();
        VarArray vars = model.GetVars();

        // Retrieve feasibility relaxation result
        cout << "\n======================== FeasRelax result ========================" << endl;

        for (int iRow = 0; iRow < cons.Size(); ++iRow)
        {
          Constraint con = cons.GetConstr(iRow);
          double lowRelax = con.Get(COPT_DBLINFO_RELAXLB);
          double uppRelax = con.Get(COPT_DBLINFO_RELAXUB);
          if (lowRelax != 0.0 || uppRelax != 0.0)
          {
            cout << "  " << con.GetName() << ": "
                 << "viol = "
                 << "(" << lowRelax << ", " << uppRelax << ")" << endl;
          }
        }

        cout << endl;
        for (int iCol = 0; iCol < vars.Size(); ++iCol)
        {
          Var var = vars.GetVar(iCol);
          double lowRelax = var.Get(COPT_DBLINFO_RELAXLB);
          double uppRelax = var.Get(COPT_DBLINFO_RELAXUB);
          if (lowRelax != 0.0 || uppRelax != 0.0)
          {
            cout << "  " << var.GetName() << ": "
                 << "viol = "
                 << "(" << lowRelax << ", " << uppRelax << ")" << endl;
          }
        }

        // Write relaxed problem to file
        model.WriteRelax("feasrelax_ex1.relax");
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
