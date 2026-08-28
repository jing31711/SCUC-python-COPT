/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
using Copt;
using System;

/*
 * We will compute feasibility relaxation for the following infeasible LP problem,
 * the problem is 'itest6' test case from netlib-infeas:
 * 
 * Minimize
 *  X2 + X3 + X4
 * Subject To
 *  ROW1: 0.8 X3 + X4 <= 10000
 *  ROW2: X1 <= 90000
 *  ROW3: 2 X6 - X8 <= 10000
 *  ROW4: - X2 + X3 >= 50000
 *  ROW5: - X2 + X4 >= 87000
 *  ROW6: X3 <= 50000
 *  ROW7: - 3 X5 + X7 >= 10000
 *  ROW8: 0.5 X5 + 0.6 X6 <= 300000
 *  ROW9: X2 - 0.05 X3 = 5000
 *  ROW10: X2 - 0.04 X3 - 0.05 X4 = 4500
 *  ROW11: X2 >= 80000
 * END
 */
public class feasrelax_ex1
{
    public static void Main()
    {
        try
        {
            Envr env = new Envr();
            Model model = env.CreateModel("feasrelax_ex1");

            // Add variables
            Var x1 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X1");
            Var x2 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X2");
            Var x3 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X3");
            Var x4 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X4");
            Var x5 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X5");
            Var x6 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X6");
            Var x7 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X7");
            Var x8 = model.AddVar(0.0, Copt.Consts.INFINITY, 0.0, Copt.Consts.CONTINUOUS, "X8");

            // Add constraints
            model.AddConstr(0.8 * x3 + x4 <= 10000, "ROW1");
            model.AddConstr(x1 <= 90000, "ROW2");
            model.AddConstr(2 * x6 - x8 <= 10000, "ROW3");
            model.AddConstr(-x2 + x3 >= 50000, "ROW4");
            model.AddConstr(-x2 + x4 >= 87000, "ROW5");
            model.AddConstr(x3 <= 50000, "ROW6");
            model.AddConstr(-3 * x5 + x7 >= 10000, "ROW7");
            model.AddConstr(0.5 * x5 + 0.6 * x6 <= 300000, "ROW8");
            model.AddConstr(x2 - 0.05 * x3 == 5000, "ROW9");
            model.AddConstr(x2 - 0.04 * x3 - 0.05 * x4 == 4500, "ROW10");
            model.AddConstr(x2 >= 80000, "ROW11");

            // Set objective
            model.SetObjective(x2 + x3 + x4, Copt.Consts.MINIMIZE);

            // Set parameters
            model.SetDblParam(Copt.DblParam.TimeLimit, 60);

            // Solve the problem
            model.Solve();

            // Compute feasibility relaxation if problem is infeasible
            if (model.GetIntAttr(Copt.IntAttr.LpStatus) == Copt.Status.INFEASIBLE)
            {
                // Set feasibility relaxation mode: minimize sum of violations
                model.SetIntParam(Copt.IntParam.FeasRelaxMode, 0);

                // Compute feasibility relaxation
                model.FeasRelax(1, 1);

                // Check if feasibility relaxation solution is available
                if (model.GetIntAttr(Copt.IntAttr.HasFeasRelaxSol) == 1)
                {
                    ConstrArray cons = model.GetConstrs();
                    VarArray vars = model.GetVars();

                    // Retrieve feasibility relaxation result
                    Console.WriteLine("\n======================== FeasRelax result ========================");

                    for (int iRow = 0; iRow < cons.Size(); ++iRow)
                    {
                        Constraint con = cons.GetConstr(iRow);
                        double lowRelax = con.Get(Copt.DblInfo.RelaxLB);
                        double uppRelax = con.Get(Copt.DblInfo.RelaxUB);
                        if (lowRelax != 0.0 || uppRelax != 0.0)
                        {
                            Console.WriteLine("{0}: viol = ({1}, {2})", con.GetName(), lowRelax, uppRelax);
                        }
                    }

                    Console.WriteLine("");
                    for (int iCol = 0; iCol < vars.Size(); ++iCol)
                    {
                        Var var = vars.GetVar(iCol);
                        double lowRelax = var.Get(Copt.DblInfo.RelaxLB);
                        double uppRelax = var.Get(Copt.DblInfo.RelaxUB);
                        if (lowRelax != 0.0 || uppRelax != 0.0)
                        {
                            Console.WriteLine("{0}: viol = ({1}, {2})", var.GetName(), lowRelax, uppRelax);
                        }
                    }

                    // Write relaxed problem to file
                    model.WriteRelax("feasrelax_ex1.relax");
                }
            }
        }
        catch (CoptException e)
        {
            Console.WriteLine("Error Code = {0}", e.GetCode());
            Console.WriteLine(e.Message);
        }
    }
}
