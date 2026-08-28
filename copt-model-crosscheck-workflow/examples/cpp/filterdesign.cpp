/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * The following example is based on an example from github:
 *     https://github.com/MOSEK/Tutorials/tree/master/filterdesign
 * and rewrote using COPT C++ interface.
 */

#include <cmath>
#include "coptcpp_pch.h"

using namespace std;

const double PI = 3.1415926535897932;

PsdExpr T_dot_X(Model& M, int n, int i, PsdVar& X, double a)
{
  if (i >= n || i <= -n)
  {
    return PsdExpr(0.0);
  }
  else if (i == 0)
  {
    return M.AddDiagMat(n, a, i) * X;
  }
  else
  {
    return M.AddDiagMat(n, a * 0.5, i) * X;
  }
}

void trigpoly_0_pi(Model& M, VarArray& x)
{
  /*
   * Add constraint : x[i] == < T(n + 1, i), X>
   */
  int n = x.Size() - 1;
  PsdVar X = M.AddPsdVar(n + 1, "X");

  for (int i = 0; i < n + 1; i++)
  {
    M.AddPsdConstr(T_dot_X(M, n + 1, i, X, 1.0) == x[i]);
  }
}

void trigpoly_0_a(Model& M, VarArray& x, double a)
{
  /*
   * Add constraint : x[i] == <T(n + 1, i), X1> + <T(n, i + 1), X2> + <T(n, i - 1), X2> - 2 * cos(a) * <T(n, i), X2>
   */
  int n = x.Size() - 1;
  PsdVar X1 = M.AddPsdVar(n + 1);
  PsdVar X2 = M.AddPsdVar(n);

  for (int i = 0; i < n + 1; i++)
  {
    M.AddPsdConstr(T_dot_X(M, n + 1, i, X1, 1.0) +       /* <T(n + 1, i), X1> */
                     T_dot_X(M, n, i + 1, X2, 1.0) +     /* <T(n, i + 1), X2> */
                     T_dot_X(M, n, i - 1, X2, 1.0) +     /* <T(n, i - 1), X2> */
                     T_dot_X(M, n, i, X2, -2.0 * cos(a)) /* 2 * cos(a) * < T(n, i), X2> */
                   == x[i]);
  }
}

void trigpoly_a_pi(Model& M, VarArray& x, double a)
{
  /*
   * Add constraint : x[i] == <T(n + 1, i), X1> - <T(n, i + 1), X2> - <T(n, i - 1), X2> + 2 * cos(a) * <T(n, i), X2>
   */
  int n = x.Size() - 1;
  PsdVar X1 = M.AddPsdVar(n + 1);
  PsdVar X2 = M.AddPsdVar(n);

  for (int i = 0; i < n + 1; i++)
  {
    M.AddPsdConstr(T_dot_X(M, n + 1, i, X1, 1.0) +      /* <T(n + 1, i), X1> */
                     T_dot_X(M, n, i + 1, X2, -1.0) +   /* <T(n, i + 1), X2> */
                     T_dot_X(M, n, i - 1, X2, -1.0) +   /* <T(n, i - 1), X2> */
                     T_dot_X(M, n, i, X2, 2.0 * cos(a)) /* 2 * cos(a) * <T(n, i), X2> */
                   == x[i]);
  }
}

template <class T> void epigraph(Model& M, VarArray& x, T t, double a, double b)
{
  /*
   * Models 0 <= H(w) <= t, for all w in[a, b]
   *  where, H(w) = x0 + 2 * x1 * cos(w) + 2 * x2 * cos(2 * w) + ... + 2 * xn * cos(n * w)
   */
  int n = x.Size() - 1;
  VarArray u = M.AddVars(n + 1, -COPT_INFINITY, COPT_INFINITY, 0, COPT_CONTINUOUS);

  M.AddConstr(t == x[0] + u[0]);
  for (int i = 0; i < n; i++)
  {
    M.AddConstr(x[i + 1] + u[i + 1] == 0);
  }

  if (a == 0.0 && b == PI)
  {
    trigpoly_0_pi(M, u);
  }
  else if (a == 0.0 && b < PI)
  {
    trigpoly_0_a(M, u, b);
  }
  else if (a < PI && b == PI)
  {
    trigpoly_a_pi(M, u, a);
  }
}

void hypograph(Model& M, VarArray& x, double t, double a, double b)
{
  /*
   * Models 0 <= t <= H(w), for all w in [a, b]
   * where, H(w) = x0 + 2*x1*cos(w) + 2*x2*cos(2*w) + ... + 2*xn*cos(n*w)
   */
  int n = x.Size() - 1;
  Var u0 = M.AddVar(-COPT_INFINITY, COPT_INFINITY, 0, COPT_CONTINUOUS);

  VarArray u;
  u.PushBack(u0);
  for (int i = 0; i < n; i++)
  {
    u.PushBack(x[i + 1]);
  }

  M.AddConstr(t == x[0] - u0);

  if (a == 0.0 && b == PI)
  {
    trigpoly_0_pi(M, u);
  }
  else if (a == 0.0 && b < PI)
  {
    trigpoly_0_a(M, u, b);
  }
  else if (a < PI && b == PI)
  {
    trigpoly_a_pi(M, u, a);
  }
}

int main(int argc, char* argv[])
{
  try
  {
    int n = 10;

    // Create COPT environment and Problem
    Envr env;
    Model M = env.CreateModel("trigpoly");

    // Add variables
    VarArray x = M.AddVars(n + 1, -COPT_INFINITY, COPT_INFINITY, 0, COPT_CONTINUOUS, "x");

    // Add constraints : H(w) >= 0
    trigpoly_0_pi(M, x);

    double wp = PI / 4.0;
    double delta = 0.05;

    // Add constraints : H(w) <= (1 + delta), w in[0, wp]
    epigraph<double>(M, x, 1.0 + delta, 0.0, wp);

    // Add constraints : (1 - delta) <= H(w), w in[0, wp]
    hypograph(M, x, 1.0 - delta, 0.0, wp);

    double ws = wp + PI / 8.0;

    // Add constraints : H(w) < t, w in[ws, pi]
    Var t = M.AddVar(0.0, COPT_INFINITY, 0, COPT_CONTINUOUS, "t");
    epigraph<Var>(M, x, t, ws, PI);

    // Set objective
    M.SetObjective(t, COPT_MINIMIZE);

    // Solve the problem
    M.Solve();
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
