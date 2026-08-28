#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "copt.h"

static void print_usage(char* cmdname)
{
  printf("Command line usage:\n");
  printf("    %s [-t timelimit] model.mps[.gz]\n", cmdname);
  printf("\n");
  printf("Example:\n");
  printf("    %s model.mps\n", cmdname);
  printf("    %s -t 7200 model.mps\n", cmdname);
}

void check_ray(copt_prob* prob)
{
  int hasRay = 0;
  COPT_GetIntAttr(prob, COPT_INTATTR_HASPRIMALRAY, &hasRay);
  if (hasRay)
  {
    printf("Primal ray is available\n");
  }
  else
  {
    printf("Primal ray is not available\n");
    return;
  }

  /* Get ray and cost */
  int nCol = 0;
  COPT_GetIntAttr(prob, COPT_INTATTR_COLS, &nCol);
  double* ray = malloc(sizeof(double) * nCol);
  double* cost = malloc(sizeof(double) * nCol);
  if (!ray || !cost)
  {
    return;
  }
  COPT_GetColInfo(prob, COPT_DBLINFO_PRIMALRAY, nCol, NULL, ray);
  COPT_GetColInfo(prob, COPT_DBLINFO_OBJ, nCol, NULL, cost);

  /* Compute the inner product of the cost and ray vector. */
  double dCostRay = 0.0;
  for (int iCol = 0; iCol < nCol; iCol++)
  {
    dCostRay += ray[iCol] * cost[iCol];
  }

  /* Check whether the cost * ray is OK. Please note that we only check one of necessary condition here. */
  int iObjSense = 0;
  COPT_GetIntAttr(prob, COPT_INTATTR_OBJSENSE, &iObjSense);
  if (iObjSense * dCostRay < 0)
  {
    printf("The objective sense is %d, ray * cost = %g - OK\n", iObjSense, dCostRay);
  }
  else
  {
    printf("The objective sense is %d, ray * cost = %g - invalid\n", iObjSense, dCostRay);
  }
  free(ray);
  free(cost);
}

void check_farkas(copt_prob* prob)
{
  int hasFaraks = 0;
  COPT_GetIntAttr(prob, COPT_INTATTR_HASDUALFARKAS, &hasFaraks);
  if (hasFaraks)
  {
    printf("Dual Farkas is available\n");
  }
  else
  {
    printf("Dual Farkas is not available\n");
    return;
  }

  /* Get farkas and row bounds */
  int nRow = 0;
  COPT_GetIntAttr(prob, COPT_INTATTR_ROWS, &nRow);
  double* farkas = malloc(sizeof(double) * nRow);
  double* rowL = malloc(sizeof(double) * nRow);
  double* rowU = malloc(sizeof(double) * nRow);
  if (!farkas || !rowL || !rowU)
  {
    return;
  }
  COPT_GetRowInfo(prob, COPT_DBLINFO_DUALFARKAS, nRow, NULL, farkas);
  COPT_GetRowInfo(prob, COPT_DBLINFO_LB, nRow, NULL, rowL);
  COPT_GetRowInfo(prob, COPT_DBLINFO_UB, nRow, NULL, rowU);

  /* Check whether the Farkas is OK. Please note that we only check one of necessary condition here. */
  int isFarkasOK = 1;
  double dFarkasRHS = 0.0;
  for (int iRow = 0; iRow < nRow; iRow++)
  {
    if ((farkas[iRow] > +1e-6 && rowL[iRow] <= -COPT_INFINITY) ||
        (farkas[iRow] < -1e-6 && rowU[iRow] >= +COPT_INFINITY))
    {
      isFarkasOK = 0;
      break;
    }

    if (farkas[iRow] > 0.0 && rowL[iRow] > -COPT_INFINITY)
    {
      dFarkasRHS += farkas[iRow] * rowL[iRow];
    }
    if (farkas[iRow] < 0.0 && rowU[iRow] < +COPT_INFINITY)
    {
      dFarkasRHS += farkas[iRow] * rowU[iRow];
    }
  }
  if (isFarkasOK)
  {
    printf("The Farkas is OK. Farkas * RHS = %g\n", dFarkasRHS);
  }
  else
  {
    printf("The Farkas is invalid");
  }

  free(farkas);
  free(rowL);
  free(rowU);
}

int main(int argc, char** argv)
{
  copt_env* env = NULL;
  copt_prob* prob = NULL;
  char buff[COPT_BUFFSIZE];
  int ifCmdOK = 0;

  COPT_GetBanner(buff, 1000);
  printf("%s\n", buff);

  int iRet = COPT_CreateEnv(&env);
  if (iRet != 0)
  {
    COPT_GetLicenseMsg(env, buff, COPT_BUFFSIZE);
    printf("%s\n", buff);
    goto exit_cleanup;
  }

  if (argc == 2)
  {
    /* Create the LP and read in from the MPS file */
    ifCmdOK = 1;
    COPT_CreateProb(env, &prob);
    COPT_SetDblParam(prob, COPT_DBLPARAM_TIMELIMIT, 3600);

    int retcode = COPT_ReadMps(prob, argv[1]);
    if (retcode != 0)
    {
      goto exit_cleanup;
    }

    /* Set to require ray or Farkas */
    COPT_SetIntParam(prob, COPT_INTPARAM_REQFARKASRAY, 1);

    /* Solve the problem */
    COPT_SolveLp(prob);

    /* Get the status */
    int lpStatus = 0;
    COPT_GetIntAttr(prob, COPT_INTATTR_LPSTATUS, &lpStatus);
    printf("The LP status is %d\n", lpStatus);

    /* Check for farkas or ray when infeasible or unbounded */
    if (lpStatus == COPT_LPSTATUS_INFEASIBLE)
    {
      check_farkas(prob);
    }
    if (lpStatus == COPT_LPSTATUS_UNBOUNDED)
    {
      check_ray(prob);
    }
  }
  else if (argc == 4)
  {
    /* Check that the first option is -t and it followed by a number */
    double dTimeLim = 0;
    if (strcmp(argv[1], "-t") == 0)
    {
      char* ptr = NULL;
      dTimeLim = strtod(argv[2], &ptr);
      if (ptr != argv[2] && dTimeLim > 0.0)
      {
        ifCmdOK = 1;
      }
    }

    if (ifCmdOK)
    {
      /* Create the problem and set the time limt */
      COPT_CreateProb(env, &prob);
      COPT_SetDblParam(prob, COPT_DBLPARAM_TIMELIMIT, dTimeLim);

      /* Read the problem */
      int retcode = COPT_ReadMps(prob, argv[3]);
      if (retcode != 0)
      {
        goto exit_cleanup;
      }

      /* Solve the problem */
      COPT_SolveLp(prob);
    }
  }

  if (ifCmdOK == 0)
  {
    print_usage(argv[0]);
  }

exit_cleanup:
  COPT_DeleteProb(&prob);
  COPT_DeleteEnv(&env);
  return 0;
}
