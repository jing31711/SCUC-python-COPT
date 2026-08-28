/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example solves the mixed SDP problem:
 *
 *                  [1,  0]
 *  minimize     Tr [     ] * X + x0 + 2 * x1 + 3
 *                  [0,  1]
 *
 *                  [0,  1]
 *  subject to   Tr [     ] * X - x0 - 2 * x1 >= 0
 *                  [1,  0]
 *
 *                  [0,  1]        [5,  1]   [1,  0]
 *             x0 * [     ] + x1 * [     ] - [     ] in PSD
 *                  [1,  5]        [1,  0]   [0,  2]
 *
 *             x0, x1 free, X in PSD
 */

#include "coptcpp_pch.h"

using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    // Create COPT environment
    Envr env;

    // Create COPT model
    Model model = env.CreateModel("lmi_ex1");

    // Add symmetric matrix C0
    SymMatrix C0 = model.AddEyeMat(2);

    // Add symmetric matrix C1
    vector<int> C1_rows = {1};
    vector<int> C1_cols = {0};
    vector<double> C1_vals = {1.0};
    SymMatrix C1 = model.AddSparseMat(2, 1, C1_rows.data(), C1_cols.data(), C1_vals.data());

    // Add symmetric matrix H1
    vector<int> H1_rows = {1, 1};
    vector<int> H1_cols = {0, 1};
    vector<double> H1_vals = {1.0, 5.0};
    SymMatrix H1 = model.AddSparseMat(2, 2, H1_rows.data(), H1_cols.data(), H1_vals.data());

    // Add symmetric matrix H2
    vector<int> H2_rows = {0, 1};
    vector<int> H2_cols = {0, 0};
    vector<double> H2_vals = {5.0, 1.0};
    SymMatrix H2 = model.AddSparseMat(2, 2, H2_rows.data(), H2_cols.data(), H2_vals.data());

    // Add symmetric matrix D1
    vector<int> D1_rows = {0, 1};
    vector<int> D1_cols = {0, 1};
    vector<double> D1_vals = {1.0, 2.0};
    SymMatrix D1 = model.AddSparseMat(2, 2, D1_rows.data(), D1_cols.data(), D1_vals.data());

    // Add PSD variable
    PsdVar X = model.AddPsdVar(2, "BAR_X");

    // Add variables
    Var x0 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0, COPT_CONTINUOUS, "x0");
    Var x1 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0, COPT_CONTINUOUS, "x1");

    // Add PSD constraint
    model.AddPsdConstr(C1 * X - x0 - 2 * x1 >= 0, "PSD_c0");

    // Add LMI constraint
    model.AddLmiConstr(H1 * x0 + H2 * x1 - D1, "LMI_c0");

    // Set PSD objective
    model.SetPsdObjective(C0 * X + x0 + 2 * x1 + 3, COPT_MINIMIZE);

    // Solve the model
    model.Solve();

    // Output solution
    if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_OPTIMAL)
    {
      cout << "\nOptimal objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;
      cout << endl;

      PsdVarArray psdvars = model.GetPsdVars();
      for (int i = 0; i < psdvars.Size(); ++i)
      {
        PsdVar psdvar = psdvars.GetPsdVar(i);
        int psdLen = psdvar.GetLen();

        vector<double> psdVal(psdLen);
        vector<double> psdDual(psdLen);

        /* Get flattened SDP primal/dual solution */
        psdvar.Get(COPT_DBLINFO_VALUE, psdVal.data(), psdLen);
        psdvar.Get(COPT_DBLINFO_DUAL, psdDual.data(), psdLen);

        cout << "SDP variable " << psdvar.GetName() << ", flattened by column:" << endl;
        cout << "Primal solution:" << endl;
        for (int j = 0; j < psdLen; ++j)
        {
          cout << "  " << psdVal[j] << endl;
        }
        cout << "Dual solution:" << endl;
        for (int j = 0; j < psdLen; ++j)
        {
          cout << "  " << psdDual[j] << endl;
        }
        cout << endl;
      }

      VarArray linvars = model.GetVars();
      int nLinCol = linvars.Size();

      vector<double> colVal(nLinCol);
      vector<double> colDual(nLinCol);

      /* Get solution for non-PSD variables */
      model.Get(COPT_DBLINFO_VALUE, linvars, colVal.data());
      model.Get(COPT_DBLINFO_REDCOST, linvars, colDual.data());

      cout << "Non-PSD variables:" << endl;
      cout << "Solution value:" << endl;
      for (int j = 0; j < nLinCol; ++j)
      {
        cout << "  " << colVal[j] << endl;
      }
      cout << "Reduced cost:" << endl;
      for (int j = 0; j < nLinCol; ++j)
      {
        cout << "  " << colDual[j] << endl;
      }
      cout << endl;

      /* Get activity and dual of PSD constraints */
      PsdConstrArray psdconstrs = model.GetPsdConstrs();
      for (int i = 0; i < psdconstrs.Size(); ++i)
      {
        PsdConstraint psdconstr = psdconstrs.GetPsdConstr(i);
        cout << "PSD constraint " << psdconstr.GetName() << ":" << endl;
        cout << "Activity: " << psdconstr.Get(COPT_DBLINFO_SLACK) << endl;
        cout << "Dual:     " << psdconstr.Get(COPT_DBLINFO_DUAL) << endl;
        cout << endl;
      }

      /* Get activity and dual of LMI constraints */
      LmiConstrArray lmiconstrs = model.GetLmiConstrs();
      for (int i = 0; i < lmiconstrs.Size(); ++i)
      {
        LmiConstraint lmiconstr = lmiconstrs.GetLmiConstr(i);
        int lmiLen = lmiconstr.GetLen();

        vector<double> lmiSlack(lmiLen);
        vector<double> lmiDual(lmiLen);

        lmiconstr.Get(COPT_DBLINFO_SLACK, lmiSlack.data(), lmiLen);
        lmiconstr.Get(COPT_DBLINFO_DUAL, lmiDual.data(), lmiLen);

        cout << "LMI constraint " << lmiconstr.GetName() << ", flattened by column:" << endl;
        cout << "Activity:" << endl;
        for (int j = 0; j < lmiLen; ++j)
        {
          cout << "  " << lmiSlack[j] << endl;
        }
        cout << "Dual:" << endl;
        for (int j = 0; j < lmiLen; ++j)
        {
          cout << "  " << lmiDual[j] << endl;
        }
        cout << endl;
      }
    }
    else
    {
      cout << "No SDP solution!" << endl;
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
