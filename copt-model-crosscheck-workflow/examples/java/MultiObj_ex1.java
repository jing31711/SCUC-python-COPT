
/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;

/*
 * Multi-Objective Knapsack Problem:
 *   Consider a set of 20 items divided into 5 groups, where each group contains
 *   mutually exclusive items (only one item per group can be selected).
 *
 *   Each item i in group j has a volume V_ij, a value P_ij, and a weight W_ij.
 *
 *   Volume = [10, 13, 24, 32, 4]
 *            [12, 15, 22, 26, 5.2]
 *            [14, 18, 25, 28, 6.8]
 *            [14, 14, 28, 32, 6.8]
 *
 *   Value  = [3, 4, 9, 15, 2]
 *            [4, 6, 8, 10, 2.5]
 *            [5, 7, 10, 12, 3]
 *            [3, 5, 10, 10, 2]]
 *
 *   Weight = [0.2, 0.3, 0.4, 0.6, 0.1]
 *            [0.25, 0.35, 0.38, 0.45, 0.15]
 *            [0.3, 0.37, 0.5, 0.5, 0.2]
 *            [0.3, 0.32, 0.45, 0.6, 0.2]
 *
 *   Objectives:
 *     Maximize total value while minimizing total weight of selected items
 *
 *   Subject to:
 *     - Exactly one item must be selected from each group
 *     - Total volume of selected items <= knapsack capacity
 */

public class MultiObj_ex1 {
  public static void main(String[] args) {
    try {
      Envr env = new Envr();

      Model model = env.createModel("Multi-Objective Knapsack");

      int nGroups = 5;
      int nItems = 4;
      int cnt = nGroups * nItems;

      // Add binary variables of size 20, representing which item is selected
      // in each group (5 groups and 4 items per group)
      VarArray vars = model.addVars(cnt, Consts.BINARY, "X");

      // Add constraint of mutual exclusivity
      for (int j = 0; j < nGroups; j++) {
        Expr exclusivity = new Expr();
        for (int i = 0; i < nItems; i++) {
          int idx = i * nGroups + j;
          exclusivity.addTerm(vars.getVar(idx), 1.0);
        }
        model.addConstr(exclusivity, Consts.EQUAL, 1.0, "Exclusivity");
      }

      // Add constraint of total volume not exceeding knapsack capacity
      double[] volume = new double[] { 10, 13, 24, 32, 4, 12, 15, 22, 26, 5.2, 14, 18, 25, 28, 6.8, 14, 14, 28, 32,
          6.8 };
      Expr capacity = new Expr();
      for (int i = 0; i < cnt; i++) {
        capacity.addTerm(vars.getVar(i), volume[i]);
      }
      model.addConstr(capacity, Consts.LESS_EQUAL, 90.0, "Capacity");

      // Add first objective: maximize total value of selected items
      double[] value = new double[] { 3, 4, 9, 15, 2, 4, 6, 8, 10, 2.5, 5, 7, 10, 12, 3, 3, 5, 10, 10, 2 };
      Expr obj_value = new Expr();
      for (int i = 0; i < cnt; i++) {
        obj_value.addTerm(vars.getVar(i), value[i]);
      }
      model.setObjectiveN(0, obj_value, Consts.MAXIMIZE);

      // Add second objective: minimize total weight of selected items
      double[] weight = new double[] { 0.2, 0.3, 0.4, 0.6, 0.1, 0.25, 0.35, 0.38, 0.45, 0.15, 0.3, 0.37, 0.5, 0.5, 0.2,
          0.3, 0.32, 0.45, 0.6, 0.2 };
      Expr obj_weight = new Expr();
      for (int i = 0; i < nGroups * nItems; i++) {
        obj_weight.addTerm(vars.getVar(i), weight[i]);
      }
      model.setObjectiveN(1, obj_weight, Consts.MINIMIZE);

      // Solve the multi-objective knapsack problem
      model.solve();

      int status = model.getIntAttr(IntAttr.MipStatus);
      if (status != Status.OPTIMAL) {
        System.out.println("Optimal solution is not available, status=" + status);
        return;
      }

      // Show the solution
      double sumVolume = 0.0;
      double sumValue = 0.0;
      double sumWeight = 0.0;

      System.out.println("");
      System.out.println(" Group\tVolume\tValue\tWeight");

      for (int k = 0; k < nGroups; k++) {
        for (int i = 0; i < nItems; i++) {
          int idx = i * nGroups + k;
          if (vars.getVar(idx).get(DblInfo.Value) > 0.99) {
            System.out.println("  " + k + "\t " + volume[idx] + "\t " + value[idx] + "\t " + weight[idx]);
            sumVolume += volume[idx];
            sumValue += value[idx];
            sumWeight += weight[idx];
          }
        }
      }

      System.out.println(" Total\t " + sumVolume + "\t " + sumValue + "\t " + sumWeight);
    } catch (CoptException e) {
      System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
    }
  }
}
