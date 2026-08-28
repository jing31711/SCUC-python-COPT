CXX       = clang++
CXX_FLAGS = -std=c++11 -O2 -DNDEBUG 

CXX_INCS  = -I$(COPT_HOME)/include -I$(COPT_HOME)/include/coptcpp_inc
CXX_LIBS  = -L$(COPT_HOME)/lib -lcopt_cpp

CXX_EXAMPLE  = lp_ex1 socp_ex1 qp_ex1 qcp_ex1 iis_ex1 sdp_ex1 feasrelax_ex1 cb_ex1 lmi_ex1 nlp_ex1 expcone_gp
CXX_EXAMPLE += cutstock cutstock_cg locationtransport locationtransport_lr
CXX_EXAMPLE += facility filterdesign multicommodity multicommodity_dw
CXX_EXAMPLE += cutstock_matrix diet_matrix transport_matrix
CXX_EXAMPLE += matfactor matqcqp multiobj_ex1 sensitivity_analysis

all: $(CXX_EXAMPLE)

lp_ex1: lp_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o lp_ex1 lp_ex1.cpp $(CXX_LIBS)

socp_ex1: socp_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o socp_ex1 socp_ex1.cpp $(CXX_LIBS)

qp_ex1: qp_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o qp_ex1 qp_ex1.cpp $(CXX_LIBS)

qcp_ex1: qcp_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o qcp_ex1 qcp_ex1.cpp $(CXX_LIBS)

iis_ex1: iis_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o iis_ex1 iis_ex1.cpp $(CXX_LIBS)

sdp_ex1: sdp_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o sdp_ex1 sdp_ex1.cpp $(CXX_LIBS)

feasrelax_ex1: feasrelax_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o feasrelax_ex1 feasrelax_ex1.cpp $(CXX_LIBS)

cb_ex1: cb_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o cb_ex1 cb_ex1.cpp $(CXX_LIBS)

lmi_ex1: lmi_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o lmi_ex1 lmi_ex1.cpp $(CXX_LIBS)

nlp_ex1: nlp_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o nlp_ex1 nlp_ex1.cpp $(CXX_LIBS)

expcone_gp: expcone_gp.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o expcone_gp expcone_gp.cpp $(CXX_LIBS)

cutstock: cutstock.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o cutstock cutstock.cpp $(CXX_LIBS)

cutstock_cg: cutstock_cg.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o cutstock_cg cutstock_cg.cpp $(CXX_LIBS)

locationtransport: locationtransport.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o locationtransport locationtransport.cpp $(CXX_LIBS)

locationtransport_lr: locationtransport_lr.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o locationtransport_lr locationtransport_lr.cpp $(CXX_LIBS)

facility: facility.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o facility facility.cpp $(CXX_LIBS)

filterdesign: filterdesign.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o filterdesign filterdesign.cpp $(CXX_LIBS)

multicommodity: multicommodity.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o multicommodity multicommodity.cpp $(CXX_LIBS)

multicommodity_dw: multicommodity_dw.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o multicommodity_dw multicommodity_dw.cpp $(CXX_LIBS)

cutstock_matrix: cutstock_matrix.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o cutstock_matrix cutstock_matrix.cpp $(CXX_LIBS)

diet_matrix: diet_matrix.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o diet_matrix diet_matrix.cpp $(CXX_LIBS)

transport_matrix: transport_matrix.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o transport_matrix transport_matrix.cpp $(CXX_LIBS)

matfactor: matfactor.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o matfactor matfactor.cpp $(CXX_LIBS)

matqcqp: matqcqp.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o matqcqp matqcqp.cpp $(CXX_LIBS)

multiobj_ex1: multiobj_ex1.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o multiobj_ex1 multiobj_ex1.cpp $(CXX_LIBS)

sensitivity_analysis: sensitivity_analysis.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -o sensitivity_analysis sensitivity_analysis.cpp $(CXX_LIBS)

clean:
	rm -f $(CXX_EXAMPLE)
