C *
C * This file is part of the Cardinal Optimizer, all rights reserved.
C *

      program runmps

      implicit none

C---- Create environment
      call coptf_createenv

C---- Create problem
      call coptf_createprob

C---- Set log file
      call coptf_setlogfile('coptfortran.log$')

C---- Set optimization parameter
      call coptf_setdblparam('TimeLimit$', 6.d+1)

C---- Read problem from file
      call coptf_readmps('cutstock.mps.gz$')

C---- Solve problem
      call coptf_solve

C---- Delete problem
      call coptf_deleteprob

C---- Delete environment
      call coptf_deleteenv

      stop
      end

C---- End of runmps
