/*===============================================================*/
/*                                                               */
/*                       face_detect.h                           */
/*                                                               */
/*     Hardware function for the Face Detection application.     */
/*                                                               */
/*===============================================================*/


#include "../host/typedefs.h"


void face_detect

(
  hls::stream<ap_uint<128> > & Input_1,
  hls::stream<ap_uint<128> > & Output_1
);

void face_detect_mono

(
  hls::stream<ap_uint<32> > & Input_1,
  hls::stream<ap_uint<32> > & Output_1
);

void data_gen
(
  hls::stream<ap_uint<128> > & Output_1
);



void sfilter0_try
(
  hls::stream<ap_uint<32> > & Input_1,
  hls::stream<ap_uint<32> > & Output_1,
  hls::stream<ap_uint<32> > & Output_2
);
