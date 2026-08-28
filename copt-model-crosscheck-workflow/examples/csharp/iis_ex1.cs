/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/* 
 * We will compute an IIS for the following infeasible LP problem, the problem
 * is 'itest6' test case from netlib-infeas:
 * 
 * Minimize
 *  X2 + X3 + X4
 * Subject To
 *  ROW1: 0.8 X3 + X4 <= 10000
 *  ROW2: X1 <= 90000
 *  ROW3: 2 X6 - X8 <= 10000
 *  ROW4: - X2 + X3 >= 50000
 *  ROW5: - X2 + X4 >= 87000
 *  ROW6: X3 <= 50000
 *  ROW7: - 3 X5 + X7 >= 10000
 *  ROW8: 0.5 X5 + 0.6 X6 <= 300000
 *  ROW9: X2 - 0.05 X3 = 5000
 *  ROW10: X2 - 0.04 X3 - 0.05 X4 = 4500
 *  ROW11: X2 >= 80000
 * END
 */

public class iis_ex1
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("iis_ex1");

      // Add variables
      Var x1 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X1");
      Var x2 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X2");
      Var x3 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X3");
      Var x4 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X4");
      Var x5 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X5");
      Var x6 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X6");
      Var x7 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X7");
      Var x8 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X8");

      // Add constraints
      model.AddConstr(0.8*x3 + x4 <= 10000, "ROW1");
      model.AddConstr(x1 <= 90000, "ROW2");
      model.AddConstr(2*x6 - x8 <= 10000, "ROW3");
      model.AddConstr(-x2 + x3 >= 50000, "ROW4");
      model.AddConstr(-x2 + x4 >= 87000, "ROW5");
      model.AddConstr(x3 <= 50000, "ROW6");
      model.AddConstr(-3*x5 + x7 >= 10000, "ROW7");
      model.AddConstr(0.5*x5 + 0.6*x6 <= 300000, "ROW8");
      model.AddConstr(x2 - 0.05*x3 == 5000, "ROW9");
      model.AddConstr(x2 - 0.04*x3 - 0.05*x4 == 4500, "ROW10");
      model.AddConstr(x2 >= 80000, "ROW11");

      // Set objective
      model.SetObjective(x2 + x3 + x4, Copt.Consts.MINIMIZE);

      // Set parameters
      model.SetDblParam(Copt.DblParam.TimeLimit, 60);

      // Solve the problem
      model.Solve();

      // Compute IIS if problem is infeasible
      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.INFEASIBLE) {
        // Compute IIS
        model.ComputeIIS();

        // Check if IIS is available
        if (model.GetIntAttr(Copt.IntAttr.HasIIS) == 1) {
          ConstrArray cons = model.GetConstrs();
          VarArray vars = model.GetVars();

          // Print variables and constraints in IIS
          Console.WriteLine("\n======================== IIS result ========================");

          for (int iRow = 0; iRow < cons.Size(); ++iRow) {
            Constraint con = cons.GetConstr(iRow);
            if (con.GetLowerIIS() == 1 || con.GetUpperIIS() == 1) {
              Console.WriteLine("  {0}: {1}", con.GetName(), (con.GetLowerIIS() == 1 ? "lower" : "upper"));
            }
          }

          Console.WriteLine("");
          for (int iCol = 0; iCol < vars.Size(); ++iCol) {
            Var var = vars.GetVar(iCol);
            if (var.GetLowerIIS() == 1 || var.GetUpperIIS() == 1) {
              Console.WriteLine("  {0}: {1}", var.GetName(), (var.GetLowerIIS() == 1 ? "lower" : "upper"));
            }
          }

          // Write IIS to file
          Console.WriteLine("");
          model.WriteIIS("iis_ex1.iis");
        }
      }
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error Code = {0}", e.GetCode());
      Console.WriteLine(e.Message);
    }
  }
}
