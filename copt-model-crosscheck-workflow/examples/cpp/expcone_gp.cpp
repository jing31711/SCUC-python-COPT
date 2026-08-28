/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */
#include "coptcpp_pch.h"

using namespace std;

/*
 * Solve the following Geometric Programming problem with exponential cone:
 *
 * maximize: x + y + z
 *
 * s.t.  log[exp(x + y + log(2 / Aw)) + exp(x + z + log(2 / Aw))] <= 0
 *             -inf <= y + z <= log(Af)
 *       log(alpha) <= x - y <= log(beta)
 *       log(gamma) <= z - y <= log(delta)
 *
 * where:
 *             -inf <= x <= +inf
 *             -inf <= y <= +inf
 *             -inf <= z <= +inf
 *
 * reformulated as exponential cone problem:
 *
 * maximize: x + y + z
 *
 * s.t.  u1 >= exp(u3)
 *       u3 == x + y + log(2 / Aw)
 *       v1 >= exp(v3)
 *       v3 = x + z + log(2 / Aw)
 *       u1 + v1 == 1.0
 *             -inf <= y + z <= log(Af)
 *       log(alpha) <= x - y <= log(beta)
 *       log(gamma) <= z - y <= log(delta)
 *
 * where:
 *             -inf <= x <= +inf
 *             -inf <= y <= +inf
 *             -inf <= z <= +inf
 *  (u1, u2, u3) \in Kexp, (v1, v2, v3) \in Kexp, u2 == 1.0, v2 == 1.0
 */

#include <cmath>

int main(int argc, char* argv[])
{
  try
  {
    Envr env;
    Model model = env.CreateModel("expcone_gp");

    double Aw = 200.0;
    double Af = 50.0;
    double alpha = 2.0;
    double beta = 10.0;
    double gamma = 2.0;
    double delta = 10.0;

    /*
     * Add variables
     *   obj: 1.0 x + 1.0 y + 1.0 z
     *
     *   bnd:
     *     -inf <= x  <= +inf
     *     -inf <= y  <= +inf
     *     -inf <= z  <= +inf
     *     -inf <= u1 <= +inf
     *             u2 == 1
     *     -inf <= u3 <= +inf
     *     -inf <= v1 <= +inf
     *             v2 == 1
     *     -inf <= v3 <= +inf
     */
    Var x = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 1.0, COPT_CONTINUOUS, "x");
    Var y = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 1.0, COPT_CONTINUOUS, "y");
    Var z = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 1.0, COPT_CONTINUOUS, "z");

    Var u1 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "u1");
    Var u2 = model.AddVar(1.0, 1.0, 0.0, COPT_CONTINUOUS, "u2");
    Var u3 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "u3");

    Var v1 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "v1");
    Var v2 = model.AddVar(1.0, 1.0, 0.0, COPT_CONTINUOUS, "v2");
    Var v3 = model.AddVar(-COPT_INFINITY, +COPT_INFINITY, 0.0, COPT_CONTINUOUS, "v3");

    // Add constraint: u1 >= exp(x + y + log(2/Aw)
    model.AddConstr(u3 == x + y + log(2.0 / Aw));

    VarArray uconevars;
    uconevars.PushBack(u1);
    uconevars.PushBack(u2);
    uconevars.PushBack(u3);
    model.AddExpCone(uconevars, COPT_EXPCONE_PRIMAL);

    // Add constraint: v1 >= exp(x + z + log(2/Aw)
    model.AddConstr(v3 == x + z + log(2.0 / Aw));

    VarArray vconevars;
    vconevars.PushBack(v1);
    vconevars.PushBack(v2);
    vconevars.PushBack(v3);
    model.AddExpCone(vconevars, COPT_EXPCONE_PRIMAL);

    // Add constraint: u1 + v1 == 1.0
    model.AddConstr(u1 + v1 == 1.0);

    /*
     * Add constraints:
     *         -inf <= y + z <= log(Af)
     *   log(alpha) <= x - y <= log(beta)
     *   log(gamma) <= z - y <= log(delta)
     */
    model.AddConstr(y + z <= log(Af));
    model.AddConstr(x - y, log(alpha), log(beta));
    model.AddConstr(z - y, log(gamma), log(delta));

    // Set objective sense
    model.SetObjSense(COPT_MAXIMIZE);

    // Solve the model
    model.Solve();

    // Analyze solution
    if (model.GetIntAttr(COPT_INTATTR_LPSTATUS) == COPT_LPSTATUS_OPTIMAL)
    {
      cout << "\nOptimal objective value: " << model.GetDblAttr(COPT_DBLATTR_LPOBJVAL) << endl;

      cout << "  " << x.GetName() << " = " << x.Get(COPT_DBLINFO_VALUE) << endl;
      cout << "  " << y.GetName() << " = " << y.Get(COPT_DBLINFO_VALUE) << endl;
      cout << "  " << z.GetName() << " = " << z.Get(COPT_DBLINFO_VALUE) << endl;
      cout << endl;
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
