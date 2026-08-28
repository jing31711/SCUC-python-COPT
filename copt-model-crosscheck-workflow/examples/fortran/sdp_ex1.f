C *
C * This file is part of the Cardinal Optimizer, all rights reserved.
C *
C *                 [2, 1, 0]   
C *  minimize    Tr [1, 2, 1] * X + x0
C *                 [0, 1, 2]
C * 
C *                 [1, 0, 0]
C *  subject to  Tr [0, 1, 0] * X <= 0.8
C *                 [0, 0, 1]
C * 
C *                 [1, 1, 1]
C *              Tr [1, 1, 1] * X + x1 + x2 = 0.6
C *                 [1, 1, 1]
C *
C *              x0 + x1 + x2 <= 0.9
C *              x0 >= (x1^2 + x2^2) ^ (1/2)
C * 
C *    x0, x1, x2 non-negative, X in PSD

      program sdp_ex1

      implicit none

C---- Dimension of problem and constants
      integer ncol, nelem
      integer ncone, nconeelem
      integer npsdcol
      integer nlinobj, npsdobj

C---- Local variables
      integer i

C---- Define dimension and constants
      parameter (ncol  = 3)
      parameter (nelem = 3)

      parameter (ncone = 1)
      parameter (nconeelem = 3)

      parameter (npsdcol = 1)

      parameter (nlinobj = 1)
      parameter (npsdobj = 1)

C---- PSD column dimension
      integer psddim(npsdcol)

C---- Symmetrix matrix
      integer nmatdim, nmatelem, matrow(6), matcol(6)
      double precision matelem(6)

C---- Solution and result information
      integer lpstatus
      double precision lpobjval, lpsol(ncol)

C---- PSD row
      integer npsdrowcnt, npsdcolcnt, psdrowsen
      integer psdrowidx(2), psdcolidx(1), psdmatidx(1)
      double precision psdrowelem(2), psdrowbnd

C---- Sparse row matrix, sense and rhs
      integer nrowmatcnt, rowmatidx(3), rowsense
      double precision rowmatelem(3), rowbound

C---- Cone data
      integer conetype(ncone)
      integer conebeg(ncone), conecnt(ncone), coneidx(nconeelem)

C---- Linear objective
      integer linobjidx(nlinobj)
      double precision linobjelem(nlinobj)

C---- PSD objective
      integer psdobjidx(npsdobj), psdobjmatidx(npsdobj)

C---- NULL data
      integer, pointer :: INULL
      double precision, pointer :: DNULL
      NULLIFY(INULL)
      NULLIFY(DNULL)

C---- Create environment
      call coptf_createenv

C---- Create problem
      call coptf_createprob

C---- Create symmetric matrix C
      nmatdim = 3
      nmatelem = 5
      matrow(1) = 0
      matrow(2) = 1
      matrow(3) = 1
      matrow(4) = 2
      matrow(5) = 2
      matcol(1) = 0
      matcol(2) = 0
      matcol(3) = 1
      matcol(4) = 1
      matcol(5) = 2
      matelem(1) = 2.0d0
      matelem(2) = 1.0d0
      matelem(3) = 2.0d0
      matelem(4) = 1.0d0
      matelem(5) = 2.0d0
      call coptf_addsymmat(nmatdim, nmatelem, matrow, matcol, matelem)

C---- Create symmetric matrix A1
      nmatdim = 3
      nmatelem = 3
      matrow(1) = 0
      matrow(2) = 1
      matrow(3) = 2
      matcol(1) = 0
      matcol(2) = 1
      matcol(3) = 2
      matelem(1) = 1.0d0
      matelem(2) = 1.0d0
      matelem(3) = 1.0d0
      call coptf_addsymmat(nmatdim, nmatelem, matrow, matcol, matelem)

C---- Create symmetric matrix A2
      nmatdim = 3
      nmatelem = 6
      matrow(1) = 0
      matrow(2) = 1
      matrow(3) = 2
      matrow(4) = 1
      matrow(5) = 2
      matrow(6) = 2
      matcol(1) = 0
      matcol(2) = 0
      matcol(3) = 0
      matcol(4) = 1
      matcol(5) = 1
      matcol(6) = 2
      matelem(1) = 1.0d0
      matelem(2) = 1.0d0
      matelem(3) = 1.0d0
      matelem(4) = 1.0d0
      matelem(5) = 1.0d0
      matelem(6) = 1.0d0
      call coptf_addsymmat(nmatdim, nmatelem, matrow, matcol, matelem)

C---- Add PSD columns
      psddim(1) = 3
      call coptf_addpsdcols(npsdcol, psddim)

C---- Add columns to problem
      call coptf_addcols(ncol, DNULL, INULL, INULL, INULL, DNULL,
     $                   INULL, DNULL, DNULL)

C---- Add PSD constraints
      npsdrowcnt = 0
      npsdcolcnt = 1
      psdcolidx(1) = 0
      psdmatidx(1) = 1
      psdrowsen = 0
      psdrowbnd = 8.0d-1
      call coptf_addpsdconstr(npsdrowcnt, psdrowidx, psdrowelem,
     $                        npsdcolcnt, psdcolidx, psdmatidx,
     $                        psdrowsen, psdrowbnd, 0.0)

      npsdrowcnt = 2
      psdrowidx(1) = 1
      psdrowidx(2) = 2
      psdrowelem(1) = 1.0d0
      psdrowelem(2) = 1.0d0
      npsdcolcnt = 1
      psdcolidx(1) = 0
      psdmatidx(1) = 2
      psdrowsen = 2
      psdrowbnd = 6.0d-1
      call coptf_addpsdconstr(npsdrowcnt, psdrowidx, psdrowelem,
     $                        npsdcolcnt, psdcolidx, psdmatidx,
     $                        psdrowsen, psdrowbnd, 0.0)

C---- Add row to problem
      nrowmatcnt = 3
      rowmatidx(1) = 0
      rowmatidx(2) = 1
      rowmatidx(3) = 2
      rowmatelem(1) = 1.0d0
      rowmatelem(2) = 1.0d0
      rowmatelem(3) = 1.0d0
      rowsense = 0
      rowbound = 9.0d-1
      call coptf_addrow(nrowmatcnt, rowmatidx, rowmatelem,
     $                  rowsense, rowbound, 0.0)

C---- Add cone
      conetype(1) = 1
      conebeg(1) = 0
      conecnt(1) = 3
      coneidx(1) = 0
      coneidx(2) = 1
      coneidx(3) = 2
      call coptf_addcones(ncone, conetype, conebeg, conecnt, coneidx)

C---- Set PSD objective
      psdobjidx(1) = 0
      psdobjmatidx(1) = 0
      call coptf_replacepsdobj(npsdobj, psdobjidx, psdobjmatidx)

C---- Set linear objective
      linobjidx(1) = 0
      linobjelem(1) = 1.0
      call coptf_setcolobj(nlinobj, linobjidx, linobjelem)

C---- Solve the problem
      call coptf_solve

C---- Get solution status
      call coptf_getintattr('LpStatus$', lpstatus)

C---- Check solution status
      if (lpstatus .eq. 1) then
C---- Get objective value
        call coptf_getdblattr('LpObjval$', lpobjval)

C---- Get solution
        call coptf_getlpsolution(lpsol, DNULL, DNULL, DNULL)

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

C---- End of sdp_ex1
