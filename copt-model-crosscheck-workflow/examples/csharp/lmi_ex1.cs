/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example solves the mixed SDP problem:
 *
 *                  [1,  0]
 *  minimize     Tr [     ] * X + x0 + 2 * x1 + 3
 *                  [0,  1]
 *
 *                  [0,  1]
 *  subject to   Tr [     ] * X - x0 - 2 * x1 >= 0
 *                  [1,  0]
 *
 *                  [0,  1]        [5,  1]   [1,  0]
 *             x0 * [     ] + x1 * [     ] - [     ] in PSD
 *                  [1,  5]        [1,  0]   [0,  2]
 *
 *             x0, x1 free, X in PSD
 */

using Copt;
using System;

public class lmi_ex1
{
  public static void Main()
  {
    try
    {
      Envr env = new Envr();
      Model model = env.CreateModel("lmi_ex1");

      // Add symmetric matrix C0
      SymMatrix C0 = model.AddEyeMat(2);

      // Add symmetric matrix C1
      int[] C1_rows = {1};
      int[] C1_cols = {0};
      double[] C1_vals = {1.0};
      SymMatrix C1 = model.AddSparseMat(2, 1, C1_rows, C1_cols, C1_vals);

      // Add symmetric matrix H1
      int[] H1_rows = {1, 1};
      int[] H1_cols = {0, 1};
      double[] H1_vals = {1.0, 5.0};
      SymMatrix H1 = model.AddSparseMat(2, 2, H1_rows, H1_cols, H1_vals);

      // Add symmetric matrix H2
      int[] H2_rows = {0, 1};
      int[] H2_cols = {0, 0};
      double[] H2_vals = {5.0, 1.0};
      SymMatrix H2 = model.AddSparseMat(2, 2, H2_rows, H2_cols, H2_vals);

      // Add symmetric matrix D1
      int[] D1_rows = {0, 1};
      int[] D1_cols = {0, 1};
      double[] D1_vals = {1.0, 2.0};
      SymMatrix D1 = model.AddSparseMat(2, 2, D1_rows, D1_cols, D1_vals);

      // Add PSD variable
      PsdVar X = model.AddPsdVar(2, "BAR_X");

      // Add variables
      Var x0 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0, Copt.Consts.CONTINUOUS, "x0");
      Var x1 = model.AddVar(-Copt.Consts.INFINITY, +Copt.Consts.INFINITY, 0, Copt.Consts.CONTINUOUS, "x1");

      // Add PSD constraint
      model.AddPsdConstr(C1 * X - x0 - 2 * x1 >= 0, "PSD_c0");

      // Add LMI constraint
      model.AddLmiConstr(H1 * x0 + H2 * x1 - D1, "LMI_c0");

      // Set PSD objective
      model.SetPsdObjective(C0 * X + x0 + 2 * x1 + 3, Copt.Consts.MINIMIZE);

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

          Console.WriteLine("SDP variable {0}, flattened by column:", psdvar.GetName());
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
        Console.WriteLine("");

        /* Get activity and dual of PSD constraints */
        PsdConstrArray psdconstrs = model.GetPsdConstrs();
        for (int i = 0; i < psdconstrs.Size(); ++i)
        {
          PsdConstraint psdconstr = psdconstrs.GetPsdConstr(i);
          Console.WriteLine("PSD constraint {0}:", psdconstr.GetName());
          Console.WriteLine("Activity: {0}", psdconstr.Get(Copt.DblInfo.Slack));
          Console.WriteLine("Dual:     {0}", psdconstr.Get(Copt.DblInfo.Dual));
          Console.WriteLine("");
        }

        /* Get activity and dual of LMI constraints */
        LmiConstrArray lmiconstrs = model.GetLmiConstrs();
        for (int i = 0; i < lmiconstrs.Size(); ++i)
        {
          LmiConstraint lmiconstr = lmiconstrs.GetLmiConstr(i);
          int lmiLen = lmiconstr.GetLen();

          double[] lmiSlack = lmiconstr.Get(Copt.DblInfo.Slack);
          double[] lmiDual = lmiconstr.Get(Copt.DblInfo.Dual);

          Console.WriteLine("LMI constraint {0}, flattened by column:", lmiconstr.GetName());
          Console.WriteLine("Activity:");
          for (int j = 0; j < lmiLen; ++j)
          {
            Console.WriteLine("  {0}", lmiSlack[j]);
          }
          Console.WriteLine("Dual:");
          for (int j = 0; j < lmiLen; ++j)
          {
            Console.WriteLine("  {0}", lmiDual[j]);
          }
          Console.WriteLine("");
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
