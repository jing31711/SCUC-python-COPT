/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

/*
 * Minimize
 *  obj: 2.1 x1 - 1.2 x2 + 3.2 x3 + x4 + x5 + x6 + 2 x7 + [ x2^2 ] / 2
 * Subject To
 *  r1: x1 + 2 x2 = 6
 *  r2: 2 x1 + x3 >= 5
 *  r3: x6 + 2 x7 <= 7
 *  r4: -x1 + 1.2 x7 >= -2.3
 *  q1: [ -1.8 x1^2 + x2^2 ] <= 0
 *  q2: [ 4.25 x3^2 - 2 x3 * x4 + 4.25 x4^2 - 2 x4 * x5 + 4 x5^2  ] + 2 x1 + 3 x3 <= 9.9
 *  q3: [ x6^2 - 2.2 x7^2 ] >= 5
 * Bounds
 *  0.2 <= x1 <= 3.8
 *  x2 Free
 *  0.1 <= x3 <= 0.7
 *  x4 Free
 *  x5 Free
 *  x7 Free
 * End
 */
public class Qcp_ex1 {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("qcp_ex1");

      // Add variables
      Var x1 = model.addVar(0.2, 3.8, 0.0, copt.Consts.CONTINUOUS, "x1");
      Var x2 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "x2");
      Var x3 = model.addVar(0.1, 0.7, 0.0, copt.Consts.CONTINUOUS, "x3");
      Var x4 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "x4");
      Var x5 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "x5");
      Var x6 = model.addVar(0, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "x6");
      Var x7 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "x7");

      // Add linear constraints
      Expr r1 = new Expr(0.0);
      r1.addTerm(x1, 1.0);
      r1.addTerm(x2, 2.0);
      model.addConstr(r1, copt.Consts.EQUAL, 6, "r1");

      Expr r2 = new Expr(0.0);
      r2.addTerm(x1, 2.0);
      r2.addTerm(x3, 1.0);
      model.addConstr(r2, copt.Consts.GREATER_EQUAL, 5, "r2");

      Expr r3 = new Expr(0.0);
      r3.addTerm(x6, 1.0);
      r3.addTerm(x7, 2.0);
      model.addConstr(r3, copt.Consts.LESS_EQUAL, 7, "r3");

      Expr r4 = new Expr(0.0);
      r4.addTerm(x1, -1);
      r4.addTerm(x7, 1.2);
      model.addConstr(r4, copt.Consts.GREATER_EQUAL, -2.3, "r4");

      // Add quadratic constraints
      QuadExpr q1 = new QuadExpr(0.0);
      q1.addTerm(x1, x1, -1.8);
      q1.addTerm(x2, x2, 1);
      model.addQConstr(q1, copt.Consts.LESS_EQUAL, 0, "q1");

      QuadExpr q2 = new QuadExpr(0.0);
      q2.addTerm(x3, x3, 4.25);
      q2.addTerm(x3, x4, -2);
      q2.addTerm(x4, x4, 4.25);
      q2.addTerm(x4, x5, -2);
      q2.addTerm(x5, x5, 4);
      q2.addTerm(x1, 2);
      q2.addTerm(x3, 3);
      model.addQConstr(q2, copt.Consts.LESS_EQUAL, 9.9, "q2");

      QuadExpr q3 = new QuadExpr(0.0);
      q3.addTerm(x6, x6, 1);
      q3.addTerm(x7, x7, -2.2);
      model.addQConstr(q3, copt.Consts.GREATER_EQUAL, 5, "q3");

      // Set quadratic objective
      QuadExpr obj = new QuadExpr(0.0);
      obj.addTerm(x1, 2.1);
      obj.addTerm(x2, -1.2);
      obj.addTerm(x3, 3.2);
      obj.addTerm(x4, 1);
      obj.addTerm(x5, 1);
      obj.addTerm(x6, 1);
      obj.addTerm(x7, 2);
      obj.addTerm(x2, x2, 0.5);
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
