/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

using namespace std;

/*
 * Production Planning Problem:
 * A company manufactures four variants of the same product and in the final part of
 * the manufacturing process there are assembly, polishing and packing operations.
 * For each variant the time required for these operations is shown below (in minutes)
 * as is the profit per unit sold.
 *
 *  Variant   Assembly      Polish    Pack        Profit ($)
 *      1       2             3         2           1.50
 *      2       4             2         3           2.50
 *      3       3             3         2           3.00
 *      4       7             4         5           4.50
 *
 * Let Xi be the number of units of variant i (i=1, 2, 3, 4) made per year.

 * Objectives:
 *  Maximize total profit
 *  1.5*X1 + 2.5*X2 + 3.0*X3 + 4.5*X4

 * Subject to:
 *  (Assembly time) 2*X1 + 4*X2 + 3*X3 + 7*X4 <= 100000
 *  (Polish time)   3*X1 + 2*X2 + 3*X3 + 4*X4 <= 50000
 *  (Pack time)     2*X1 + 3*X2 + 2*X3 + 5*X4 <= 60000
 */

int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("Production Planning Problem");

    model.SetIntParam(COPT_INTPARAM_REQSENSITIVITY, 1);

    // Add variable Xi, the number of units of variant i of the same product
    Var X1 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "X1");
    Var X2 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "X2");
    Var X3 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "X3");
    Var X4 = model.AddVar(0.0, COPT_INFINITY, 0.0, COPT_CONTINUOUS, "X4");

    // The total assembly time should be less than 100000 miniutes
    model.AddConstr(2 * X1 + 4 * X2 + 3 * X3 + 7 * X4 <= 100000, "AssemblyTime");

    // The total polishing time should be less than 50000 miniutes
    model.AddConstr(3 * X1 + 2 * X2 + 3 * X3 + 4 * X4 <= 50000, "PolishTime");

    // The total packing time should be less than 60000 miniutes
    model.AddConstr(2 * X1 + 3 * X2 + 2 * X3 + 5 * X4 <= 60000, "PackTime");

    // The objective is to maximize total profit
    model.SetObjective(1.5 * X1 + 2.5 * X2 + 3.0 * X3 + 4.5 * X4, COPT_MAXIMIZE);

    model.Solve();

    int status = model.GetIntAttr(COPT_INTATTR_LPSTATUS);
    if (status != COPT_LPSTATUS_OPTIMAL)
    {
      cout << "Optimal solution is not available, status=" << status << endl;
      return 0;
    }

    if (model.GetIntAttr(COPT_INTATTR_HASSENSITIVITY) != 1)
    {
      cout << "Sensitivity information is not available" << endl;
      return 0;
    }

    int nCol = model.GetIntAttr(COPT_INTATTR_COLS);
    int nRow = model.GetIntAttr(COPT_INTATTR_ROWS);
    int nColRow = nCol + nRow;

    // setup data for the sensitivity information
    std::vector<double> dObjUpp(nCol);
    std::vector<double> dObjLow(nCol);
    std::vector<double> dLbUpp(nColRow);
    std::vector<double> dLbLow(nColRow);
    std::vector<double> dUbUpp(nColRow);
    std::vector<double> dUbLow(nColRow);

    VarArray vars = model.GetVars();
    ConstrArray constrs = model.GetConstrs();

    cout << endl;
    cout << "Sensitivity information of production planning problem:" << endl;
    cout << endl;

    model.Get(COPT_DBLINFO_SAOBJLOW, vars, dObjLow.data());
    model.Get(COPT_DBLINFO_SAOBJUP, vars, dObjUpp.data());

    cout << "Variant\t" << "Objective Range" << endl;
    for (int i = 0; i < nCol; i++)
    {
      cout << "  " << vars[i].GetName() << "\t(" << dObjLow[i] << ", " << dObjUpp[i] << ")" << endl;
    }
    cout << endl;

    model.Get(COPT_DBLINFO_SALBLOW, vars, dLbLow.data());
    model.Get(COPT_DBLINFO_SALBUP, vars, dLbUpp.data());
    model.Get(COPT_DBLINFO_SAUBLOW, vars, dUbLow.data());
    model.Get(COPT_DBLINFO_SAUBUP, vars, dUbUpp.data());

    cout << "Variant\t" << "LowBound Range  \t" << "UppBound Range" << endl;
    for (int i = 0; i < nCol; i++)
    {
      cout << "  " << vars[i].GetName();
      cout << "\t(" << dLbLow[i] << ", " << dLbUpp[i] << ")  ";
      cout << "\t(" << dUbLow[i] << ", " << dUbUpp[i] << ")" << endl;
    }
    cout << endl;

    model.Get(COPT_DBLINFO_SALBLOW, constrs, dLbLow.data() + nCol);
    model.Get(COPT_DBLINFO_SALBUP, constrs, dLbUpp.data() + nCol);
    model.Get(COPT_DBLINFO_SAUBLOW, constrs, dUbLow.data() + nCol);
    model.Get(COPT_DBLINFO_SAUBUP, constrs, dUbUpp.data() + nCol);

    cout << "Constraint\t" << "LHS Range    \t" << "RHS Range" << endl;
    for (int i = nCol; i < nColRow; i++)
    {
      int idx = i - nCol;
      cout << "  " << constrs[idx].GetName();
      cout << "\t(" << dLbLow[i] << ", " << dLbUpp[i] << ")";
      cout << "\t(" << dUbLow[i] << ", " << dUbUpp[i] << ")" << endl;
    }
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
