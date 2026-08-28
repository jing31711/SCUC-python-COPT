/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "coptcpp_pch.h"

using namespace std;

#define nORIG 3
#define nDEST 7
#define nPROD 3

int main(int argc, char* argv[])
{
  try
  {
    vector<string> ORIG = {"GARY", "CLEV", "PITT"};
    vector<string> DEST = {"FRA", "DET", "LAN", "WIN", "STL", "FRE", "LAF"};
    vector<string> PROD = {"Bands", "Coils", "Plate"};

    vector<int> supplies = {400, 800, 200, 700, 1600, 300, 800, 1800, 300};

    map<string, map<string, int>> mapSupply;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < PROD.size(); j++)
      {
        mapSupply[ORIG[i]][PROD[j]] = supplies[j + i * PROD.size()];
      }
    }

    vector<int> demands = {300, 500, 100, 300, 750, 100, 100, 400, 0, 75, 250, 50, 650, 950, 200, 225, 850, 100, 250,
      500, 250};

    map<string, map<string, int>> mapDemand;
    for (int i = 0; i < DEST.size(); i++)
    {
      for (int j = 0; j < PROD.size(); j++)
      {
        mapDemand[DEST[i]][PROD[j]] = demands[j + i * PROD.size()];
      }
    }

    vector<double> limits;
    limits.assign(ORIG.size() * DEST.size(), 625.0);

    map<string, map<string, double>> mapLimit;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        mapLimit[ORIG[i]][DEST[j]] = limits[j + i * DEST.size()];
      }
    }

    int costs[nORIG][nDEST][nPROD] = {
      {{30, 39, 41}, {10, 14, 15}, {8, 11, 12}, {10, 14, 16}, {11, 16, 17}, {71, 82, 86}, {6, 8, 8}},
      {{22, 27, 29}, {7, 9, 9}, {10, 12, 13}, {7, 9, 9}, {21, 26, 28}, {82, 95, 99}, {13, 17, 18}},
      {{19, 24, 26}, {11, 14, 14}, {12, 17, 17}, {10, 13, 13}, {25, 28, 31}, {83, 99, 104}, {15, 20, 20}}};

    map<string, map<string, map<string, int>>> mapCost;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        for (int k = 0; k < PROD.size(); k++)
        {
          mapCost[ORIG[i]][DEST[j]][PROD[k]] = costs[i][j][k];
        }
      }
    }

    // Create COPT environment
    Envr env;

    // Create COPT problem
    Model model = env.CreateModel("multicommodity");

    // Add variables to problem
    VarArray vtrans[nORIG][nDEST];
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        for (int k = 0; k < PROD.size(); k++)
        {
          string name = "Trans_" + ORIG[i] + "_" + DEST[j] + "_" + PROD[k];
          vtrans[i][j].PushBack(model.AddVar(0, COPT_INFINITY, 0, COPT_CONTINUOUS, name.c_str()));
        }
      }
    }

    // Add constraints to problem
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int k = 0; k < PROD.size(); k++)
      {
        Expr expr;
        for (int j = 0; j < DEST.size(); j++)
        {
          expr += vtrans[i][j][k];
        }
        string name = "Supply_" + ORIG[i] + "_" + PROD[k];
        model.AddConstr(expr == mapSupply[ORIG[i]][PROD[k]], name.c_str());
      }
    }

    for (int j = 0; j < DEST.size(); j++)
    {
      for (int k = 0; k < PROD.size(); k++)
      {
        Expr expr;
        for (int i = 0; i < ORIG.size(); i++)
        {
          expr += vtrans[i][j][k];
        }
        string name = "Demand_" + DEST[j] + "_" + PROD[k];
        model.AddConstr(expr == mapDemand[DEST[j]][PROD[k]], name.c_str());
      }
    }

    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        Expr expr;
        for (int k = 0; k < PROD.size(); k++)
        {
          expr += vtrans[i][j][k];
        }
        string name = "Limit_" + ORIG[i] + "_" + DEST[j];
        model.AddConstr(expr <= mapLimit[ORIG[i]][DEST[j]], name.c_str());
      }
    }

    // Set objective function
    Expr obj;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        for (int k = 0; k < PROD.size(); k++)
        {
          obj += vtrans[i][j][k] * mapCost[ORIG[i]][DEST[j]][PROD[k]];
        }
      }
    }
    model.SetObjective(obj, COPT_MINIMIZE);

    // Set optimization parameters
    model.SetDblParam(COPT_DBLPARAM_TIMELIMIT, 100);

    // Solve the problem
    model.Solve();

    // Display solution
    if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_OPTIMAL)
    {
      cout << "\nOptimal objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;

      VarArray vars = model.GetVars();
      cout << "Solution of variables: " << endl;
      for (int i = 0; i < vars.Size(); i++)
      {
        if (abs(vars[i].Get(COPT_DBLINFO_VALUE)) >= 1e-6)
        {
          cout << "  " << vars[i].GetName() << " = " << vars[i].Get(COPT_DBLINFO_VALUE) << endl;
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
