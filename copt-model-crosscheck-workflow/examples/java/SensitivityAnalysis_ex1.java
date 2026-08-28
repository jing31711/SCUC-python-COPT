 /*
  * This file is part of the Cardinal Optimizer, all rights reserved.
  */

import copt.*;
import java.lang.*;

/*
 *  Production Planning Problem:
 *  A company manufactures four variants of the same product and in the final part of
 *  the manufacturing process there are assembly, polishing and packing operations.
 *  For each variant the time required for these operations is shown below (in minutes)
 *  as is the profit per unit sold.
 *
 *  Variant   Assembly      Polish    Pack        Profit ($)
 *  1       2             3         2           1.50
 *  2       4             2         3           2.50
 *  3       3             3         2           3.00
 *  4       7             4         5           4.50
 *
 *  Let Xi be the number of units of variant i (i=1, 2, 3, 4) made per year.
 *
 *  Objectives:
 *  Maximize total profit
 *  1.5*X1 + 2.5*X2 + 3.0*X3 + 4.5*X4
 *
 *  Subject to:
 *  (Assembly time) 2*X1 + 4*X2 + 3*X3 + 7*X4 <= 100000
 *  (Polish time)   3*X1 + 2*X2 + 3*X3 + 4*X4 <= 50000
 *  (Pack time)     2*X1 + 3*X2 + 2*X3 + 5*X4 <= 60000
 */

public class SensitivityAnalysis_ex1 {

  public static void main(String[] args) {
    try {
      // Create COPT environment
      Envr env = new Envr();

      // Create COPT model and enable sensitivity analysis
      Model model = env.createModel("Production Planning Problem");
      // Set parameters
      model.setIntParam(copt.IntParam.ReqSensitivity, 1);

      /*
       * Add variable Xi, the number of units of variant i of the same product
       */
      Var x1 = model.addVar(0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X1");
      Var x2 = model.addVar(0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X2");
      Var x3 = model.addVar(0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X3");
      Var x4 = model.addVar(0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X4");

      /*
       * Add constraints
       *
       *  AssemblyTime  2*X1 + 4*X2 + 3*X3 + 7*X4 <= 100000
       *  PolishTime    3*X1 + 2*X2 + 3*X3 + 4*X4 <= 50000
       *  PackTime      2*X1 + 3*X2 + 2*X3 + 5*X4 <= 60000
       *
       */
      Expr expr = new Expr();
      expr.addTerm(x1, 2.0);
      expr.addTerm(x2, 4.0);
      expr.addTerm(x3, 3.0);
      expr.addTerm(x4, 7.0);
      // The total assembly time should be less than 100000 miniutes
      model.addConstr(expr, copt.Consts.LESS_EQUAL, 100000.0, "AssemblyTime");

      expr = new Expr();
      expr.addTerm(x1, 3.0);
      expr.addTerm(x2, 2.0);
      expr.addTerm(x3, 3.0);
      expr.addTerm(x4, 4.0);
      // The total polishing time should be less than 50000 miniutes
      model.addConstr(expr, copt.Consts.LESS_EQUAL, 50000.0, "PolishTime");

      expr = new Expr();
      expr.addTerm(x1, 2.0);
      expr.addTerm(x2, 3.0);
      expr.addTerm(x3, 2.0);
      expr.addTerm(x4, 5.0);
      // The total packing time should be less than 60000 miniutes
      model.addConstr(expr, copt.Consts.LESS_EQUAL, 60000.0, "PackTime");

      // Set objective: Maximize total profit
      //  1.5*X1 + 2.5*X2 + 3.0*X3 + 4.5*X4
      expr = new Expr();
      expr.addTerm(x1, 1.5);
      expr.addTerm(x2, 2.5);
      expr.addTerm(x3, 3.0);
      expr.addTerm(x4, 4.5);
      model.setObjective(expr, copt.Consts.MAXIMIZE);

      // Solve the model
      model.solve();

      // Show the results of sensitivity analysis
      if (model.getIntAttr(copt.IntAttr.LpStatus) == copt.Status.OPTIMAL) {
        if (model.getIntAttr(copt.IntAttr.HasSensitivity) == 1) {
          System.out.println("");
          System.out.println("Sensitivity information of production planning problem:");
          printSensitivity(model);
        } else {
          System.out.println("Sensitivity information is not available");
        }
      } else {
        System.out.println("Optimal solution is not available, with status" + model.getIntAttr(copt.IntAttr.LpStatus));
      }
    } catch (CoptException e) {
      System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
    }
  }

  public static void printSensitivity(Model model) throws CoptException {
    VarArray vars = model.getVars();
    ConstrArray cons = model.getConstrs();

    System.out.println("");
    System.out.println("Variant\tObjective Range");
    for (int i = 0; i < vars.size(); i++) {
      Var xi = vars.getVar(i);
      System.out.println(String.format("  %s\t(%e, %e)",
              xi.getName(), xi.get(copt.DblInfo.SAObjLow), xi.get(copt.DblInfo.SAObjUp)));
    }

    System.out.println("");
    System.out.println("Variant\tLowBound Range               \tUppBound Range");
    for (int i = 0; i < vars.size(); i++) {
      Var xi = vars.getVar(i);
      System.out.println(String.format("  %s\t(%e, %e)\t(%e, %e)",
              xi.getName(), xi.get(copt.DblInfo.SALBLow), xi.get(copt.DblInfo.SALBUp), xi.get(copt.DblInfo.SAUBLow), xi.get(copt.DblInfo.SAUBUp)));
    }

    System.out.println("");
    System.out.println("Constraint  \tLHS Range                \tRHS Range");
    for (int i = 0; i < cons.size(); i++) {
      Constraint ci = cons.getConstr(i);
      System.out.println(String.format("  %s\t(%e, %e)\t(%e, %e)",
              ci.getName(), ci.get(copt.DblInfo.SALBLow), ci.get(copt.DblInfo.SALBUp), ci.get(copt.DblInfo.SAUBLow), ci.get(copt.DblInfo.SAUBUp))); }
  }

}
