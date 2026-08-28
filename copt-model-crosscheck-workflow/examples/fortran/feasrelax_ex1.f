C *
C * This file is part of the Cardinal Optimizer, all rights reserved.
C *
C * We will compute feasibility relaxation for the following infeasible LP problem,
C * the problem is 'itest6' test case from netlib-infeas:
C *
C * Minimize
C *  X2 + X3 + X4
C * Subject To
C *  ROW1: 0.8 X3 + X4 <= 10000
C *  ROW2: X1 <= 90000
C *  ROW3: 2 X6 - X8 <= 10000
C *  ROW4: - X2 + X3 >= 50000
C *  ROW5: - X2 + X4 >= 87000
C *  ROW6: X3 <= 50000
C *  ROW7: - 3 X5 + X7 >= 10000
C *  ROW8: 0.5 X5 + 0.6 X6 <= 300000
C *  ROW9: X2 - 0.05 X3 = 5000
C *  ROW10: X2 - 0.04 X3 - 0.05 X4 = 4500
C *  ROW11: X2 >= 80000
C * END

      program feasrelax_ex1

      implicit none

C---- Dimension of problem and constants
      integer nrow, ncol, nelem

C---- Local variables
      integer i

C---- Define dimension and constants
      parameter (nrow  = 11)
      parameter (ncol  = 8)
      parameter (nelem = 20)

C---- Solution and result information
      integer lpstatus, hasrelaxsol
      double precision rowbndpen(nrow)
      double precision collowpen(ncol), colupppen(ncol)
      double precision rowlowrelax(nrow), rowupprelax(nrow)
      double precision collowrelax(ncol), colupprelax(ncol)

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
      colcost(1) = 0.0d0
      colcost(2) = 1.0d0
      colcost(3) = 1.0d0
      colcost(4) = 1.0d0
      colcost(5) = 0.0d0
      colcost(6) = 0.0d0
      colcost(7) = 0.0d0
      colcost(8) = 0.0d0

      do i=1,ncol
        coltype(i) = 0
        collb(i) = 0.0d0
        colub(i) = 1.0d+30
      end do

      rowbeg(1) = 0
      rowbeg(2) = 2
      rowbeg(3) = 3
      rowbeg(4) = 5
      rowbeg(5) = 7
      rowbeg(6) = 9
      rowbeg(7) = 10
      rowbeg(8) = 12
      rowbeg(9) = 14
      rowbeg(10) = 16
      rowbeg(11) = 19

      rowcnt(1) = 2
      rowcnt(2) = 1
      rowcnt(3) = 2
      rowcnt(4) = 2
      rowcnt(5) = 2
      rowcnt(6) = 1
      rowcnt(7) = 2
      rowcnt(8) = 2
      rowcnt(9) = 2
      rowcnt(10) = 3
      rowcnt(11) = 1

      rowind(1) = 2
      rowind(2) = 3
      rowind(3) = 0
      rowind(4) = 5
      rowind(5) = 7
      rowind(6) = 1
      rowind(7) = 2
      rowind(8) = 1
      rowind(9) = 3
      rowind(10) = 2
      rowind(11) = 4
      rowind(12) = 6
      rowind(13) = 4
      rowind(14) = 5
      rowind(15) = 1
      rowind(16) = 2
      rowind(17) = 1
      rowind(18) = 2
      rowind(19) = 3
      rowind(20) = 1

      rowelem(1) = +8.0d-1
      rowelem(2) = +1.0d0
      rowelem(3) = +1.0d0
      rowelem(4) = +2.0d0
      rowelem(5) = -1.0d0
      rowelem(6) = -1.0d0
      rowelem(7) = +1.0d0
      rowelem(8) = -1.0d0
      rowelem(9) = +1.0d0
      rowelem(10) = +1.0d0
      rowelem(11) = -3.0d0
      rowelem(12) = +1.0d0
      rowelem(13) = +5.0d-1
      rowelem(14) = +6.0d-1
      rowelem(15) = +1.0d0
      rowelem(16) = -5.0d-2
      rowelem(17) = +1.0d0
      rowelem(18) = -4.0d-2
      rowelem(19) = -5.0d-2
      rowelem(20) = +1.0d0

      rowsen(1) = 0
      rowsen(2) = 0
      rowsen(3) = 0
      rowsen(4) = 1
      rowsen(5) = 1
      rowsen(6) = 0
      rowsen(7) = 1
      rowsen(8) = 0
      rowsen(9) = 2
      rowsen(10) = 2
      rowsen(11) = 1

      rowrhs(1) = 1.0d4
      rowrhs(2) = 9.0d4
      rowrhs(3) = 1.0d4
      rowrhs(4) = 5.0d4
      rowrhs(5) = 8.7d4
      rowrhs(6) = 5.0d4
      rowrhs(7) = 1.0d4
      rowrhs(8) = 3.0d5
      rowrhs(9) = 5.0d3
      rowrhs(10) = 4.5d3
      rowrhs(11) = 8.0d4

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
      call coptf_solve

C---- Get solution status
      call coptf_getintattr('LpStatus$', lpstatus)

C---- Check solution status
      if (lpstatus .eq. 2) then
C---- Assign row/column penalties
        do i=1,nrow
          rowbndpen(i) = 1.0d0
        end do

        do i=1,ncol
          collowpen(i) = 1.0d0
          colupppen(i) = 1.0d0
        end do

C---- Set feasibility relaxation mode: minimize sum of violations
        call coptf_setintparam('FeasRelaxMode$', 0)

C---- Compute feasibility relaxation
        call coptf_feasrelax(collowpen, colupppen, rowbndpen, DNULL)

C---- Check if feasibility relaxation solution is available
        call coptf_getintattr('HasFeasRelaxSol$', hasrelaxsol)

C---- Retrieve feasibility relaxation result
        if (hasrelaxsol .eq. 1) then
C---- Get relaxaion violations for variables and constraints
          call coptf_getrowinfo('RelaxLB$', nrow, INULL, rowlowrelax)
          call coptf_getrowinfo('RelaxUB$', nrow, INULL, rowupprelax)

          call coptf_getcolinfo('RelaxLB$', ncol, INULL, collowrelax)
          call coptf_getcolinfo('RelaxUB$', ncol, INULL, colupprelax)

C---- Print violations of variables and constraints
          write(*, 1001)

          do i=1,nrow
            write(*, 1002) rowlowrelax(i), rowupprelax(i)
          end do

          write(*, 1003)

          do i=1,ncol
            write(*, 1004) collowrelax(i), colupprelax(i)
          end do

C---- Write relaxed problem to file
          call coptf_writerelax('feasrelax_ex1.relax$')
        endif

C---- Print formats
 1001 format('Violations of rows:')
 1002 format('  lower:', f12.3, '  upper:', f12.3)
 1003 format('Violations of columns:')
 1004 format('  lower:', f12.3, '  upper:', f12.3)
      endif

C---- Delete problem
      call coptf_deleteprob

C---- Delete environment
      call coptf_deleteenv

      stop
      end

C---- End of feasrelax_ex1
