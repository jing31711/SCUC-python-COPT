/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

#include "copt.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/*
 * Multi-Objective Knapsack Problem:
 *   Consider a set of 20 items divided into 5 groups, where each group contains
 *   mutually exclusive items (only one item per group can be selected).
 *
 *   Each item i in group j has a volume V_ij, a value P_ij, and a weight W_ij.
 *
 *   Volume = [10, 13, 24, 32, 4]
 *            [12, 15, 22, 26, 5.2]
 *            [14, 18, 25, 28, 6.8]
 *            [14, 14, 28, 32, 6.8]
 *
 *   Value  = [3, 4, 9, 15, 2]
 *            [4, 6, 8, 10, 2.5]
 *            [5, 7, 10, 12, 3]
 *            [3, 5, 10, 10, 2]]
 *
 *   Weight = [0.2, 0.3, 0.4, 0.6, 0.1]
 *            [0.25, 0.35, 0.38, 0.45, 0.15]
 *            [0.3, 0.37, 0.5, 0.5, 0.2]
 *            [0.3, 0.32, 0.45, 0.6, 0.2]
 *
 *   Objectives:
 *     Maximize total value while minimizing total weight of selected items
 *
 *   Subject to:
 *     - Exactly one item must be selected from each group
 *     - Total volume of selected items <= knapsack capacity
 */

int main(int argc, char* argv[])
{
  int errcode = 0;
  double* sol = NULL;

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
 * A set of 20 items is divided into groups (5 groups and 4 items per group),
 * where each group is represented by a column in the matrix.
 */
#define CNT 20
  int nGroups = 5;
  int nItems = 4;
  int cnt = CNT;

  int idxAll[CNT];
  for (int i = 0; i < cnt; i++)
  {
    idxAll[i] = i;
  }

  char vtype[CNT];
  memset(vtype, COPT_BINARY, cnt * sizeof(char));

  // Add binary variables of size 20, representing which item is selected
  errcode = COPT_AddCols(prob, cnt, NULL, NULL, NULL, NULL, NULL, vtype, NULL, NULL, NULL);
  if (errcode)
    goto COPT_EXIT;

  /*
   * Add constraint of mutual exclusivity, that is,
   * exactly one item must be selected from each group.
   */
  int nrow = nGroups;
  int rowbeg[] = {0, 4, 8, 12, 16};
  int rowcnt[] = {4, 4, 4, 4, 4};
  int rowind[] = {0, 5, 10, 15, 1, 6, 11, 16, 2, 7, 12, 17, 3, 8, 13, 18, 4, 9, 14, 19};
  double rowelem[CNT];
  for (int i = 0; i < cnt; i++)
  {
    rowelem[i] = 1.0;
  }
  char rowsen[] = {COPT_EQUAL, COPT_EQUAL, COPT_EQUAL, COPT_EQUAL, COPT_EQUAL};
  double rowrhs[] = {1.0, 1.0, 1.0, 1.0, 1.0};

  errcode = COPT_AddRows(prob, nrow, rowbeg, rowcnt, rowind, rowelem, rowsen, rowrhs, NULL, NULL);
  if (errcode)
    goto COPT_EXIT;

  // Add constraint of total volume not exceeding knapsack capacity
  double volume[] = {10, 13, 24, 32, 4, 12, 15, 22, 26, 5.2, 14, 18, 25, 28, 6.8, 14, 14, 28, 32, 6.8};
  errcode = COPT_AddRow(prob, cnt, idxAll, volume, COPT_LESS_EQUAL, 90.0, 0.0, NULL);
  if (errcode)
    goto COPT_EXIT;

  // Add first objective: maximize total value of selected items
  double value[] = {3, 4, 9, 15, 2, 4, 6, 8, 10, 2.5, 5, 7, 10, 12, 3, 3, 5, 10, 10, 2};
  errcode = COPT_MultiObjSetColObj(prob, 0, cnt, idxAll, value);
  if (errcode)
    goto COPT_EXIT;
  errcode = COPT_MultiObjSetObjSense(prob, 0, COPT_MAXIMIZE);
  if (errcode)
    goto COPT_EXIT;
  errcode = COPT_MultiObjSetObjParam(prob, 0, COPT_MULTIOBJ_PRIORITY, 2);
  if (errcode)
    goto COPT_EXIT;

  // Add second objective: minimize total weight of selected items
  double weight[] = {0.2, 0.3, 0.4, 0.6, 0.1, 0.25, 0.35, 0.38, 0.45, 0.15, 0.3, 0.37, 0.5, 0.5, 0.2, 0.3, 0.32, 0.45,
    0.6, 0.2};
  errcode = COPT_MultiObjSetColObj(prob, 1, cnt, idxAll, weight);
  if (errcode)
    goto COPT_EXIT;
  errcode = COPT_MultiObjSetObjParam(prob, 0, COPT_MULTIOBJ_PRIORITY, 1);
  if (errcode)
    goto COPT_EXIT;

  // Solve the multi-objective knapsack problem
  errcode = COPT_Solve(prob);
  if (errcode)
    goto COPT_EXIT;

  // Check if the solution is optimal
  int status = COPT_MIPSTATUS_UNSTARTED;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_MIPSTATUS, &status);
  if (errcode)
    goto COPT_EXIT;

  if (status != COPT_MIPSTATUS_OPTIMAL)
  {
    printf("Optimal solution is not available, status=%d\n", status);
    goto COPT_EXIT;
  }

  // Analyze and print solution
  sol = (double*)malloc(CNT * sizeof(double));
  errcode = COPT_GetSolution(prob, sol);
  if (errcode)
    goto COPT_EXIT;

  double sumVolume = 0.0;
  double sumValue = 0.0;
  double sumWeight = 0.0;
  printf(" Group\t Volume\t Value\t Weight\n");
  for (int k = 0; k < nGroups; ++k)
  {
    for (int i = 0; i < nItems; ++i)
    {
      int idx = i * nGroups + k;
      if (sol[idx] > 0.99)
      {
        printf("  %d\t %.2f\t %.2f\t %.2f\n", k, volume[idx], value[idx], weight[idx]);
        sumVolume += volume[idx];
        sumValue += value[idx];
        sumWeight += weight[idx];
      }
    }
  }
  printf(" Total\t %.2f\t %.2f\t %.2f\n", sumVolume, sumValue, sumWeight);

  // Error handling
COPT_EXIT:
  if (sol)
  {
    free(sol);
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
