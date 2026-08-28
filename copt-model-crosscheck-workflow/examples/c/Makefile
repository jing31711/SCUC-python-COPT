CC     = clang
CFLAGS = -std=c99 -O2 -DNDEBUG

C_INCS = -I$(COPT_HOME)/include
C_LIBS = -L$(COPT_HOME)/lib -lcopt -lm

C_EXAMPLE  = lp_ex1 solvemps socp_ex1 qp_ex1 qcp_ex1 iis_ex1 sdp_ex1 feasrelax_ex1
C_EXAMPLE += cb_ex1 lmi_ex1.c expcone_gp nlp_ex1 multiobj_ex1 sensitivity_analysis

all: $(C_EXAMPLE)

lp_ex1: lp_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o lp_ex1 lp_ex1.c $(C_LIBS)

solvemps: solvemps.c
	$(CC) $(CFLAGS) $(C_INCS) -o solvemps solvemps.c $(C_LIBS)

socp_ex1: socp_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o socp_ex1 socp_ex1.c $(C_LIBS)

qp_ex1: qp_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o qp_ex1 qp_ex1.c $(C_LIBS)

qcp_ex1: qcp_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o qcp_ex1 qcp_ex1.c $(C_LIBS)

iis_ex1: iis_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o iis_ex1 iis_ex1.c $(C_LIBS)

sdp_ex1: sdp_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o sdp_ex1 sdp_ex1.c $(C_LIBS)

feasrelax_ex1: feasrelax_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o feasrelax_ex1 feasrelax_ex1.c $(C_LIBS)

cb_ex1: cb_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o cb_ex1 cb_ex1.c $(C_LIBS)

lmi_ex1: lmi_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o lmi_ex1 lmi_ex1.c $(C_LIBS)

expcone_gp: expcone_gp.c
	$(CC) $(CFLAGS) $(C_INCS) -o expcone_gp expcone_gp.c $(C_LIBS)

nlp_ex1: nlp_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o nlp_ex1 nlp_ex1.c $(C_LIBS)

multiobj_ex1: multiobj_ex1.c
	$(CC) $(CFLAGS) $(C_INCS) -o multiobj_ex1 multiobj_ex1.c $(C_LIBS)

sensitivity_analysis: sensitivity_analysis.c
	$(CC) $(CFLAGS) $(C_INCS) -o sensitivity_analysis sensitivity_analysis.c $(C_LIBS)

clean:
	rm -f $(C_EXAMPLE)
