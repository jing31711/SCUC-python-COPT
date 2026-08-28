/*
 * This file is part of the Cardinal Optimizer, all rights reserved.
 */

#include <assert.h>
#include <cmath>
#include <string>

#include "coptcpp_pch.h"
#include "tspcallback.h"

using namespace std;

// Calculate euclidean distance between i-th and j-th points
static double GetDistance(double* px, double* py, int i, int j)
{
  double dx = px[i] - px[j];
  double dy = py[i] - py[j];
  return sqrt(dx * dx + dy * dy);
}

static int NextUnvisited(vector<bool> visited)
{
  int node = 0;
  while (node < (int)visited.size() && visited[node])
  {
    node++;
  }
  return node;
}

/*
 * Given an integer-feasible solution 'sols' of size nCities * nCities,
 * find the smallest sub-tour. Result is returned as 'tour' list
 */
static int FindSubTour(int nCities, double* sols, int* bestTour)
{
  vector<int> tours;
  vector<bool> visited(nCities, false);
  int bestLen = nCities + 1;
  int bestStart = -1;

  int start = 0;
  int curr = 0;
  while (start < nCities && curr < nCities)
  {
    // Search a tour starting from current city until the tour is closed
    int len = 0;
    bool closed = false;
    while (!closed)
    {
      len++;
      tours.push_back(curr);
      visited[curr] = true;

      // Enumerate all unvisited neighbors of current city
      for (int j = 0; j < nCities; j++)
      {
        if (sols[curr * nCities + j] > 0.5 && !visited[j])
        {
          curr = j;
          break;
        }
        // If all neighbors are visited, the tour is closed
        closed = (j == nCities - 1);
      }
    }

    // Save the current tour if it is better
    if (len < bestLen)
    {
      bestLen = len;
      bestStart = start;
    }

    // Update start and curr for next tour
    start += len;
    curr = NextUnvisited(visited);
  }

  for (int i = 0; i < bestLen; i++)
  {
    bestTour[i] = tours[bestStart + i];
  }
  return bestLen;
}

int main(int argc, char* argv[])
{
  int nCities = argc > 1 ? atoi(argv[1]) : 10;
  if (nCities <= 0)
  {
    cout << "[Error] Number of TSP cities must be positive." << endl;
    return -1;
  }

  if (nCities > 100)
  {
    cout << "[Warn] TSP problem of " << nCities;
    cout << " cities might take long time to solve." << endl;
  }

  // Create random points
  vector<double> px(nCities);
  vector<double> py(nCities);
  for (int i = 0; i < nCities; i++)
  {
    px[i] = ((double)rand()) / RAND_MAX;
    py[i] = ((double)rand()) / RAND_MAX;
  }

  try
  {
    // Create COPT environment and model for asymmetric TSP
    Envr env;
    Model model = env.CreateModel("TSP Callback C++ Example");

    // Add binary variables - one for every pair of cities
    VarArray vars;
    for (int i = 0; i < nCities; i++)
    {
      for (int j = 0; j < nCities; j++)
      {
        string name = "E_" + to_string(i) + "_" + to_string(j);
        double vobj = GetDistance(px.data(), py.data(), i, j);
        vars.PushBack(model.AddVar(0, 1, vobj, COPT_BINARY, name.c_str()));
      }
    }

    // Add outdegree-1 constraints for asymmetric TSP loop
    for (int i = 0; i < nCities; i++)
    {
      Expr expr;
      for (int j = 0; j < nCities; j++)
      {
        expr += vars[i * nCities + j];
      }
      string name = "OutDegree_1_" + to_string(i);
      model.AddConstr(expr == 1, name.c_str());
    }

    // Add indegree-1 constraints for asymmetric TSP loop
    for (int j = 0; j < nCities; j++)
    {
      Expr expr;
      for (int i = 0; i < nCities; i++)
      {
        expr += vars[i * nCities + j];
      }
      string name = "InDegree_1_" + to_string(j);
      model.AddConstr(expr == 1, name.c_str());
    }

    // Add constraints to exclude self loop
    for (int i = 0; i < nCities; i++)
    {
      vars[i * nCities + i].Set(COPT_DBLINFO_UB, 0.0);
    }

    // Set TSP callback instance
    TspCallback tcb(vars, nCities);
    model.SetCallback(&tcb, COPT_CBCONTEXT_MIPSOL);

    // Solve the TSP problem
    model.Solve();

    // Get solution if having MIP solution, and print tour
    int hasMipSol = model.GetIntAttr(COPT_INTATTR_HASMIPSOL);
    cout << "\n[HasMipSol] = " << hasMipSol << endl;
    if (hasMipSol)
    {
      vector<double> sols(nCities * nCities);
      model.Get(COPT_DBLINFO_VALUE, vars, sols.data());

      vector<int> tour(nCities);
      int len = FindSubTour(nCities, sols.data(), tour.data());
      assert(len == nCities);

      cout << "\n  Best Tour:";
      for (int i = 0; i < len; i++)
      {
        cout << " " << tour[i];
      }
      cout << "\n  Best Cost: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << endl;
    }

    cout << "  TSP Callback C++ Example Finished" << endl;
  }
  catch (const CoptException& e)
  {
    cout << "Error code = " << e.GetCode() << endl;
    cout << e.what() << endl;
  }
  catch (...)
  {
    cout << "Exception during optimization" << endl;
  }
}
