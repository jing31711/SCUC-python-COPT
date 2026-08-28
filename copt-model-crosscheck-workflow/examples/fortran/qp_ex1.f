C *
C * This file is part of the Cardinal Optimizer, all rights reserved.
C *
C * Minimize
C *  OBJ.FUNC: [ 2 X0 ^2 + 4 X0 * X1 + 4 X1 ^2
C *            + 4 X1 * X2 + 4 X2 ^2
C *            + 4 X2 * X3 + 4 X3 ^2
C *            + 4 X3 * X4 + 4 X4 ^2
C *            + 4 X4 * X5 + 4 X5 ^2
C *            + 4 X5 * X6 + 4 X6 ^2
C *            + 4 X6 * X7 + 4 X7 ^2
C *            + 4 X7 * X8 + 4 X8 ^2
C *            + 4 X8 * X9 + 2 X9 ^2 ] / 2
C * Subject To
C *  ROW0: X0 + 2 X1 + 3 X2  = 1
C *  ROW1: X1 + 2 X2 + 3 X3  = 1
C *  ROW2: X2 + 2 X3 + 3 X4  = 1
C *  ROW3: X3 + 2 X4 + 3 X5  = 1
C *  ROW4: X4 + 2 X5 + 3 X6  = 1
C *  ROW5: X5 + 2 X6 + 3 X7  = 1
C *  ROW6: X6 + 2 X7 + 3 X8  = 1
C *  ROW7: X7 + 2 X8 + 3 X9  = 1
C * Bounds
C *       X0 Free
C *       X1 Free
C *       X2 Free
C *       X3 Free
C *       X4 Free
C *       X5 Free
C *       X6 Free
C *       X7 Free
C *       X8 Free
C *       X9 Free
C * End

      program qp_ex1

      implicit none

C---- Dimension of problem and constants
      integer nrow, ncol, nelem, nqelem

C---- Local variables
      integer i

C---- Define dimension and constants
      parameter (nrow  = 8)
      parameter (ncol  = 10)
      parameter (nelem = 24)
      parameter (nqelem = 19)

C---- Solution and result information
      integer lpstatus
      double precision lpobjval(1), lpsol(ncol), redcost(ncol),
     $                 rowslack(nrow), rowdual(nrow)

C---- Cost, lower/upper bounds of columns
      double precision colcost(ncol), collb(ncol), colub(ncol)
      integer coltype(ncol)

C---- Sparse row matrix, sense and rhs
      integer rowbeg(nrow), rowcnt(nrow), rowind(nelem)
      double precision rowelem(nelem), rowrhs(nrow)
      integer rowsen(nrow)

C---- Quadratic objective
      integer qrow(nqelem), qcol(nqelem)
      double precision qelem(nqelem)

C---- NULL data
      integer, pointer :: INULL
      double precision, pointer :: DNULL
      NULLIFY(INULL)
      NULLIFY(DNULL)

