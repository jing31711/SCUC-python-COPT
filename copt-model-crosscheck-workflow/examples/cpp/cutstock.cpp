/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include <iostream>
#include <string>
#include <vector>
#include "coptcpp_pch.h"

using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    const int rollWidth = 115;
    vector<int> rollSize = {25, 40, 50, 55, 70};
    vector<int> rollDemand = {50, 36, 24, 8, 30};
    const int nKind = 5;
    const int nDemand = 200;

    // Create COPT environment
    Envr env;

    // Create COPT problem
    Model model = env.CreateModel("cutstock");

    // Add variables to problem
    VarArray vcut[nKind];
    for (int i = 0; i < nKind; i++)
    {
      for (int j = 0; j < nDemand; j++)
      {
        string name = "X_" + to_string(i) + "_" + to_string(j);
        vcut[i].PushBack(model.AddVar(0, COPT_INFINITY, 0, COPT_INTEGER, name.c_str()));
      }
    }

    VarArray bcut = model.AddVars(nDemand, COPT_BINARY, "IfCut");

    // Add constraints to problem
    for (int i = 0; i < nKind; i++)
    {
      Expr expr;
      for (int j = 0; j < nDemand; j++)
      {
        expr += vcut[i][j];
      }
      model.AddConstr(expr >= rollDemand[i]);
    }

    for (int j = 0; j < nDemand; j++)
    {
      Expr expr;
      for (int i = 0; i < nKind; i++)
      {
        expr += vcut[i][j] * rollSize[i];
      }
      model.AddConstr(expr <= rollWidth * bcut[j]);
    }

    // Optional: Symmetry breaking constraints
    for (int i = 1; i < nDemand; i++)
    {
      model.AddConstr(bcut[i - 1] >= bcut[i]);
    }

    // Set objective function
    Expr obj;
    for (int i = 0; i < bcut.Size(); i++)
    {
      obj += bcut[i];
    }
    model.SetObjective(obj, COPT_MINIMIZE);

    // Set optimization parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 120);

    // Solve the problem
    model.Solve();

    // Display the solution
    if (model.GetIntAttr(COPT_INTATTR_HASMIPSOL))
    {
      cout << "\nBest MIP objective value: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << endl;
      cout << "Cut patterns:" << endl;
      for (int i = 0; i < nKind; i++)
      {
        for (int j = 0; j < nDemand; j++)
        {
          if (vcut[i][j].Get(COPT_DBLINFO_VALUE) > 1e-6)
          {
            cout << "  " << vcut[i][j].GetName() << " = " << vcut[i][j].Get(COPT_DBLINFO_VALUE) << endl;
          }
        }
      }
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
