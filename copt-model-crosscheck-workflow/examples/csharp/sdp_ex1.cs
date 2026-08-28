/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 *                 [2, 1, 0]   
 *  minimize    Tr [1, 2, 1] * X + x0
 *                 [0, 1, 2]
 * 
 *                 [1, 0, 0]
 *  subject to  Tr [0, 1, 0] * X <= 0.8
 *                 [0, 0, 1]
 * 
 *                 [1, 1, 1]
 *              Tr [1, 1, 1] * X + x1 + x2 = 0.6
 *                 [1, 1, 1]
 *
 *              x0 + x1 + x2 <= 0.9
 *              x0 >= (x1^2 + x2^2) ^ (1/2)
 * 
 *    x0, x1, x2 non-negative, X in PSD
 */

using Copt;
using System;

public class sdp_ex1
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("sdp_ex1");

      // Add symmetric matrix C
      int[] rows = {0, 1, 1, 2, 2};
      int[] cols = {0, 0, 1, 1, 2};
      double[] vals = {2.0, 1.0, 2.0, 1.0, 2.0};
      SymMatrix C = model.AddSparseMat(3, 5, rows, cols, vals);

      // Add identity matrix A1
      SymMatrix A1 = model.AddEyeMat(3);
      // Add ones matrix A2
      SymMatrix A2 = model.AddOnesMat(3);

      // Add PSD variable
      PsdVar barX = model.AddPsdVar(3, "BAR_X");

      // Add variables
      Var x0 = model.AddVar(0.0, Copt.Consts.INFINITY, 0, Copt.Consts.CONTINUOUS, "x0");
      Var x1 = model.AddVar(0.0, Copt.Consts.INFINITY, 0, Copt.Consts.CONTINUOUS, "x1");
      Var x2 = model.AddVar(0.0, Copt.Consts.INFINITY, 0, Copt.Consts.CONTINUOUS, "x2");

      // Add PSD constraints
      model.AddPsdConstr(A1 * barX <= 0.8, "PSD_R1");
      model.AddPsdConstr(A2 * barX + x1 + x2 == 0.6, "PSD_R2");

      // Add linear constraint: x0 + x1 + x2 <= 0.9
      model.AddConstr(x0 + x1 + x2 <= 0.9, "r1");

      // Add SOC constraint: x0^2 >= x1^2 + x2^2
      VarArray vars = new VarArray();
      vars.PushBack(x0);
      vars.PushBack(x1);
      vars.PushBack(x2);
      model.AddCone(vars, Copt.Consts.CONE_QUAD);

      // Set PSD objective
      model.SetPsdObjective(C * barX + x0, Copt.Consts.MINIMIZE);

      // Optimize model
      model.Solve();

      // Output solution
      if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.OPTIMAL) {
        Console.WriteLine("");
        Console.WriteLine("Optimal objective value: {0}", model.GetDblAttr(Copt.DblAttr.LpObjVal));
        Console.WriteLine("");

        PsdVarArray psdvars = model.GetPsdVars();
        for (int i = 0; i < psdvars.Size(); ++i) {
          PsdVar psdvar = psdvars.GetPsdVar(i);
          int psdLen = psdvar.GetLen();

          /* Get flattened SDP primal/dual solution */
          double[] psdVal = psdvar.Get(Copt.DblInfo.Value);
          double[] psdDual = psdvar.Get(Copt.DblInfo.Dual);

          Console.WriteLine("SDP variable {0}, flattened by column:", i);
          Console.WriteLine("Primal solution:");
          for (int j = 0; j < psdLen; ++j) {
            Console.WriteLine("  {0}", psdVal[j]);
          }
          Console.WriteLine("");
          Console.WriteLine("Dual solution:");
          for (int j = 0; j < psdLen; ++j) {
            Console.WriteLine("  {0}", psdDual[j]);
          }
          Console.WriteLine("");
        }

        VarArray linvars = model.GetVars();
        int nLinCol = linvars.Size();

        /* Get solution for non-PSD variables */
        double[] colVal = model.Get(Copt.DblInfo.Value, linvars);
        double[] colDual = model.Get(Copt.DblInfo.RedCost, linvars);

        Console.WriteLine("Non-PSD variables:");
        Console.WriteLine("Solution value:");
        for (int j = 0; j < nLinCol; ++j) {
          Console.WriteLine("  {0}", colVal[j]);
        }
        Console.WriteLine("");
        Console.WriteLine("Reduced cost:");
        for (int j = 0; j < nLinCol; ++j) {
          Console.WriteLine("  {0}", colDual[j]);
        }
      } else {
        Console.WriteLine("No SDP solution!");
      }
    }
    catch (CoptException e) {
      Console.WriteLine("Error Code = {0}", e.GetCode());
      Console.WriteLine(e.Message);
    }
  }
}
