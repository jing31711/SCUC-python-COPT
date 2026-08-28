/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/* Minimize
 *   OBJ.FUNC: [ 2 X0 ^2 + 4 X0 * X1 + 4 X1 ^2
 *             + 4 X1 * X2 + 4 X2 ^2
 *             + 4 X2 * X3 + 4 X3 ^2
 *             + 4 X3 * X4 + 4 X4 ^2
 *             + 4 X4 * X5 + 4 X5 ^2
 *             + 4 X5 * X6 + 4 X6 ^2
 *             + 4 X6 * X7 + 4 X7 ^2
 *             + 4 X7 * X8 + 4 X8 ^2
 *             + 4 X8 * X9 + 2 X9 ^2 ] / 2
 *  Subject To
 *   ROW0: X0 + 2 X1 + 3 X2  = 1
 *   ROW1: X1 + 2 X2 + 3 X3  = 1
 *   ROW2: X2 + 2 X3 + 3 X4  = 1
 *   ROW3: X3 + 2 X4 + 3 X5  = 1
 *   ROW4: X4 + 2 X5 + 3 X6  = 1
 *   ROW5: X5 + 2 X6 + 3 X7  = 1
 *   ROW6: X6 + 2 X7 + 3 X8  = 1
 *   ROW7: X7 + 2 X8 + 3 X9  = 1
 *  Bounds
 *      X0 Free
 *      X1 Free
 *      X2 Free
 *      X3 Free
 *      X4 Free
 *      X5 Free
 *      X6 Free
 *      X7 Free
 *      X8 Free
 *      X9 Free
 * End
 */
public class qp_ex1
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("qp_ex1");

      // Add variables
      Var x0 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X0");
      Var x1 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X1");
      Var x2 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X2");
      Var x3 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X3");
      Var x4 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X4");
      Var x5 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X5");
      Var x6 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X6");
      Var x7 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X7");
      Var x8 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X8");
      Var x9 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X9");

      // Add constraints
      model.AddConstr(x0 + 2*x1 + 3*x2 == 1, "ROW0");
      model.AddConstr(x1 + 2*x2 + 3*x3 == 1, "ROW1");
      model.AddConstr(x2 + 2*x3 + 3*x4 == 1, "ROW2");
      model.AddConstr(x3 + 2*x4 + 3*x5 == 1, "ROW3");
      model.AddConstr(x4 + 2*x5 + 3*x6 == 1, "ROW4");
      model.AddConstr(x5 + 2*x6 + 3*x7 == 1, "ROW5");
      model.AddConstr(x6 + 2*x7 + 3*x8 == 1, "ROW6");
      model.AddConstr(x7 + 2*x8 + 3*x9 == 1, "ROW7");

      // Set quadratic objective
      QuadExpr obj = new QuadExpr(0.0);
      obj += x0*x0 + x9*x9;
      obj += 2*x1*x1 + 2*x2*x2 + 2*x3*x3 + 2*x4*x4;
      obj += 2*x5*x5 + 2*x6*x6 + 2*x7*x7 + 2*x8*x8;
      obj += 2*x0*x1 + 2*x1*x2 + 2*x2*x3 + 2*x3*x4 + 2*x4*x5;
      obj += 2*x5*x6 + 2*x6*x7 + 2*x7*x8 + 2*x8*x9;
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
