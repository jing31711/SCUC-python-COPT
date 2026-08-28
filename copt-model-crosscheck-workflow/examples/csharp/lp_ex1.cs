/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/*
 * This C# example solves the following LP model:
 *
 * 
 * Maximize:
 *  1.2 x + 1.8 y + 2.1 z
 *
 * Subject to:
 *  1.5 x + 1.2 y + 1.8 z <= 2.6
 *  0.8 x + 0.6 y + 0.9 z >= 1.2

 * where:
 *  0.1 <= x <= 0.6
 *  0.2 <= y <= 1.5
 *  0.3 <= z <= 2.8
 */
public class lp_ex1
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("lp_ex1");

      /* 
       * Add variables x, y, z
       *
       * obj: 1.2 x + 1.8 y + 2.1 z
       *
       * var:
       *  0.1 <= x <= 0.6
       *  0.2 <= y <= 1.5
       *  0.3 <= z <= 2.8
       */
      Var x = model.AddVar(0.1, 0.6, 0.0, Copt.Consts.CONTINUOUS, "x");
      Var y = model.AddVar(0.2, 1.5, 0.0, Copt.Consts.CONTINUOUS, "y");
      Var z = model.AddVar(0.3, 2.8, 0.0, Copt.Consts.CONTINUOUS, "z");

      model.SetObjective(1.2 * x + 1.8 * y + 2.1 * z, Copt.Consts.MAXIMIZE);

      /*
       * Add two constraints using linear expression
       *
       * r0: 1.5 x + 1.2 y + 1.8 z <= 2.6
       * r1: 0.8 x + 0.6 y + 0.9 z >= 1.2
       */
      model.AddConstr(1.5 * x + 1.2 * y + 1.8 * z <= 2.6, "r0");

      Expr expr = new Expr(x, 0.8);
      expr.AddTerm(y, 0.6);
      expr += 0.9 * z;
      model.AddConstr(expr >= 1.2, "r1");

      // Set parameters
      model.SetDblParam(Copt.DblParam.TimeLimit, 10);

      // Solve problem
      model.Solve();

      // Output solution
      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.OPTIMAL)
      {
        Console.WriteLine("\nFound optimal solution:");
        VarArray vars = model.GetVars();
        for (int i = 0; i < vars.Size(); i++)
        {
          Var xi = vars.GetVar(i);
          Console.WriteLine("  {0} = {1}", xi.GetName(), xi.Get(Copt.DblInfo.Value));
        }
        Console.WriteLine("Obj = {0}", model.GetDblAttr(Copt.DblAttr.LpObjVal));
      }

      Console.WriteLine("\nDone");
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error Code = {0}", e.GetCode());
      Console.WriteLine(e.Message);
    }
  }
}
