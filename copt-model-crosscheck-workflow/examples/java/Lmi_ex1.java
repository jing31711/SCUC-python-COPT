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

import copt.*;

import java.lang.*;

public class Lmi_ex1 {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("Lmi_ex1");

      // Add symmetric matrix C0
      SymMatrix C0 = model.addEyeMat(2);

      // Add symmetric matrix C1
      int[] C1_rows = {1};
      int[] C1_cols = {0};
      double[] C1_vals = {1.0};
      SymMatrix C1 = model.addSparseMat(2, 1, C1_rows, C1_cols, C1_vals);

      // Add symmetric matrix H1
      int[] H1_rows = {1, 1};
      int[] H1_cols = {0, 1};
      double[] H1_vals = {1.0, 5.0};
      SymMatrix H1 = model.addSparseMat(2, 2, H1_rows, H1_cols, H1_vals);

      // Add symmetric matrix H2
      int[] H2_rows = {0, 1};
      int[] H2_cols = {0, 0};
      double[] H2_vals = {5.0, 1.0};
      SymMatrix H2 = model.addSparseMat(2, 2, H2_rows, H2_cols, H2_vals);

      // Add symmetric matrix D1
      int[] D1_rows = {0, 1};
      int[] D1_cols = {0, 1};
      double[] D1_vals = {-1.0, -2.0};
      SymMatrix D1 = model.addSparseMat(2, 2, D1_rows, D1_cols, D1_vals);

      // Add PSD variable
      PsdVar X = model.addPsdVar(2, "BAR_X");

      // Add variables
      Var x0 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0, copt.Consts.CONTINUOUS, "x0");
      Var x1 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0, copt.Consts.CONTINUOUS, "x1");

      // Add PSD constraint
      PsdExpr pexpr = new PsdExpr(0.0);
      pexpr.addTerm(X, C1);
      pexpr.addTerm(x0, -1.0);
      pexpr.addTerm(x1, -2.0);
      model.addPsdConstr(pexpr, copt.Consts.GREATER_EQUAL, 0.0, "PSD_c0");

      // Add LMI constraint
      LmiExpr lmiexpr = new LmiExpr();
      lmiexpr.addTerm(x0, H1);
      lmiexpr.addTerm(x1, H2);
      lmiexpr.setConstant(D1);
      model.addLmiConstr(lmiexpr, "LMI_c0");

      // Set PSD objective
      PsdExpr obj = new PsdExpr(0.0);
      obj.addTerm(X, C0);
      obj.addTerm(x0, 1.0);
      obj.addTerm(x1, 2.0);
      obj.setConstant(3.0);
      model.setPsdObjective(obj, copt.Consts.MINIMIZE);

      // Optimize model
      model.solve();

      // Output solution
      if (model.getIntAttr(copt.IntAttr.LpStatus) == copt.Status.OPTIMAL) {
        System.out.println("");
        System.out.println("Optimal objective value: " + model.getDblAttr(copt.DblAttr.LpObjVal));
        System.out.println("");

        PsdVarArray psdvars = model.getPsdVars();
        for (int i = 0; i < psdvars.size(); ++i) {
          PsdVar psdvar = psdvars.getPsdVar(i);
          int psdLen = psdvar.getLen();

          /* Get flattened SDP primal/dual solution */
          double[] psdVal = psdvar.get(copt.DblInfo.Value);
          double[] psdDual = psdvar.get(copt.DblInfo.Dual);

          System.out.println("SDP variable " + psdvar.getName() + ", flattened by column:");
          System.out.println("Primal solution:");
          for (int j = 0; j < psdLen; ++j) {
            System.out.println("  " + psdVal[j]);
          }
          System.out.println("Dual solution:");
          for (int j = 0; j < psdLen; ++j) {
            System.out.println("  " + psdDual[j]);
          }
          System.out.println("");
        }

        VarArray linvars = model.getVars();
        int nLinCol = linvars.size();

        /* Get solution for non-PSD variables */
        double[] colVal = model.get(copt.DblInfo.Value, linvars);
        double[] colDual = model.get(copt.DblInfo.RedCost, linvars);

        System.out.println("Non-PSD variables:");
        System.out.println("Solution value:");
        for (int j = 0; j < nLinCol; ++j) {
          System.out.println("  " + colVal[j]);
        }
        System.out.println("Reduced cost:");
        for (int j = 0; j < nLinCol; ++j) {
          System.out.println("  " + colDual[j]);
        }
        System.out.println("");

        /* Get activity and dual of PSD constraints */
        PsdConstrArray psdconstrs = model.getPsdConstrs();
        for (int i = 0; i < psdconstrs.size(); ++i)
        {
          PsdConstraint psdconstr = psdconstrs.getPsdConstr(i);
          System.out.println("PSD constraint " + psdconstr.getName() + ":");
          System.out.println("Activity: " + psdconstr.get(copt.DblInfo.Slack));
          System.out.println("Dual:     " + psdconstr.get(copt.DblInfo.Dual));
          System.out.println("");
        }

        /* Get activity and dual of LMI constraints */
        LmiConstrArray lmiconstrs = model.getLmiConstrs();
        for (int i = 0; i < lmiconstrs.size(); ++i)
        {
          LmiConstraint lmiconstr = lmiconstrs.getLmiConstr(i);
          int lmiLen = lmiconstr.getLen();

          double[] lmiSlack = lmiconstr.get(copt.DblInfo.Slack);
          double[] lmiDual = lmiconstr.get(copt.DblInfo.Dual);

          System.out.println("LMI constraint " + lmiconstr.getName() + ", flattened by column:");
          System.out.println("Activity:");
          for (int j = 0; j < lmiLen; ++j)
          {
            System.out.println("  " + lmiSlack[j]);
          }
          System.out.println("Dual:");
          for (int j = 0; j < lmiLen; ++j)
          {
            System.out.println("  " + lmiDual[j]);
          }
          System.out.println("");
        }
      } else {
        System.out.println("No SDP solution!");
      }
    }
    catch (CoptException e) {
      System.out.println("Error Code = " + e.getCode());
      System.out.println(e.getMessage());
    }
  }
}
