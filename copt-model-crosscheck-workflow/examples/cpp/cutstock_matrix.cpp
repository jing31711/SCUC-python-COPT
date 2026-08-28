/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * Using the matrix modeling API to solve the cutstock problem:
 * Coefficients:
 *   d = [50, 36, 24, 8, 30]^T
 *   w = [25, 40, 50, 55, 70]^T
 *   W = 115
 *   e is ones(200)
 *
 * Minimize:
 *   e^T @ y
 *
 * Subject to:
 *  X @ e >= d
 *  X^T @ w >= W * y
 *  y[:-1] >= y[1:]
 *
 * Bounds:
 *   X >= 0 and integer
 *   y binary
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
    int rszData[5] = {25, 40, 50, 55, 70};
    NdArray<int, 1> rollsize(Shape<1>(5), rszData, 5);

    int rdmData[5] = {50, 36, 24, 8, 30};
    NdArray<int, 1> rolldemand(Shape<1>(5), rdmData, 5);

    int rollwidth = 115;
    int nkind = rollsize.GetSize();
    int ndemand = 200;

    // Create COPT environment and model
    Envr env;
    Model model = env.CreateModel("cutstock matrix");

    // Add 2-dimensional integer variables X (MVar)
    MVar<2> ncut = model.AddMVar(Shape<2>(nkind, ndemand), COPT_INTEGER, "ncut");

    // Add 1-dimensional binary variables y (MVar)
    MVar<1> ifcut = model.AddMVar(Shape<1>(ndemand), COPT_BINARY, "ifcut");

    // Add demand constraints: X @ e >= d, here e is ones
    NdArray<int, 1> ones(Shape<1>(ndemand), 1);
    model.AddMConstr(Mat::matmult(ncut, ones) >= rolldemand);

    // Add rollwidth constraints: X^T @ w >= W * y
    model.AddMConstr(Mat::matmult(rollsize, ncut) <= rollwidth * ifcut);

    // Add Optional constraints: Symmetry breaking constraints
    model.AddMConstr(ifcut[Mat::make_view(0, -1)] >= ifcut[Mat::make_view(1, ndemand)]);

    // Set objective function: minimize e^T @ y
    model.SetObjective(Mat::matmult(ones, ifcut).Item(), COPT_MINIMIZE);

    // Optimize model
    model.Solve();

    // Get and print solution if having MIP solution
    int hasMipSol = model.GetIntAttr(COPT_INTATTR_HASMIPSOL);
    cout << "\n[HasMipSol] = " << hasMipSol << endl;
    if (hasMipSol)
    {
      for (int i = 0; i < nkind; i++)
      {
        for (int j = 0; j < ndemand; j++)
        {
          Var item = ncut[i][j].Item(0);
          if (item.Get("Value") > 1e-6)
          {
            cout << item.GetName() << " = " << item.Get("Value") << endl;
          }
        }
      }
      cout << "Best MIP objective value:: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << endl;
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
