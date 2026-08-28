/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "coptcpp_pch.h"

#include <map>

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

    // Optimization parameters for Dantzig - Wolfe decomposition
    int iterCnt = 0;
    double convexPrice = 1.0;

    map<string, map<string, double>> mapPrice;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        mapPrice[ORIG[i]][DEST[j]] = 0;
      }
    }
    vector<double> propCosts;
    vector<map<string, map<string, double>>> allProdShips;

    VarArray vweight;

    // Create COPT environment
    Envr env;

    // Create COPT problem
    Model mMaster = env.CreateModel("mMaster");
    Model mSub = env.CreateModel("mSub");

    // Disable log information
    mMaster.SetIntParam(COPT_INTPARAM_LOGGING, 0);
    mSub.SetIntParam(COPT_INTPARAM_LOGGING, 0);

    // Add variable to the MASTER problem
    Var vexcess = mMaster.AddVar(0, COPT_INFINITY, 0, COPT_CONTINUOUS, "excess");

    // Add constraints to the MASTER problem
    ConstrArray cmulti[nORIG];
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        string name = "Multi_" + ORIG[i] + "_" + DEST[j];
        cmulti[i].PushBack(mMaster.AddConstr(-vexcess <= mapLimit[ORIG[i]][DEST[j]], name.c_str()));
      }
    }

    // Add initial 'convex' constraint to the MASTER problem
    Constraint cconvex = mMaster.AddConstr(Expr(0.0) == 1.0, "convex");

    // Add variables to the SUB problem
    VarArray vtrans[nORIG][nDEST];
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        for (int k = 0; k < PROD.size(); k++)
        {
          string name = "Trans_" + ORIG[i] + "_" + DEST[j] + "_" + PROD[k];
          vtrans[i][j].PushBack(mSub.AddVar(0, COPT_INFINITY, 0, COPT_CONTINUOUS, name.c_str()));
        }
      }
    }

    // Add constraints to the SUB problem
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
        mSub.AddConstr(expr == mapSupply[ORIG[i]][PROD[k]], name.c_str());
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
        mSub.AddConstr(expr == mapDemand[DEST[j]][PROD[k]], name.c_str());
      }
    }

    // Set Set objective function for MASTER problem
    mMaster.SetObjective(vexcess, COPT_MINIMIZE);

    // Dantzig - Wolfe decomposition
    cout << "************ Dantzig-Wolfe Decomposition ************" << endl;

    // Phase I
    cout << "Phase I: " << endl;

    while (true)
    {
      // Display the iteration number
      cout << "   Iteration: " << iterCnt << endl;

      // Set objective function 'Artificial Reduced Cost' for the SUB problem
      Expr obj;
      for (int i = 0; i < ORIG.size(); i++)
      {
        for (int j = 0; j < DEST.size(); j++)
        {
          for (int k = 0; k < PROD.size(); k++)
          {
            obj -= mapPrice[ORIG[i]][DEST[j]] * vtrans[i][j][k];
          }
        }
      }
      mSub.SetObjective(obj - convexPrice, COPT_MINIMIZE);

      // Solve the SUB problem
      mSub.Solve();

      // Check if problem is feasible
      if (mSub.GetDblAttr(COPT_DBLATTR_LPOBJVAL) >= -1e-6)
      {
        cout << "No feasible solution..." << endl;
        break;
      }
      else
      {
        iterCnt += 1;

        // Get the solution of 'trans' variables in the SUB problem
        vector<vector<vector<double>>> solTrans;
        solTrans.resize(ORIG.size());
        for (int i = 0; i < ORIG.size(); i++)
        {
          solTrans[i].resize(DEST.size());
        }
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            solTrans[i][j].reserve(PROD.size());
            mSub.Get(COPT_DBLINFO_VALUE, vtrans[i][j], solTrans[i][j].data());
          }
        }

        // Calculate parameters for the MASTER problem
        map<string, map<string, double>> mapPropShip;
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            double val = 0;
            for (int k = 0; k < PROD.size(); k++)
            {
              val += solTrans[i][j][k];
            }
            mapPropShip[ORIG[i]][DEST[j]] = val;
          }
        }
        double sum = 0;
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            for (int k = 0; k < PROD.size(); k++)
            {
              sum += solTrans[i][j][k] * mapCost[ORIG[i]][DEST[j]][PROD[k]];
            }
          }
        }
        propCosts.push_back(sum);
        allProdShips.push_back(mapPropShip);

        // Update constraints 'multi' in the MASTER problem
        Column colWeight;
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            colWeight.AddTerm(cmulti[i][j], mapPropShip[ORIG[i]][DEST[j]]);
          }
        }
        colWeight.AddTerm(cconvex, 1.0);

        // Add variables 'weight' to the MASTER problem
        vweight.PushBack(mMaster.AddVar(0, COPT_INFINITY, 0, COPT_CONTINUOUS, colWeight));

        // Solve the MASTER problem
        mMaster.Solve();

        // Update 'mapPrice'
        if (mMaster.GetDblAttr(COPT_DBLATTR_LPOBJVAL) <= 1e-6)
        {
          break;
        }
        else
        {
          for (int i = 0; i < ORIG.size(); i++)
          {
            for (int j = 0; j < DEST.size(); j++)
            {
              mapPrice[ORIG[i]][DEST[j]] = cmulti[i][j].Get(COPT_DBLINFO_DUAL);
            }
          }
          convexPrice = cconvex.Get(COPT_DBLINFO_DUAL);
        }
      }
    }

    // Phase II
    cout << "Phase II: " << endl;

    // Fix variable 'excess'
    double vexcess_x = vexcess.Get(COPT_DBLINFO_VALUE);
    vexcess.Set(COPT_DBLINFO_LB, vexcess_x);
    vexcess.Set(COPT_DBLINFO_UB, vexcess_x);

    // Set objective function 'Total Cost' for the MASTER problem
    Expr objTotalCost;
    for (int i = 0; i < iterCnt; i++)
    {
      objTotalCost += propCosts[i] * vweight[i];
    }
    mMaster.SetObjective(objTotalCost, COPT_MINIMIZE);

    // Solve the MASTER problem
    mMaster.Solve();

    // Update 'mapPrice' and 'priceconvex'
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        mapPrice[ORIG[i]][DEST[j]] = cmulti[i][j].Get(COPT_DBLINFO_DUAL);
      }
    }
    convexPrice = cconvex.Get(COPT_DBLINFO_DUAL);

    // Set the objection function 'Reduced Cost' for the SUB problem
    Expr objReducedCost;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        for (int k = 0; k < PROD.size(); k++)
        {
          objReducedCost += (mapCost[ORIG[i]][DEST[j]][PROD[k]] - mapPrice[ORIG[i]][DEST[j]]) * vtrans[i][j][k];
        }
      }
    }
    mSub.SetObjective(objReducedCost - convexPrice, COPT_MINIMIZE);

    while (true)
    {
      cout << "   Iteration: " << iterCnt << endl;

      // Solve the SUB problem
      mSub.Solve();

      if (mSub.GetDblAttr(COPT_DBLATTR_LPOBJVAL) >= -1e-6)
      {
        cout << "   Found optimal solution" << endl;
        break;
      }
      else
      {
        iterCnt += 1;

        // Get the solution of 'trans' variables in the SUB problem
        vector<vector<vector<double>>> solTrans;
        solTrans.resize(ORIG.size());
        for (int i = 0; i < ORIG.size(); i++)
        {
          solTrans[i].resize(DEST.size());
        }
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            for (int k = 0; k < PROD.size(); k++)
            {
              solTrans[i][j].push_back(vtrans[i][j][k].Get(COPT_DBLINFO_VALUE));
            }
          }
        }

        // Calculate parameters for the MASTER problem
        map<string, map<string, double>> mapPropShip;
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            double val = 0;
            for (int k = 0; k < PROD.size(); k++)
            {
              val += solTrans[i][j][k];
            }
            mapPropShip[ORIG[i]][DEST[j]] = val;
          }
        }

        double sum = 0;
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            for (int k = 0; k < PROD.size(); k++)
            {
              sum += solTrans[i][j][k] * mapCost[ORIG[i]][DEST[j]][PROD[k]];
            }
          }
        }
        propCosts.push_back(sum);
        allProdShips.push_back(mapPropShip);

        // Update constraints 'multi' in the MASTER problem
        Column colWeight;
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            colWeight.AddTerm(cmulti[i][j], mapPropShip[ORIG[i]][DEST[j]]);
          }
        }
        colWeight.AddTerm(cconvex, 1.0);

        // Add variables 'weight' to the MASTER problem
        vweight.PushBack(mMaster.AddVar(0, COPT_INFINITY, propCosts[iterCnt - 1], COPT_CONTINUOUS, colWeight));

        // Solve the MASTER problem
        mMaster.Solve();

        // Update 'mapPrice' and 'convexPrice'
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            mapPrice[ORIG[i]][DEST[j]] = cmulti[i][j].Get(COPT_DBLINFO_DUAL);
          }
        }
        convexPrice = cconvex.Get(COPT_DBLINFO_DUAL);

        Expr objReducedCost2;
        for (int i = 0; i < ORIG.size(); i++)
        {
          for (int j = 0; j < DEST.size(); j++)
          {
            for (int k = 0; k < PROD.size(); k++)
            {
              objReducedCost2 += (mapCost[ORIG[i]][DEST[j]][PROD[k]] - mapPrice[ORIG[i]][DEST[j]]) * vtrans[i][j][k];
            }
          }
        }
        mSub.SetObjective(objReducedCost2 - convexPrice, COPT_MINIMIZE);
      }
    }

    // Phase III
    cout << "Phase III: " << endl;

    map<string, map<string, double>> mapOptShip;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        double val = 0;
        for (int k = 0; k < iterCnt; k++)
        {
          val += allProdShips[k][ORIG[i]][DEST[j]] * vweight[k].Get(COPT_DBLINFO_VALUE);
        }
        mapOptShip[ORIG[i]][DEST[j]] = val;
      }
    }

    // Set objective function 'Opt Cost' for SUB problem
    Expr obj;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        for (int k = 0; k < PROD.size(); k++)
        {
          obj += mapCost[ORIG[i]][DEST[j]][PROD[k]] * vtrans[i][j][k];
        }
      }
    }
    mSub.SetObjective(obj, COPT_MINIMIZE);

    // Add new constraints to the SUB problem
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        Expr expr;
        for (int k = 0; k < PROD.size(); k++)
        {
          expr += vtrans[i][j][k];
        }
        mSub.AddConstr(expr == mapOptShip[ORIG[i]][DEST[j]]);
      }
    }

    // Solve the SUB problem
    mSub.Solve();
    cout << "******************** End Loop ********************" << endl;

    // Report solution
    cout << "\n***************** Summary Report *****************" << endl;

    cout << "Optimal objective value: " << mSub.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;
    cout << "Solution of variables: " << endl;
    for (int i = 0; i < ORIG.size(); i++)
    {
      for (int j = 0; j < DEST.size(); j++)
      {
        for (int k = 0; k < PROD.size(); k++)
        {
          if (abs(vtrans[i][j][k].Get(COPT_DBLINFO_VALUE)) >= 1e-6)
          {
            cout << "  " << vtrans[i][j][k].GetName() << " = " << vtrans[i][j][k].Get(COPT_DBLINFO_VALUE) << endl;
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
