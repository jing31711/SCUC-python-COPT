/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example solves the mixed SDP problem:
 *
 *                  [1,  0]
 *  minimize     Tr [     ] * X + x0 + 2 * x1 + 3
 *                  [0,  1]
 *
 *                  [0,  1]
 *  subject to   Tr [     ] * X - x0 - 2 * x1 >= 0
 *                  [1,  0]
 *
 *                  [0,  1]        [5,  1]   [1,  0]
 *             x0 * [     ] + x1 * [     ] - [     ] in PSD
 *                  [1,  5]        [1,  0]   [0,  2]
 *
 *             x0, x1 free, X in PSD
 */

#include "copt.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  int retcode = 0;
  copt_env* env = NULL;
  copt_prob* prob = NULL;

  /* Creat environment and problem */
  retcode = COPT_CreateEnv(&env);
  if (retcode)
    goto exit_cleanup;
  retcode = COPT_CreateProb(env, &prob);
  if (retcode)
    goto exit_cleanup;

  /* Create symmetric matrices */
  {
    /* Matrix C0 */
    int ndim = 2;
    int nelem = 2;
    int rows[] = {0, 1};
    int cols[] = {0, 1};
    double elems[] = {1.0, 1.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  {
    /* Matrix C1 */
    int ndim = 2;
    int nelem = 1;
    int rows[] = {1};
    int cols[] = {0};
    double elems[] = {1.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  {
    /* Matrix H1 */
    int ndim = 2;
    int nelem = 2;
    int rows[] = {1, 1};
    int cols[] = {0, 1};
    double elems[] = {1.0, 5.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  {
    /* Matrix H2 */
    int ndim = 2;
    int nelem = 2;
    int rows[] = {0, 1};
    int cols[] = {0, 0};
    double elems[] = {5.0, 1.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  {
    /* Matrix D1 */
    int ndim = 2;
    int nelem = 2;
    int rows[] = {0, 1};
    int cols[] = {0, 1};
    double elems[] = {-1.0, -2.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  /* Add PSD column */
  int nPSDCol = 1;
  int colDims[] = {2};
  retcode = COPT_AddPSDCols(prob, nPSDCol, colDims, NULL);
  if (retcode)
    goto exit_cleanup;

  /* Add columns */
  int nCol = 2;
  double colLower[] = {-COPT_INFINITY, -COPT_INFINITY};
  double colUpper[] = {+COPT_INFINITY, +COPT_INFINITY};
  retcode = COPT_AddCols(prob, nCol, NULL, NULL, NULL, NULL, NULL, NULL, colLower, colUpper, NULL);
  if (retcode)
    goto exit_cleanup;

  /* Add PSD constraint */
  {
    int nRowMatCnt = 2;
    int rowMatIdx[] = {0, 1};
    double rowMatElem[] = {-1.0, -2.0};
    int nColCnt = 1;
    int psdColIdx[] = {0};
    int symMatIdx[] = {1};
    char cRowSense = COPT_GREATER_EQUAL;
    double dRowBound = 0.0;
    retcode = COPT_AddPSDConstr(prob, nRowMatCnt, rowMatIdx, rowMatElem, nColCnt, psdColIdx, symMatIdx, cRowSense,
      dRowBound, 0.0, NULL);
    if (retcode)
      goto exit_cleanup;
  }

  /* Add LMI constraint */
  {
    int nLMIDim = 2;
    int nLMIMatCnt = 2;
    int colIdx[] = {0, 1};
    int symMatIdx[] = {2, 3};
    int constMatIdx = 4;
    retcode = COPT_AddLMIConstr(prob, nLMIDim, nLMIMatCnt, colIdx, symMatIdx, constMatIdx, NULL);
    if (retcode)
      goto exit_cleanup;
  }

  /* Set PSD costs */
  int numPSDObj = 1;
  int psdIdx[] = {0};
  int matIdx[] = {0};
  retcode = COPT_ReplacePSDObj(prob, numPSDObj, psdIdx, matIdx);
  if (retcode)
    goto exit_cleanup;

  /* Set linear costs */
  int numObj = 2;
  int colIdx[] = {0, 1};
  double colObj[] = {1.0, 2.0};
  retcode = COPT_SetColObj(prob, numObj, colIdx, colObj);
  if (retcode)
    goto exit_cleanup;

  /* Set objective constant */
  double dObjConst = 3.0;
  retcode = COPT_SetObjConst(prob, dObjConst);
  if (retcode)
    goto exit_cleanup;

  /* Solve the problem */
  retcode = COPT_Solve(prob);
  if (retcode)
    goto exit_cleanup;

  /* Get solution status */
  int lpStatus = COPT_LPSTATUS_UNSTARTED;
  retcode = COPT_GetIntAttr(prob, COPT_INTATTR_LPSTATUS, &lpStatus);
  if (retcode)
    goto exit_cleanup;

  /* Get SDP solution */
  if (lpStatus == COPT_LPSTATUS_OPTIMAL)
  {
    /* Get objective value */
    double dObjVal;
    retcode = COPT_GetDblAttr(prob, COPT_DBLATTR_LPOBJVAL, &dObjVal);
    if (retcode)
      goto exit_cleanup;

    printf("\nOptimal objective value: %.12e\n", dObjVal);
    printf("\n");

    /* Get number of PSD columns */
    int nPSD = 0;
    retcode = COPT_GetIntAttr(prob, COPT_INTATTR_PSDCOLS, &nPSD);
    if (retcode)
      goto exit_cleanup;

    for (int i = 0; i < nPSD; ++i)
    {
      int psdLen = 0;
      double* psdVal = NULL;
      double* psdDual = NULL;

      /* Get flattened length */
      retcode = COPT_GetPSDCols(prob, 1, &i, NULL, &psdLen);
      if (retcode)
        goto exit_cleanup;

      psdVal = (double*)malloc(psdLen * sizeof(double));
      psdDual = (double*)malloc(psdLen * sizeof(double));
      if (psdVal == NULL || psdDual == NULL)
      {
        retcode = COPT_RETCODE_MEMORY;
        goto exit_cleanup;
      }

      /* Get flattened SDP primal/dual solution */
      retcode = COPT_GetPSDColInfo(prob, COPT_DBLINFO_VALUE, i, psdVal);
      if (retcode)
        goto exit_cleanup;
      retcode = COPT_GetPSDColInfo(prob, COPT_DBLINFO_DUAL, i, psdDual);
      if (retcode)
        goto exit_cleanup;

      printf("SDP variable %d, flattened by column:\n", i);
      printf("Primal solution:\n");
      for (int j = 0; j < psdLen; ++j)
      {
        printf("  %.12e\n", psdVal[j]);
      }
      printf("Dual solution:\n");
      for (int j = 0; j < psdLen; ++j)
      {
        printf("  %.12e\n", psdDual[j]);
      }
      printf("\n");

      free(psdVal);
      free(psdDual);
    }

    /* Get solution for non-PSD columns */
    int nLinCol = 0;
    retcode = COPT_GetIntAttr(prob, COPT_INTATTR_COLS, &nLinCol);
    if (retcode)
      goto exit_cleanup;

    double* colVal = NULL;
    double* colDual = NULL;

    colVal = (double*)malloc(nLinCol * sizeof(double));
    colDual = (double*)malloc(nLinCol * sizeof(double));
    if (colVal == NULL || colDual == NULL)
    {
      retcode = COPT_RETCODE_MEMORY;
      goto exit_cleanup;
    }

    retcode = COPT_GetColInfo(prob, COPT_DBLINFO_VALUE, nLinCol, NULL, colVal);
    if (retcode)
      goto exit_cleanup;
    retcode = COPT_GetColInfo(prob, COPT_DBLINFO_REDCOST, nLinCol, NULL, colDual);
    if (retcode)
      goto exit_cleanup;

    printf("Non-PSD variables:\n");
    printf("Solution value:\n");
    for (int j = 0; j < nLinCol; ++j)
    {
      printf("  %.12e\n", colVal[j]);
    }
    printf("Reduced cost:\n");
    for (int j = 0; j < nLinCol; ++j)
    {
      printf("  %.12e\n", colDual[j]);
    }
    printf("\n");

    free(colVal);
    free(colDual);

    /* Get number of PSD constraints */
    int nPSDConstr = 0;
    retcode = COPT_GetIntAttr(prob, COPT_INTATTR_PSDCONSTRS, &nPSDConstr);
    if (retcode)
      goto exit_cleanup;

    /* Get activity and dual for PSD constraint */
    for (int i = 0; i < nPSDConstr; ++i)
    {
      double psdConSlack = 0.0;
      double psdConDual = 0.0;

      retcode = COPT_GetPSDConstrInfo(prob, COPT_DBLINFO_SLACK, 1, &i, &psdConSlack);
      if (retcode)
        goto exit_cleanup;

      retcode = COPT_GetPSDConstrInfo(prob, COPT_DBLINFO_DUAL, 1, &i, &psdConDual);
      if (retcode)
        goto exit_cleanup;

      printf("PSD constraint %d:\n", i);
      printf("Activity: %.12e\n", psdConSlack);
      printf("Dual:     %.12e\n", psdConDual);
      printf("\n");
    }

    /* Get number of LMI constraints */
    int nLMIConstr = 0;
    retcode = COPT_GetIntAttr(prob, COPT_INTATTR_LMICONSTRS, &nLMIConstr);
    if (retcode)
      goto exit_cleanup;

    /* Get activity and dual of LMI constraint */
    for (int i = 0; i < nLMIConstr; ++i)
    {
      int lmiLen = 0;
      double* lmiSlack = NULL;
      double* lmiDual = NULL;

      /* Get flattend LMI length */
      retcode = COPT_GetLMIConstr(prob, i, NULL, &lmiLen, NULL, NULL, NULL, 0, NULL);
      if (retcode)
        goto exit_cleanup;

      lmiSlack = (double*)malloc(lmiLen * sizeof(double));
      lmiDual = (double*)malloc(lmiLen * sizeof(double));
      if (lmiSlack == NULL || lmiDual == NULL)
      {
        retcode = COPT_RETCODE_MEMORY;
        goto exit_cleanup;
      }

      /* Get flattened LMI slack and dual solution */
      retcode = COPT_GetLMIConstrInfo(prob, COPT_DBLINFO_SLACK, i, lmiSlack);
      if (retcode)
        goto exit_cleanup;
      retcode = COPT_GetLMIConstrInfo(prob, COPT_DBLINFO_DUAL, i, lmiDual);
      if (retcode)
        goto exit_cleanup;

      printf("LMI constraint %d, flattened by column:\n", i);
      printf("Activity:\n");
      for (int j = 0; j < lmiLen; ++j)
      {
        printf("  %.12e\n", lmiSlack[j]);
      }
      printf("Dual:\n");
      for (int j = 0; j < lmiLen; ++j)
      {
        printf("  %.12e\n", lmiDual[j]);
      }
      printf("\n");

      free(lmiSlack);
      free(lmiDual);
    }
  }
  else
  {
    printf("No SDP solution!\n");
  }

exit_cleanup:
  if (retcode != 0)
  {
    char errmsg[COPT_BUFFSIZE];
    COPT_GetRetcodeMsg(retcode, errmsg, COPT_BUFFSIZE);
    printf("ERROR %d: %s\n", retcode, errmsg);
    return 0;
  }

  /* Delete problem and environment */
  COPT_DeleteProb(&prob);
  COPT_DeleteEnv(&env);

  return 0;
}
