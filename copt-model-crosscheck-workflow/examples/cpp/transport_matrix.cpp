/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * Using the matrix modeling API to solve the transport problem:
 * Coefficients:
 *   C = [[39, 14, 11, 14, 16, 82,  8],
 *        [27,  9, 12,  9, 26, 95, 17],
 *        [24, 14, 17, 13, 28, 99, 20]]
 *   s = [1400, 2600, 2900]^T
 *   d = [900, 1200, 600, 400, 1700, 1100, 1000]^T
 *   e1 is ones(7)
 *   e2 is ones(3)
 *
 * Minimize:
 *   C * X
 *
 * Subject to:
 *   X @ e1 <= s
 *   X^T @ e2 >= d
 *
 * Bounds:
 *   X >= 0
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
    // Build supply and demand constriant bound vector
    int sData[3] = {1400, 2600, 2900};
    int dData[7] = {900, 1200, 600, 400, 1700, 1100, 1000};

    NdArray<int, 1> supply(Shape<1>(3), sData, 3);
    NdArray<int, 1> demand(Shape<1>(7), dData, 7);


    // Create COPT environment and model
    Envr env;
    Model model = env.CreateModel("transport matrix");


    // Add 2-dimensional variables transport (MVar)
    MVar<2> transport = model.AddMVar(Shape<2>(3, 7), COPT_CONTINUOUS, "city");

    // Add supply constraints: X @ e1 <= s
    NdArray<int, 1> e1(Shape<1>(7), 1);
    model.AddMConstr(Mat::matmult(transport, e1) <= supply);

    // Add demand constraints: X^T @ e2 >= d
    NdArray<int, 1> e2(Shape<1>(3), 1);
    model.AddMConstr(Mat::matmult(e2, transport) >= demand);

    int cData[3][7] = {{39, 14, 11, 14, 16, 82, 8}, {27, 9, 12, 9, 26, 95, 17}, {24, 14, 17, 13, 28, 99, 20}};
    NdArray<int, 2> cost(Shape<2>(3, 7), reinterpret_cast<int*>(cData), 3 * 7);

    // Set matrix objective function: minimize dot(C, X)
    model.SetObjective((cost * transport).Sum(), COPT_MINIMIZE);

    // Optimize model
    model.Solve();

    // Display solutions
    if (model.GetIntAttr(COPT_INTATTR_HASLPSOL))
    {
      cout << "Variable solution:" << endl;
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 7; j++)
        {
          Var item = transport[i][j].Item();
          cout << item.GetName() << " : " << item.Get("Value") << endl;
        }
      }
      cout << "Variable basis:" << endl;
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 7; j++)
        {
          Var item = transport[i][j].Item();
          cout << item.GetName() << " : " << item.GetBasis() << endl;
        }
      }

      cout << "Best objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;
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
