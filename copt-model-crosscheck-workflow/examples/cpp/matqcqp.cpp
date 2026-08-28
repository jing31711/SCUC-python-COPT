/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

#include <algorithm>
#include <functional>
#include <cmath>
#include <random>

using namespace std;

/* SDP relaxation for solving MIQCP problem */
void sdp_relaxation(Model& model, int n, const NdArray<double, 2>& P, const NdArray<double, 1>& q)
{
  PsdVar Z = model.AddPsdVar(n + 1, "Z");

  MPsdExpr<2> X = Mat::slice(Z, Mat::make_view(0, n)(0, n));
  MPsdExpr<1> x = Mat::slice(Z, Mat::make_view(0, n)(n)).Squeeze(1);
  model.AddMPsdConstr(X.Diagonal(0) >= x);

  NdArray<int, 2> rows = {{n, n}};
  model.AddMPsdConstr(Mat::pick(Z, rows) == 1.0);

  model.SetPsdObjective((P * X).Sum() + 2.0 * Mat::matmult(x, q), COPT_MINIMIZE);

  model.Solve();
  std::cout << "Objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << std::endl;
}

/* Least-squares models for minimizing ||Ax - b|| */
MVar<1> least_squares(Model& model, int n, const NdArray<double, 2>& A, const NdArray<double, 1>& b)
{
  MVar<1> x = model.AddMVar(Shape<1>(n), -COPT_INFINITY, COPT_INFINITY, 0, COPT_INTEGER, "x");
  Var t = model.AddVar(-COPT_INFINITY, COPT_INFINITY, 0, COPT_CONTINUOUS, "t");

  model.AddAffineCone(Mat::vstack(t, Mat::matmult(A, x) - b), COPT_CONE_QUAD);
  model.SetObjective(t, COPT_MINIMIZE);

  model.Solve();
  std::cout << "Best objective value: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << std::endl;

  return x;
}

int main(int argc, char* argv[])
{
  std::default_random_engine generator;
  generator.seed(time(0));
  std::uniform_real_distribution<double> unif_distr(0., 1.);
  std::normal_distribution<double> normal_distr(0., 1.);

  int n = 20;
  int m = 2 * n;

  vector<double> cbuf(n);
  vector<double> Abuf(n * m);
  std::generate(Abuf.begin(), Abuf.end(), std::bind(normal_distr, generator));
  std::generate(cbuf.begin(), cbuf.end(), std::bind(unif_distr, generator));

  NdArray<double, 1> c(Shape<1>(n), cbuf.data(), n);
  NdArray<double, 2> A(Shape<2>(m, n), Abuf.data(), m * n);
  NdArray<double, 2> P = Mat::matmult(A.Transpose(), A);
  NdArray<double, 1> q = 0.0 - Mat::matmult(P, c);
  NdArray<double, 1> b = Mat::matmult(A, c);

  Envr env;

  Model model = env.CreateModel("sdp_relaxation");
  sdp_relaxation(model, n, P, q);

  Model modelint = env.CreateModel("least_squares");
  MVar<1> x = least_squares(modelint, n, A, b);

  NdArray<double, 1> xOpt = x.Get(COPT_DBLINFO_VALUE);
  std::cout << "xOpt = " << xOpt.GetShape() << std::endl;
  std::cout << xOpt << std::endl;

  PsdVar Z = model.GetPsdVarByName("Z");
  NdArray<double, 2> xRound = Z.Get(COPT_DBLINFO_VALUE);
  xRound = xRound[Mat::make_view(0, n)(0, n)];
  std::cout << "xRound = " << xRound.GetShape() << std::endl;
  std::cout << xRound << std::endl;
}
