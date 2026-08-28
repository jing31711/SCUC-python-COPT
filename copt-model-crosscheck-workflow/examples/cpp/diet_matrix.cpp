/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * Using the matrix modeling API to solve the diet problem:
 * Coefficients:
 *   c = [3.19, 2.59, 2.29, 2.89, 1.89, 1.99, 1.99, 2.49]
 *   A = [[60, 20, 10, 15],
 *        [ 8,  0, 20, 20],
 *        [ 8, 10, 15, 10],
 *        [40, 40, 35, 10],
 *        [15, 35, 15, 15],
 *        [ 0, 30, 15, 15],
 *        [25, 50, 25, 15],
 *        [60, 20, 15, 10]]
 *
 * Minimize:
 *   c^T @ x
 *
 * Subject to:
 *   700 <= A^T @ x <= 10000
 *
 * Bounds:
 *   x >= 0
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
    int coeffs[8][4] = {{60, 20, 10, 15}, {8, 0, 20, 20}, {8, 10, 15, 10}, {40, 40, 35, 10}, {15, 35, 15, 15},
      {0, 30, 15, 15}, {25, 50, 25, 15}, {60, 20, 15, 10}};

    NdArray<int, 2> A(Shape<2>(8, 4), reinterpret_cast<int*>(coeffs), 8 * 4);

    // Create COPT environment and model
    Envr env;
    Model model = env.CreateModel("diet matrix");

    // Add 1-dimensional variables mx (MVar)
    MVar<1> mx = model.AddMVar(Shape<1>(8), COPT_CONTINUOUS, "food");

    // Build lower and upper bound nutrition requirement vector
    NdArray<double, 1> nuri_upper(Shape<1>(4), 10000);
    NdArray<double, 1> nuri_lower(Shape<1>(4), 700);

    // Add constraints for nutrition lower and upper bound : 700 <= A^T @ x <= 10000
    model.AddMConstr(Mat::matmult(mx, A) <= nuri_upper);
    model.AddMConstr(Mat::matmult(A.Transpose(), mx) >= nuri_lower);

    double ctData[8] = {3.19, 2.59, 2.29, 2.89, 1.89, 1.99, 1.99, 2.49};
    NdArray<double, 1> cost(Shape<1>(8), ctData, 8);

    // Set matrix objective function: minimize c^T @ x
    model.SetObjective(Mat::matmult(cost, mx).Item(), COPT_MINIMIZE);

    // Optimize model
    model.Solve();

    // Display solutions
    if (model.GetIntAttr(COPT_INTATTR_HASLPSOL))
    {
      cout << "\nVariable solution:" << endl;
      for (int i = 0; i < 8; i++)
      {
        Var item = mx[i].Item(0);
        cout << item.GetName() << " : " << item.Get("Value") << endl;
      }
      cout << "Variable basis:" << endl;
      for (int i = 0; i < 8; i++)
      {
        Var item = mx[i].Item(0);
        cout << item.GetName() << " : " << item.GetBasis() << endl;
      }

      cout << "\nBest objective value:: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;
      cout << "Solution in format of ndarray: " << mx.Get("Value");
      cout << "Basis in format of ndarray: " << mx.GetBasis() << endl;
    }
    else
    {
      cout << "Error: no solution found" << endl;
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
