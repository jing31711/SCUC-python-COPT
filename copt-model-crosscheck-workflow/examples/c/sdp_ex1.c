/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/**
 *                 [2, 1, 0]
 *  minimize    Tr [1, 2, 1] * X + x0
 *                 [0, 1, 2]
 *
 *                 [1, 0, 0]
 *  subject to  Tr [0, 1, 0] * X <= 0.8
 *                 [0, 0, 1]
 *
 *                 [1, 1, 1]
 *              Tr [1, 1, 1] * X + x1 + x2 = 0.6
 *                 [1, 1, 1]
 *
 *              x0 + x1 + x2 <= 0.9
 *              x0 >= (x1^2 + x2^2) ^ (1/2)
 *
 *    x0, x1, x2 non-negative, X in PSD
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

  /* Create symmetric matrix */
  {
    /* Matrix C */
    int ndim = 3;
    int nelem = 5;
    int rows[] = {0, 1, 1, 2, 2};
    int cols[] = {0, 0, 1, 1, 2};
    double elems[] = {2.0, 1.0, 2.0, 1.0, 2.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  {
    /* Matrix A1 */
    int ndim = 3;
    int nelem = 3;
    int rows[] = {0, 1, 2};
    int cols[] = {0, 1, 2};
    double elems[] = {1.0, 1.0, 1.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  {
    /* Matrix A2 */
    int ndim = 3;
    int nelem = 6;
    int rows[] = {0, 1, 2, 1, 2, 2};
    int cols[] = {0, 0, 0, 1, 1, 2};
    double elems[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

    retcode = COPT_AddSymMat(prob, ndim, nelem, rows, cols, elems);
    if (retcode)
      goto exit_cleanup;
  }

  /* Add PSD columns */
  int nPSDCol = 1;
  int colDims[] = {3};
  retcode = COPT_AddPSDCols(prob, nPSDCol, colDims, NULL);
  if (retcode)
    goto exit_cleanup;

  /* Add columns */
  int nCol = 3;
  retcode = COPT_AddCols(prob, nCol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  if (retcode)
    goto exit_cleanup;

  /* Add PSD constraints */
  {
    int nRowMatCnt = 0;
    int* rowMatIdx = NULL;
    double* rowMatElem = NULL;
    int nColCnt = 1;
    int psdColIdx[] = {0};
    int symMatIdx[] = {1};
    char cRowSense = COPT_LESS_EQUAL;
    double dRowBound = 0.8;
    retcode = COPT_AddPSDConstr(prob, nRowMatCnt, rowMatIdx, rowMatElem, nColCnt, psdColIdx, symMatIdx, cRowSense,
      dRowBound, 0.0, NULL);
    if (retcode)
      goto exit_cleanup;
  }

  {
    int nRowMatCnt = 2;
    int rowMatIdx[] = {1, 2};
    double rowMatElem[] = {1.0, 1.0};
    int nColCnt = 1;
    int psdColIdx[] = {0};
    int symMatIdx[] = {2};
    char cRowSense = COPT_EQUAL;
    double dRowBound = 0.6;
    retcode = COPT_AddPSDConstr(prob, nRowMatCnt, rowMatIdx, rowMatElem, nColCnt, psdColIdx, symMatIdx, cRowSense,
      dRowBound, 0.0, NULL);
    if (retcode)
      goto exit_cleanup;
  }

  {
    int nRowMatCnt = 3;
    int rowMatIdx[] = {0, 1, 2};
    double rowMatElem[] = {1.0, 1.0, 1.0};
    char cRowSense = COPT_LESS_EQUAL;
    double dRowBound = 0.9;
    retcode = COPT_AddRow(prob, nRowMatCnt, rowMatIdx, rowMatElem, cRowSense, dRowBound, 0.0, NULL);
    if (retcode)
      goto exit_cleanup;
  }

  /* Add cone */
  int nCone = 1;
  int coneType[] = {COPT_CONE_QUAD};
  int coneBeg[] = {0};
  int coneCnt[] = {3};
  int coneIdx[] = {0, 1, 2};
  retcode = COPT_AddCones(prob, nCone, coneType, coneBeg, coneCnt, coneIdx);
  if (retcode)
    goto exit_cleanup;

  /* Set PSD costs */
  int numPSDObj = 1;
  int psdIdx[] = {0};
  int matIdx[] = {0};
  retcode = COPT_ReplacePSDObj(prob, numPSDObj, psdIdx, matIdx);
  if (retcode)
    goto exit_cleanup;

  /* Set linear costs */
  int numObj = 1;
  int colIdx[] = {0};
  double colObj[] = {1.0};
  retcode = COPT_SetColObj(prob, numObj, colIdx, colObj);
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
      printf("\n");

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
    printf("\n");

    printf("Reduced cost:\n");
    for (int j = 0; j < nLinCol; ++j)
    {
      printf("  %.12e\n", colDual[j]);
    }

    free(colVal);
    free(colDual);
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
