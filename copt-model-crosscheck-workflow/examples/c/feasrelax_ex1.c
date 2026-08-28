/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * We will compute feasibility relaxation for the following infeasible LP problem,
 * the problem is 'itest6' test case from netlib-infeas:
 *
 * Minimize
 *  X2 + X3 + X4
 * Subject To
 *  ROW1: 0.8 X3 + X4 <= 10000
 *  ROW2: X1 <= 90000
 *  ROW3: 2 X6 - X8 <= 10000
 *  ROW4: - X2 + X3 >= 50000
 *  ROW5: - X2 + X4 >= 87000
 *  ROW6: X3 <= 50000
 *  ROW7: - 3 X5 + X7 >= 10000
 *  ROW8: 0.5 X5 + 0.6 X6 <= 300000
 *  ROW9: X2 - 0.05 X3 = 5000
 *  ROW10: X2 - 0.04 X3 - 0.05 X4 = 4500
 *  ROW11: X2 >= 80000
 * END
 */

#include "copt.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  int errcode = 0;
  copt_env* env = NULL;
  copt_prob* prob = NULL;

  // Create COPT environment and problem
  errcode = COPT_CreateEnv(&env);
  if (errcode)
    goto COPT_EXIT;
  errcode = COPT_CreateProb(env, &prob);
  if (errcode)
    goto COPT_EXIT;

  // Add variables
  int nCol = 8;
  double colCost[] = {0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0};
  double colLower[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double colUpper[] = {COPT_INFINITY, COPT_INFINITY, COPT_INFINITY, COPT_INFINITY, COPT_INFINITY, COPT_INFINITY,
    COPT_INFINITY, COPT_INFINITY};
  char* colNames[] = {"X1", "X2", "X3", "X4", "X5", "X6", "X7", "X8"};
  errcode = COPT_AddCols(prob, nCol, colCost, NULL, NULL, NULL, NULL, NULL, colLower, colUpper, colNames);
  if (errcode)
    goto COPT_EXIT;

  // Add constraints
  int nRow = 11;
  int rowBeg[] = {0, 2, 3, 5, 7, 9, 10, 12, 14, 16, 19};
  int rowCnt[] = {2, 1, 2, 2, 2, 1, 2, 2, 2, 3, 1};
  int rowIdx[] = {2, 3, 0, 5, 7, 1, 2, 1, 3, 2, 4, 6, 4, 5, 1, 2, 1, 2, 3, 1};
  double rowElem[] = {0.8, 1.0, 1.0, 2.0, -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -3.0, 1.0, 0.5, 0.6, 1.0, -0.05, 1.0, -0.04,
    -0.05, 1.0};
  char rowSense[] = {COPT_LESS_EQUAL, COPT_LESS_EQUAL, COPT_LESS_EQUAL, COPT_GREATER_EQUAL, COPT_GREATER_EQUAL,
    COPT_LESS_EQUAL, COPT_GREATER_EQUAL, COPT_LESS_EQUAL, COPT_EQUAL, COPT_EQUAL, COPT_GREATER_EQUAL};
  double rowRhs[] = {10000, 90000, 10000, 50000, 87000, 50000, 10000, 300000, 5000, 4500, 80000};
  char* rowNames[] = {"ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8", "ROW9", "ROW10", "ROW11"};
  errcode = COPT_AddRows(prob, nRow, rowBeg, rowCnt, rowIdx, rowElem, rowSense, rowRhs, NULL, rowNames);
  if (errcode)
    goto COPT_EXIT;

  // Set parameters and attributes
  errcode = COPT_SetDblParam(prob, COPT_DBLPARAM_TIMELIMIT, 60);
  if (errcode)
    goto COPT_EXIT;

  errcode = COPT_SetObjSense(prob, COPT_MINIMIZE);
  if (errcode)
    goto COPT_EXIT;

  // Solve the problem
  errcode = COPT_Solve(prob);
  if (errcode)
    goto COPT_EXIT;

  // Get solution status
  int iLpStatus = COPT_LPSTATUS_UNSTARTED;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_LPSTATUS, &iLpStatus);
  if (errcode)
    goto COPT_EXIT;

  // Compute feasibility relaxation if problem is infeasible
  if (iLpStatus == COPT_LPSTATUS_INFEASIBLE)
  {
    double* colLowPen = NULL;
    double* colUppPen = NULL;
    double* rowBndPen = NULL;

    // Allocate memory for penalty of row/column bounds
    colLowPen = (double*)malloc(nCol * sizeof(double));
    colUppPen = (double*)malloc(nCol * sizeof(double));
    rowBndPen = (double*)malloc(nRow * sizeof(double));
    if (!colLowPen || !colUppPen || !rowBndPen)
    {
      errcode = COPT_RETCODE_MEMORY;
      goto COPT_EXIT;
    }

    // Assign row/column penalties
    for (int i = 0; i < nCol; ++i)
    {
      colLowPen[i] = 1.0;
      colUppPen[i] = 1.0;
    }
    for (int i = 0; i < nRow; ++i)
    {
      rowBndPen[i] = 1.0;
    }

    // Set feasibility relaxation mode: minimize sum of violations
    errcode = COPT_SetIntParam(prob, COPT_INTPARAM_FEASRELAXMODE, 0);
    if (errcode)
      goto COPT_EXIT;

    // Compute feasibility relaxation
    errcode = COPT_FeasRelax(prob, colLowPen, colUppPen, rowBndPen, NULL);
    if (errcode)
      goto COPT_EXIT;

    // Free memory for penalty data
    if (colLowPen != NULL)
    {
      free(colLowPen);
    }
    if (colUppPen != NULL)
    {
      free(colUppPen);
    }
    if (rowBndPen != NULL)
    {
      free(rowBndPen);
    }

    // Check if feasibility relaxation solution is available
    int hasRelaxSol = 0;
    errcode = COPT_GetIntAttr(prob, COPT_INTATTR_HASFEASRELAXSOL, &hasRelaxSol);
    if (errcode)
      goto COPT_EXIT;

    // Retrieve feasibility relaxation result
    if (hasRelaxSol)
    {
      // Get relaxaion violations for variables and constraints
      double* rowLowRelax = (double*)malloc(nRow * sizeof(double));
      double* rowUppRelax = (double*)malloc(nRow * sizeof(double));
      double* colLowRelax = (double*)malloc(nCol * sizeof(double));
      double* colUppRelax = (double*)malloc(nCol * sizeof(double));
      if (!rowLowRelax || !rowUppRelax || !colLowRelax || !colUppRelax)
      {
        errcode = COPT_RETCODE_MEMORY;
        goto COPT_EXIT;
      }

      errcode = COPT_GetRowInfo(prob, COPT_DBLINFO_RELAXLB, nRow, NULL, rowLowRelax);
      if (errcode)
        goto COPT_EXIT;
      errcode = COPT_GetRowInfo(prob, COPT_DBLINFO_RELAXUB, nRow, NULL, rowUppRelax);
      if (errcode)
        goto COPT_EXIT;
      errcode = COPT_GetColInfo(prob, COPT_DBLINFO_RELAXLB, nCol, NULL, colLowRelax);
      if (errcode)
        goto COPT_EXIT;
      errcode = COPT_GetColInfo(prob, COPT_DBLINFO_RELAXUB, nCol, NULL, colUppRelax);
      if (errcode)
        goto COPT_EXIT;

      // Print violations of variables and constraints
      printf("\n======================== FeasRelax result ========================\n");
      char rowName[COPT_BUFFSIZE];
      for (int iRow = 0; iRow < nRow; ++iRow)
      {
        if (rowLowRelax[iRow] != 0.0 || rowUppRelax[iRow] != 0.0)
        {
          errcode = COPT_GetRowName(prob, iRow, rowName, COPT_BUFFSIZE, NULL);
          if (errcode)
            goto COPT_EXIT;
          printf("%5s: viol = (%+.12e, %+.12e)\n", rowName, rowLowRelax[iRow], rowUppRelax[iRow]);
        }
      }

      printf("\n");
      char colName[COPT_BUFFSIZE];
      for (int iCol = 0; iCol < nCol; ++iCol)
      {
        if (colLowRelax[iCol] != 0.0 || colUppRelax[iCol] != 0.0)
        {
          errcode = COPT_GetColName(prob, iCol, colName, COPT_BUFFSIZE, NULL);
          if (errcode)
            goto COPT_EXIT;
          printf("%5s: viol = (%+.12e, %+.12e)\n", colName, colLowRelax[iCol], colUppRelax[iCol]);
        }
      }

      // Write relaxed problem to file
      errcode = COPT_WriteRelax(prob, "feasrelax_ex1.relax");
      if (errcode)
        goto COPT_EXIT;

      /* Free memory */
      if (rowLowRelax != NULL)
      {
        free(rowLowRelax);
      }
      if (rowUppRelax != NULL)
      {
        free(rowUppRelax);
      }
      if (colLowRelax != NULL)
      {
        free(colLowRelax);
      }
      if (colUppRelax != NULL)
      {
        free(colUppRelax);
      }
    }
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
