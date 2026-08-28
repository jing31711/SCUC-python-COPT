/* 
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
import copt.*;
import java.util.*;

public class Cb_ex1 {
  public static class TspCallback extends CallbackBase {
    private static VarArray _vars;
    private static int _nCities;

    public TspCallback(VarArray vars, int nCities) {
      _vars = vars;
      _nCities = nCities;
    }

    @Override
    public void callback() {
      if (where() == copt.Consts.CBCONTEXT_MIPSOL) {
        try {
          double[] sols = getSolution(_vars);
          int[] tour = findSubTour(_nCities, sols);

          if (tour.length < _nCities) {
            Expr expr = new Expr();
            for (int i = 0; i < tour.length; i++) {
              int j = (i + 1) % tour.length;
              expr.addTerm(_vars.getVar(tour[i] * _nCities + tour[j]), 1.0);
            }

            addLazyConstr(expr, copt.Consts.LESS_EQUAL, tour.length - 1);
          }
        } catch (CoptException e) {
          System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
        }
      }
    }
  }

  public static void main(String[] args) {
    int nCities = 10;
    if (args.length >= 1) {
      nCities = Integer.parseInt(args[0]);
    }

    // Create random cities.
    double[] px = new double[nCities];
    double[] py = new double[nCities];

    Random rand = new Random();
    for (int i = 0; i < nCities; i++) {
      px[i] = rand.nextDouble();
      py[i] = rand.nextDouble();
    }

    try {
      // Create COPT environment and model.
      Envr env = new Envr();
      Model model = env.createModel("TSP Callback Java Example");

      // Add binary variables - one for every pair of nodes.
      VarArray vars = new VarArray();
      for (int i = 0; i < nCities; i++) {
        for (int j = 0; j < nCities; j++) {
          double obj = getDistance(px, py, i, j);
          String name = String.format("E_{0}_{1}", i, j);
          vars.pushBack(model.addVar(0, 1, obj, copt.Consts.BINARY, name));
        }
      }

      // Add outdegree-1 constraints for asymmetric TSP loop.
      for (int i = 0; i < nCities; i++) {
        Expr expr = new Expr();
        for (int j = 0; j < nCities; j++) {
          expr.addTerm(vars.getVar(i * nCities + j), 1.0);
        }
        model.addConstr(expr, copt.Consts.EQUAL, 1, "OutDegree_1_" + i);
      }

      // Add indegree-1 constraints for asymmetric TSP loop.
      for (int j = 0; j < nCities; j++) {
        Expr expr = new Expr();
        for (int i = 0; i < nCities; i++) {
          expr.addTerm(vars.getVar(i * nCities + j), 1.0);
        }
        model.addConstr(expr, copt.Consts.EQUAL, 1, "InDegree_1_" + j);
      }

      // Add constraints to exclude self loop.
      for (int i = 0; i < nCities; i++) {
        vars.getVar(i * nCities + i).set(copt.DblInfo.UB, 0.0);
      }

      // Set TSP callback instance.
      TspCallback tcb = new TspCallback(vars, nCities);
      model.setCallback(tcb, copt.Consts.CBCONTEXT_MIPSOL);

      // Solve the TSP problem.
      model.solve();
      int hasMipSol = model.getIntAttr(copt.IntAttr.HasMipSol);
      System.out.println("\n[HasMipSol] = " + hasMipSol);

      if (hasMipSol > 0) {
        double[] sols = model.get(copt.DblInfo.Value, vars);

        int[] tour = findSubTour(nCities, sols);
        System.out.print("\n  Best Tour:");
        for (int i = 0; i < tour.length; i++) {
          System.out.print(" " + tour[i]);
        }
        System.out.println("\n  Best Cost: " + model.getDblAttr(copt.DblAttr.BestObj));
      }
    } catch (CoptException e) {
      System.out.println("Error code: " + e.getCode() + ". " + e.getMessage());
    }
  }

  // Calculate euclidean distance between i-th and j-th points.
  private static double getDistance(double[] px, double[] py, int i, int j) {
    double dx = px[i] - px[j];
    double dy = py[i] - py[j];
    return Math.sqrt(dx * dx + dy * dy);
  }

  private static int nextUnvisited(boolean[] visited, int nCities) {
    int node = 0;
    while (node < nCities && visited[node]) {
      node++;
    }
    return node;
  }

  /*
   * Given an integer-feasible solution 'sols' of size nCities * nCities,
   * find the smallest sub-tour. Result is returned as 'tour' list.
   */
  private static int[] findSubTour(int nCities, double[] sols) {
    boolean[] visited = new boolean[nCities];
    List<Integer> bestTour = new ArrayList<Integer>();

    int curr = 0;
    while (curr < nCities) {
      List<Integer> tour = new ArrayList<Integer>();
      boolean closed = false;

      // Search a tour starting from current city until the tour is closed.
      while (!closed) {
        tour.add(curr);
        visited[curr] = true;
        // Enumerate all neighbors of current city.
        for (int j = 0; j < nCities; j++) {
          if (sols[curr * nCities + j] > 0.5 && !visited[j]) {
            curr = j;
            break;
          }
          // If all neighbors are visited, the tour is closed.
          closed = (j == nCities - 1);
        }
      }

      // Save the current tour if it is better.
      if (bestTour.size() == 0 || bestTour.size() > tour.size()) {
        bestTour = tour;
      }

      // Update curr for next tour.
      curr = nextUnvisited(visited, nCities);
    }

    return bestTour.stream().mapToInt(i -> i).toArray();
  }
}
