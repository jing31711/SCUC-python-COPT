/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

#include "copt.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * Production Planning Problem:
 * A company manufactures four variants of the same product and in the final part of
 * the manufacturing process there are assembly, polishing and packing operations.
 * For each variant the time required for these operations is shown below (in minutes)
 * as is the profit per unit sold.
 *
 *  Variant   Assembly      Polish    Pack        Profit ($)
 *      1       2             3         2           1.50
 *      2       4             2         3           2.50
 *      3       3             3         2           3.00
 *      4       7             4         5           4.50
 *
 * Let Xi be the number of units of variant i (i=1, 2, 3, 4) made per year.

 * Objectives:
 *  Maximize total profit
 *  1.5*X1 + 2.5*X2 + 3.0*X3 + 4.5*X4

 * Subject to:
 *  (Assembly time) 2*X1 + 4*X2 + 3*X3 + 7*X4 <= 100000
 *  (Polish time)   3*X1 + 2*X2 + 3*X3 + 4*X4 <= 50000
 *  (Pack time)     2*X1 + 3*X2 + 2*X3 + 5*X4 <= 60000
 */

int main(int argc, char* argv[])
{
  int errcode = COPT_RETCODE_OK;
  double* dObjUpp = NULL;
  double* dObjLow = NULL;
  double* dLbUpp = NULL;
  double* dLbLow = NULL;
  double* dUbUpp = NULL;
  double* dUbLow = NULL;

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


  // Add 4 variables, representing the number of units of variant i of the same product
  double colcost[] = {1.5, 2.5, 3.0, 4.5};
  errcode = COPT_AddCols(prob, 4, colcost, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  if (errcode)
    goto COPT_EXIT;

  /*
   * Add time constraints
   *
   *   2*X1 + 4*X2 + 3*X3 + 7*X4 <= 100000
   *   3*X1 + 2*X2 + 3*X3 + 4*X4 <= 50000
   *   2*X1 + 3*X2 + 2*X3 + 5*X4 <= 60000
   */
  int nrow = 3;
  int rowbeg[] = {0, 4, 8};
  int rowcnt[] = {4, 4, 4};
  int rowind[] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
  double rowelem[] = {2, 4, 3, 7, 3, 2, 3, 4, 2, 3, 2, 5};
  char rowsen[] = {COPT_LESS_EQUAL, COPT_LESS_EQUAL, COPT_LESS_EQUAL};
  double rowrhs[] = {100000, 50000, 60000};

  errcode = COPT_AddRows(prob, nrow, rowbeg, rowcnt, rowind, rowelem, rowsen, rowrhs, NULL, NULL);
  if (errcode)
    goto COPT_EXIT;

  // Set to require sensitivity analysis
  errcode = COPT_SetIntParam(prob, COPT_INTPARAM_REQSENSITIVITY, 1);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_SetObjSense(prob, COPT_MAXIMIZE);
  if (errcode)
    goto COPT_EXIT;

  // Solve the production planning problem
  errcode = COPT_Solve(prob);
  if (errcode)
    goto COPT_EXIT;

  // Check if the solution is optimal
  int status = COPT_LPSTATUS_UNSTARTED;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_LPSTATUS, &status);
  if (errcode)
    goto COPT_EXIT;

  if (status != COPT_LPSTATUS_OPTIMAL)
  {
    printf("Optimal solution is not available, status=%d\n", status);
    goto COPT_EXIT;
  }

  int hasSensitivity = 0;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_HASSENSITIVITY, &hasSensitivity);
  if (errcode)
    goto COPT_EXIT;

  if (hasSensitivity != 1)
  {
    printf("Sensitivity information is not available");
    goto COPT_EXIT;
  }

  int nCol = 0;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_COLS, &nCol);
  if (errcode)
    goto COPT_EXIT;

  int nRow = 0;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_ROWS, &nRow);
  if (errcode)
    goto COPT_EXIT;

  dObjUpp = (double*)malloc(nCol * sizeof(double));
  dObjLow = (double*)malloc(nCol * sizeof(double));
  dLbUpp = (double*)malloc((nCol + nRow) * sizeof(double));
  dLbLow = (double*)malloc((nCol + nRow) * sizeof(double));
  dUbUpp = (double*)malloc((nCol + nRow) * sizeof(double));
  dUbLow = (double*)malloc((nCol + nRow) * sizeof(double));

  if (dObjUpp == NULL || dObjLow == NULL || dLbUpp == NULL || dLbLow == NULL || dUbUpp == NULL || dUbLow == NULL)
  {
    errcode = COPT_RETCODE_MEMORY;
    goto COPT_EXIT;
  }

  printf("\nSensitivity information of production planning problem:\n");

  errcode = COPT_GetColInfo(prob, COPT_DBLINFO_SAOBJUP, nCol, NULL, dObjUpp);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_GetColInfo(prob, COPT_DBLINFO_SAOBJLOW, nCol, NULL, dObjLow);
  if (errcode)
    goto COPT_EXIT;

  printf("Variant\tObjective Range\n");
  for (int i = 0; i < nCol; i++)
  {
    printf(" %d\t(%e, %e)\n", i, dObjLow[i], dObjUpp[i]);
  }

  errcode = COPT_GetColInfo(prob, COPT_DBLINFO_SALBUP, nCol, NULL, dLbUpp);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_GetColInfo(prob, COPT_DBLINFO_SALBLOW, nCol, NULL, dLbLow);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_GetColInfo(prob, COPT_DBLINFO_SAUBUP, nCol, NULL, dUbUpp);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_GetColInfo(prob, COPT_DBLINFO_SAUBLOW, nCol, NULL, dUbLow);
  if (errcode)
    goto COPT_EXIT;

  printf("Variant\tLowBound Range           \tUppBound Range\n");
  for (int i = 0; i < nCol; i++)
  {
    printf(" %d\t(%e, %e)\t(%e, %e)\n", i, dLbLow[i], dLbUpp[i], dUbLow[i], dUbUpp[i]);
  }

  errcode = COPT_GetRowInfo(prob, COPT_DBLINFO_SALBUP, nRow, NULL, dLbUpp + nCol);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_GetRowInfo(prob, COPT_DBLINFO_SALBLOW, nRow, NULL, dLbLow + nCol);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_GetRowInfo(prob, COPT_DBLINFO_SAUBUP, nRow, NULL, dUbUpp + nCol);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_GetRowInfo(prob, COPT_DBLINFO_SAUBLOW, nRow, NULL, dUbLow + nCol);
  if (errcode)
    goto COPT_EXIT;

  printf("Constraint\tLHS Range           \tRHS Range\n");
  for (int i = nCol; i < nCol + nRow; i++)
  {
    printf(" %d\t(%e, %e)\t(%e, %e)\n", i - nCol, dLbLow[i], dLbUpp[i], dUbLow[i], dUbUpp[i]);
  }

  // Error handling
COPT_EXIT:
  if (dObjUpp)
  {
    free(dObjUpp);
  }
  if (dObjLow)
  {
    free(dObjLow);
  }
  if (dLbLow)
  {
    free(dLbLow);
  }
  if (dLbUpp)
  {
    free(dLbUpp);
  }
  if (dUbLow)
  {
    free(dUbLow);
  }
  if (dUbUpp)
  {
    free(dUbUpp);
  }

  if (errcode)
  {
    char errmsg[COPT_BUFFSIZE];
    COPT_GetRetcodeMsg(errcode, errmsg, COPT_BUFFSIZE);
    printf("ERROR %d: %s\n", errcode, errmsg);
  }

  // Delete problem and environment
  COPT_DeleteProb(&prob);
  COPT_DeleteEnv(&env);

  return 0;
}
