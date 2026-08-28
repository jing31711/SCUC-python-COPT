/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using System;
using Copt;

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

class MultiObj_ex1
{
  static void Main()
  {
    try
    {
      Envr env = new Envr();

      Model model = env.CreateModel("Multi-Objective Knapsack");

      // Add binary variables in shape of (4, 5), representing which item is selected
      // in each group (5 groups and 4 items per group)
      MVar mv = model.AddMVar(new Shape(4, 5), Consts.BINARY, "MX");

      // Add constraint of mutual exclusivity
      model.AddMConstr(mv.Sum(0) == 1, "Exclusivity");

      // Add constraint of total volume not exceeding knapsack capacity
      double[,] volumeData = new double[,] { { 10, 13, 24, 32, 4 }, { 12, 15, 22, 26, 5.2 }, { 14, 18, 25, 28, 6.8 }, { 14, 14, 28, 32, 6.8 } };
      NdArray<double> volume = new NdArray<double>(volumeData);
      model.AddMConstr((volume * mv).Sum() <= 90.0, "Capacity");

      // Add first objective: maximize total value of selected items
      double[,] valueData = new double[,] { { 3, 4, 9, 15, 2 }, { 4, 6, 8, 10, 2.5 }, { 5, 7, 10, 12, 3 }, { 3, 5, 10, 10, 2 } };
      NdArray<double> value = new NdArray<double>(valueData);
      model.SetObjectiveN(0, (value * mv).Sum(), Consts.MAXIMIZE, 2.0);

      // Add second objective: minimize total weight of selected items
      double[,] weightData = new double[,] { { 0.2, 0.3, 0.4, 0.6, 0.1 }, { 0.25, 0.35, 0.38, 0.45, 0.15 }, { 0.3, 0.37, 0.5, 0.5, 0.2 }, { 0.3, 0.32, 0.45, 0.6, 0.2 } };
      NdArray<double> weight = new NdArray<double>(weightData);
      model.SetObjectiveN(1, (weight * mv).Sum(), Consts.MINIMIZE, 1.0);

      // Solve the multi-objective knapsack problem
      model.Solve();

      int status = model.GetIntAttr(IntAttr.MipStatus);
      if (status != Status.OPTIMAL)
      {
        Console.WriteLine("Optimal solution is not available, status={0}", status);
        return;
      }

      // Show the solution
      double sumVolume = 0.0;
      double sumValue = 0.0;
      double sumWeight = 0.0;

      NdArray<double> sol = mv.Get(DblInfo.Value);

      Console.WriteLine("");
      Console.WriteLine(" Group\tVolume\tValue\tWeight");

      for (int k = 0; k < 5; k++)
      {
        for (int i = 0; i < 4; i++)
        {
          if (sol[i][k].GetItem() > 0.99)
          {
            Console.WriteLine("  {0}\t {1}\t {2}\t {3}", k, volume[i][k], value[i][k], weight[i][k]);
            sumVolume += volume[i][k].GetItem();
            sumValue += value[i][k].GetItem();
            sumWeight += weight[i][k].GetItem();
          }
        }
      }

      Console.WriteLine(" Total\t {0}\t {1}\t {2}", sumVolume, sumValue, sumWeight);
    }
    catch (CoptException e)
    {
      Console.WriteLine("Error code: " + e.GetCode() + ". " + e.Message);
    }
  }
}
