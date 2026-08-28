/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include <iostream>
#include <string>
#include <vector>
#include "coptcpp_pch.h"

using namespace std;

// Cutting-stock data
const int rollWidth = 115;
vector<int> rollSize = {25, 40, 50, 55, 70};
vector<int> rollDemand = {50, 36, 24, 8, 30};
const int nKind = rollSize.size();
const int nInitPat = nKind;

// Maximal number of CG iterations
const int MAX_CGTIME = 1000;

// Report solution of the RMP model
void reportRMP(Model model)
{
  if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_OPTIMAL)
  {
    cout << "Using " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << " rolls" << endl;

    VarArray vars = model.GetVars();
    for (int i = 0; i < vars.Size(); i++)
    {
      if (vars[i].Get(COPT_DBLINFO_VALUE) > 1e-6)
      {
        cout << "  " << vars[i].GetName() << " = " << vars[i].Get(COPT_DBLINFO_VALUE) << endl;
      }
    }
  }
}

// Report solution of the SUB model
void reportSUB(Model model)
{
  if (model.GetIntAttr(COPT_INTATTR_MIPSTATUS) == COPT_MIPSTATUS_OPTIMAL)
  {
    cout << "Price: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << endl;
  }
}

// Report solution of the MIP model
void reportMIP(Model model)
{
  if (model.GetIntAttr(COPT_INTATTR_MIPSTATUS) == COPT_MIPSTATUS_OPTIMAL)
  {
    cout << "\nBest MIP objective value: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << " rolls" << endl;

    VarArray vars = model.GetVars();
    for (int i = 0; i < vars.Size(); i++)
    {
      if (vars[i].Get(COPT_DBLINFO_VALUE) > 1e-6)
      {
        cout << "  " << vars[i].GetName() << " = " << vars[i].Get(COPT_DBLINFO_VALUE) << endl;
      }
    }
  }
}

int main(int argc, char* argv[])
{
  try
  {
    // Create COPT environment
    Envr env;

    // Create RMP and SUB model
    Model mCutOpt = env.CreateModel("mCutOpt");
    Model mPatGen = env.CreateModel("mPatGen");

    // Disable log information
    mCutOpt.SetIntParam(COPT_INTPARAM_LOGGING, 0);
    mPatGen.SetIntParam(COPT_INTPARAM_LOGGING, 0);

    // Build the RMP model
    VarArray vcut = mCutOpt.AddVars(nInitPat, COPT_CONTINUOUS, "VCut");

    // For each width, roll cuts should meet demands
    vector<vector<int>> nbr;
    nbr.resize(nKind);
    for (int i = 0; i < nKind; i++)
    {
      nbr[i].resize(nInitPat);
      nbr[i][i] = rollWidth / rollSize[i];
    }

    ConstrArray cfill;
    for (int i = 0; i < nKind; i++)
    {
      Expr expr;
      for (int j = 0; j < nInitPat; j++)
      {
        expr += nbr[i][j] * vcut[j];
      }
      string name = "CFill" + to_string(i);
      cfill.PushBack(mCutOpt.AddConstr(expr >= rollDemand[i], name.c_str()));
    }

    // Minimize total rolls cut
    Expr obj;
    for (int i = 0; i < vcut.Size(); i++)
    {
      obj += vcut[i];
    }
    mCutOpt.SetObjective(obj, COPT_MINIMIZE);

    // Build the SUB model
    VarArray vuse = mPatGen.AddVars(nKind, COPT_INTEGER, "VUse");

    Expr expr;
    for (int i = 0; i < rollSize.size(); i++)
    {
      expr += vuse[i] * rollSize[i];
    }
    mPatGen.AddConstr(expr <= rollWidth, "Width_Limit");

    // Main CG loop
    cout << "************** Column Generation Loop **************" << endl;
    for (int iter = 0; iter < MAX_CGTIME; iter++)
    {
      cout << "Iteration " << iter << ":" << endl;

      // Solve the RMP modeland report solution
      mCutOpt.Solve();
      reportRMP(mCutOpt);

      // Get the dual values of constraints
      vector<double> prices(cfill.Size());
      mCutOpt.Get(COPT_DBLINFO_DUAL, cfill, prices.data());

      // Update objective function of SUB model
      Expr subObj(1.0);
      for (int i = 0; i < vuse.Size(); i++)
      {
        subObj -= vuse[i] * prices[i];
      }
      mPatGen.SetObjective(subObj, COPT_MINIMIZE);

      // Solve the SUB modeland report solution
      mPatGen.Solve();
      reportSUB(mPatGen);

      // Test if CG iteration has converged
      if (mPatGen.GetDblAttr(COPT_DBLATTR_BESTOBJ) >= -1e-6)
      {
        break;
      }

      // Add new variable to RMP model
      vector<double> newnbr(vuse.Size());
      mPatGen.Get(COPT_DBLINFO_VALUE, vuse, newnbr.data());

      Column cutcol;
      cutcol.AddTerms(cfill, newnbr.data(), vuse.Size());
      string name = "NPat_" + to_string(iter);
      mCutOpt.AddVar(0, COPT_INFINITY, 1, COPT_CONTINUOUS, cutcol, name.c_str());
    }
    cout << "**************** End Loop ****************" << endl;

    // Set all variables in RMP model to integers
    VarArray vars = mCutOpt.GetVars();
    for (int i = 0; i < vars.Size(); i++)
    {
      vars[i].SetType(COPT_INTEGER);
    }

    // Solve the MIP model and report solution
    mCutOpt.Solve();
    reportMIP(mCutOpt);
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
