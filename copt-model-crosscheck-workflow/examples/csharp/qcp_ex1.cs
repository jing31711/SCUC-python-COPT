/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/*
 * This C# example solves the following LP model:
 *
 * 
 * Minimize
 *  obj: 2.1 x1 - 1.2 x2 + 3.2 x3 + x4 + x5 + x6 + 2 x7 + [ x2^2 ] / 2
 *  Subject To
 *  r1: x1 + 2 x2 = 6
 *  r2: 2 x1 + x3 >= 5
 *  r3: x6 + 2 x7 <= 7
 *  r4: -x1 + 1.2 x7 >= -2.3
 *  q1: [ -1.8 x1^2 + x2^2 ] <= 0
 *  q2: [ 4.25 x3^2 - 2 x3 * x4 + 4.25 x4^2 - 2 x4 * x5 + 4 x5^2  ] + 2 x1 + 3 x3 <= 9.9
 *  q3: [ x6^2 - 2.2 x7^2 ] >= 5
 * Bounds
 *  0.2 <= x1 <= 3.8
 *  x2 Free
 *  0.1 <= x3 <= 0.7
 *  x4 Free
 *  x5 Free
 *  x7 Free
 */
public class qcp_ex1
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("qcp_ex1");

      // Add variables
      Var x1 = model.AddVar(0.2, 3.8, 0.0, Copt.Consts.CONTINUOUS, "x1");
      Var x2 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "x2");
      Var x3 = model.AddVar(0.1, 0.7, 0.0, Copt.Consts.CONTINUOUS, "x3");
      Var x4 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "x4");
      Var x5 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "x5");
      Var x6 = model.AddVar(0, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "x6");
      Var x7 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "x7");

      // Add linear constraints
      model.AddConstr(x1 + 2*x2 == 6, "r1");
      model.AddConstr(2*x1 + x3 >= 5, "r2");
      model.AddConstr(x6 + 2*x7 <= 7, "r3");
      model.AddConstr(-x1 + 1.2*x7 >= -2.3, "r4");

      // Add quadratic constraints
      model.AddQConstr(-1.8*x1*x1 + x2*x2 <= 0, "q1");
      model.AddQConstr(4.25*x3*x3 - 2*x3*x4 + 4.25*x4*x4 - 2*x4*x5 + 4*x5*x5 + 2*x1 + 3*x3 <= 9.9, "q2");
      model.AddQConstr(x6*x6 - 2.2*x7*x7 >= 5, "q3");

      // Set quadratic objective
      QuadExpr obj = 2.1*x1 - 1.2*x2 + 3.2*x3 + x4 + x5 + x6 + 2*x7 + 0.5*x2*x2;
      model.SetQuadObjective(obj, Copt.Consts.MINIMIZE);

      // Set parameters
      model.SetDblParam(Copt.DblParam.TimeLimit, 60);

      // Solve the problem
      model.Solve();

      // Output solution
      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.OPTIMAL) {
        Console.WriteLine("\nOptimal objective value: {0}", model.GetDblAttr(Copt.DblAttr.LpObjVal));

        Console.WriteLine("Variable solution: ");
        VarArray vars = model.GetVars();
        for (int iCol = 0; iCol < vars.Size(); ++iCol) {
          Var var = vars.GetVar(iCol);
          Console.WriteLine("  {0}: {1}", var.GetName(), var.Get(Copt.DblInfo.Value));
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
