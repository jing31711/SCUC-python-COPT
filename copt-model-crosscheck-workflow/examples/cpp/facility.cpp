/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

using namespace std;

/*
 * Facility Location Problem:
 *
 *  A company currently ships its product from 5 plants to 4 warehouses.
 *  It is considering closing some plants to reduce costs. What plants should
 *  the company close, in order to minimize transportation and fixed costs?
 *
 *  Each plant has fixed cost to maintain and given per-unit transport cost
 *  from plant i to warehouse j.
 *
 *  Objective:
 *    Minimize transportation and fixed costs
 *
 *  Subject to:
 *    - Each warehouse has a demand that must be satisfied by plants.
 *    - Each plant makes products up to capacity.
 *
 */
int main(int argc, char* argv[])
{
  // Declare number of plants and warehouses
  const int nPlants = 5;
  const int nWarehouses = 4;

  // Demand in units for each warehouse
  double Demand[] = {15, 18, 14, 20};

  // Capacity in units for each plant
  double Capacity[] = {20, 22, 17, 19, 18};

  // Fixed costs for each plant
  double FixedCosts[] = {12000, 15000, 17000, 13000, 16000};

  // Transportation costs per unit from plants to warehouses
  double TransCosts[][nPlants] = {{4000, 2000, 3000, 2500, 4500}, {2500, 2600, 3400, 3000, 4000},
    {1200, 1800, 2600, 4100, 3000}, {2200, 2600, 3100, 3700, 3200}};

  try
  {
    Envr env;
    Model model = env.CreateModel("Facility");

    // Plant decision variables: vp[i] indicates whether plant i is open.
    vector<char> vtype(nPlants, COPT_BINARY);
    VarArray vp = model.AddCols(nPlants, FixedCosts, vtype.data(), NULL, NULL, NULL);

    // Transportation variables: transport vx[j][i] units from plant i to warehouse j
    vector<VarArray> vx(nWarehouses);
    for (int j = 0; j < nWarehouses; ++j)
    {
      vx[j] = model.AddCols(nPlants, TransCosts[j], NULL, NULL, NULL, NULL);
    }

    // Capacity constarint: each open plant makes products up to capacity.
    for (int i = 0; i < nPlants; ++i)
    {
      vector<int> idx(1, vp[i].GetIdx());
      vector<double> elem(1, -Capacity[i]);

      for (int j = 0; j < nWarehouses; ++j)
      {
        idx.push_back(vx[j][i].GetIdx());
        elem.push_back(1.0);
      }
      model.AddRow(nWarehouses + 1, idx.data(), elem.data(), COPT_LESS_EQUAL, 0.0);
    }

    // Demand constraint: each warehouse has a demand that must be satisfied by plants.
    vector<int> rowBeg(nWarehouses + 1, 0);
    vector<int> rowIdx;
    vector<double> rowElem;
    for (int j = 0; j < nWarehouses; ++j)
    {
      rowBeg[j + 1] = rowBeg[j] + nPlants;
      for (int i = 0; i < nPlants; ++i)
      {
        rowIdx.push_back(vx[j][i].GetIdx());
        rowElem.push_back(1.0);
      }
    }

    vector<char> sense(nWarehouses, COPT_EQUAL);
    const char* names[nWarehouses] = {"Demand0", "Demand1", "Demand2", "Demand3"};
    model.AddRows(nWarehouses, rowBeg.data(), NULL, rowIdx.data(), rowElem.data(), sense.data(), Demand, NULL, names);

    // The objective is to minimize transportation and fixed costs
    model.SetObjSense(COPT_MINIMIZE);

    model.Solve();

    int status = model.GetIntAttr(COPT_INTATTR_MIPSTATUS);
    if (status != COPT_MIPSTATUS_OPTIMAL)
    {
      cout << "Optimal solution is not available, status=" << status << endl;
      return 0;
    }

    cout << "\nTotal Cost: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << endl;

    for (int i = 0; i < nPlants; ++i)
    {
      // If plant decision variable is less than 0.99, we consider it "closed" and skip further processing
      if (vp[i].Get(COPT_DBLINFO_VALUE) < 0.99)
      {
        cout << "Plant " << i << " closed" << endl;
        continue;
      }

      cout << "Plant " << i << " open:" << endl;
      for (int j = 0; j < nWarehouses; ++j)
      {
        // Get the transportation variable from plant i to warehouse j
        double val = vx[j][i].Get(COPT_DBLINFO_VALUE);

        // Only report significant transportation flow
        if (val > 0.0001)
        {
          cout << "  Transport " << val << " units to warehouse " << j << endl;
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
