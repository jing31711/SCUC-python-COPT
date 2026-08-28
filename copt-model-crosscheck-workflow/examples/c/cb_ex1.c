/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

/*
 * This example solves the traveling salesman problem on a randomly generated
 * set of points using lazy constraint callback.
 */

#include "copt.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Structure for user-defined callback data */
struct callback_usrdata
{
  int n; /* Number of points */
};

/* Compute the euclidean distance between two points */
static double euclid_distance(int i, int j, double* x, double* y)
{
  double dx = x[i] - x[j];
  double dy = y[i] - y[j];
  return sqrt(dx * dx + dy * dy);
}

/* For a given MIP candidate solution, find the smallest subtour */
static int findsubtour(int n, double* sol, int* tour, int* p_tourlen)
{
  int errcode = COPT_RETCODE_OK;
  int i = 0;
  int node = 0;
  int* seen = NULL;

  seen = (int*)calloc(n, sizeof(int));
  if (seen == NULL)
  {
    errcode = COPT_RETCODE_MEMORY;
    goto exit_cleanup;
  }

  int start = 0;
  int bestlen = n + 1;
  int bestind = -1;
  while (start < n)
  {
    /* Find the first node we've not yet seen. */
    for (node = 0; node < n; node++)
    {
      if (seen[node] == 0)
      {
        break;
      }
    }
    /* We're done if we've seen all nodes by now. */
    if (node == n)
    {
      break;
    }
    for (int len = 0; len < n; len++)
    {
      tour[start + len] = node;
      seen[node] = 1;
      for (i = 0; i < n; i++)
      {
        if (sol[node * n + i] > 0.5 && !seen[i])
        {
          node = i;
          break;
        }
      }
      if (i == n)
      {
        len++;
        if (len < bestlen)
        {
          bestlen = len;
          bestind = start;
        }
        start += len;
        break;
      }
    }
  }

  for (int i = 0; i < bestlen; i++)
  {
    tour[i] = tour[bestind + i];
  }

  *p_tourlen = bestlen;

exit_cleanup:
  /* Free memory */
  free(seen);
  return errcode;
}

/*
 * Subtour elimination callback
 *   Find the shortest subtour whenever a MIP candidate solution was found, and
 *   add a subtour elimination constraint if the subtour doesn't visit every node.
 */
int COPT_CALL subtourelim(copt_prob* prob, void* cbdata, int cbctx, void* usrdata)
{
  struct callback_usrdata* mydata = (struct callback_usrdata*)usrdata;
  int errcode = COPT_RETCODE_OK;
  int n = mydata->n;
  int len = 0;
  int* ind = NULL;
  double* val = NULL;
  int* tour = NULL;
  double* sol = NULL;

  if (cbctx == COPT_CBCONTEXT_MIPSOL)
  {
    sol = (double*)malloc(n * n * sizeof(double));
    tour = (int*)malloc(n * sizeof(int));
    if (sol == NULL || tour == NULL)
    {
      errcode = COPT_RETCODE_MEMORY;
      goto exit_cleanup;
    }

    /* Get the MIP candidate solution */
    errcode = COPT_GetCallbackInfo(cbdata, COPT_CBINFO_MIPCANDIDATE, sol);
    if (errcode)
      goto exit_cleanup;

    /* Find the smallest subtour */
    errcode = findsubtour(n, sol, tour, &len);
    if (errcode)
      goto exit_cleanup;

    if (len < n)
    {
      ind = (int*)malloc(len * (len - 1) / 2 * sizeof(int));
      val = (double*)malloc(len * (len - 1) / 2 * sizeof(double));
      if (ind == NULL || val == NULL)
      {
        errcode = COPT_RETCODE_MEMORY;
        goto exit_cleanup;
      }

      /* Add subtour elimination constraint, i.e.
       *
       *   sum x_i_j <= len - 1, where edge from i to j in is in the subtour
       *
       * By (3), this constraint also implies that
       *
       *   sum x_j_i <= len - 1
       */
      int nz = 0;
      for (int i = 0; i < len; i++)
      {
        for (int j = i + 1; j < len; j++)
        {
          ind[nz++] = tour[i] * n + tour[j];
        }
      }
      for (int i = 0; i < nz; i++)
      {
        val[i] = 1.0;
      }

      /* Add lazy constraint */
      errcode = COPT_AddCallbackLazyConstr(cbdata, nz, ind, val, COPT_LESS_EQUAL, len - 1);
      if (errcode)
        goto exit_cleanup;
    }
  }
  else
  {
    fprintf(stderr, "Unregistered callback context\n");
    errcode = COPT_RETCODE_INVALID;
    goto exit_cleanup;
  }

exit_cleanup:
  /* Free memory */
  free(ind);
  free(val);
  free(tour);
  free(sol);
  return errcode;
}

