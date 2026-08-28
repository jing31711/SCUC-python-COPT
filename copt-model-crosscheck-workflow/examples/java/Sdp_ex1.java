/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/**
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

import copt.*;

import java.lang.*;

public class Sdp_ex1 {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("sdp_ex1");

      // Add symmetric matrix C
      int[] rows = {0, 1, 1, 2, 2};
      int[] cols = {0, 0, 1, 1, 2};
      double[] vals = {2.0, 1.0, 2.0, 1.0, 2.0};
      SymMatrix C = model.addSparseMat(3, 5, rows, cols, vals);

      // Add identity matrix A1
      SymMatrix A1 = model.addEyeMat(3);
      // Add ones matrix A2
      SymMatrix A2 = model.addOnesMat(3);

      // Add PSD variable
      PsdVar barX = model.addPsdVar(3, "BAR_X");

      // Add variables
      Var x0 = model.addVar(0.0, copt.Consts.INFINITY, 0, copt.Consts.CONTINUOUS, "x0");
      Var x1 = model.addVar(0.0, copt.Consts.INFINITY, 0, copt.Consts.CONTINUOUS, "x1");
      Var x2 = model.addVar(0.0, copt.Consts.INFINITY, 0, copt.Consts.CONTINUOUS, "x2");

      // Add PSD constraints
      PsdExpr pexpr1 = new PsdExpr(0.0);
      pexpr1.addTerm(barX, A1);
      model.addPsdConstr(pexpr1, copt.Consts.LESS_EQUAL, 0.8, "PSD_R1");

      PsdExpr pexpr2 = new PsdExpr(x1, 1.0);
      pexpr2.addTerm(x2, 1.0);
      pexpr2.addTerm(barX, A2);
      model.addPsdConstr(pexpr2, copt.Consts.EQUAL, 0.6, "PSD_R2");

      // Add linear constraint: x0 + x1 + x2 <= 0.9
      Expr expr = new Expr(0.0);
      expr.addTerm(x0, 1.0);
      expr.addTerm(x1, 1.0);
      expr.addTerm(x2, 1.0);
      model.addConstr(expr, copt.Consts.LESS_EQUAL, 0.9, "r1");

      // Add SOC constraint: x0^2 >= x1^2 + x2^2
      VarArray vars = new VarArray();
      vars.pushBack(x0);
      vars.pushBack(x1);
      vars.pushBack(x2);
      model.addCone(vars, copt.Consts.CONE_QUAD);

      // Set PSD objective
      PsdExpr obj = new PsdExpr(x0, 1.0);
      obj.addTerm(barX, C);
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

          System.out.println("SDP variable " + i + ", flattened by column:");
          System.out.println("Primal solution:");
          for (int j = 0; j < psdLen; ++j) {
            System.out.println("  " + psdVal[j]);
          }
          System.out.println("");
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
        System.out.println("");
        System.out.println("Reduced cost:");
        for (int j = 0; j < nLinCol; ++j) {
          System.out.println("  " + colDual[j]);
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
