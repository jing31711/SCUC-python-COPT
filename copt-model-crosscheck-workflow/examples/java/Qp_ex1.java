/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

/*
 * Minimize
 *  OBJ.FUNC: [ 2 X0 ^2 + 4 X0 * X1 + 4 X1 ^2
 *            + 4 X1 * X2 + 4 X2 ^2
 *            + 4 X2 * X3 + 4 X3 ^2
 *            + 4 X3 * X4 + 4 X4 ^2
 *            + 4 X4 * X5 + 4 X5 ^2
 *            + 4 X5 * X6 + 4 X6 ^2
 *            + 4 X6 * X7 + 4 X7 ^2
 *            + 4 X7 * X8 + 4 X8 ^2
 *            + 4 X8 * X9 + 2 X9 ^2 ] / 2
 * Subject To
 *  ROW0: X0 + 2 X1 + 3 X2  = 1
 *  ROW1: X1 + 2 X2 + 3 X3  = 1
 *  ROW2: X2 + 2 X3 + 3 X4  = 1
 *  ROW3: X3 + 2 X4 + 3 X5  = 1
 *  ROW4: X4 + 2 X5 + 3 X6  = 1
 *  ROW5: X5 + 2 X6 + 3 X7  = 1
 *  ROW6: X6 + 2 X7 + 3 X8  = 1
 *  ROW7: X7 + 2 X8 + 3 X9  = 1
 * Bounds
 *       X0 Free
 *       X1 Free
 *       X2 Free
 *       X3 Free
 *       X4 Free
 *       X5 Free
 *       X6 Free
 *       X7 Free
 *       X8 Free
 *       X9 Free
 * End
 */ 

public class Qp_ex1 {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("qp_ex1");

      // Add variables
      Var x0 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X0");
      Var x1 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X1");
      Var x2 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X2");
      Var x3 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X3");
      Var x4 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X4");
      Var x5 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X5");
      Var x6 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X6");
      Var x7 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X7");
      Var x8 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X8");
      Var x9 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "X9");

      // Add constraints
      Expr r0 = new Expr(0.0);
      r0.addTerm(x0, 1.0);
      r0.addTerm(x1, 2.0);
      r0.addTerm(x2, 3.0);
      model.addConstr(r0, copt.Consts.EQUAL, 1, "ROW0");

      Expr r1 = new Expr(0.0);
      r1.addTerm(x1, 1.0);
      r1.addTerm(x2, 2.0);
      r1.addTerm(x3, 3.0);
      model.addConstr(r1, copt.Consts.EQUAL, 1, "ROW1");

      Expr r2 = new Expr(0.0);
      r2.addTerm(x2, 1.0);
      r2.addTerm(x3, 2.0);
      r2.addTerm(x4, 3.0);
      model.addConstr(r2, copt.Consts.EQUAL, 1, "ROW2");

      Expr r3 = new Expr(0.0);
      r3.addTerm(x3, 1.0);
      r3.addTerm(x4, 2.0);
      r3.addTerm(x5, 3.0);
      model.addConstr(r3, copt.Consts.EQUAL, 1, "ROW3");

      Expr r4 = new Expr(0.0);
      r4.addTerm(x4, 1.0);
      r4.addTerm(x5, 2.0);
      r4.addTerm(x6, 3.0);
      model.addConstr(r4, copt.Consts.EQUAL, 1, "ROW4");

      Expr r5 = new Expr(0.0);
      r5.addTerm(x5, 1.0);
      r5.addTerm(x6, 2.0);
      r5.addTerm(x7, 3.0);
      model.addConstr(r5, copt.Consts.EQUAL, 1, "ROW5");

      Expr r6 = new Expr(0.0);
      r6.addTerm(x6, 1.0);
      r6.addTerm(x7, 2.0);
      r6.addTerm(x8, 3.0);
      model.addConstr(r6, copt.Consts.EQUAL, 1, "ROW6");

      Expr r7 = new Expr(0.0);
      r7.addTerm(x7, 1.0);
      r7.addTerm(x8, 2.0);
      r7.addTerm(x9, 3.0);
      model.addConstr(r7, copt.Consts.EQUAL, 1, "ROW7");

      // Set quadratic objective
      QuadExpr obj = new QuadExpr(0.0);
      obj.addTerm(x0, x0, 1.0);
      obj.addTerm(x1, x1, 2.0);
      obj.addTerm(x2, x2, 2.0);
      obj.addTerm(x3, x3, 2.0);
      obj.addTerm(x4, x4, 2.0);
      obj.addTerm(x5, x5, 2.0);
      obj.addTerm(x6, x6, 2.0);
      obj.addTerm(x7, x7, 2.0);
      obj.addTerm(x8, x8, 2.0);
      obj.addTerm(x9, x9, 1.0);
      obj.addTerm(x0, x1, 2.0);
      obj.addTerm(x1, x2, 2.0);
      obj.addTerm(x2, x3, 2.0);
      obj.addTerm(x3, x4, 2.0);
      obj.addTerm(x4, x5, 2.0);
      obj.addTerm(x5, x6, 2.0);
      obj.addTerm(x6, x7, 2.0);
      obj.addTerm(x7, x8, 2.0);
      obj.addTerm(x8, x9, 2.0);
      model.setQuadObjective(obj, copt.Consts.MINIMIZE);

      // Set parameters
      model.setDblParam(copt.DblParam.TimeLimit, 60);

      // Solve the problem
      model.solve();

      // Output solution
      if (model.getIntAttr(copt.IntAttr.LpStatus) == copt.Status.OPTIMAL) {
        System.out.println("\nOptimal objective value: " + model.getDblAttr(copt.DblAttr.LpObjVal));

        System.out.println("Variable solution: ");
        VarArray vars = model.getVars();
        for (int iCol = 0; iCol < vars.size(); ++iCol) {
          Var var = vars.getVar(iCol);
          System.out.println("  " + var.getName() + " = " + var.get(copt.DblInfo.Value));
        }
      }
    } catch (CoptException e) {
      System.out.println("Error Code = " + e.getCode());
      System.out.println(e.getMessage());
    }
  }
}
