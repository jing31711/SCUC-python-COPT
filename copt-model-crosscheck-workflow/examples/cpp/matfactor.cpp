/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

using namespace std;

/*
 * Computes the optimal portfolio for a given risk
 *
 * n:          Number of assets
 * mu:         Expected returns
 * G_factor_T: The factor of factorized risk
 * theta:      The specific risk
 * x0:         Initial holdings
 * w:          Initial cash holding
 * gamma:      Accpeted maximum risk
 */
double FactorMarkowitz(int n,
  const NdArray<double, 1>& mu,
  const NdArray<double, 2>& G_factor_T,
  const NdArray<double, 1>& theta,
  const NdArray<double, 1>& x0,
  double w,
  double gamma)
{
  Envr env;
  Model model = env.CreateModel("FactorMarkowitz");

  // Defines the variables (holdings): Shortselling is not allowed
  MVar<1> mx = model.AddMVar(Shape<1>(n), COPT_CONTINUOUS, "mx");

  // Maximize expected return
  model.SetObjective((mu * mx).Sum(), COPT_MAXIMIZE);

  // The amount invested must be identical to initial wealth
  model.AddMConstr(mx.Sum() == w + x0.Sum());

  // Imposes a bound on the risk
  size_t size = theta.GetSize();
  NdArray<double, 1> theta_sqrt(size, 0.0);
  for (size_t i = 0; i < size; ++i)
  {
    theta_sqrt.SetItem(i, std::sqrt(theta.Item(i)));
  }
  AffineCone affqcone = model.AddAffineCone(
    Mat::vstack(Mat::vstack(gamma, Mat::matmult(G_factor_T, mx)), theta_sqrt * mx), COPT_CONE_QUAD, "affqcone");

  // Solves the model
  model.SetIntParam(COPT_INTPARAM_LOGGING, 0);
  model.Solve();

  // Print the optimal value
  std::cout << "Objective value:    " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << std::endl;

  return Mat::dot(mu, mx.Get("Value"));
}

int main(int argc, char* argv[])
{
  int n = 8;
  double w = 1.0;
  NdArray<double, 1> mu = {0.07197, 0.15518, 0.17535, 0.08981, 0.42896, 0.39292, 0.32171, 0.18379};
  NdArray<double, 1> x0 = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  // Factor exposure matrix
  NdArray<double, 2> beta = {{0.4256, 0.1869}, {0.2413, 0.3877}, {0.2235, 0.3697}, {0.1503, 0.4612}, {1.5325, -0.2633},
    {1.2741, -0.2613}, {0.6939, 0.2372}, {0.5425, 0.2116}};

  // Factor covariance matrix
  NdArray<double, 2> S_F = {{0.0620, 0.0577}, {0.0577, 0.0908}};

  // Specific risk components
  NdArray<double, 1> theta = {0.0720, 0.0508, 0.0377, 0.0394, 0.0663, 0.0224, 0.0417, 0.0459};
  NdArray<double, 2> P = {{0.2489, 0.0}, {0.2317, 0.1926}};
  NdArray<double, 2> G_factor_T = Mat::matmult(beta, P).Transpose();
  NdArray<double, 1> gammas = {0.24, 0.28, 0.32, 0.36, 0.4, 0.44, 0.48};

  std::cout << std::endl
            << "--------------------------------------------------------" << std::endl
            << "Markowitz portfolio optimization based on a factor model" << std::endl
            << "--------------------------------------------------------" << std::endl;

  for (size_t i = 0; i < gammas.GetSize(); i++)
  {
    double gamma = gammas[i].Item();
    double expect = FactorMarkowitz(n, mu, G_factor_T, theta, x0, w, gamma);
    std::cout << "Expected return:    " << expect << std::endl;
    std::cout << "Standard deviation: " << gamma << std::endl;
  }
}
