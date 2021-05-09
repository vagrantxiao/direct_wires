
prj_name = rendering
# prj_name = bnn
# prj_name = dg_reg
# prj_name = optical_flow
# prj_name = face_detection
# prj_name = spam_filter
# prj_name = data_shift

.PHONY: quick_overlay
quick_overlay:
	python2 ./pr_flow.py rendering  -g
	cp common/overlay/floorplan_static.dcp workspace/F001_static_32_leaves
	cp common/overlay/big_static_routed_32.dcp workspace/F001_static_32_leaves
	touch workspace/F001_static_32_leaves/big_static_routed_32.dcp

.PHONY: overlay
overlay:
	# ./TimeStamp.sh
	# python2 ./pr_flow.py ${prj_name} -g | write ylxiao pts/1
	# python2 ./pr_flow.py ${prj_name} -g  -q  1>>./out.txt 2>>./error.txt
	# python2 ./pr_flow.py ${prj_name}   
	# write ylxiao pts/2 < ./out.txt
	# write ylxiao pts/2 < ./error.txt
	#./detect.sh
	python2 ./pr_flow.py rendering  -g
	cd workspace/F001_static_32_leaves && ./main.sh

.PHONY: optical_flow
optical_flow:
	python2 ./pr_flow.py optical_flow  

.PHONY: dg_reg
dg_reg:
	python2 ./pr_flow.py dg_reg 

.PHONY: bnn
bnn:
	python2 ./pr_flow.py bnn 

.PHONY: rendering
rendering:
	python2 ./pr_flow.py rendering  
	cd workspace/F002_hls_rendering && ./main.sh
	cd workspace/F003_syn_rendering && ./main.sh
	cd workspace/F004_pr_rendering && ./main.sh

.PHONY: spam_filter
spam_filter:
	python2 ./pr_flow.py  spam_filter

.PHONY: face_detection
face_detection:
	python2 ./pr_flow.py  face_detection


.PHONY: partial
partial:
	python2 ./pr_flow.py ${prj_name} -q


.PHONY: report 
report:
	 python2 ./pr_flow/gen_report.py bnn
	 python2 ./pr_flow/gen_report.py dg_reg
	 python2 ./pr_flow/gen_report.py face_detection
	 python2 ./pr_flow/gen_report.py optical_flow
	 python2 ./pr_flow/gen_report.py rendering
	 python2 ./pr_flow/gen_report.py spam_filter

clean:

	rm -rf ./workspace
