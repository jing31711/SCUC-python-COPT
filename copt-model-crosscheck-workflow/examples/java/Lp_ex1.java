/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

/*
 * This Java example solves the following LP model:
 *
 * Maximize:
 *  1.2 x + 1.8 y + 2.1 z
 *
 * Subject to:
 *  1.5 x + 1.2 y + 1.8 z <= 2.6
 *  0.8 x + 0.6 y + 0.9 z >= 1.2
 *
 * where:
 *  0.1 <= x <= 0.6
 *  0.2 <= y <= 1.5
 *  0.3 <= z <= 2.8
 */
public class Lp_ex1 {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("lp_ex1");

      /* 
       * Add variables x, y, z
       *
       * obj: 1.2 x + 1.8 y + 2.1 z
       *
       * var:
       *  0.1 <= x <= 0.6
       *  0.2 <= y <= 1.5
       *  0.3 <= z <= 2.8
       */
      Var x = model.addVar(0.1, 0.6, 1.2, copt.Consts.CONTINUOUS, "x");
      Var y = model.addVar(0.2, 1.5, 1.8, copt.Consts.CONTINUOUS, "y");
      Var z = model.addVar(0.3, 2.8, 2.1, copt.Consts.CONTINUOUS, "z");

      /*
       * Add two constraints using linear expression
       *
       * r0: 1.5 x + 1.2 y + 1.8 z <= 2.6
       * r1: 0.8 x + 0.6 y + 0.9 z >= 1.2
       */
      Expr e0 = new Expr(x, 1.5);
      e0.addTerm(y, 1.2);
      e0.addTerm(z, 1.8);
      model.addConstr(e0, copt.Consts.LESS_EQUAL, 2.6, "r0");

      Expr e1 = new Expr(x, 0.8);
      e1.addTerm(y, 0.6);
      e1.addTerm(z, 0.9);
      model.addConstr(e1, copt.Consts.GREATER_EQUAL, 1.2, "r1");

      // Set parameters and attributes
      model.setDblParam(copt.DblParam.TimeLimit, 10);
      model.setObjSense(copt.Consts.MAXIMIZE);

      // Solve problem
      model.solve();

      // Output solution
      if (model.getIntAttr(copt.IntAttr.HasLpSol) != 0) {
        System.out.println("\nFound optimal solution:");
        VarArray vars = model.getVars();
        for (int i = 0; i < vars.size(); i++) {
          Var xi = vars.getVar(i);
          System.out.println("  " + xi.getName() + " = " + xi.get(copt.DblInfo.Value));
        }
        System.out.println("Obj = " + model.getDblAttr(copt.DblAttr.LpObjVal));
      }

      System.out.println("\nDone");
    } catch (CoptException e) {
      System.out.println("Error Code = " + e.getCode());
      System.out.println(e.getMessage());
    }
  }
}
