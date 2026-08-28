/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/*
 * Production Planning Problem:
 *   A company manufactures four variants of the same product and in the final part of
 *   the manufacturing process there are assembly, polishing and packing operations.
 *   For each variant the time required for these operations is shown below (in minutes)
 *   as is the profit per unit sold.
 *
 *     Variant   Assembly      Polish    Pack        Profit ($)
 *         1       2             3         2           1.50
 *         2       4             2         3           2.50
 *         3       3             3         2           3.00
 *         4       7             4         5           4.50
 *
 *   Let Xi be the number of units of variant i (i=1, 2, 3, 4) made per year.
 *
 * Objectives:
 *   Maximize total profit
 *   1.5*X1 + 2.5*X2 + 3.0*X3 + 4.5*X4
 *
 * Subject to:
 *   (Assembly time) 2*X1 + 4*X2 + 3*X3 + 7*X4 <= 100000
 *   (Polish time)   3*X1 + 2*X2 + 3*X3 + 4*X4 <= 50000
 *   (Pack time)     2*X1 + 3*X2 + 2*X3 + 5*X4 <= 60000
 */
public class Sensitivity_Analysis
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("Production Planning Problem");

      // Set parameter to enable sensitivity analysis
      model.SetIntParam(Copt.IntParam.ReqSensitivity, 1);

      // Add variable Xi, the number of units of variant i of the same product
      Var x1 = model.AddVar(0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X1");
      Var x2 = model.AddVar(0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X2");
      Var x3 = model.AddVar(0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X3");
      Var x4 = model.AddVar(0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X4");

      // The total assembly time should be less than 100000 miniutes
      model.AddConstr(2 * x1 + 4 * x2 + 3 * x3 + 7 * x4 <= 100000, "AssemblyTime");
      // The total polishing time should be less than 50000 miniutes
      model.AddConstr(3 * x1 + 2 * x2 + 3 * x3 + 4 * x4 <= 50000, "PolishTime");
      // The total packing time should be less than 60000 miniutes
      model.AddConstr(2 * x1 + 3 * x2 + 2 * x3 + 5 * x4 <= 60000, "PackTime");

      // The objective is to maximize total profit
      model.SetObjective(1.5 * x1 + 2.5 * x2 + 3.0 * x3 + 4.5 * x4, Copt.Consts.MAXIMIZE);

      // Solve problem
      model.Solve();

      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.OPTIMAL)
      {
        if (model.GetIntAttr(Copt.IntAttr.HasSensitivity) == 1)
        {
          Console.WriteLine("");
          Console.WriteLine("Sensitivity information of production planning problem:");
          PrintSensitivity(model);
        } else {
            Console.WriteLine("Sensitivity information is not available");
        }
      } else {
        Console.WriteLine("Optimal solution is not available, with status {0}", model.GetIntAttr(Copt.IntAttr.LpStatus));
      }
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error Code = {0}", e.GetCode());
      Console.WriteLine(e.Message);
    }
  }

  static void PrintSensitivity(Model model)
  {
    VarArray vars = model.GetVars();
    ConstrArray cons = model.GetConstrs();

    Console.WriteLine("");
    Console.WriteLine("Variant\tObjective Range");
    for (int i = 0; i < vars.Size(); ++i)
    {
        double objLow = vars[i].Get(Copt.DblInfo.SAObjLow);
        double objUpp = vars[i].Get(Copt.DblInfo.SAObjUp);
        Console.WriteLine("  {0}\t({1}, {2})", vars[i].GetName(), objLow, objUpp);
    }

    Console.WriteLine("");
    Console.WriteLine("Variant\tLowBound Range \tUppBound Range");
    for (int i = 0; i < vars.Size(); ++i)
    {
        double lbLow = vars[i].Get(Copt.DblInfo.SALBLow);
        double lbUpp = vars[i].Get(Copt.DblInfo.SALBUp);
        double ubLow = vars[i].Get(Copt.DblInfo.SAUBLow);
        double ubUpp = vars[i].Get(Copt.DblInfo.SAUBUp);
        Console.WriteLine("  {0}\t({1}, {2})\t({3}, {4})", vars[i].GetName(), lbLow, lbUpp, ubLow, ubUpp);
    }

    Console.WriteLine("");
    Console.WriteLine("Constraint\tLHS Range     \tRHS Range");
    for (int i = 0; i < cons.Size(); ++i)
    {
        double lbLow = cons[i].Get(Copt.DblInfo.SALBLow);
        double lbUpp = cons[i].Get(Copt.DblInfo.SALBUp);
        double ubLow = cons[i].Get(Copt.DblInfo.SAUBLow);
        double ubUpp = cons[i].Get(Copt.DblInfo.SAUBUp);
        Console.WriteLine("  {0}\t({1}, {2})\t({3}, {4})", cons[i].GetName(), lbLow, lbUpp, ubLow, ubUpp);
    }
  }

}
