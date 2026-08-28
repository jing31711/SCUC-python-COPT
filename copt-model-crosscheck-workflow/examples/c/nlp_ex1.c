/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

#include "copt.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Solve the following nonlinear problem via nonlinear expresion input:
 *
 * min:    x1 * x4 * (sin(x1 + x2) + cos(x2 * x3) + tan(x3 / x4)) + x3
 * s.t.    x1 * x2 * x3 * x4 + x1 + x2 >= 35
 *         log(x1) + 2 * log(x2) + 3 * log(x3) + 4 * log(x4) + x3 + x4 >= 15
 *         x1^2 + x2^2 + x3^2 + x4^2 + x1 + x3 >= 50
 *
 * where:
 *         1 <= x1, x2, x3, x4 <= 5
 */

int main(int argc, char* argv[])
{
  int errcode = 0;

  copt_env* env = NULL;
  copt_prob* prob = NULL;

  // Create COPT environment
  errcode = COPT_CreateEnv(&env);
  if (errcode)
    goto exit_cleanup;

  // Create COPT problem
  errcode = COPT_CreateProb(env, &prob);
  if (errcode)
    goto exit_cleanup;

  /* Add variables (dimension and bounds) */
  int nCol = 4;
  double colLower[] = {1.0, 1.0, 1.0, 1.0};
  double colUpper[] = {5.0, 5.0, 5.0, 5.0};
  char* colNames[] = {"x1", "x2", "x3", "x4"};

  errcode = COPT_AddCols(prob, nCol, NULL, NULL, NULL, NULL, NULL, NULL, colLower, colUpper, colNames);
  if (errcode)
    goto exit_cleanup;

  /*
   * Add constraint:
   *   x1 * x2 * x3 * x4 + x1 + x2 >= 35
   *
   * Expression tree for nonlinear part:
   *     *
   *    / \
   *  x1   *
   *      / \
   *    x2   *
   *        / \
   *      x3   x4
   *
   * Prefix notation:
   *   *, x1, *, x2, *, x3, x4
   *
   */
  {
    /* Nonlinear expression by prefix notation */
    int nToken = 7;
    int nTokenElem = 0;
    int token[] = {COPT_NL_MULT, 0, COPT_NL_MULT, 1, COPT_NL_MULT, 2, 3};
    double* tokenElem = NULL;

    /* Linear part */
    int nRowMatCnt = 2;
    int rowMatIdx[] = {0, 1};
    double rowMatElem[] = {1.0, 1.0};

    /* Sense and bound */
    char cRowSense = COPT_GREATER_EQUAL;
    double dRowBound = 35.0;

    /* Name */
    char* name = "nlrow1";

    errcode = COPT_AddNLConstr(prob, nToken, nTokenElem, token, tokenElem, nRowMatCnt, rowMatIdx, rowMatElem, cRowSense,
      dRowBound, 0.0, name);
    if (errcode)
      goto exit_cleanup;
  }

  /*
   * Add constraint:
   *   log(x1) + 2 * log(x2) + 3 * log(x3) + 4 * log(x4) + x3 + x4 >= 15
   *
   * Expression tree for nonlinear part:
   *      (sum,        4)
   *      /   /   \     \
   *    log  *     *     *
   *     |  / \   / \   / \
   *    x1 2 log 3 log 4 log
   *          |     |     |
   *         x2    x3    x4
   *
   * Prefix notation:
   *   sum, 4, log, x1, *, 2.0, log, x2, *, 3.0, log, x3, *, 4.0, log, x4
   *
   */
  {
    /* Nonlinear expression by prefix notation */
    int nToken = 16;
    int nTokenElem = 3;
    int token[] = {COPT_NL_SUM, 4, COPT_NL_LOG, 0, COPT_NL_MULT, COPT_NL_GET, COPT_NL_LOG, 1, COPT_NL_MULT, COPT_NL_GET,
      COPT_NL_LOG, 2, COPT_NL_MULT, COPT_NL_GET, COPT_NL_LOG, 3};
    double tokenElem[] = {2.0, 3.0, 4.0};

    /* Linear part */
    int nRowMatCnt = 2;
    int rowMatIdx[] = {2, 3};
    double rowMatElem[] = {1.0, 1.0};

    /* Sense and bound */
    char cRowSense = COPT_GREATER_EQUAL;
    double dRowBound = 15.0;

    /* Name */
    char* name = "nlrow2";

    errcode = COPT_AddNLConstr(prob, nToken, nTokenElem, token, tokenElem, nRowMatCnt, rowMatIdx, rowMatElem, cRowSense,
      dRowBound, 0.0, name);
    if (errcode)
      goto exit_cleanup;
  }

  /*
   * Add constraint:
   *   x1^2 + x2^2 + x3^2 + x4^2 + x1 + x3 >= 50
   */
  {
    int nQMatCnt = 4;
    int qMatRow[] = {0, 1, 2, 3};
    int qMatCol[] = {0, 1, 2, 3};
    double qMatElem[] = {1.0, 1.0, 1.0, 1.0};
    int nRowMatCnt = 2;
    int rowMatIdx[] = {0, 2};
    double rowMatElem[] = {1.0, 1.0};
    char cRowSense = COPT_GREATER_EQUAL;
    double dRowBound = 50;
    char* name = "q1";
    errcode = COPT_AddQConstr(prob, nRowMatCnt, rowMatIdx, rowMatElem, nQMatCnt, qMatRow, qMatCol, qMatElem, cRowSense,
      dRowBound, name);
    if (errcode)
      goto exit_cleanup;
  }

  /*
   * Add objective
   *   x1 * x4 * (sin(x1 + x2) + cos(x2 * x3) + tan(x3 / x4)) + x3
   *
   * Expression tree for nonlinear part:
   *         *
   *        / \
   *      x1   *
   *          / \
   *        x4  (sum,    3)
   *            /    |    \
   *          sin   cos   tan
   *           |     |     |
   *           +     *     /
   *          / \   / \   / \
   *         x1 x2 x2 x3 x3 x4
   *
   * Prefix notation:
   *   *, x1, *, x4, sum, 3, sin, +, x1, x2, cos, *, x2, x3, tan, /, x3, x4
   */
  {
    /* Nonlinear expression by prefix notation */
    int nToken = 18;
    int nTokenElem = 0;
    int token[] = {COPT_NL_MULT, 0, COPT_NL_MULT, 3, COPT_NL_SUM, 3, COPT_NL_SIN, COPT_NL_PLUS, 0, 1, COPT_NL_COS,
      COPT_NL_MULT, 1, 2, COPT_NL_TAN, COPT_NL_DIV, 2, 3};
    double* tokenElem = NULL;

    errcode = COPT_SetNLObj(prob, nToken, nTokenElem, token, tokenElem);
    if (errcode)
      goto exit_cleanup;

    /* Linear part */
    int iColIdx = 2;
    double dColObj = 1.0;

    errcode = COPT_SetColObj(prob, 1, &iColIdx, &dColObj);
    if (errcode)
      goto exit_cleanup;
  }

  // Set parameters and attributes
  errcode = COPT_SetDblParam(prob, COPT_DBLPARAM_TIMELIMIT, 60);
  if (errcode)
    goto exit_cleanup;

  errcode = COPT_SetObjSense(prob, COPT_MINIMIZE);
  if (errcode)
    goto exit_cleanup;

  // Solve the problem
  errcode = COPT_Solve(prob);
  if (errcode)
    goto exit_cleanup;

  // Analyze solution
  int iLpStatus = COPT_LPSTATUS_UNSTARTED;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_LPSTATUS, &iLpStatus);
  if (errcode)
    goto exit_cleanup;

  // Retrieve optimal solution
  if (iLpStatus == COPT_LPSTATUS_OPTIMAL)
  {
    double dLpObjVal;
    double* colVal = NULL;

    colVal = (double*)malloc(nCol * sizeof(double));
    if (!colVal)
    {
      errcode = COPT_RETCODE_MEMORY;
      goto exit_cleanup;
    }

    errcode = COPT_GetDblAttr(prob, COPT_DBLATTR_LPOBJVAL, &dLpObjVal);
    if (errcode)
      goto exit_cleanup;

    errcode = COPT_GetColInfo(prob, COPT_DBLINFO_VALUE, nCol, NULL, colVal);
    if (errcode)
      goto exit_cleanup;

    printf("\nOptimal objective value: %.9e\n", dLpObjVal);

    printf("Variable solution: \n");
    char colName[COPT_BUFFSIZE];
    for (int iCol = 0; iCol < nCol; ++iCol)
    {
      errcode = COPT_GetColName(prob, iCol, colName, COPT_BUFFSIZE, NULL);
      if (errcode)
        goto exit_cleanup;
      printf("  %s = %.9e\n", colName, colVal[iCol]);
    }

    // Free memory
    if (colVal != NULL)
    {
      free(colVal);
    }
  }

  // Error handling
exit_cleanup:
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
