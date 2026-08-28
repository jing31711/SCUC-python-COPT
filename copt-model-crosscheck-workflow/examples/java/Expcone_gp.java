/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

import java.lang.*;

public class Expcone_gp {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("expcone_gp");

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
      Var x = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 1.0, copt.Consts.CONTINUOUS, "x");
      Var y = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 1.0, copt.Consts.CONTINUOUS, "y");
      Var z = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 1.0, copt.Consts.CONTINUOUS, "z");

      Var u1 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "u1");
      Var u2 = model.addVar(1.0, 1.0, 0.0, copt.Consts.CONTINUOUS, "u2");
      Var u3 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "u3");

      Var v1 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "v1");
      Var v2 = model.addVar(1.0, 1.0, 0.0, copt.Consts.CONTINUOUS, "v2");
      Var v3 = model.addVar(-copt.Consts.INFINITY, +copt.Consts.INFINITY, 0.0, copt.Consts.CONTINUOUS, "v3");

      // Add constraint: u1 >= exp(x + y + log(2/Aw)
      Expr expr_u3 = new Expr(0.0);
      expr_u3.addTerm(x, -1.0);
      expr_u3.addTerm(y, -1.0);
      expr_u3.addTerm(u3, 1.0);
      model.addConstr(expr_u3, copt.Consts.EQUAL, Math.log(2.0 / Aw), "");

      VarArray uconevars = new VarArray();
      uconevars.pushBack(u1);
      uconevars.pushBack(u2);
      uconevars.pushBack(u3);
      model.addExpCone(uconevars, copt.Consts.EXPCONE_PRIMAL);

      // Add constraint: v1 >= exp(x + z + log(2/Aw)
      Expr expr_v3 = new Expr(0.0);
      expr_v3.addTerm(x, -1.0);
      expr_v3.addTerm(z, -1.0);
      expr_v3.addTerm(v3, 1.0);
      model.addConstr(expr_v3, copt.Consts.EQUAL, Math.log(2.0 / Aw), "");

      VarArray vconevars = new VarArray();
      vconevars.pushBack(v1);
      vconevars.pushBack(v2);
      vconevars.pushBack(v3);
      model.addExpCone(vconevars, copt.Consts.EXPCONE_PRIMAL);

      // Add constraint: u1 + v1 == 1.0
      Expr expr_uv1 = new Expr(0.0);
      expr_uv1.addTerm(u1, 1.0);
      expr_uv1.addTerm(v1, 1.0);
      model.addConstr(expr_uv1, copt.Consts.EQUAL, 1.0, "");

      /*
       * Add constraints:
       *         -inf <= y + z <= log(Af)
       *   log(alpha) <= x - y <= log(beta)
       *   log(gamma) <= z - y <= log(delta)
       */
      Expr expr_yz = new Expr(0.0);
      expr_yz.addTerm(y, 1.0);
      expr_yz.addTerm(z, 1.0);
      model.addConstr(expr_yz, copt.Consts.LESS_EQUAL, Math.log(Af), "");

      Expr expr_xy = new Expr(0.0);
      expr_xy.addTerm(x, 1.0);
      expr_xy.addTerm(y, -1.0);
      model.addConstr(expr_xy, Math.log(alpha), Math.log(beta), "");

      Expr expr_zy = new Expr(0.0);
      expr_zy.addTerm(y, -1.0);
      expr_zy.addTerm(z, 1.0);
      model.addConstr(expr_zy, Math.log(gamma), Math.log(delta), "");

      // Set objective sense
      model.setObjSense(copt.Consts.MAXIMIZE);

      // Solve the model
      model.solve();

      // Analyze solution
      if (model.getIntAttr(copt.IntAttr.LpStatus) == copt.Status.OPTIMAL) {
        System.out.println("");
        System.out.println("Objective value: " + model.getDblAttr(copt.DblAttr.LpObjVal));

        System.out.println("Variable solution:");
        System.out.println("  " + x.getName() + " = " + x.get(copt.DblInfo.Value));
        System.out.println("  " + y.getName() + " = " + y.get(copt.DblInfo.Value));
        System.out.println("  " + z.getName() + " = " + z.get(copt.DblInfo.Value));
        System.out.println("");
      }
    } catch (CoptException e) {
      System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
    }
  }
}
