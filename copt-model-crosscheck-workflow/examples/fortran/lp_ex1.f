C *
C * This file is part of the Cardinal Optimizer, all rights reserved.
C *
C * The problem to solve:
C *
C *  Maximize:
C *    1.2 x + 1.8 y + 2.1 z
C *
C *  Subject to:
C *    1.5 x + 1.2 y + 1.8 z <= 2.6
C *    0.8 x + 0.6 y + 0.9 z >= 1.2
C *
C *  where:
C *    0.1 <= x <= 0.6
C *    0.2 <= y <= 1.5
C *    0.3 <= z <= 2.8

      program lp_ex1

      implicit none

C---- Dimension of problem and constants
      integer nrow, ncol, nelem

C---- Local variables
      integer i

C---- Define dimension and constants
      parameter (nrow  = 2)
      parameter (ncol  = 3)
      parameter (nelem = 6)

C---- Solution and result information
      integer lpstatus, colstat(ncol), rowstat(nrow)
      double precision lpobjval(1), lpsol(ncol), redcost(ncol),
     $                 rowslack(nrow), rowdual(nrow)

C---- Cost, lower/upper bounds of columns
      double precision colcost(ncol), collb(ncol), colub(ncol)
      integer coltype(ncol)

C---- Sparse row matrix, sense and rhs
      integer rowbeg(nrow), rowcnt(nrow), rowind(nelem)
      double precision rowelem(nelem), rowrhs(nrow)
      integer rowsen(nrow)

C---- NULL data
      integer, pointer :: INULL
      double precision, pointer :: DNULL
      NULLIFY(INULL)
      NULLIFY(DNULL)

C---- Problem data
      colcost(1) = 1.2d0
      colcost(2) = 1.8d0
      colcost(3) = 2.1d0

      do i=1,ncol
        coltype(i) = 0    ! 0: COPT_CONTINUOUS, 1: COPT_BINARY, 2: COPT_INTEGER
      end do

      collb(1) = 0.1d0
      collb(2) = 0.2d0
      collb(3) = 0.3d0

      colub(1) = 0.6d0
      colub(2) = 1.5d0
      colub(3) = 1.8d0

      rowbeg(1) = 0
      rowbeg(2) = 3

      rowcnt(1) = 3
      rowcnt(2) = 3

      rowind(1) = 0
      rowind(2) = 1
      rowind(3) = 2
      rowind(4) = 0
      rowind(5) = 1
      rowind(6) = 2

      rowelem(1) = 1.5d0
      rowelem(2) = 1.2d0
      rowelem(3) = 1.8d0
      rowelem(4) = 0.8d0
      rowelem(5) = 0.6d0
      rowelem(6) = 0.9d0

      rowsen(1) = 0    ! 0: COPT_LESS_EQUAL
      rowsen(2) = 1    ! 1: COPT_GREATER_EQUAL
                       ! 2: COPT_EQUAL
                       ! 3: COPT_FREE
                       ! 4: COPT_RANGE

      rowrhs(1) = 2.6d0
      rowrhs(2) = 1.2d0

C---- Create environment
      call coptf_createenv

C---- Create problem
      call coptf_createprob

C---- Add columns to problem
      call coptf_addcols(ncol, colcost, INULL, INULL, INULL, DNULL,
     $                   coltype, collb, colub)

C---- Add rows to problem
      call coptf_addrows(nrow, rowbeg, rowcnt, rowind, rowelem,
     $                   rowsen, rowrhs, DNULL)

C---- Set parameters
      call coptf_setdblparam('TimeLimit$', 1.d+1)

C---- Set objective sense
      call coptf_setobjsense(-1)  ! -1: COPT_MAXIMIZE, 1: COPT_MINIMIZE

C---- Solve the problem
      call coptf_solvelp

C---- Get solution status
      call coptf_getintattr('LpStatus$', lpstatus)

C---- Check solution status
      if (lpstatus .eq. 1) then  ! 1: COPT_OPTIMAL
C---- Get objective value
        call coptf_getdblattr('LpObjval$', lpobjval)

C---- Get solution
        call coptf_getlpsolution(lpsol, rowslack, rowdual, redcost)

C---- Get basis status
        call coptf_getbasis(colstat, rowstat)

C---- Write problem, solution and modified parameters
        call coptf_writemps('lp_ex1.mps$')
        call coptf_writebasis('lp_ex1.bas$')
        call coptf_writesol('lp_ex1.sol$')
        call coptf_writeparam('lp_ex1.par$')

C---- Display objective value
        write(*, 1000) lpobjval

C---- Display solution of columns
        write(*, 1001)

        do i=1,ncol
          write(*, 1002) lpsol(i)
        end do

C---- Display basis status of columns
        write(*, 1003)

        do i=1,ncol
          write(*, 1004) colstat(i)
        end do

C---- Print formats
 1000 format(/'Objective value:', f12.9)
 1001 format('Variable solution:')
 1002 format('  ', f12.9)
 1003 format('Variable basis status:')
 1004 format('  ', i2)
      endif

C---- Delete problem
      call coptf_deleteprob

C---- Delete environment
      call coptf_deleteenv

      stop
      end

C---- End of lp_ex1
