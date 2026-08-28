/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example solves the following SDP model:
 *
 *                 [2, 1, 0]
 *  Minimize:   Tr [1, 2, 1] * X + x0
 *                 [0, 1, 2]
 *
 *  Subject to:
 *                         [1, 0, 0]
 *              PSD_R1: Tr [0, 1, 0] * X <= 0.8
 *                         [0, 0, 1]
 *
 *                         [1, 1, 1]
 *              PSD_R2: Tr [1, 1, 1] * X + x1 + x2 = 0.6
 *                         [1, 1, 1]
 *
 *                  R1: x0 + x1 + x2 <= 0.9
 *                  C0: x0^2 >= x1^2 + x2^2    (regular cone)
 *
 *  Where:
 *              x0, x1, x2 are non-negative, X in PSD
 */

#include "coptcpp_pch.h"

using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    Envr env;

    Model model = env.CreateModel("sdp_ex1");

    // Add symmetric matrix C
    vector<int> rows = {0, 1, 1, 2, 2};
    vector<int> cols = {0, 0, 1, 1, 2};
    vector<double> vals = {2.0, 1.0, 2.0, 1.0, 2.0};
    SymMatrix C = model.AddSparseMat(3, 5, rows.data(), cols.data(), vals.data());

    // Add identity matrix A1
    SymMatrix A1 = model.AddEyeMat(3);
    // Add ones matrix A2
    SymMatrix A2 = model.AddOnesMat(3);

    // Add PSD variable
    PsdVar barX = model.AddPsdVar(3, "BAR_X");

    // Add variables
    Var x0 = model.AddVar(0.0, COPT_INFINITY, 0, COPT_CONTINUOUS, "x0");
    Var x1 = model.AddVar(0.0, COPT_INFINITY, 0, COPT_CONTINUOUS, "x1");
    Var x2 = model.AddVar(0.0, COPT_INFINITY, 0, COPT_CONTINUOUS, "x2");

    // Add PSD constraints
    model.AddPsdConstr(A1 * barX <= 0.8, "PSD_R1");
    model.AddPsdConstr(A2 * barX + x1 + x2 == 0.6, "PSD_R2");

    // Add linear constraint
    model.AddConstr(x0 + x1 + x2 <= 0.9, "R1");

    // Add regular cone
    VarArray vars;
    vars.PushBack(x0);
    vars.PushBack(x1);
    vars.PushBack(x2);
    model.AddCone(vars, COPT_CONE_QUAD);

    // Set PSD objective
    model.SetPsdObjective(C * barX + x0, COPT_MINIMIZE);

    // Optimize model
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

        cout << "SDP variable " << i << ", flattened by column:" << endl;
        cout << "Primal solution:" << endl;
        for (int j = 0; j < psdLen; ++j)
        {
          cout << "  " << psdVal[j] << endl;
        }
        cout << "\nDual solution:" << endl;
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
      cout << "\nReduced cost:" << endl;
      for (int j = 0; j < nLinCol; ++j)
      {
        cout << "  " << colDual[j] << endl;
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
