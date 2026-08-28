/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

/*
 * We will compute an IIS for the following infeasible LP problem, the problem
 * is 'itest6' test case from netlib-infeas:
 *
 * Minimize
 *  X2 + X3 + X4
 * Subject To
 *  ROW1: 0.8 X3 + X4 <= 10000
 *  ROW2: X1 <= 90000
 *  ROW3: 2 X6 - X8 <= 10000
 *  ROW4: - X2 + X3 >= 50000
 *  ROW5: - X2 + X4 >= 87000
 *  ROW6: X3 <= 50000
 *  ROW7: - 3 X5 + X7 >= 10000
 *  ROW8: 0.5 X5 + 0.6 X6 <= 300000
 *  ROW9: X2 - 0.05 X3 = 5000
 *  ROW10: X2 - 0.04 X3 - 0.05 X4 = 4500
 *  ROW11: X2 >= 80000
 * END
 */ 

public class IIS_ex1 {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("iis_ex1");

      // Add variables
      Var x1 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X1");
      Var x2 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X2");
      Var x3 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X3");
      Var x4 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X4");
      Var x5 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X5");
      Var x6 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X6");
      Var x7 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X7");
      Var x8 = model.addVar(0.0, copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X8");

      // Add constraints
      Expr r1 = new Expr(0.0);
      r1.addTerm(x3, 0.8);
      r1.addTerm(x4, 1);
      model.addConstr(r1, copt.Consts.LESS_EQUAL, 10000, "ROW1");

      Expr r2 = new Expr(0.0);
      r2.addTerm(x1, 1);
      model.addConstr(r2, copt.Consts.LESS_EQUAL, 90000, "ROW2");

      Expr r3 = new Expr(0.0);
      r3.addTerm(x6, 2);
      r3.addTerm(x8, -1);
      model.addConstr(r3, copt.Consts.LESS_EQUAL, 10000, "ROW3");

      Expr r4 = new Expr(0.0);
      r4.addTerm(x2, -1);
      r4.addTerm(x3, 1);
      model.addConstr(r4, copt.Consts.GREATER_EQUAL, 50000, "ROW4");

      Expr r5 = new Expr(0.0);
      r5.addTerm(x2, -1);
      r5.addTerm(x4, 1);
      model.addConstr(r5, copt.Consts.GREATER_EQUAL, 87000, "ROW5");

      Expr r6 = new Expr(0.0);
      r6.addTerm(x3, 1);
      model.addConstr(r6, copt.Consts.LESS_EQUAL, 50000, "ROW6");

      Expr r7 = new Expr(0.0);
      r7.addTerm(x5, -3);
      r7.addTerm(x7, 1);
      model.addConstr(r7, copt.Consts.GREATER_EQUAL, 10000, "ROW7");

      Expr r8 = new Expr(0.0);
      r8.addTerm(x5, 0.5);
      r8.addTerm(x6, 0.6);
      model.addConstr(r8, copt.Consts.LESS_EQUAL, 300000, "ROW8");

      Expr r9 = new Expr(0.0);
      r9.addTerm(x2, 1);
      r9.addTerm(x3, -0.05);
      model.addConstr(r9, copt.Consts.EQUAL, 5000, "ROW9");

      Expr r10 = new Expr(0.0);
      r10.addTerm(x2, 1);
      r10.addTerm(x3, -0.04);
      r10.addTerm(x4, -0.05);
      model.addConstr(r10, copt.Consts.EQUAL, 4500, "ROW10");

      Expr r11 = new Expr(0.0);
      r11.addTerm(x2, 1);
      model.addConstr(r11, copt.Consts.GREATER_EQUAL, 80000, "ROW11");

      // Set objective
      Expr obj = new Expr(0.0);
      obj.addTerm(x2, 1);
      obj.addTerm(x3, 1);
      obj.addTerm(x4, 1);
      model.setObjective(obj, copt.Consts.MINIMIZE);

      // Set parameters
      model.setDblParam(copt.DblParam.TimeLimit, 60);

      // Solve the problem
      model.solve();

      // Compute IIS if problem is infeasible
      if (model.getIntAttr(copt.IntAttr.LpStatus) == copt.Status.INFEASIBLE) {
        // Compute IIS
        model.computeIIS();

        // Check if IIS is available
        if (model.getIntAttr(copt.IntAttr.HasIIS) == 1) {
          ConstrArray cons = model.getConstrs();
          VarArray vars = model.getVars();

          // Print variables and constraints in IIS
          System.out.println("\n======================== IIS result ========================");

          for (int iRow = 0; iRow < cons.size(); ++iRow) {
            Constraint con = cons.getConstr(iRow);
            if (con.getLowerIIS() == 1 || con.getUpperIIS() == 1) {
              System.out.println("  " + con.getName() + ": " + (con.getLowerIIS() == 1 ? "lower" : "upper"));
            }
          }

          System.out.println("");
          for (int iCol = 0; iCol < vars.size(); ++iCol) {
            Var var = vars.getVar(iCol);
            if (var.getLowerIIS() == 1 || var.getUpperIIS() == 1) {
              System.out.println("  " + var.getName() + ": " + (var.getLowerIIS() == 1 ? "lower" : "upper"));
            }
          }

          // Write IIS to file
          System.out.println("");
          model.writeIIS("iis_ex1.iis");
        }
      }
    } catch (CoptException e) {
      System.out.println("Error Code = " + e.getCode());
      System.out.println(e.getMessage());
    }
  }
}
