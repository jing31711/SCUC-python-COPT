C *
C * This file is part of the Cardinal Optimizer, all rights reserved.
C *
C *   minimize: z
C *   r0: 3*x + y >= 1
C *   c0: z^2 >= x^2 + 2*y^2
C *
C * c0 is converted to:
C *
C *   r1: sqrt(2.0)*y - t = 0
C *   c1: z^2 >= x^2 + t^2
C *
C *   bnds:
C *     x, y, t free, z non-negative

      program socp_ex1

      implicit none

C---- Dimension of problem and constants
      integer nrow, ncol, ncone, nelem, ncelem

C---- Local variables
      integer i

C---- Define dimension and constants
      parameter (nrow  = 2)
      parameter (ncol  = 4)
      parameter (ncone = 1)
      parameter (nelem = 4)
      parameter (ncelem = 3)

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

C---- Cone data
      integer conetype(ncone), conebeg(ncone), conecnt(ncone),
     $        coneind(ncelem)

C---- NULL data
      integer, pointer :: INULL
      double precision, pointer :: DNULL
      NULLIFY(INULL)
      NULLIFY(DNULL)

C---- Problem data
      colcost(1) = 0.0d0
      colcost(2) = 0.0d0
      colcost(3) = 1.0d0
      colcost(4) = 0.0d0

      do i=1,ncol
        coltype(i) = 0
      end do

      collb(1) = -1.0d30
      collb(2) = -1.0d30
      collb(3) = 0.0d0
      collb(4) = -1.0d30

      do i=1,ncol
        colub(i) = 1.0d30
      end do

      rowbeg(1) = 0
      rowbeg(2) = 2

      rowcnt(1) = 2
      rowcnt(2) = 2

      rowind(1) = 0
      rowind(2) = 1
      rowind(3) = 1
      rowind(4) = 3

      rowelem(1) = 3.0d0
      rowelem(2) = 1.0d0
      rowelem(3) = sqrt(2.0d0)
      rowelem(4) = -1.0d0

      rowsen(1) = 1
      rowsen(2) = 2

      rowrhs(1) = 1.0d0
      rowrhs(2) = 0.0d0

      conetype(1) = 1
      conebeg(1) = 0
      conecnt(1) = 3
      coneind(1) = 2
      coneind(2) = 0
      coneind(3) = 3

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

C---- Add cone constraint
      call coptf_addcones(ncone, conetype, conebeg, conecnt, coneind)

C---- Set parameters
      call coptf_setdblparam('TimeLimit$', 1.d+1)

C---- Set objective sense
      call coptf_setobjsense(1)

C---- Write problem to file
      call coptf_writemps('socp_ex1.mps$')

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

C---- End of socp_ex1
