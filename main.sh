#!/bin/bash -e
qsub -N face_detect  -q 70s@icgrid49 -hold_jid hls_data_mover -m abe -M qsub@qsub.com -l mem=120G -pe onenode 1  -cwd ./run_face_detection.sh
qsub -N bnn          -q 70s@icgrid49 -hold_jid hls_data_mover -m abe -M qsub@qsub.com -l mem=120G -pe onenode 1  -cwd ./run_bnn.sh
qsub -N dg_reg       -q 70s@icgrid49 -hold_jid hls_data_mover -m abe -M qsub@qsub.com -l mem=120G -pe onenode 1  -cwd ./run_dg_reg.sh
qsub -N rendering    -q 70s@icgrid49 -hold_jid hls_data_mover -m abe -M qsub@qsub.com -l mem=120G -pe onenode 1  -cwd ./run_rendering.sh
qsub -N optical_flow -q 70s@icgrid49 -hold_jid hls_data_mover -m abe -M qsub@qsub.com -l mem=120G -pe onenode 1  -cwd ./run_optical_flow.sh
qsub -N spam_filter  -q 70s@icgrid49 -hold_jid hls_data_mover -m abe -M qsub@qsub.com -l mem=120G -pe onenode 1  -cwd ./run_spam_filter.sh
