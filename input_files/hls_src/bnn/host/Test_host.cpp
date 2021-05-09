#include "../sdsoc/accel.h"

#include "../sdsoc/input_data.h"
#include <ap_int.h>

typedef ap_uint<128> bit128;


#define IMAGE_SIZE (32 * 32)

void data_gen
(
  hls::stream< bit128 > & Output_1
);

void wt_36_0_start(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1, hls::stream<bft_word> & Output_2);
void wt_36_1(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_36_2(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_36_3(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);

void wt_32_4_start(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1, hls::stream<bft_word> & Output_2);
void wt_32_5(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_36_6(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_36_7(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_10_8(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);


void wt_20_9_start(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_20_10(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_20_11(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_36_12(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_36_13(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);
void wt_12_14(hls::stream<bft_word> & Input_1, hls::stream<bft_word> & Output_1);


void mem
(
  hls::stream<bft_word> & Input_6, // bin_dense
  hls::stream<bft_word> & Input_7, // bin_dense
  hls::stream< bit128 > & Output_4, // Main output
  hls::stream<bft_word> & Output_5, // Start bin_conv_wt
  hls::stream<bft_word> & Output_6, // Start bin_dense_wt
  hls::stream<bft_word> & Output_7 // Start bin_dense_wt
);

void fp_conv
(
  hls::stream< bit128 > & Input_1,
  hls::stream<bft_word> & Output_1,
  hls::stream<bft_word> & Output_2
);

void bin_conv
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Input_2,
  hls::stream<bft_word> & Output_1
);

void bin_dense
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Input_2,
  hls::stream<bft_word> & Input_3,
  hls::stream<bit128> & Output_1
);

void bin_conv_wt_0
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1,
  hls::stream<bft_word> & Output_2
);

void bin_conv_wt
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_top
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_bot
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);




void bin_conv_wt_1
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_conv_wt_2
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_conv_wt_3
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_conv_wt_4
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_conv_wt_5
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_conv_wt_6
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_conv_wt_7
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_conv_wt_8
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_0
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1,
  hls::stream<bft_word> & Output_2
);

void bin_dense_wt_1
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_2
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_3
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_4
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_5
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_6
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_7
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_8
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_9
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_10
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_11
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_12
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_13
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_14
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void bin_dense_wt_15
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
);

void print_profile();


int main()
{
  hls::stream< bit128 > Input_1("sb1");
  hls::stream<bft_word> Start_0("sb2");
  hls::stream<bft_word> Start_1("sb3");
  hls::stream<bft_word> Start_2("sb4");
  hls::stream<bft_word> Weight_0("sb5");
  hls::stream<bft_word> Weight_1("sb6");
  hls::stream<bft_word> Weight_2("sb7");
  hls::stream<bft_word> Weight_3("sb8");
  hls::stream<bft_word> Weight_4("sb9");
  hls::stream<bft_word> Weight_5("sb10");
  hls::stream<bft_word> Weight_6("sb11");
  hls::stream<bft_word> Weight_6_0("sb12");
  hls::stream<bft_word> Weight_9_in("sb121");
  hls::stream<bft_word> Weight_10_in("sb123");
  hls::stream<bft_word> Weight_7("sb13");
  hls::stream<bft_word> Weight_18_in("sb13222");
  hls::stream<bft_word> Weight_19_in("sb13111");
  hls::stream<bft_word> Weight_8("sb14");
  hls::stream<bft_word> Weight_9("sb15");
  hls::stream<bft_word> Weight_10("sb16");
  hls::stream<bft_word> Weight_11("sb17");
  hls::stream<bft_word> Weight_12("sb18");
  hls::stream<bft_word> Weight_13("sb19");
  hls::stream<bft_word> Weight_14("sb20");
  hls::stream<bft_word> Weight_15("sb21");
  hls::stream<bft_word> Weight_16("sb22");
  hls::stream<bft_word> Weight_17("sb23");
  hls::stream<bft_word> Weight_18("sb24");
  hls::stream<bft_word> Weight_19("sb25");
  hls::stream<bft_word> Weight_20("sb26");

  hls::stream<bft_word> Stream_0("sb27");
  hls::stream<bft_word> Stream_1_0("sb28");
  hls::stream<bft_word> Stream_1_1("sb29");
  hls::stream<bft_word> Stream_2("sb30");
  hls::stream<bft_word> Stream_3("sb31");
  hls::stream<bft_word> Stream_4("sb32");
  hls::stream<bft_word> Stream_5("sb33");
  hls::stream<bit128> Stream_6("sb34");
  hls::stream<bft_word> Stream_7("sb35");
  hls::stream<bft_word> Stream_8("sb36");
  hls::stream<bft_word> Stream_9("sb37");
  hls::stream<bft_word> Stream_10("sb38");
  hls::stream<bft_word> Stream_11("sb39");
  hls::stream<bft_word> Stream_12("sb40");
  hls::stream<bft_word> Stream_13("sb41");

  hls::stream< bit128 > Output_1;

  data_gen(Input_1);


  printf("fp_conv\n");
  fp_conv(Input_1, Stream_2, Start_0);


  printf("bin_conv_wt_0\n");
  wt_36_0_start(Start_0, Weight_0, Start_1);

  printf("bin_conv_wt_1\n");
  wt_36_1(Weight_0, Weight_1);

  printf("bin_conv_wt_2\n");
  wt_36_2(Weight_1, Weight_2);

  printf("bin_conv_wt_3\n");
  wt_36_3(Weight_2,Stream_0);


  printf("bin_dense_wt_0\n");
  wt_32_4_start(Start_1, Weight_7, Start_2);

  printf("bin_dense_wt_1\n");
  wt_32_5(Weight_7, Weight_8);

  printf("bin_dense_wt_3\n");
  wt_36_6(Weight_8, Weight_10);

  printf("bin_dense_wt_4\n");
  wt_36_7(Weight_10, Weight_11);

  printf("bin_dense_wt_5\n");
  wt_10_8(Weight_11, Stream_1_0);


  ////////////////////////////////////////////////////////

  printf("bin_dense_wt_8\n");
  wt_20_9_start(Start_2, Weight_14);

  printf("bin_dense_wt_9\n");
  wt_20_10(Weight_14, Weight_15);

  printf("bin_dense_wt_10\n");
  wt_20_11(Weight_15, Weight_16);

  printf("bin_dense_wt_11\n");
  wt_36_12(Weight_16, Weight_17);

  wt_36_13(Weight_17, Weight_18);

  printf("bin_dense_wt_12\n");
  wt_12_14(Weight_18, Stream_1_1);



  // bin_conv_wt(Start_0, Stream_0);
  // bin_dense_wt_top(Start_1, Stream_1_0);
  // bin_dense_wt_bot(Start_2, Stream_1_1);





  printf("bin_conv_0\n");
  bin_conv(Stream_2, Stream_0, Stream_4);

  printf("bin_conv_1\n");
  bin_conv(Stream_2, Stream_0, Stream_4);

  printf("bin_conv_2\n");
  bin_conv(Stream_2, Stream_0, Stream_4);

  printf("bin_conv_3\n");
  bin_conv(Stream_2, Stream_0, Stream_4);

  printf("bin_conv_4\n");
  bin_conv(Stream_2, Stream_0, Stream_4);

  printf("bin_dense_0\n");
  bin_dense(Stream_4, Stream_1_0, Stream_1_1, Stream_6);

  printf("bin_dense_1\n");
  bin_dense(Stream_4, Stream_1_0, Stream_1_1, Stream_6);

  printf("bin_dense_2\n");
  bin_dense(Stream_4, Stream_1_0, Stream_1_1, Stream_6);

  bit128 word1 = Stream_6.read();
  bit128 word2 = Stream_6.read();
  printf("The output is : %d\n", (unsigned int) word2(31, 0));
  for(int i = 0; i<30; i++){
  	word2 = Stream_6.read();
  }

  print_profile();
}



void data_source_dyn
(
  int num,
  hls::stream< bit32 > & Output_1
);

void data_sink_dyn
(
  int num,
  hls::stream< bit32 > & Input_1
);

/*
int main()
{
  hls::stream< bit128 > Input_1;

  hls::stream<bft_word> Start_0;
  hls::stream<bft_word> Start_1;
  hls::stream<bft_word> Start_2;

  hls::stream<bft_word> Weight_0;
  hls::stream<bft_word> Weight_1;
  hls::stream<bft_word> Weight_2;
  hls::stream<bft_word> Weight_3;
  hls::stream<bft_word> Weight_4;
  hls::stream<bft_word> Weight_5;
  hls::stream<bft_word> Weight_6;
  hls::stream<bft_word> Weight_6_0;
  hls::stream<bft_word> Weight_7;
  hls::stream<bft_word> Weight_8;
  hls::stream<bft_word> Weight_9;
  hls::stream<bft_word> Weight_10;
  hls::stream<bft_word> Weight_11;
  hls::stream<bft_word> Weight_12;
  hls::stream<bft_word> Weight_13;
  hls::stream<bft_word> Weight_14;
  hls::stream<bft_word> Weight_15;
  hls::stream<bft_word> Weight_16;
  hls::stream<bft_word> Weight_17;
  hls::stream<bft_word> Weight_18;
  hls::stream<bft_word> Weight_19;
  hls::stream<bft_word> Weight_20;

  hls::stream<bft_word> Stream_0;
  hls::stream<bft_word> Stream_1_0;
  hls::stream<bft_word> Stream_1_1;
  hls::stream<bft_word> Stream_2;
  hls::stream<bft_word> Stream_3;
  hls::stream<bft_word> Stream_4;
  hls::stream<bft_word> Stream_5;
  hls::stream<bft_word> Stream_6;
  hls::stream<bft_word> Stream_7;
  hls::stream<bft_word> Stream_8;
  hls::stream<bft_word> Stream_9;
  hls::stream<bft_word> Stream_10;
  hls::stream<bft_word> Stream_11;
  hls::stream<bft_word> Stream_12;
  hls::stream<bft_word> Stream_13;

  hls::stream< bit128 > Output_1;

  data_source_dyn(256, Stream_12);
  data_source_dyn(131072, Stream_1_0);
  data_source_dyn(131072, Stream_1_1);
  printf("bin_dense_0\n");
  bin_dense(Stream_12, Stream_1_0, Stream_1_1, Stream_6, Stream_7);
  data_sink_dyn(16, Stream_6);
  data_sink_dyn(16, Stream_7);

  data_source_dyn(32, Stream_12);
  data_source_dyn(16384, Stream_1_0);
  data_source_dyn(16384, Stream_1_1);
  printf("bin_dense_1\n");
  bin_dense(Stream_12, Stream_1_0, Stream_1_1, Stream_6, Stream_7);
  data_sink_dyn(16, Stream_6);
  data_sink_dyn(16, Stream_7);

  data_source_dyn(32, Stream_12);
  data_source_dyn(160, Stream_1_0);
  data_source_dyn(160, Stream_1_1);
  printf("bin_dense_2\n");
  bin_dense(Stream_12, Stream_1_0, Stream_1_1, Stream_6, Stream_7);
  data_sink_dyn(1, Stream_6);
  data_sink_dyn(0, Stream_7);

}

*/
