
/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

/*
 * This Java example solves the following nonlinear problem:
 *
 * Minimize:
 *  x1 * x4 * (sin(x1 + x2) + cos(x2 * x3) + tan(x3 / x4)) + x3
 *
 * Subject to:
 *  x1 * x2 * x3 * x4 + x1 + x2 >= 35
 *  log(x1) + 2 * log(x2) + 3 * log(x3) + 4 * log(x4) + x3 + x4 >= 15
 *  x1^2 + x2^2 + x3^2 + x4^2 + x1 + x3 >= 50
 *
 * where:
 *  1 <= x1 <= 5
 *  1 <= x2 <= 5
 *  1 <= x3 <= 5
 *  1 <= x4 <= 5
 */
public class Nlp_ex1 {
  public static void main(final String argv[]) {
    try {
      Envr env = new Envr();
      Model model = env.createModel("nlp_ex1");

      // Add variables x1, x2, x3, x4
      Var x1 = model.addVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x1");
      Var x2 = model.addVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x2");
      Var x3 = model.addVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x3");
      Var x4 = model.addVar(1.0, 5.0, 0.0, Consts.CONTINUOUS, "x4");

      /*
       * Add two nonlinear constraints using nonlinear expression
       *
       * nlrow1: x1 * x2 * x3 * x4 + x1 + x2 >= 35
       * nlrow2: log(x1) + 2 * log(x2) + 3 * log(x3) + 4 * log(x4) + x3 + x4 >= 15
       *
       */
      NlExpr nle1 = new NlExpr(x1).multiply(x2).multiply(x3).multiply(x4);
      nle1.add(x1).add(x2);
      model.addNlConstr(nle1, Consts.GREATER_EQUAL, 35.0, "nlrow1");

      NlExpr nle2 = NL.log(x1);
      nle2.add(NL.log(x2).multiply(2.0))
          .add(NL.log(x3).multiply(3.0))
          .add(NL.log(x4).multiply(4.0))
          .add(x3)
          .add(x4);
      model.addNlConstr(nle2, Consts.GREATER_EQUAL, 15.0, "nlrow2");

      /*
       * Add a quadratic constraint using quadratic expression
       *
       * qrow1: x1^2 + x2^2 + x3^2 + x4^2 + x1 + x3 >= 50
       *
       */
      QuadExpr qe1 = new QuadExpr(x1, x1);
      qe1.addTerm(x2, x2, 1.0);
      qe1.addTerm(x3, x3, 1.0);
      qe1.addTerm(x4, x4, 1.0);
      qe1.add(x1).add(x3);
      model.addQConstr(qe1, Consts.GREATER_EQUAL, 50.0, "qrow1");

      // Minimize obj: x1 * x4 * (sin(x1 + x2) + cos(x2 * x3) + tan(x3 / x4)) + x3
      NlExpr nle3 = NL.sin(new Expr(x1).add(x2));
      NlExpr nle4 = NL.cos(new QuadExpr(x2, x3));
      NlExpr nle5 = NL.tan(new NlExpr(x3).divide(x4));

      NlExpr obj = new NlExpr(x1).multiply(x4);
      obj.multiply(NL.sum(nle3, nle4, nle5));
      obj.add(x3);

      model.setNlObjective(obj, Consts.MINIMIZE);

      // Set parameters
      model.setDblParam(DblParam.TimeLimit, 60);

      // Solve problem
      model.solve();

      // Output solution
      System.out.println("");
      int hasLpSol = model.getIntAttr(IntAttr.HasLpSol);
      if (hasLpSol == 0) {
        System.out.println("Error: no solution available!");
        return;
      }

      System.out.println("Optimal objective value = " + model.getDblAttr(DblAttr.LpObjVal));

      VarArray vars = model.getVars();
      for (int i = 0; i < vars.size(); i++) {
        Var xi = vars.getVar(i);
        System.out.println("  " + xi.getName() + " = " + xi.get(DblInfo.Value));
      }

    } catch (CoptException e) {
      System.out.println("Error Code = " + e.getCode());
      System.out.println(e.getMessage());
    }
  }
}
