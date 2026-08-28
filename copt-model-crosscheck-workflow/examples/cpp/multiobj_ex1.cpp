/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

using namespace std;

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

int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("Multi-Objective Knapsack");

    // Add binary variables in shape of (4, 5), representing which item is selected
    // in each group (5 groups and 4 items per group)
    MVar<2> mv = model.AddMVar(Shape<2>(4, 5), COPT_BINARY, "MX");

    // Add constraint of mutual exclusivity
    model.AddMConstr(mv.Sum(0) == 1, "Exclusivity");

    // Add constraint of total volume not exceeding knapsack capacity
    NdArray<double, 2> volume = {{10, 13, 24, 32, 4}, {12, 15, 22, 26, 5.2}, {14, 18, 25, 28, 6.8},
      {14, 14, 28, 32, 6.8}};
    model.AddMConstr((volume * mv).Sum() <= 90.0, "Capacity");

    // Add first objective: maximize total value of selected items
    NdArray<double, 2> value = {{3, 4, 9, 15, 2}, {4, 6, 8, 10, 2.5}, {5, 7, 10, 12, 3}, {3, 5, 10, 10, 2}};
    model.SetObjectiveN(0, (value * mv).Sum(), COPT_MAXIMIZE, 2.0);

    // Add second objective: minimize total weight of selected items
    NdArray<double, 2> weight = {{0.2, 0.3, 0.4, 0.6, 0.1}, {0.25, 0.35, 0.38, 0.45, 0.15}, {0.3, 0.37, 0.5, 0.5, 0.2},
      {0.3, 0.32, 0.45, 0.6, 0.2}};
    model.SetObjectiveN(1, (weight * mv).Sum(), COPT_MINIMIZE, 1.0);

    // Solve the multi-objective knapsack problem
    model.Solve();

    int status = model.GetIntAttr(COPT_INTATTR_MIPSTATUS);
    if (status != COPT_MIPSTATUS_OPTIMAL)
    {
      cout << "Optimal solution is not available, status=" << status << endl;
      return 1;
    }

    // Show the solution
    double sumVolume = 0.0;
    double sumValue = 0.0;
    double sumWeight = 0.0;
    NdArray<double, 2> sol = mv.Get(COPT_DBLINFO_VALUE);

    cout << endl;
    cout << " Group\t" << "Volume\t" << "Value\t" << "Weight" << endl;
    for (int k = 0; k < 5; k++)
    {
      for (int i = 0; i < 4; i++)
      {
        if (sol[i][k].Item() > 0.99)
        {
          cout << "  " << k << "\t " << volume[i][k] << "\t " << value[i][k] << "\t " << weight[i][k] << endl;
          sumVolume += volume[i][k].Item();
          sumValue += value[i][k].Item();
          sumWeight += weight[i][k].Item();
        }
      }
    }

    cout << " Total\t " << sumVolume << "\t " << sumValue << "\t " << sumWeight << endl;
  }
  catch (CoptException e)
  {
    cout << "Error Code = " << e.GetCode() << endl;
    cout << e.what() << endl;
  }
  catch (...)
  {
    cout << "Unknown exception occurs!";
  }
}
