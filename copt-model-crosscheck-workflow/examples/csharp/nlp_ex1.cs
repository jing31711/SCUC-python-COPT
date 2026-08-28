/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/*
 * This C# example solves the following nonlinear problem:
 *
 * 
 * Minimize:
 *  x1 * x4 * (sin(x1 + x2) + cos(x2 * x3) + tan(x3 / x4)) + x3
 *
 * Subject to:
 *  x1 * x2 * x3 * x4 + x1 + x2 >= 35
 *  log(x1) + 2 * log(x2) + 3 * log(x3) + 4 * log(x4) + x3 + x4 >= 15
 *  x1^2 + x2^2 + x3^2 + x4^2 + x1 + x3 >= 50

 * where:
 *  1 <= x1 <= 5
 *  1 <= x2 <= 5
 *  1 <= x3 <= 5
 *  1 <= x4 <= 5
 */
public class Nlp_ex1
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("nlp_ex1");

      // Add variables x1, x2, x3, x4
      Var x1 = model.AddVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x1");
      Var x2 = model.AddVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x2");
      Var x3 = model.AddVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x3");
      Var x4 = model.AddVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x4");

      // Add non-linear constraints
      model.AddNlConstr(x1 * x2 * x3 * x4 + x1 + x2 >= 35, "nlrow1");
      model.AddNlConstr(NL.Log(x1) + 2 * NL.Log(x2) + 3 * NL.Log(x3) + 4 * NL.Log(x4) + x3 + x4 >= 15, "nlrow2");

      // Add a quadratic constraint
      model.AddQConstr(x1 * x1 + x2 * x2 + x3 * x3 + x4 * x4 + x1 + x3 >= 50, "qrow1");

      // Set non-linear objective
      NlExpr obj = x1 * x4 * NL.Sum(NL.Sin(x1 + x2), NL.Cos(x2 * x3), NL.Tan(x3 / x4)) + x3;
      model.SetNlObjective(obj, Consts.MINIMIZE);

      // Set parameters
      model.SetDblParam(DblParam.TimeLimit, 60);

      // Solve problem
      model.Solve();

      // Output solution
      Console.WriteLine("");
      if (model.GetIntAttr(IntAttr.LpStatus) == Status.OPTIMAL)
      {
        Console.WriteLine("Optimal objective value = {0}", model.GetDblAttr(DblAttr.LpObjVal));

        VarArray vars = model.GetVars();
        for (int i = 0; i < vars.Size(); i++)
        {
          Var xi = vars[i];
          Console.WriteLine("  {0} = {1}", xi.GetName(), xi.Get(DblInfo.Value));
        }
      }
      else
      {
        Console.WriteLine("Error: no solution available!");
      }
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error Code = {0}", e.GetCode());
      Console.WriteLine(e.Message);
    }
  }
}
