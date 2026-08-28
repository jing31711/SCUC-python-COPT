#!/usr/bin/env bash

javac -cp .:${COPT_HOME}/lib/copt.jar Lp_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Lp_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar Socp_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Socp_ex1 ../data/estein5_A.mps

javac -cp .:${COPT_HOME}/lib/copt.jar Qp_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Qp_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar Qcp_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Qcp_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar IIS_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ IIS_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar Sdp_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Sdp_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar FeasRelax_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ FeasRelax_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar Cb_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Cb_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar Lmi_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Lmi_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar Expcone_gp.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Expcone_gp

javac -cp .:${COPT_HOME}/lib/copt.jar Nlp_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ Nlp_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar MultiObj_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ MultiObj_ex1

javac -cp .:${COPT_HOME}/lib/copt.jar SensitivityAnalysis_ex1.java
java  -cp .:${COPT_HOME}/lib/copt.jar -Djava.library.path=${COPT_HOME}/lib/ SensitivityAnalysis_ex1
