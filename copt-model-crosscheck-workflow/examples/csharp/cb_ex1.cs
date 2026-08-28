/* 
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;
using System.Collections.Generic;

public class Cb_ex1
{
    internal class TspCallback : CallbackBase
    {
        internal readonly VarArray vars;
        internal readonly int nCities;

        internal TspCallback(VarArray vars, int nCities)
        {
            this.vars = vars;
            this.nCities = nCities;
        }

        public override void callback()
        {
            if (Where() == Consts.CBCONTEXT_MIPSOL)
            {
                double[] sols = GetSolution(vars);
                int[] tour = FindSubTour(nCities, sols);

                if (tour.Length < nCities)
                {
                    Expr expr = new Expr();
                    for (int i = 0; i < tour.Length; i++)
                    {
                        int j = (i + 1) % tour.Length;
                        expr += vars[tour[i] * nCities + tour[j]];
                    }

                    AddLazyConstr(expr <= tour.Length - 1);
                }
            }
        }
    }

    public static void Main(string[] args)
    {
        int nCities = 10;
        if (args.Length >= 1)
        {
            nCities = int.Parse(args[0]);
        }

        // Create random cities.
        double[] px = new double[nCities];
        double[] py = new double[nCities];
        var rand = new Random();
        for (int i = 0; i < nCities; i++)
        {
            px[i] = rand.NextDouble();
            py[i] = rand.NextDouble();
        }

        try
        {
            // Create COPT environment and model.
            Envr env = new Envr();
            Model model = env.CreateModel("TSP Callback C# Example");

            // Add binary variables - one for every pair of cities.
            VarArray vars = new VarArray();
            for (int i = 0; i < nCities; i++)
            {
                for (int j = 0; j < nCities; j++)
                {
                    double obj = GetDistance(px, py, i, j);
                    String name = String.Format("E_{0}_{1}", i, j);
                    vars.PushBack(model.AddVar(0, 1, obj, Consts.BINARY, name));
                }
            }

            // Add outdegree-1 constraints for asymmetric TSP loop.
            for (int i = 0; i < nCities; i++)
            {
                Expr expr = new Expr();
                for (int j = 0; j < nCities; j++)
                {
                    expr += vars[i * nCities + j];
                }
                model.AddConstr(expr == 1, "OutDegree_1_" + i);
            }

            // Add indegree-1 constraints for asymmetric TSP loop.
            for (int j = 0; j < nCities; j++)
            {
                Expr expr = new Expr();
                for (int i = 0; i < nCities; i++)
                {
                    expr += vars[i * nCities + j];
                }
                model.AddConstr(expr == 1, "InDegree_1_" + j);
            }

            // Add constraint to exclude self loop.
            for (int i = 0; i < nCities; i++)
            {
                vars[i * nCities + i].Set(Copt.DblInfo.UB, 0.0);
            }

            // Set TSP callback instance.
            TspCallback tcb = new TspCallback(vars, nCities);
            model.SetCallback(tcb, Copt.Consts.CBCONTEXT_MIPSOL);

            // Solve the TSP problem.
            model.Solve();
            int hasMipSol = model.GetIntAttr(Copt.IntAttr.HasMipSol);
            Console.WriteLine("\n[HasMipSol] = " + hasMipSol);

            if (hasMipSol > 0)
            {
                double[] sols = model.Get(Copt.DblInfo.Value, vars);

                int[] tour = FindSubTour(nCities, sols);
                Console.Write("\n  Best Tour:");
                for (int i = 0; i < tour.Length; i++)
                {
                    Console.Write(" " + tour[i]);
                }
                Console.WriteLine("\n  Best Cost: " + model.GetDblAttr(Copt.DblAttr.BestObj));
            }
        }
        catch (CoptException e)
        {
            Console.WriteLine("Error code: " + e.GetCode() + ". " + e.Message);
        }
    }

    // Calculate euclidean distance between i-th and j-th points.
    private static double GetDistance(double[] px, double[] py, int i, int j)
    {
        double dx = px[i] - px[j];
        double dy = py[i] - py[j];
        return Math.Sqrt(dx * dx + dy * dy);
    }

    private static int NextUnvisited(bool[] visited, int nCities)
    {
        int node = 0;
        while (node < nCities && visited[node])
        {
            node++;
        }
        return node;
    }

    /*
     * Given an integer-feasible solution 'sols' of size nCities * nCities,
     * find the smallest sub-tour. Result is returned as 'tour' list.
     */
    private static int[] FindSubTour(int nCities, double[] sols)
    {
        bool[] visited = new bool[nCities];
        List<int> bestTour = new List<int>();

        int curr = 0;
        while (curr < nCities)
        {
            // Search a tour starting from current city until the tour is closed.
            List<int> tour = new List<int>();
            bool closed = false;
            while (!closed)
            {
                tour.Add(curr);
                visited[curr] = true;
                // Enumerate all unvisited neighbors of current city.
                for (int j = 0; j < nCities; j++)
                {
                    if (sols[curr * nCities + j] > 0.5 && !visited[j])
                    {
                        curr = j;
                        break;
                    }
                    // If all neighbors are visited, the tour is closed.
                    closed = (j == nCities - 1);
                }
            }

            // Save the current tour if it is better.
            if (bestTour.Count == 0 || bestTour.Count > tour.Count)
            {
                bestTour = tour;
            }

            // Update curr for next tour.
            curr = NextUnvisited(visited, nCities);
        }

        return bestTour.ToArray();
    }
}
