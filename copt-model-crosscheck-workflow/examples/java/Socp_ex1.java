/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

import java.lang.*;

public class Socp_ex1 {
  public static void solve_soc() {
    try {
      // Create COPT environment
      Envr env = new Envr();

      // Create COPT model
      Model model = env.createModel("solve_soc");
      
      /*
       * Add variables
       *
       *   minimize: z
       *
       *   bnds:
       *     x, y, t free, z non-negative
       *
       */ 
      double infinity = copt.Consts.INFINITY;

      Var x = model.addVar(-infinity, infinity, 0.0, copt.Consts.CONTINUOUS, "x");
      Var y = model.addVar(-infinity, infinity, 0.0, copt.Consts.CONTINUOUS, "y");
      Var z = model.addVar(0, infinity, 0.0, copt.Consts.CONTINUOUS, "z");
      Var t = model.addVar(-infinity, infinity, 0.0, copt.Consts.CONTINUOUS, "t");

      // Set objective: z
      model.setObjective(z, copt.Consts.MINIMIZE);
      
      /*
       * Add constraints
       *
       *   r0: 3*x + y >= 1
       *   c0: z^2 >= x^2 + 2*y^2
       *
       * c0 is converted to:
       *
       *   r1: sqrt(2.0)*y - t = 0
       *   c1: z^2 >= x^2 + t^2
       */
      Expr expr = new Expr();
      expr.addTerm(x, 3.0);
      expr.addTerm(y, 1.0);
      model.addConstr(expr, copt.Consts.GREATER_EQUAL, 1.0, "r0");

      expr = new Expr();
      expr.addTerm(y, Math.sqrt(2.0));
      expr.addTerm(t, -1.0);
      model.addConstr(expr, copt.Consts.EQUAL, 0.0, "r1");

      VarArray cvars = new VarArray();
      cvars.pushBack(z);
      cvars.pushBack(x);
      cvars.pushBack(t);
      model.addCone(cvars, copt.Consts.CONE_QUAD);

      // Set parameters
      model.setDblParam(copt.DblParam.TimeLimit, 10);

      // Solve the model
      model.solve();

      // Analyze solution
      if (model.getIntAttr(copt.IntAttr.LpStatus) == copt.Status.OPTIMAL) {
        System.out.println("");
        System.out.println("Objective value: " + model.getDblAttr(copt.DblAttr.LpObjVal));

        VarArray vars = model.getVars();
        System.out.println("Variable solution:");
        for (int i = 0; i < vars.size(); i++) {
          Var var = vars.getVar(i);
          System.out.println("  " + var.getName() + " = "
                                  + var.get(copt.DblInfo.Value));
        }
        System.out.println("");
      }
    } catch (CoptException e) {
      System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
    }
  }

  public static void solve_rsoc() {
    try {
      // Create COPT environment
      Envr env = new Envr();

      // Create COPT model
      Model model = env.createModel("solve_rsoc");

      /*
       * Add variables
       *
       *   minimize: 1.5*x - 2*y + z
       *
       *   bnds:
       *     0 <= x <= 20
       *     y, z, r >= 0
       *     s, t free
       */
      double infinity = copt.Consts.INFINITY;

      Var x = model.addVar(0.0, 20.0, 0.0, copt.Consts.CONTINUOUS, "x");
      Var y = model.addVar(0.0, infinity, 0.0, copt.Consts.CONTINUOUS, "y");
      Var z = model.addVar(0.0, infinity, 0.0, copt.Consts.CONTINUOUS, "z");
      Var r = model.addVar(0.0, infinity, 0.0, copt.Consts.CONTINUOUS, "r");
      Var s = model.addVar(-infinity, infinity, 0.0, copt.Consts.CONTINUOUS, "s");
      Var t = model.addVar(-infinity, infinity, 0.0, copt.Consts.CONTINUOUS, "t");

      // Set objective: 1.5*x - 2*y + z
      Expr obj = new Expr();
      obj.addTerm(x, 1.5);
      obj.addTerm(y, -2.0);
      obj.addTerm(z, 1.0);
      model.setObjective(obj, copt.Consts.MINIMIZE);
      
      /*
       * Add constraints
       *
       *   r0: 2*x + y >= 2
       *   r1: -x + 2*y <= 6
       *   r2: r = 1
       *   r3: 2.8284271247 * x + 0.7071067811 * y - s = 0
       *   r4: 3.0822070014 * y - t = 0
       *   c0: 2*z*r >= s^2 + t^2
       */
      Expr expr = new Expr();
      expr.addTerm(x, 2.0);
      expr.addTerm(y, 1.0);
      model.addConstr(expr, copt.Consts.GREATER_EQUAL, 2.0, "r0");

      expr = new Expr();
      expr.addTerm(y, 2.0);
      expr.addTerm(x, -1.0);
      model.addConstr(expr, copt.Consts.LESS_EQUAL, 6.0, "r1");

      expr = new Expr();
      expr.addTerm(r, 1.0);
      model.addConstr(expr, copt.Consts.EQUAL, 1.0, "r2");

      expr = new Expr();
      expr.addTerm(x, 2.8284271247);
      expr.addTerm(y, 0.7071067811);
      expr.addTerm(s, -1.0);
      model.addConstr(expr, copt.Consts.EQUAL, 0, "r3");

      expr = new Expr();
      expr.addTerm(y, 3.0822070014);
      expr.addTerm(t, -1.0);
      model.addConstr(expr, copt.Consts.EQUAL, 0, "r4");

      VarArray rvars = new VarArray();
      rvars.pushBack(z);
      rvars.pushBack(r);
      rvars.pushBack(s);
      rvars.pushBack(t);
      model.addCone(rvars, copt.Consts.CONE_RQUAD);

      // Set parameters
      model.setDblParam(copt.DblParam.TimeLimit, 10);

      // Solve the model
      model.solve();

      // Analyze solution
      if (model.getIntAttr(copt.IntAttr.LpStatus) == copt.Status.OPTIMAL) {
        System.out.println("");
        System.out.println("Objective value: " + model.getDblAttr(copt.DblAttr.LpObjVal));

        VarArray vars = model.getVars();
        System.out.println("Variable solution:");
        for (int i = 0; i < vars.size(); i++) {
          Var var = vars.getVar(i);
          System.out.println("  " + var.getName() + " = "
                                  + var.get(copt.DblInfo.Value));
        }
        System.out.println("");
      }
    } catch (CoptException e) {
      System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
    }
  }

  public static void solve_mps(String filename) {
    try {
      // Create COPT environment
      Envr env = new Envr();

      // Create COPT model
      Model model = env.createModel("solve_mps");

      // Read SOCP from MPS file
      model.readMps(filename);

      // Set parameters
      model.setDblParam(copt.DblParam.TimeLimit, 10);

      // Solve the model
      model.solve();
    } catch (CoptException e) {
      System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
    }
  }

  public static void main(String[] args) {
    // Solve SOCP with regular cone
    solve_soc();

    // Solve SOCP with rotated cone
    solve_rsoc();

    // Solve SOCP from MPS file
    if (args.length >= 1) {
      solve_mps(args[0]);
    }
  }
}
