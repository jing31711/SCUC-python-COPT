C *
C * This file is part of the Cardinal Optimizer, all rights reserved.
C *
C * Minimize
C *  obj: 2.1 x1 - 1.2 x2 + 3.2 x3 + x4 + x5 + x6 + 2 x7 + [ x2^2 ] / 2
C * Subject To
C *  r1: x1 + 2 x2 = 6
C *  r2: 2 x1 + x3 >= 5
C *  r3: x6 + 2 x7 <= 7
C *  r4: -x1 + 1.2 x7 >= -2.3
C *  q1: [ -1.8 x1^2 + x2^2 ] <= 0
C *  q2: [ 4.25 x3^2 - 2 x3 * x4 + 4.25 x4^2 - 2 x4 * x5 + 4 x5^2  ] + 2 x1 + 3 x3 <= 9.9
C *  q3: [ x6^2 - 2.2 x7^2 ] >= 5
C * Bounds
C *  0.2 <= x1 <= 3.8
C *  x2 Free
C *  0.1 <= x3 <= 0.7
C *  x4 Free
C *  x5 Free
C *  x7 Free
C * End

      program qcp_ex1

      implicit none

C---- Dimension of problem and constants
      integer nrow, ncol, nelem, nqelem

C---- Local variables
      integer i

C---- Define dimension and constants
      parameter (nrow  = 4)
      parameter (ncol  = 7)
      parameter (nelem = 8)
      parameter (nqelem = 1)

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

C---- Quadratic constraint
      integer nrowmatcnt, nqmatcnt, crowsense,
     $        rowmatidx(2),
     $        qmatrow(5), qmatcol(5)
      double precision drowbnd, rowmatelem(2), qmatelem(5)

C---- NULL data
      integer, pointer :: INULL
      double precision, pointer :: DNULL
      NULLIFY(INULL)
      NULLIFY(DNULL)

C---- Problem data
      colcost(1) = 2.1d0
      colcost(2) = -1.2d0
      colcost(3) = 3.2d0
      colcost(4) = 1.0d0
      colcost(5) = 1.0d0
      colcost(6) = 1.0d0
      colcost(7) = 2.0d0

      do i=1,ncol
        coltype(i) = 0
      end do

      collb(1) = 2.0d-1
      collb(2) = -1.0d30
      collb(3) = 1.0d-1
      collb(4) = -1.0d30
      collb(5) = -1.0d30
      collb(6) = 0.0d0
      collb(7) = -1.0d30

      colub(1) = 3.8d0
      colub(2) = 1.0d30
      colub(3) = 7.0d-1
      colub(4) = 1.0d30
      colub(5) = 1.0d30
      colub(6) = 1.0d30
      colub(7) = 1.0d30

      rowbeg(1) = 0
      rowbeg(2) = 2
      rowbeg(3) = 4
      rowbeg(4) = 6

      rowcnt(1) = 2
      rowcnt(2) = 2
      rowcnt(3) = 2
      rowcnt(4) = 2

      rowind(1) = 0
      rowind(2) = 1
      rowind(3) = 0
      rowind(4) = 2
      rowind(5) = 5
      rowind(6) = 6
      rowind(7) = 0
      rowind(8) = 6

      rowelem(1) = 1.0d0
      rowelem(2) = 2.0d0
      rowelem(3) = 2.0d0
      rowelem(4) = 1.0d0
      rowelem(5) = 1.0d0
      rowelem(6) = 2.0d0
      rowelem(7) = -1.0d0
      rowelem(8) = 1.2d0

      rowsen(1) = 2
      rowsen(2) = 1
      rowsen(3) = 0
      rowsen(4) = 1

      rowrhs(1) = 6.0d0
      rowrhs(2) = 5.0d0
      rowrhs(3) = 7.0d0
      rowrhs(4) = -2.3d0

      qrow(1) = 1
      qcol(1) = 1
      qelem(1) = 5.0d-1

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

C---- Add quadratic constraint
      nrowmatcnt = 0
      nqmatcnt = 2
      qmatrow(1) = 0
      qmatrow(2) = 1
      qmatcol(1) = 0
      qmatcol(2) = 1
      qmatelem(1) = -1.8d0
      qmatelem(2) = 1.0d0
      crowsense = 0
      drowbnd = 0.0d0
      call coptf_addqconstr(nrowmatcnt, rowmatidx, rowmatelem, 
     $                      nqmatcnt, qmatrow, qmatcol, qmatelem,
     $                      crowsense, drowbnd)

      nrowmatcnt = 2
      rowmatidx(1) = 0
      rowmatidx(2) = 2
      rowmatelem(1) = 2.0d0
      rowmatelem(2) = 3.0d0
      nqmatcnt = 5
      qmatrow(1) = 2
      qmatrow(2) = 2
      qmatrow(3) = 3
      qmatrow(4) = 3
      qmatrow(5) = 4
      qmatcol(1) = 2
      qmatcol(2) = 3
      qmatcol(3) = 3
      qmatcol(4) = 4
      qmatcol(5) = 4
      qmatelem(1) = 4.25d0
      qmatelem(2) = -2.0d0
      qmatelem(3) = 4.25d0
      qmatelem(4) = -2.0d0
      qmatelem(5) = 4.0d0
      crowsense = 0
      drowbnd = 9.9d0
      call coptf_addqconstr(nrowmatcnt, rowmatidx, rowmatelem, 
     $                      nqmatcnt, qmatrow, qmatcol, qmatelem,
     $                      crowsense, drowbnd)

      nrowmatcnt = 0
      nqmatcnt = 2
      qmatrow(1) = 5
      qmatrow(2) = 6
      qmatcol(1) = 5
      qmatcol(2) = 6
      qmatelem(1) = 1.0d0
      qmatelem(2) = -2.2d0
      crowsense = 1
      drowbnd = 5.0d0
      call coptf_addqconstr(nrowmatcnt, rowmatidx, rowmatelem, 
     $                      nqmatcnt, qmatrow, qmatcol, qmatelem,
     $                      crowsense, drowbnd)

C---- Add quadratic objective
      call coptf_setquadobj(nqelem, qrow, qcol, qelem)

C---- Set parameters
      call coptf_setdblparam('TimeLimit$', 1.d+1)

C---- Set objective sense
      call coptf_setobjsense(1)

C---- Write problem to file
      call coptf_writelp('qcp_ex1.lp$')

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

C---- End of qcp_ex1
