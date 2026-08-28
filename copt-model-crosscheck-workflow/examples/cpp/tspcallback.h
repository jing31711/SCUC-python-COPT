#pragma once
#include <vector>
#include "callbackbase.h"

static int FindSubTour(int nCities, double* sols, int* bestTour);

class TspCallback : public CallbackBase {
public:
  TspCallback(VarArray vars, int n) : m_nCities(n), m_vars(vars), m_sols(n * n) {}

  void callback() override
  {
    if (Where() == COPT_CBCONTEXT_MIPSOL)
    {
      GetSolution(m_vars, m_sols.data());

      std::vector<int> tour(m_nCities);
      int len = FindSubTour(m_nCities, m_sols.data(), tour.data());

      if (len < m_nCities)
      {
        Expr expr;
        for (int i = 0; i < len; i++)
        {
          int j = (i + 1) % len;
          expr += m_vars[tour[i] * m_nCities + tour[j]];
        }
        AddLazyConstr(expr <= len - 1);
      }
    }
  }

private:
  int m_nCities;
  VarArray m_vars;
  std::vector<double> m_sols;
};
