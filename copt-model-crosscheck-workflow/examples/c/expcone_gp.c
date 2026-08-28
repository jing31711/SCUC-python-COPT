/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

#include "copt.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char* argv[])
{
  int errcode = 0;

  copt_env* env = NULL;
  copt_prob* prob = NULL;

  // Create COPT environment
  errcode = COPT_CreateEnv(&env);
  if (errcode)
    goto COPT_EXIT;

  // Create COPT problem
  errcode = COPT_CreateProb(env, &prob);
  if (errcode)
    goto COPT_EXIT;

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
  {
    int ncol = 9;
    double colcost[] = {1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double collb[] = {-COPT_INFINITY, -COPT_INFINITY, -COPT_INFINITY, -COPT_INFINITY, 1.0, -COPT_INFINITY,
      -COPT_INFINITY, 1.0, -COPT_INFINITY};
    double colub[] = {+COPT_INFINITY, +COPT_INFINITY, +COPT_INFINITY, +COPT_INFINITY, 1.0, +COPT_INFINITY,
      +COPT_INFINITY, 1.0, +COPT_INFINITY};

    errcode = COPT_AddCols(prob, ncol, colcost, NULL, NULL, NULL, NULL, NULL, collb, colub, NULL);
    if (errcode)
      goto COPT_EXIT;
  }

  /*
   * Add constraints:
   *   linear:  u3 == x + y + log(2.0 / Aw)
   *   expcone: (u1, u2, u3)
   */
  {
    int nrowcnt = 3;
    int rowidx[] = {0, 1, 5};
    double rowelem[] = {-1.0, -1.0, 1.0};
    double rowbnd = log(2.0 / Aw);

    errcode = COPT_AddRow(prob, nrowcnt, rowidx, rowelem, 0, rowbnd, rowbnd, NULL);
    if (errcode)
      goto COPT_EXIT;

    int nexpcone = 1;
    int expconetype = COPT_EXPCONE_PRIMAL;
    int expconeidx[] = {3, 4, 5};
    errcode = COPT_AddExpCones(prob, nexpcone, &expconetype, expconeidx);
    if (errcode)
      goto COPT_EXIT;
  }

  /*
   * Add constraints:
   *   linear:  v3 == x + z + log(2.0 / Aw)
   *   expcone: (v1, v2, v3)
   */
  {
    int nrowcnt = 3;
    int rowidx[] = {0, 2, 8};
    double rowelem[] = {-1.0, -1.0, 1.0};
    double rowbnd = log(2.0 / Aw);

    errcode = COPT_AddRow(prob, nrowcnt, rowidx, rowelem, 0, rowbnd, rowbnd, NULL);
    if (errcode)
      goto COPT_EXIT;

    int nexpcone = 1;
    int expconetype = COPT_EXPCONE_PRIMAL;
    int expconeidx[] = {6, 7, 8};
    errcode = COPT_AddExpCones(prob, nexpcone, &expconetype, expconeidx);
    if (errcode)
      goto COPT_EXIT;
  }

  /*
   * Add constraint:
   *   u1 + v1 == 1.0
   */
  {
    int nrowcnt = 2;
    int rowidx[] = {3, 6};
    double rowelem[] = {1.0, 1.0};
    double rowbnd = 1.0;

    errcode = COPT_AddRow(prob, nrowcnt, rowidx, rowelem, 0, rowbnd, rowbnd, NULL);
    if (errcode)
      goto COPT_EXIT;
  }

  /*
   * Add constraints:
   *         -inf <= y + z <= log(Af)
   *   log(alpha) <= x - y <= log(beta)
   *   log(gamma) <= z - y <= log(delta)
   */
  {
    int nrow = 3;
    int rowbeg[] = {0, 2, 4};
    int rowcnt[] = {2, 2, 2};
    int rowind[] = {1, 2, 0, 1, 1, 2};
    double rowelem[] = {1.0, 1.0, 1.0, -1.0, -1.0, 1.0};
    double rowlb[] = {-COPT_INFINITY, log(alpha), log(gamma)};
    double rowub[] = {log(Af), log(beta), log(delta)};

    errcode = COPT_AddRows(prob, nrow, rowbeg, rowcnt, rowind, rowelem, NULL, rowlb, rowub, NULL);
    if (errcode)
      goto COPT_EXIT;
  }

  // Set objective sense
  errcode = COPT_SetObjSense(prob, COPT_MAXIMIZE);
  if (errcode)
    goto COPT_EXIT;

  // Solve problem
  errcode = COPT_SolveLp(prob);
  if (errcode)
    goto COPT_EXIT;

  // Analyze solution
  int lpstat = COPT_LPSTATUS_UNSTARTED;
  double lpobjval;
  double* lpsol = NULL;

  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_LPSTATUS, &lpstat);
  if (errcode)
    goto COPT_EXIT;

  if (lpstat == COPT_LPSTATUS_OPTIMAL)
  {
    lpsol = (double*)malloc(3 * sizeof(double));

    errcode = COPT_GetColInfo(prob, COPT_DBLINFO_VALUE, 3, NULL, lpsol);
    if (errcode)
      goto COPT_EXIT;

    errcode = COPT_GetDblAttr(prob, COPT_DBLATTR_LPOBJVAL, &lpobjval);
    if (errcode)
      goto COPT_EXIT;

    printf("\nObjective value: %.12f\n", lpobjval);

    printf("Variable solution: \n");
    for (int i = 0; i < 3; ++i)
      printf("  x[%d] = %.12f\n", i, lpsol[i]);

    free(lpsol);
  }

  // Error handling
COPT_EXIT:
  if (errcode)
  {
    char errmsg[COPT_BUFFSIZE];

    COPT_GetRetcodeMsg(errcode, errmsg, COPT_BUFFSIZE);
    printf("ERROR %d: %s\n", errcode, errmsg);

    return 0;
  }

  // Delete problem and environment
  COPT_DeleteProb(&prob);

  COPT_DeleteEnv(&env);

  return 0;
}
