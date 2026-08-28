FC     = gfortran
FFLAGS = -O2

F_INCS = 
F_LIBS = -L$(COPT_HOME)/lib -lcopt_fortran -lcopt

F_EXAMPLE = lp_ex1 socp_ex1 qp_ex1 qcp_ex1 iis_ex1 sdp_ex1 feasrelax_ex1

all: $(F_EXAMPLE)

lp_ex1: lp_ex1.f
	$(FC) $(FFLAGS) $(F_INCS) -o lp_ex1 lp_ex1.f $(F_LIBS)

socp_ex1: socp_ex1.f
	$(FC) $(FFLAGS) $(F_INCS) -o socp_ex1 socp_ex1.f $(F_LIBS)

qp_ex1: qp_ex1.f
	$(FC) $(FFLAGS) $(F_INCS) -o qp_ex1 qp_ex1.f $(F_LIBS)

qcp_ex1: qcp_ex1.f
	$(FC) $(FFLAGS) $(F_INCS) -o qcp_ex1 qcp_ex1.f $(F_LIBS)

iis_ex1: iis_ex1.f
	$(FC) $(FFLAGS) $(F_INCS) -o iis_ex1 iis_ex1.f $(F_LIBS)

sdp_ex1: sdp_ex1.f
	$(FC) $(FFLAGS) $(F_INCS) -o sdp_ex1 sdp_ex1.f $(F_LIBS)

feasrelax_ex1: feasrelax_ex1.f
	$(FC) $(FFLAGS) $(F_INCS) -o feasrelax_ex1 feasrelax_ex1.f $(F_LIBS)

clean:
	rm -f $(F_EXAMPLE)
