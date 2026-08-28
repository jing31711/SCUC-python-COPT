/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/*
 * Solve the following Geometric Programming problem with exponential cone:
 *
 * maximize: x + y + z
 *
 * s.t.  log[exp(x + y + log(2 / Aw)) + exp(x + z + log(2 / Aw))] <= 0
 *             -inf <= y + z <= log(Af)
 *       log(alpha) <= x - y <= log(beta)
 *       log(gamma) <= z - y <= log(delta)
 *
 * where:
 *             -inf <= x <= +inf
 *             -inf <= y <= +inf
 *             -inf <= z <= +inf
 *
 * reformulated as exponential cone problem:
 *
 * maximize: x + y + z
 *
 * s.t.  u1 >= exp(u3)
 *       u3 == x + y + log(2 / Aw)
 *       v1 >= exp(v3)
 *       v3 = x + z + log(2 / Aw)
 *       u1 + v1 == 1.0
 *             -inf <= y + z <= log(Af)
 *       log(alpha) <= x - y <= log(beta)
 *       log(gamma) <= z - y <= log(delta)
 *
 * where:
 *             -inf <= x <= +inf
 *             -inf <= y <= +inf
 *             -inf <= z <= +inf
 *  (u1, u2, u3) \in Kexp, (v1, v2, v3) \in Kexp, u2 == 1.0, v2 == 1.0
 */
public class expcone_gp
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("expcone_gp");

      double Aw = 200.0;
      double Af = 50.0;
      double alpha = 2.0;
      double beta = 10.0;
      double gamma = 2.0;
      double delta = 10.0;

      /*
       * Add variables
       *   obj: 1.0 x + 1.0 y + 1.0 z
       *
       *   bnd:
       *     -inf <= x  <= +inf
       *     -inf <= y  <= +inf
       *     -inf <= z  <= +inf
       *     -inf <= u1 <= +inf
       *             u2 == 1
       *     -inf <= u3 <= +inf
       *     -inf <= v1 <= +inf
       *             v2 == 1
       *     -inf <= v3 <= +inf
       */
      Var x = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 1.0, Copt.Consts.CONTINUOUS, "x");
      Var y = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 1.0, Copt.Consts.CONTINUOUS, "y");
      Var z = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 1.0, Copt.Consts.CONTINUOUS, "z");

      Var u1 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "u1");
      Var u2 = model.AddVar(1.0, 1.0, 0.0, Copt.Consts.CONTINUOUS, "u2");
      Var u3 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "u3");

      Var v1 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "v1");
      Var v2 = model.AddVar(1.0, 1.0, 0.0, Copt.Consts.CONTINUOUS, "v2");
      Var v3 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "v3");

      // Add constraint: u1 >= exp(x + y + log(2/Aw)
      model.AddConstr(u3 == x + y + Math.Log(2.0 / Aw));

      VarArray uconevars = new VarArray();
      uconevars.PushBack(u1);
      uconevars.PushBack(u2);
      uconevars.PushBack(u3);
      model.AddExpCone(uconevars, Copt.Consts.EXPCONE_PRIMAL);

      // Add constraint: v1 >= exp(x + z + log(2/Aw)
      model.AddConstr(v3 == x + z + Math.Log(2.0 / Aw));

      VarArray vconevars = new VarArray();
      vconevars.PushBack(v1);
      vconevars.PushBack(v2);
      vconevars.PushBack(v3);
      model.AddExpCone(vconevars, Copt.Consts.EXPCONE_PRIMAL);

      // Add constraint: u1 + v1 == 1.0
      model.AddConstr(u1 + v1 == 1.0);

      /*
       * Add constraints:
       *         -inf <= y + z <= log(Af)
       *   log(alpha) <= x - y <= log(beta)
       *   log(gamma) <= z - y <= log(delta)
       */
      model.AddConstr(y + z <= Math.Log(Af));
      model.AddConstr(x - y, Math.Log(alpha), Math.Log(beta));
      model.AddConstr(z - y, Math.Log(gamma), Math.Log(delta));

      // Set objective sense
      model.SetObjSense(Copt.Consts.MAXIMIZE);

      // Solve the model
      model.Solve();

      // Analyze solution
      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.OPTIMAL)
      {
        Console.WriteLine("");
        Console.WriteLine("Objective value: {0}", model.GetDblAttr(Copt.DblAttr.LpObjVal));

        Console.WriteLine("Variable solution:");
        Console.WriteLine("  {0} = {1}", x.GetName(), x.Get(Copt.DblInfo.Value));
        Console.WriteLine("  {0} = {1}", y.GetName(), y.Get(Copt.DblInfo.Value));
        Console.WriteLine("  {0} = {1}", z.GetName(), z.Get(Copt.DblInfo.Value));
        Console.WriteLine("");
      }
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error code: " + e.GetCode() + ". " + e.Message);
    }
  }
}
