/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * The problem to solve:
 *
 *  Maximize:
 *    1.2 x + 1.8 y + 2.1 z
 *
 *  Subject to:
 *    1.5 x + 1.2 y + 1.8 z <= 2.6
 *    0.8 x + 0.6 y + 0.9 z >= 1.2
 *
 *  where:
 *    0.1 <= x <= 0.6
 *    0.2 <= y <= 1.5
 *    0.3 <= z <= 2.8
 */

#include "copt.h"

#include <stdio.h>
#include <stdlib.h>

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
  
  /*
   * Add variables
   *
   *   obj: 1.2 C0 + 1.8 C1 + 2.1 C2
   *
   *   var:
   *     0.1 <= C0 <= 0.6
   *     0.2 <= C1 <= 1.5
   *     0.3 <= C2 <= 2.8
   *
   */
  int ncol = 3;
  double colcost[] = {1.2, 1.8, 2.1};
  double collb[] = {0.1, 0.2, 0.3};
  double colub[] = {0.6, 1.5, 1.8};

  errcode = COPT_AddCols(prob, ncol, colcost, NULL, NULL, NULL, NULL, NULL, collb, colub, NULL);
  if (errcode)
    goto COPT_EXIT;

  /*
   * Add constraints
   *
   *   r0: 1.5 C0 + 1.2 C1 + 1.8 C2 <= 2.6
   *   r1: 0.8 C0 + 0.6 C1 + 0.9 C2 >= 1.2
   */
  int nrow = 2;
  int rowbeg[] = {0, 3};
  int rowcnt[] = {3, 3};
  int rowind[] = {0, 1, 2, 0, 1, 2};
  double rowelem[] = {1.5, 1.2, 1.8, 0.8, 0.6, 0.9};
  char rowsen[] = {COPT_LESS_EQUAL, COPT_GREATER_EQUAL};
  double rowrhs[] = {2.6, 1.2};

  errcode = COPT_AddRows(prob, nrow, rowbeg, rowcnt, rowind, rowelem, rowsen, rowrhs, NULL, NULL);
  if (errcode)
    goto COPT_EXIT;

  // Set parameters and attributes
  errcode = COPT_SetDblParam(prob, COPT_DBLPARAM_TIMELIMIT, 10);
  if (errcode)
    goto COPT_EXIT;
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
  int* colstat = NULL;

  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_LPSTATUS, &lpstat);
  if (errcode)
    goto COPT_EXIT;

  if (lpstat == COPT_LPSTATUS_OPTIMAL)
  {
    lpsol = (double*)malloc(ncol * sizeof(double));
    colstat = (int*)malloc(ncol * sizeof(int));

    errcode = COPT_GetLpSolution(prob, lpsol, NULL, NULL, NULL);
    if (errcode)
      goto COPT_EXIT;

    errcode = COPT_GetBasis(prob, colstat, NULL);
    if (errcode)
      goto COPT_EXIT;

    errcode = COPT_GetDblAttr(prob, COPT_DBLATTR_LPOBJVAL, &lpobjval);
    if (errcode)
      goto COPT_EXIT;

    printf("\nObjective value: %.6f\n", lpobjval);

    printf("Variable solution: \n");
    for (int i = 0; i < ncol; ++i)
      printf("  x[%d] = %.6f\n", i, lpsol[i]);

    printf("Variable basis status: \n");
    for (int i = 0; i < ncol; ++i)
      printf("  x[%d]: %d\n", i, colstat[i]);

    free(lpsol);
    free(colstat);
  }

  // Write problem, solution and modified parameters to files
  errcode = COPT_WriteMps(prob, "lp_ex1.mps");
  if (errcode)
    goto COPT_EXIT;
  errcode = COPT_WriteBasis(prob, "lp_ex1.bas");
  if (errcode)
    goto COPT_EXIT;
  errcode = COPT_WriteSol(prob, "lp_ex1.sol");
  if (errcode)
    goto COPT_EXIT;
  errcode = COPT_WriteParam(prob, "lp_ex1.par");
  if (errcode)
    goto COPT_EXIT;

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
