/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

class socp_ex1
{
  static void Solve_SOC()
  {
    try
    {
      // Create COPT environment
      Envr env = new Envr();

      // Create COPT model
      Model model = env.CreateModel("solve_soc");

      /*
       * Add variables
       *
       * minimize: z
       *
       * bnds:
       *  x, y, t free, z non-negative
       */
      double infinity = Copt.Consts.INFINITY;

      Var x = model.AddVar(-infinity, infinity, 0.0, Copt.Consts.CONTINUOUS, "x");
      Var y = model.AddVar(-infinity, infinity, 0.0, Copt.Consts.CONTINUOUS, "y");
      Var z = model.AddVar(0, infinity, 0.0, Copt.Consts.CONTINUOUS, "z");
      Var t = model.AddVar(-infinity, infinity, 0.0, Copt.Consts.CONTINUOUS, "t");

      // Set objective: z
      model.SetObjective(z, Copt.Consts.MINIMIZE);

      /* 
       * Add constraints
       * 
       *    r0: 3*x + y >= 1
       *    c0: z^2 >= x^2 + 2*y^2
       * 
       * c0 is converted to:
       * 
       *    r1: sqrt(2.0)*y - t = 0
       *    c1: z^2 >= x^2 + t^2
       */
      model.AddConstr(3.0 * x + y >= 1.0, "r0");
      model.AddConstr(Math.Sqrt(2.0) * y - t == 0.0, "r1");

      VarArray cvars = new VarArray();
      cvars.PushBack(z);
      cvars.PushBack(x);
      cvars.PushBack(t);
      model.AddCone(cvars, Copt.Consts.CONE_QUAD);

      // Set parameters
      model.SetDblParam(Copt.DblParam.TimeLimit, 10);

      // Solve the model
      model.Solve();

      // Analyze solution
      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.OPTIMAL)
      {
        Console.WriteLine("");
        Console.WriteLine("Objective value: {0}", model.GetDblAttr(Copt.DblAttr.LpObjVal));

        VarArray vars = model.GetVars();
        Console.WriteLine("Variable solution:");
        for (int i = 0; i < vars.Size(); ++i)
        {
          Var var = vars.GetVar(i);
          Console.WriteLine("  {0} = {1}", var.GetName(), var.Get(Copt.DblInfo.Value));
        }
        Console.WriteLine("");
      }
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error code: " + e.GetCode() + ". " + e.Message);
    }
  }

  static void Solve_RSOC()
  {
    try
    {
      // Create COPT environment
      Envr env = new Envr();

      // Create COPT model
      Model model = env.CreateModel("solve_rsoc");
      
      /*
       * Add variables
       *
       * minimize: 1.5*x - 2*y + z
       * 
       * bnds:
       *  0 <= x <= 20
       *  y, z, r >= 0
       *  s, t free
       */
      double infinity = Copt.Consts.INFINITY;

      Var x = model.AddVar(0.0, 20.0, 0.0, Copt.Consts.CONTINUOUS, "x");
      Var y = model.AddVar(0.0, infinity, 0.0, Copt.Consts.CONTINUOUS, "y");
      Var z = model.AddVar(0.0, infinity, 0.0, Copt.Consts.CONTINUOUS, "z");
      Var r = model.AddVar(0.0, infinity, 0.0, Copt.Consts.CONTINUOUS, "r");
      Var s = model.AddVar(-infinity, infinity, 0.0, Copt.Consts.CONTINUOUS, "s");
      Var t = model.AddVar(-infinity, infinity, 0.0, Copt.Consts.CONTINUOUS, "t");

      // Set objective: 1.5*x - 2*y + z
      model.SetObjective(1.5*x - 2*y + z, Copt.Consts.MINIMIZE);
      
      /*
       * Add constraints
       *
       *  r0: 2*x + y >= 2
       *  r1: -x + 2*y <= 6
       *  r2: r = 1
       *  r3: 2.8284271247 * x + 0.7071067811 * y - s = 0
       *  r4: 3.0822070014 * y - t = 0
       *  c0: 2*z*r >= s^2 + t^2
       */
      model.AddConstr(2*x + y >= 2, "r0");
      model.AddConstr(-x + 2 * y <= 6, "r1");
      model.AddConstr(r == 1, "r2");
      model.AddConstr(2.8284271247 * x + 0.7071067811 * y - s == 0, "r3");
      model.AddConstr(3.0822070014 * y - t == 0, "r4");

      VarArray rvars = new VarArray();
      rvars.PushBack(z);
      rvars.PushBack(r);
      rvars.PushBack(s);
      rvars.PushBack(t);
      model.AddCone(rvars, Copt.Consts.CONE_RQUAD);

      // Set parameters
      model.SetDblParam(Copt.DblParam.TimeLimit, 10);

      // Solve the model
      model.Solve();

      // Analyze solution
      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.OPTIMAL)
      {
        Console.WriteLine("");
        Console.WriteLine("Objective value: {0}", model.GetDblAttr(Copt.DblAttr.LpObjVal));

        VarArray vars = model.GetVars();
        Console.WriteLine("Variable solution:");
        for (int i = 0; i < vars.Size(); ++i)
        {
          Var var = vars.GetVar(i);
          Console.WriteLine("  {0} = {1}", var.GetName(), var.Get(Copt.DblInfo.Value));
        }
        Console.WriteLine("");
      }
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error code: " + e.GetCode() + ". " + e.Message);
    }
  }

  static void Solve_MPS(string filename)
  {
    try
    {
      // Create COPT environment
      Envr env = new Envr();

      // Create COPT model
      Model model = env.CreateModel("solve_mps");

      // Read SOCP from MPS file
      model.ReadMps(filename);

      // Set parameters
      model.SetDblParam(Copt.DblParam.TimeLimit, 10);

      // Solve the model
      model.Solve();
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error code: " + e.GetCode() + ". " + e.Message);
    }
  }

  static void Main(string[] args)
  {
    // Solve SOCP with regular cone
    Solve_SOC();

    // Solve SOCP with rotated cone
    Solve_RSOC();

    // Solve SOCP from MPS file
    if (args.Length >= 1)
    {
      Solve_MPS(args[0]);
    }
  }
}