C---- Problem data
      do i=1,ncol
        colcost(i) = 0.0d0
        coltype(i) = 0
        collb(i) = -1.0d+30
        colub(i) = +1.0d+30
      end do

      rowbeg(1) = 0
      rowbeg(2) = 3
      rowbeg(3) = 6
      rowbeg(4) = 9
      rowbeg(5) = 12
      rowbeg(6) = 15
      rowbeg(7) = 18
      rowbeg(8) = 21

      rowcnt(1) = 3
      rowcnt(2) = 3
      rowcnt(3) = 3
      rowcnt(4) = 3
      rowcnt(5) = 3
      rowcnt(6) = 3
      rowcnt(7) = 3
      rowcnt(8) = 3

      rowind(1) = 0
      rowind(2) = 1
      rowind(3) = 2
      rowind(4) = 1
      rowind(5) = 2
      rowind(6) = 3
      rowind(7) = 2
      rowind(8) = 3
      rowind(9) = 4
      rowind(10) = 3
      rowind(11) = 4
      rowind(12) = 5
      rowind(13) = 4
      rowind(14) = 5
      rowind(15) = 6
      rowind(16) = 5
      rowind(17) = 6
      rowind(18) = 7
      rowind(19) = 6
      rowind(20) = 7
      rowind(21) = 8
      rowind(22) = 7
      rowind(23) = 8
      rowind(24) = 9

      rowelem(1) = 1.0d0
      rowelem(2) = 2.0d0
      rowelem(3) = 3.0d0
      rowelem(4) = 1.0d0
      rowelem(5) = 2.0d0
      rowelem(6) = 3.0d0
      rowelem(7) = 1.0d0
      rowelem(8) = 2.0d0
      rowelem(9) = 3.0d0
      rowelem(10) = 1.0d0
      rowelem(11) = 2.0d0
      rowelem(12) = 3.0d0
      rowelem(13) = 1.0d0
      rowelem(14) = 2.0d0
      rowelem(15) = 3.0d0
      rowelem(16) = 1.0d0
      rowelem(17) = 2.0d0
      rowelem(18) = 3.0d0
      rowelem(19) = 1.0d0
      rowelem(20) = 2.0d0
      rowelem(21) = 3.0d0
      rowelem(22) = 1.0d0
      rowelem(23) = 2.0d0
      rowelem(24) = 3.0d0

      rowsen(1) = 2
      rowsen(2) = 2
      rowsen(3) = 2
      rowsen(4) = 2
      rowsen(5) = 2
      rowsen(6) = 2
      rowsen(7) = 2
      rowsen(8) = 2

      rowrhs(1) = 1.0d0
      rowrhs(2) = 1.0d0
      rowrhs(3) = 1.0d0
      rowrhs(4) = 1.0d0
      rowrhs(5) = 1.0d0
      rowrhs(6) = 1.0d0
      rowrhs(7) = 1.0d0
      rowrhs(8) = 1.0d0

      qrow(1) = 0
      qrow(2) = 0
      qrow(3) = 1
      qrow(4) = 1
      qrow(5) = 2
      qrow(6) = 2
      qrow(7) = 3
      qrow(8) = 3
      qrow(9) = 4
      qrow(10) = 4
      qrow(11) = 5
      qrow(12) = 5
      qrow(13) = 6
      qrow(14) = 6
      qrow(15) = 7
      qrow(16) = 7
      qrow(17) = 8
      qrow(18) = 8
      qrow(19) = 9

      qcol(1) = 0
      qcol(2) = 1
      qcol(3) = 1
      qcol(4) = 2
      qcol(5) = 2
      qcol(6) = 3
      qcol(7) = 3
      qcol(8) = 4
      qcol(9) = 4
      qcol(10) = 5
      qcol(11) = 5
      qcol(12) = 6
      qcol(13) = 6
      qcol(14) = 7
      qcol(15) = 7
      qcol(16) = 8
      qcol(17) = 8
      qcol(18) = 9
      qcol(19) = 9

      qelem(1) = 1.0d0
      qelem(2) = 2.0d0
      qelem(3) = 2.0d0
      qelem(4) = 2.0d0
      qelem(5) = 2.0d0
      qelem(6) = 2.0d0
      qelem(7) = 2.0d0
      qelem(8) = 2.0d0
      qelem(9) = 2.0d0
      qelem(10) = 2.0d0
      qelem(11) = 2.0d0
      qelem(12) = 2.0d0
      qelem(13) = 2.0d0
      qelem(14) = 2.0d0
      qelem(15) = 2.0d0
      qelem(16) = 2.0d0
      qelem(17) = 2.0d0
      qelem(18) = 2.0d0
      qelem(19) = 1.0d0

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

C---- Add quadratic objective
      call coptf_setquadobj(nqelem, qrow, qcol, qelem)

C---- Set parameters
      call coptf_setdblparam('TimeLimit$', 1.d+1)

C---- Set objective sense
      call coptf_setobjsense(1)

C---- Write problem to file
      call coptf_writelp('qp_ex1.lp$')

C---- Solve the problem
      call coptf_solve

C---- Get solution status
      call coptf_getintattr('LpStatus$', lpstatus)

C---- Check solution status
      if (lpstatus .eq. 1) then
C---- Get objective value
        call coptf_getdblattr('LpObjval$', lpobjval)

C---- Get solution
        call coptf_getlpsolution(lpsol, rowslack, rowdual, redcost)

C---- Display objective value
        write(*, 1000) lpobjval

C---- Display solution of columns
        write(*, 1001)

        do i=1,ncol
          write(*, 1002) lpsol(i)
        end do

C---- Print formats
 1000 format(/'Objective value:', f12.9)
 1001 format('Variable solution:')
 1002 format('  ', f12.9)
      endif

C---- Delete problem
      call coptf_deleteprob

C---- Delete environment
      call coptf_deleteenv

      stop
      end

C---- End of qp_ex1