int main(int argc, char* argv[])
{
  int errcode = COPT_RETCODE_OK;
  copt_env* env = NULL;
  copt_prob* prob = NULL;
  int n = 0;
  int len = 0;
  char name[100];
  double* x = NULL;
  double* y = NULL;
  int* ind = NULL;
  double* val = NULL;
  int* tour = NULL;
  double* sol = NULL;
  struct callback_usrdata mydata;

  /* Check the input arguments */
  if (argc < 2)
  {
    n = 10;
  }
  else
  {
    n = atoi(argv[1]);
    if (n == 0)
    {
      fprintf(stderr, "Number of points must be a positive integer\n");
      goto exit_cleanup;
    }
    else if (n > 100)
    {
      fprintf(stdout, "Number of points is large, the TSP may be challenging to solve\n");
    }
  }

  /* Create random points */
  x = (double*)malloc(n * sizeof(double));
  y = (double*)malloc(n * sizeof(double));
  if (x == NULL || y == NULL)
  {
    errcode = COPT_RETCODE_MEMORY;
    goto exit_cleanup;
  }

  for (int i = 0; i < n; i++)
  {
    x[i] = ((double)rand()) / RAND_MAX;
    y[i] = ((double)rand()) / RAND_MAX;
  }

  /* Create COPT environment */
  errcode = COPT_CreateEnv(&env);
  if (errcode)
    goto exit_cleanup;

  /* Create COPT problem */
  errcode = COPT_CreateProb(env, &prob);
  if (errcode)
    goto exit_cleanup;

  /*
   * Add variables
   * If an edge from i to j is chosen, then x_i_j = x_j_i = 1, which means
   * we can go along the route forward or backward.
   * The cost of choosing edge ij is equal to its euclidean distance, thus
   * we set the cost of x_i_j and x_j_i to euclid_distance(i, j)/2.
   */
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      sprintf(name, "x_%d_%d", i, j);
      errcode = COPT_AddCol(prob, euclid_distance(i, j, x, y) / 2, 0, NULL, NULL, COPT_BINARY, 0.0, 1.0, name);
      if (errcode)
        goto exit_cleanup;
    }
  }

  /* Add the degree-2 constraints:
   * (1)  sum_j x_i_j = 2
   */
  ind = (int*)malloc(n * sizeof(int));
  val = (double*)malloc(n * sizeof(double));
  if (ind == NULL || val == NULL)
  {
    errcode = COPT_RETCODE_MEMORY;
    goto exit_cleanup;
  }

  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      ind[j] = i * n + j;
      val[j] = 1.0;
    }

    sprintf(name, "deg2_%d", i);
    errcode = COPT_AddRow(prob, n, ind, val, COPT_EQUAL, 2, 0.0, name);
    if (errcode)
      goto exit_cleanup;
  }

  /* Forbid an edge from node back to itself
   * (2)  x_i_i = 0
   */
  for (int i = 0; i < n; i++)
  {
    int j = i * n + i;
    double zero = 0.0;
    errcode = COPT_SetColUpper(prob, 1, &j, &zero);
    if (errcode)
      goto exit_cleanup;
  }

  /* Constraints for symmetric variables
   * (3)  x_i_j = x_j_i
   */
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < i; j++)
    {
      ind[0] = i * n + j;
      ind[1] = i + j * n;
      val[0] = 1;
      val[1] = -1;
      errcode = COPT_AddRow(prob, 2, ind, val, COPT_EQUAL, 0.0, 0.0, NULL);
      if (errcode)
        goto exit_cleanup;
    }
  }

  /* Set user-defined callback data */
  mydata.n = n;

  /* Set callback function */
  errcode = COPT_SetCallback(prob, subtourelim, COPT_CBCONTEXT_MIPSOL, (void*)(&mydata));
  if (errcode)
    goto exit_cleanup;

  /* Solve the problem */
  errcode = COPT_Solve(prob);
  if (errcode)
    goto exit_cleanup;

  /* Get the solution information */
  int hasmipsol = 0;
  errcode = COPT_GetIntAttr(prob, COPT_INTATTR_HASMIPSOL, &hasmipsol);
  if (errcode)
    goto exit_cleanup;

  if (hasmipsol)
  {
    sol = (double*)malloc(n * n * sizeof(double));
    tour = (int*)malloc(n * sizeof(int));
    if (sol == NULL || tour == NULL)
    {
      errcode = COPT_RETCODE_MEMORY;
      goto exit_cleanup;
    }

    errcode = COPT_GetColInfo(prob, COPT_DBLINFO_VALUE, n * n, NULL, sol);
    if (errcode)
      goto exit_cleanup;

    /* Print tour */
    findsubtour(n, sol, tour, &len);
    printf("Tour: ");
    for (int i = 0; i < len; i++)
    {
      printf("%d ", tour[i]);
    }
    printf("\n");
  }

exit_cleanup:
  /* Free memory */
  free(x);
  free(y);
  free(ind);
  free(val);
  free(tour);
  free(sol);

  /* Error message */
  if (errcode)
  {
    char errmsg[COPT_BUFFSIZE];
    COPT_GetRetcodeMsg(errcode, errmsg, COPT_BUFFSIZE);
    printf("ERROR %d: %s\n", errcode, errmsg);
  }

  /* Delete problem and environment */
  COPT_DeleteProb(&prob);
  COPT_DeleteEnv(&env);

  return 0;
}
