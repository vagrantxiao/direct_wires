#define MAX_IMG_SIZE 32 * 32
#include <ap_int.h>
#include "bin_conv_para.h"
#include "bin_dense_para_0.h"
#include "bin_dense_para_1.h"
//#define PROFILE

#ifdef PROFILE
int mem_in_1 = 0;
int mem_in_2 = 0;
int mem_in_3 = 0;
int mem_in_4 = 0;
int mem_in_5 = 0;
int mem_in_6 = 0;
int mem_in_7 = 0;
int mem_out_1 = 0;
int mem_out_2 = 0;
int mem_out_3 = 0;
int mem_out_4 = 0;
int mem_out_5 = 0;
int mem_out_6 = 0;
int mem_out_7 = 0;
int fp_conv_in_1 = 0;
int fp_conv_out_1 = 0;
int fp_conv_out_2 = 0;
int bin_conv_in_1 = 0;
int bin_conv_in_2 = 0;
int bin_conv_out_1 = 0;
int bin_conv_out_2 = 0;
int bin_dense_in_1 = 0;
int bin_dense_in_2 = 0;
int bin_dense_in_3 = 0;
int bin_dense_out_1 = 0;
int bin_dense_out_2 = 0;
int bin_conv_wt_0_in_1 = 0;
int bin_conv_wt_0_out_1 = 0;
int bin_conv_wt_1_in_1 = 0;
int bin_conv_wt_1_out_1 = 0;
int bin_conv_wt_2_in_1 = 0;
int bin_conv_wt_2_out_1 = 0;
int bin_conv_wt_3_in_1 = 0;
int bin_conv_wt_3_out_1 = 0;
int bin_conv_wt_4_in_1 = 0;
int bin_conv_wt_4_out_1 = 0;
int bin_conv_wt_5_in_1 = 0;
int bin_conv_wt_5_out_1 = 0;
int bin_conv_wt_6_in_1 = 0;
int bin_conv_wt_6_out_1 = 0;
int bin_conv_wt_7_in_1 = 0;
int bin_conv_wt_7_out_1 = 0;
int bin_conv_wt_8_in_1 = 0;
int bin_conv_wt_8_out_1 = 0;
int bin_dense_wt_0_in_1 = 0;
int bin_dense_wt_0_out_1 = 0;
int bin_dense_wt_1_in_1 = 0;
int bin_dense_wt_1_out_1 = 0;
int bin_dense_wt_2_in_1 = 0;
int bin_dense_wt_2_out_1 = 0;
int bin_dense_wt_3_in_1 = 0;
int bin_dense_wt_3_out_1 = 0;
int bin_dense_wt_4_in_1 = 0;
int bin_dense_wt_4_out_1 = 0;
int bin_dense_wt_5_in_1 = 0;
int bin_dense_wt_5_out_1 = 0;
int bin_dense_wt_6_in_1 = 0;
int bin_dense_wt_6_out_1 = 0;
int bin_dense_wt_7_in_1 = 0;
int bin_dense_wt_7_out_1 = 0;
int bin_dense_wt_8_in_1 = 0;
int bin_dense_wt_8_out_1 = 0;
int bin_dense_wt_9_in_1 = 0;
int bin_dense_wt_9_out_1 = 0;
int bin_dense_wt_10_in_1 = 0;
int bin_dense_wt_10_out_1 = 0;
int bin_dense_wt_11_in_1 = 0;
int bin_dense_wt_11_out_1 = 0;
int bin_dense_wt_12_in_1 = 0;
int bin_dense_wt_12_out_1 = 0;
int bin_dense_wt_13_in_1 = 0;
int bin_dense_wt_13_out_1 = 0;
int bin_dense_wt_14_in_1 = 0;
int bin_dense_wt_14_out_1 = 0;
int bin_dense_wt_15_in_1 = 0;
int bin_dense_wt_15_out_1 = 0;
#endif





#include "accel.h"
typedef ap_uint<128> bit128;


static TwoBit encode_bit(const Bit& b) {
#pragma HLS INLINE
  return (b == 0) ? TwoBit(1) : TwoBit(-1);
}

// -----------------------------------------------------------------------
// Conv
// -----------------------------------------------------------------------
static ConvOut conv3x3b(
    const TwoBit line_buffer_m[CONV_BANKS][CONV_ROWS][CONV_COLS],
    const Bit conv_params_m[K][K],
    const ap_uint<4> bank,
    const IdxType cc
) {
#pragma HLS INLINE
  ConvOut sum = 0;
  for (ap_uint<2> kr = 0; kr < K; ++kr) {
    for (ap_uint<2> kc = 0; kc < K; ++kc) {
      TwoBit data = line_buffer_m[bank][kr][cc+kc];
      const Bit& wt = conv_params_m[2-kr][2-kc];
      data[1] = (wt & data[0]) ^ data[1];
      sum += data;
    }
  }
  return sum;
}

// -----------------------------------------------------------------------
// Produce 32 elements of conv results
// -----------------------------------------------------------------------
static void conv_word(
    const TwoBit line_buffer_m[CONV_BANKS][CONV_ROWS][CONV_COLS],
    const Bit conv_params_m[K][K],
    ConvOut conv_out_buffer_m[WORD_SIZE]
) {
#pragma HLS PIPELINE
  for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank) {
    for (ap_uint<4> cc = 0; cc < BANK_WIDTH; ++cc) {
      conv_out_buffer_m[bank*BANK_WIDTH+cc] = conv3x3b( line_buffer_m, conv_params_m, bank, cc );
    }
  }
}

// -----------------------------------------------------------------------
// Process each line in a word, we need to outline this loop to
// avoid false control dependencies in Vivado HLS
// -----------------------------------------------------------------------
static void process_word(
    const TwoBit  word_buffer_m[CONV_BANKS][CONV_COLS],
    const TwoBit  old_word_buffer_m[CONV_BANKS][CONV_COLS],
    const bool lb[CONV_BANKS],
    const bool rb[CONV_BANKS],
    TwoBit  line_buffer_m[CONV_BANKS][CONV_ROWS][CONV_COLS],
    const   Bit conv_params_m[K][K],
    ConvOut conv_out_buffer_m[WORD_SIZE],
    const   ap_uint<3> log_width,
    const   ap_uint<6> words_per_image,
    const   IdxType wrd
) {
#pragma HLS INLINE
  // slices_per_line = width / BANK_WIDTH
  const ap_uint<5> slices_per_line = 1 << (log_width - LOG_BANK_WIDTH);
  const bool first_wrd = (wrd == 0);
  const bool last_wrd = (wrd == words_per_image);

  // Prologue
  // Update bottom row, slices are shifted left. Some slices copied from previous word (middle row)
  for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank) {
    ap_int<6> s_idx = bank + slices_per_line - CONV_BANKS;
    if (s_idx < 0) {
      // set to zero or copy from old word (middle row)
      for (ap_uint<4> cc = 1; cc < CONV_COLS-1; ++cc) {
        line_buffer_m[bank][CONV_ROWS-1][cc] = old_word_buffer_m[CONV_BANKS+s_idx][cc];
      }
      line_buffer_m[bank][CONV_ROWS-1][0          ] = lb[bank] ? TwoBit(0) : old_word_buffer_m[CONV_BANKS+s_idx][0];
      line_buffer_m[bank][CONV_ROWS-1][CONV_COLS-1] = rb[bank] ? TwoBit(0) : old_word_buffer_m[CONV_BANKS+s_idx][CONV_COLS-1];
    } else {
      // fill from new word
      for (ap_uint<4> cc = 1; cc < CONV_COLS-1; ++cc) {
        line_buffer_m[bank][CONV_ROWS-1][cc] = (last_wrd) ? TwoBit(0) : word_buffer_m[s_idx][cc];
      }
      line_buffer_m[bank][CONV_ROWS-1][0          ] = (last_wrd || lb[bank]) ? TwoBit(0) : word_buffer_m[s_idx][0];
      line_buffer_m[bank][CONV_ROWS-1][CONV_COLS-1] = (last_wrd || rb[bank]) ? TwoBit(0) : word_buffer_m[s_idx][CONV_COLS-1];
    }
  }

  // Convolution
  conv_word( line_buffer_m, conv_params_m, conv_out_buffer_m );

  // Update
  // Fill line buffer with lines from the new word
  for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank) {
    // --------------------------------------------------------------
    // Top row, slices are shifted right by slices_per_line
    ap_int<6> s_idx0 = bank - slices_per_line;
    if (s_idx0 >= 0) {
      // slice from input word
      for (ap_uint<4> cc = 1; cc < CONV_COLS-1; ++cc) {
        line_buffer_m[bank][0][cc] = word_buffer_m[s_idx0][cc];
      }
      line_buffer_m[bank][0][0          ] = lb[bank] ? TwoBit(0) : word_buffer_m[s_idx0][0];
      line_buffer_m[bank][0][CONV_COLS-1] = rb[bank] ? TwoBit(0) : word_buffer_m[s_idx0][CONV_COLS-1];
    } else {
      // set to zero or copy from old word (middle row)
      for (ap_uint<4> cc = 1; cc < CONV_COLS-1; ++cc) {
        line_buffer_m[bank][0][cc] = (first_wrd) ? TwoBit(0) : old_word_buffer_m[CONV_BANKS+s_idx0][cc];
      }
      line_buffer_m[bank][0][0          ] = (first_wrd || lb[bank]) ? TwoBit(0) : old_word_buffer_m[CONV_BANKS+s_idx0][0];
      line_buffer_m[bank][0][CONV_COLS-1] = (first_wrd || rb[bank]) ? TwoBit(0) : old_word_buffer_m[CONV_BANKS+s_idx0][CONV_COLS-1];
    }

    // --------------------------------------------------------------
    // Middle row, simply copy the word into the line buffer
    for (ap_uint<4> cc = 1; cc < CONV_COLS-1; ++cc) {
      line_buffer_m[bank][1][cc] = word_buffer_m[bank][cc];
    }
    // Fill end buffer bits
    line_buffer_m[bank][1][0          ] = lb[bank] ? TwoBit(0) : word_buffer_m[bank][0];
    line_buffer_m[bank][1][CONV_COLS-1] = rb[bank] ? TwoBit(0) : word_buffer_m[bank][CONV_COLS-1];
  }
}

void bin_conv
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Input_2,
  hls::stream<bft_word> & Output_1

)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1

  const Address    n_inputs_table[]   = {128, 128, 256, 256, 512};
  const Address    n_outputs_table[]  = {128, 256, 256, 512, 512};
  const ap_uint<2> width_mode_table[] = {  2,   1,   1,   0,   0};
  const ap_uint<2> norm_mode_table[]  = {  2,   1,   2,   1,   2};

  // ---------------------------------------------------------------------
  // buffers
  // ---------------------------------------------------------------------
  Word dmem[128 * 32 * 32 / WORD_SIZE];
  static Word dmem_tmp[128 * 32 * 32 / WORD_SIZE];

  TwoBit  line_buffer[CONV_BANKS][CONV_ROWS][CONV_COLS];
#pragma HLS ARRAY_PARTITION variable=line_buffer complete dim=0
  Bit     conv_params[K][K];
#pragma HLS ARRAY_PARTITION variable=conv_params complete dim=0
  ConvSum fixed_buffer[MAX_IMG_SIZE / WORD_SIZE][WORD_SIZE];
#pragma HLS ARRAY_RESHAPE variable=fixed_buffer cyclic factor=32 dim=2
  TwoBit  word_buffer[CONV_BANKS][CONV_COLS];
#pragma HLS ARRAY_PARTITION variable=word_buffer complete dim=0
  TwoBit  old_word_buffer[CONV_BANKS][CONV_COLS];
//memset(old_word_buffer,0,sizeof(old_word_buffer));
#pragma HLS ARRAY_PARTITION variable=old_word_buffer complete dim=0
  ConvOut conv_out_buffer[WORD_SIZE];
#pragma HLS ARRAY_PARTITION variable=conv_out_buffer complete dim=0
  // edge padding flag bits
  bool lb[CONV_BANKS];
#pragma HLS ARRAY_PARTITION variable=lb complete dim=0
  bool rb[CONV_BANKS];
#pragma HLS ARRAY_PARTITION variable=rb complete dim=0

  static unsigned layer = 0;
  int dmem_addr = 0;
    const Address n_inputs = n_inputs_table[layer];
    const Address n_outputs = n_outputs_table[layer];
    const ap_uint<2> width_mode = width_mode_table[layer];
    const ap_uint<2> norm_mode = norm_mode_table[layer];

    const ap_uint<3> log_width = width_mode + LOG_BANK_WIDTH;
    const unsigned words_per_image = 1 << (2*width_mode);

    ap_uint<2> out_cnt = 0;
    Word outword;
    Word wt_word;
    ap_uint<3> wt_offset = 0;      // offset 0..6 of param


	for (unsigned i = 0; i < n_inputs * words_per_image; i++)
	{
#pragma HLS PIPELINE
	  if (layer == 0){
		  dmem[i](63, 32) = Input_1.read();
		  dmem[i](31, 0)  = Input_1.read();
	  }else{
		  dmem[i] = dmem_tmp[i];
	  }
	}

/*
    for(int i=0; i<128 * 32 * 32 / WORD_SIZE; i++)
    {
      printf("dmem[%d]=%x_%x\n", i, (unsigned int)dmem[i](63,32), (unsigned int)dmem[i](31,0));
    }
*/

    for (IdxType k = 0; k < n_outputs; k++)
    {
      // ---------------------------------------------------------------------
      // Calculate edge padding flag bits
      const ap_uint<4> log_slice = log_width - LOG_BANK_WIDTH;
      const ap_uint<4> w_div_8 = (1 << log_width) >> 3;
      ap_uint<4> mask = ~ap_uint<4>(0);   // set mask to all 1s
      mask = mask >> (4-log_slice);
      for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank) {
        #pragma HLS unroll
        const ap_uint<4> x = bank & mask;
        lb[bank] = (x == 0);          // (bank % w_div_8) == 0
        rb[bank] = (x+1 == w_div_8);  // (bank % w_div_8) == w_div_8-1
      }
  
      // ---------------------------------------------------------------------
      // Reset conv buffer
      for (IdxType i = 0; i < words_per_image; ++i) {
        for (IdxType j = 0; j < WORD_SIZE; ++j) {
          #pragma HLS UNROLL
          fixed_buffer[i][j] = 0;
        }
      }
  
      // wrd = which word in the current image
      // wrd_phase = which wrd in the current phase
      ap_uint<8> wrd = 0;
      ap_uint<16> wrd_phase = 0;
  
      // Load a word each iteration, and then process it
      // We load images_per_phase words per phase, however we also need 1 extra "empty"
      // iteration per image in the phase to do the loop epilogue, so the loop bound
      // is WORDS_PER_PHASE + images_per_phase
      LOOP_WORDS_IN_PHASE:
      for (ap_uint<16> count = 0; count < n_inputs * (words_per_image + 1); ++count) {
#pragma HLS DEPENDENCE variable=fixed_buffer inter false
#pragma HLS PIPELINE
        // First word of an image
        if (wrd == 0) {
          // -------------------------------------------------------------------
          // Load param word
          // Each word contains CONV_W_PER_WORD weight filters, after we use
          // them all we should load the next word
          // -------------------------------------------------------------------
          if (wt_offset == 0)
            wt_word(31, 0) = Input_2.read();
          else if (wt_offset == 1)
            wt_word(63, 32) = Input_2.read();
          Word wt_word_buffer = wt_word >> ap_uint<6>(WT_SIZE*wt_offset);
          if (wt_offset == CONV_W_PER_WORD-1) {
            wt_offset = 0;
          } else {
            ++wt_offset;
          }
  
          // -------------------------------------------------------------------
          // Load params
          // Each word contains CONV_W_PER_WORD weight filters packed into the first
          // 63 bits, the last bit is unused. Wts are stored in output-major order.
          // -------------------------------------------------------------------
          LOOP_LOAD_WTS:
          for (ap_uint<2> kr = 0; kr < K; ++kr) {
            for (ap_uint<2> kc = 0; kc < K; ++kc)
              conv_params[kr][kc] = wt_word_buffer[kr*K+kc];
          }
        }
  
        int i_ylxiao, j_ylxiao, k_ylxiao;
/*
for(i_ylxiao=0; i_ylxiao<8; i_ylxiao++){
  for(j_ylxiao=0; j_ylxiao<10; j_ylxiao++)
printf("Before: word_buffer[%d][%d] = %x, old_word_buffer[%d][%d] = %x,\n",
		 i_ylxiao, j_ylxiao,
		 (unsigned int)word_buffer[i_ylxiao][j_ylxiao],
		 i_ylxiao, j_ylxiao,
		 (unsigned int)old_word_buffer[i_ylxiao][j_ylxiao]);
}

for(int i=0; i<128 * 32 * 32 / WORD_SIZE; i++)
{
  printf("dmem[%d]=%x_%x\n", i, (unsigned int)dmem[i](63,32), (unsigned int)dmem[i](31,0));
}

*/
        // -------------------------------------------------------------------
        // Every word in an image
        // -------------------------------------------------------------------
        // Load word
        // (wrd_phase-wrd) is which wrd in the current phase, aligned to img boundary
        if (wrd != words_per_image) {
          Word word = dmem[wrd_phase];
          for (IdxType bank = 0; bank < CONV_BANKS; ++bank) {
            for (IdxType cc = 0; cc < CONV_COLS-2; ++cc) {
              word_buffer[bank][cc+1] = encode_bit(word[ap_uint<6>(bank*BANK_WIDTH+cc)]);
            }
            word_buffer[bank][0          ] = (bank==0)            ?
              TwoBit(0) : encode_bit(word[ap_uint<6>(bank*BANK_WIDTH-1)]);
            word_buffer[bank][CONV_COLS-1] = (bank==CONV_BANKS-1) ?
              TwoBit(0) : encode_bit(word[ap_uint<6>(bank*BANK_WIDTH+BANK_WIDTH)]);
          }
        }
  
        // Compute
        // Do the following for each word in an image
        process_word( word_buffer, old_word_buffer, lb, rb, line_buffer, conv_params,
            conv_out_buffer, log_width, words_per_image, wrd );

        /*
for(i_ylxiao=0; i_ylxiao<8; i_ylxiao++){
  for(j_ylxiao=0; j_ylxiao<10; j_ylxiao++)
printf("3: word_buffer[%d][%d] = %x, old_word_buffer[%d][%d] = %x,\n",
		 i_ylxiao, j_ylxiao,
		 (unsigned int)word_buffer[i_ylxiao][j_ylxiao],
		 i_ylxiao, j_ylxiao,
		 (unsigned int)old_word_buffer[i_ylxiao][j_ylxiao]);
}


for(i_ylxiao=0; i_ylxiao<8; i_ylxiao++){
  printf("3: lb[%d] = %x, rb[%d] = %x\n", i_ylxiao, (unsigned int)lb[i_ylxiao], i_ylxiao, (unsigned int)rb[i_ylxiao]);
}

for(i_ylxiao=0; i_ylxiao<8; i_ylxiao++){
  for(j_ylxiao=0; j_ylxiao<3; j_ylxiao++)
	for(int k_ylxiao=0; k_ylxiao<10; k_ylxiao++)
printf("3: conv_params[%d][%d][%d] = %x\n", i_ylxiao, j_ylxiao, k_ylxiao,
		 (unsigned int)line_buffer[i_ylxiao][j_ylxiao][k_ylxiao]);
}

for(i_ylxiao=0; i_ylxiao<3; i_ylxiao++){
  for(j_ylxiao=0; j_ylxiao<3; j_ylxiao++)
printf("3: conv_params[%d][%d] = %x\n", i_ylxiao, j_ylxiao,
		 (unsigned int)conv_params[i_ylxiao][j_ylxiao]);
}

for(i_ylxiao=0; i_ylxiao<64; i_ylxiao++){
  printf("3: conv_out_buffer[%d] = %x\n", i_ylxiao, (unsigned int)conv_out_buffer[i_ylxiao]);
}

printf("3: log_width = %x, words_per_image=%x, wrd=%x\n",
		(unsigned int)log_width,
		(unsigned int)words_per_image,
		(unsigned int)wrd);

*/

        for (IdxType bank = 0; bank < CONV_BANKS; ++bank) {
          for (IdxType cc = 0; cc < CONV_COLS; ++cc) {
            old_word_buffer[bank][cc] = word_buffer[bank][cc];
          }
        }
  
        // -------------------------------------------------------------------
        // Sum results across convolvers
        // -------------------------------------------------------------------
        for (IdxType i = 0; i < WORD_SIZE; ++i) {
          // Ignore conv results after processing the first word
          if (wrd > 0) {
            fixed_buffer[wrd-1][i] += conv_out_buffer[i];
          }
        }
  
        // -------------------------------------------------------------------
        // Increment counters
        // -------------------------------------------------------------------
        if (wrd != words_per_image) {
          wrd++;
          wrd_phase++;
        } else {
          wrd = 0;
        }
      } // wrd_phase = 0 .. WORDS_PER_PHASE
  
      NormComp nc = Input_2.read();

      const ap_uint<5> pool_width = 1 << (log_width-1);
  
      LOOP_BATCH_NORM: for (ap_uint<6> w = 0; w < words_per_image; ++w) {
        Word binword;
        const ap_uint<6> out_offset = (w % 4) << 4;
  
        for (ap_uint<7> i = 0; i < WORD_SIZE; ++i) {
#pragma HLS PIPELINE
          binword[i] = (fixed_buffer[w][i] >= nc) ? 0 : 1;

        }
  
        if (norm_mode == 1) {
          outword = binword;
        }
        else if (norm_mode == 2) {
          // horizontal pooling first
          ap_int<WORD_SIZE/2> poolword_h;
          for (ap_uint<6> i = 0; i < WORD_SIZE/2; ++i) {
#pragma HLS unroll
            poolword_h[i] = binword[2*i] & binword[2*i+1];
          }
  
          // vertical pooling
          ap_uint<WORD_SIZE / 4> poolword;
          for (ap_uint<6> i = 0; i < WORD_SIZE/4; ++i) {
#pragma HLS unroll
            // source indices
            ap_uint<5> i0 = i >> (log_width-1);
            i0 = (i0 << log_width) + i(log_width-2,0);
            ap_uint<5> i1 = i0 + pool_width;
            poolword[i] = poolword_h[i0] & poolword_h[i1];
          }
  
          outword >>= WORD_SIZE/4;
          outword(63,48) = poolword(15,0);
//printf("2: binword = %x_%x\n", (unsigned int)binword(63,32),(unsigned int)binword(31,0));

          out_cnt++;
        }
  
        if (out_cnt == 0)
        {
#ifdef CHECK_OUTPUT
          printf("bc: %li\n", outword.to_long());
#endif
          if(layer == 4){
        	  Output_1.write(outword(63, 32));
        	  Output_1.write(outword(31, 0));
          }else{
        	  dmem_tmp[dmem_addr] = outword;
        	  dmem_addr++;
          }

          //if((int)w<10){
              //printf("w=%d, outword = %x_%x\n", (int)w, (unsigned int)outword(63,32),(unsigned int)outword(31,0));
          //}
        }
      }
    }
   //printf("After layer = %d\n", layer);
   layer++;
   if(layer == 5){
	   layer = 0;
   }
}





#define MAX_INPUTS 8192


static Word read_kh(unsigned layer, Address addr)
{
  const long kh_mem_0[256] =
  {
    -133701125033230669,
    -110621607181156142,
    83316348370419990,
    -2533300527693832,
    257550136274386786,
    -35186102971400776,
    60801163384717483,
    22237176015159838,
    -76278932727922792,
    117935266458369858,
    -144115278255751562,
    162411486757912199,
    154810700274532251,
    -20828959274303724,
    14074577745018862,
    79938309273025475,
    -26177726912331900,
    -20265781749284503,
    90352136098086708,
    -1686775391453070,
    119063270890078128,
    -116532878021164656,
    197594909643767988,
    -14352105657139195,
    -288794915250241358,
    -20829697979055247,
    2812838501875538,
    -146366287799189565,
    -37999461151211731,
    10696427060920136,
    -44192091066072851,
    -72057220371972182,
    -145521708251676171,
    133137195866717893,
    -88946127014395927,
    2813860714118876,
    -36592026142703126,
    -199004161419968728,
    164942682761527994,
    -146365141050130827,
    -121035923597492536,
    -153966885286838420,
    55729932255036506,
    -15480410762772338,
    -98796308075511730,
    28990226097504005,
    -141861442650439214,
    42502884717887191,
    -363102423597973356,
    188025954440183392,
    -148055919345663597,
    -226305503296946494,
    -63051769192972229,
    111182972278538290,
    10976944287776092,
    -74026471481475346,
    -17170175419743760,
    -39687709728899240,
    39684484220649601,
    151715708240855276,
    -61359693809713464,
    -191402421499461789,
    -52635494239502202,
    -66427372848283279,
    -75716949102231331,
    77969921467547771,
    -191122213547343906,
    48697047850942498,
    36591811367075665,
    -152557126695125790,
    141299961580879734,
    -96828439992140030,
    -36872788173586686,
    -14918276857397598,
    -44473746398445809,
    -81066648681643874,
    -46163721520414103,
    -47849607879720098,
    64458615773135078,
    -43347618835660565,
    35469038228406269,
    -50947271444857172,
    13792845093994478,
    -208291916575604914,
    66426492440412008,
    19985208648466425,
    -88662972675457019,
    35463918611595307,
    11823838805163992,
    -79095696794910988,
    115403954483887971,
    13510094510620775,
    -51229652616348203,
    103020678963199874,
    -87537678345240821,
    88660825225232246,
    11540040228077666,
    -122163507152945735,
    3935865075793784,
    -14073259203493983,
    -30961388440584457,
    -24487850497147177,
    83038231969005440,
    13229757700767594,
    5910545016225981,
    -2533438000791404,
    -45881095524974695,
    -104425554242961150,
    154529916856238380,
    138485830259769226,
    -67835198775754899,
    135952568355978917,
    -54324481497562922,
    4221454635106214,
    4503642587398003,
    57702683760394335,
    -11542415384575404,
    23364351538299058,
    158750632715878212,
    -112306475601952480,
    138768155650949087,
    161284796572434367,
    -47008129260453699,
    21109433531302265,
    101894487026827292,
    25332189545037411,
    16042626250637547,
    -55732440542806110,
    123003611348468046,
    89791943527366988,
    34060392018280449,
    -12667129859538828,
    844455005388668,
    107240845191806063,
    -76561975349346487,
    11259497278800203,
    -41941571161686005,
    115685356418236363,
    -16044537521635510,
    844463583395555,
    -79657693268999967,
    16888494300724820,
    3939627479596982,
    145241375746228108,
    88100271859433530,
    16045173168603577,
    -119063034703379799,
    176767268926718051,
    79656323204448463,
    15763045381897873,
    70367206595035425,
    -72901031143932183,
    80503088897982848,
    -33774806760161282,
    -39963425184809449,
    44473454373502826,
    -39970335753175259,
    16608239094202039,
    -31242150451609498,
    79939498983358893,
    -56575340746834013,
    68118030995226655,
    -12947080140882437,
    36874149636276063,
    45316741115543822,
    95981133126173111,
    11539207025328004,
    66147345375297588,
    -56012154558087315,
    -39968205451230676,
    -40817041307664123,
    -83599661537165413,
    -52918528264044404,
    176765568129433583,
    26172319566987375,
    -69806554420740299,
    -16608934907674845,
    -121877101561446423,
    60799707342241803,
    -84725153392296053,
    -16326021097979669,
    -38843984882956784,
    230245762179334022,
    -4785474049212700,
    13511451739553624,
    46726388017397621,
    -165508450225815633,
    41376280430837797,
    56577831833305578,
    57421548083544292,
    85008578277081180,
    31526932560346893,
    7880457519366755,
    59673249084735231,
    343116506238877768,
    -167761362460147926,
    91761529599492324,
    -566012252782909,
    12947801580895811,
    50101725541171326,
    -84161267174211610,
    46726744521572595,
    27304601694961619,
    41657115445690091,
    13790169338872137,
    6192428017516602,
    47568575306858137,
    45881417646145399,
    31807303715717065,
    -122725731265740762,
    40534758893879660,
    -61641585378460005,
    -36872448780533679,
    -14070965712125808,
    -103301410942288392,
    82755236570005635,
    47288444635381691,
    167197729615380443,
    -143836882801459204,
    -111182151923662568,
    -1690503407468832,
    76279336437874806,
    52631521406681119,
    -56294634569989705,
    -21675046375391279,
    -23362891187813769,
    -15197651581337546,
    -99079582655054060,
    112589556863401745,
    1125968637395249,
    47001996049252519,
    -77968740367859872,
    -19983280254222384,
    49821315668836091,
    46441786308820598,
    54043229889101782,
    -45878883619766299,
    -38282250368909250,
    -19703042227633998,
    -127227252119765658,
    -97671258588577803,
    -6472008933114043,
    -65582939421344070,
    -37998198405660838,
    56855973859688779,
    -1688068181394021,
    -14356954718142739,
    -3941689083494640,
    -61924902907608730,
    -75433910784884541,
    -69806597336399701,
    -54605428192509751,
    -19139336359837778,
    51793667765239768,
    -6755708688007350,
    114278346655465424
  };

  const long kh_mem_1[256] =
  {
    -844412045754139,
    -1689021662101584,
    19702788802609296,
    20828813288275907,
    5628962660286312,
    -4784993005469539,
    -1970500927881398,
    -37717702711574542,
    13792230909870013,
    1970178805596254,
    50383973586698223,
    4222562729525276,
    63050446324236244,
    31525425024335807,
    -45317501313679328,
    -21110554528251841,
    -11821678441660312,
    -468142194660,
    -18858728950595578,
    -6473791322259450,
    -4504162269724673,
    -5629705689956326,
    36873449578102889,
    2533390753792107,
    6473744072769587,
    39407007838240676,
    562507570741375,
    -3377489257299887,
    -17452414933205083,
    -18295684507631657,
    19421528582258723,
    -22799241189195770,
    34621757151838263,
    -21954936512446651,
    -7036981793259326,
    -11259080682307511,
    3377961710387284,
    22799443060129854,
    80783374154072010,
    7037059107520653,
    3377897288826919,
    -24770248924004415,
    -2533167418048275,
    17170519039016866,
    -1689189167071254,
    -27303317553283053,
    -6191908326998133,
    -14355507278315537,
    -7599536605691871,
    -84442273972551716,
    25614596539088982,
    -5629319141654481,
    -4784275742261141,
    11259089259855882,
    8163397097488522,
    40813725594353709,
    -15200052475854863,
    17733335849107373,
    -8725608315879562,
    -10132085551792126,
    -3940177226301486,
    -70649931393990749,
    -18859240056750103,
    -563482528972769,
    28710155563958446,
    -20829238471753707,
    -3378442762125204,
    14355649013022660,
    73183631377956894,
    -281281706131493,
    -51510229974843522,
    -30399065561301001,
    13229006084046836,
    23080406925508616,
    -2251924362952678,
    3096194683043824,
    10696195146317883,
    16888219430354842,
    4504909590364112,
    8725200292347881,
    27302900935491590,
    18295585715454063,
    17732687313108938,
    6755214751432716,
    13511267032367240,
    -281736976072697,
    14354858724556835,
    -1407675529297903,
    -12103097584582654,
    15761855668551657,
    15480668452159514,
    19421747619168212,
    -3378103445815227,
    -844214475620324,
    11541539206070253,
    -2814535021559921,
    -16044193928904958,
    21673513080324333,
    54043457516077146,
    -37716504414715912,
    55169361722605580,
    6191530367057983,
    18295628678037321,
    27021632119963557,
    28428955469086706,
    28992261900730431,
    23362457422331834,
    43628307854786522,
    36029170683543724,
    -844038378815810,
    -20265614206566534,
    1970457973161957,
    67834606099758918,
    -8444270773665744,
    3095979934482475,
    31244005875580875,
    -562932771389492,
    -3096263404421080,
    26740243048562612,
    76279873308000161,
    59391928767676373,
    34058493643391000,
    5629675628199939,
    -22236179564920839,
    5347680957431781,
    -68714037237,
    28428757903278151,
    -2251670981771268,
    27585136124362774,
    10976682274062305,
    42784303832629456,
    -22800332110561317,
    -11259067786854390,
    -24206749224271867,
    18577241082167334,
    12666086193496046,
    33496175065104185,
    15199618681929761,
    -38562174883528712,
    8163173753290794,
    -563156112244749,
    -17451577403178884,
    -29555710077173777,
    -1407903163219951,
    10132300302385296,
    19984521487712242,
    -38562170593411195,
    45597932616941544,
    9288755834650448,
    5066480865378219,
    -34058214478446540,
    -18858746123190181,
    -3660136774500335,
    36029020370239563,
    19703441626169312,
    -17169767422164949,
    -29554752299794310,
    -10695890199248892,
    17451366948798345,
    11541487660368053,
    -12103402519789559,
    44473235299368947,
    -22236566107324435,
    -43065456702324564,
    -280822144565256,
    -7037278144233408,
    -281208696340358,
    -844326143131653,
    -26740779917705162,
    43628690111725580,
    -5347745382596640,
    -5067176637497299,
    -3659449568460810,
    -844270304296858,
    5066609715183489,
    -14918315501223932,
    -7599875909353673,
    18577442952052711,
    21391943614070697,
    -12666077597073380,
    24488924281634545,
    -13510700100878319,
    4785684498940005,
    10977605705727967,
    -16607036494381321,
    -12385057883422905,
    -16606624199737205,
    36310521102073924,
    -29554236900310935,
    30118642842861599,
    -6192367884697550,
    5629641274753037,
    -19422052561584242,
    7318688699973664,
    -10415484663365624,
    3377669655494554,
    12948007833436099,
    -562634358761,
    12385448731344887,
    19703931269283887,
    -281195789025275,
    20828894877777954,
    -36873763116941369,
    -28991780868390820,
    25613681720492194,
    30962505151938533,
    -16325565829480478,
    -22236660598570770,
    57420959672958915,
    -844824361435191,
    15199678809309238,
    -281097024569340,
    19984809246851071,
    32651174603915080,
    3940271717482486,
    -5911468435636372,
    -562739498909596,
    -39969360796712888,
    -14355429974212548,
    -77686900297826258,
    11540491228610523,
    16606856120303622,
    7881076010844091,
    47005900202770433,
    4221922771861623,
    -10977541268897826,
    9570256583262239,
    -16043897582911537,
    -11259183746711517,
    3659080219033396,
    -16888644626677703,
    16325965263470647,
    -14355292532768784,
    39406741547450437,
    6192286264786921,
    -281337546670074,
    9570106258227197,
    31524699172962315,
    19421897956524074,
    -10977953590869885,
    28991836689924014,
    -61361549216776176,
    2532931197009708,
    -9007478426304542,
    -563598490402786,
    281977485787055,
    -53199109917376540,
    -9288502430204128,
    -12103235013771216,
    12665965940375584,
    32932628106772429,
    -8444236399837128,
    22517547159781415,
    8163165170434130,
    -7036255943196684,
    -3941225203498952
  };

  const long kh_mem_2[5] =
  {
    -2571554369758691121,
    -2303027400855650103,
    -2982226485528428334,
    -2220273659064876838,
    -1396959416625266487
  };

  switch(layer)
  {
    case 0:
      return kh_mem_0[addr];
    case 1:
      return kh_mem_1[addr];
    default:
      return kh_mem_2[addr];
  };
}

void bin_dense
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Input_2,
  hls::stream<bft_word> & Input_3,
  hls::stream< bit128 > & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Input_3
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2

  int input_2_cnt = 0;
  int output_1_cnt = 0;
  int output_2_cnt = 0;
  const Word m1("0x5555555555555555", 16);
  const Word m2("0x3333333333333333", 16);
  const Word m4("0x0f0f0f0f0f0f0f0f", 16);
  const Word h01("0x0101010101010101", 16);

  const Address n_inputs_table[]  = {8192, 1024, 1024};
  const Address n_outputs_table[] = {1024, 1024,   10};

  Word dmem[MAX_INPUTS / WORD_SIZE];
  static Word dmem_tmp[MAX_INPUTS / WORD_SIZE];

  static unsigned layer = 0;

    const Address n_inputs = n_inputs_table[layer];
    const Address n_outputs = n_outputs_table[layer];


	unsigned int i = 0;
	for (i = 0; i < n_inputs / WORD_SIZE; i++)
	{
#pragma HLS PIPELINE
	  if(layer == 0){
	    dmem[i](63, 32) = Input_1.read();
	    dmem[i](31, 0)  = Input_1.read();
	  }else{
		dmem[i] = dmem_tmp[i];
	  }
#ifdef PROFILE
  bin_dense_in_1+=2;
#endif

	}


#ifdef YLXIAO
    printf("Ben_dense should get %d data for Input_1\n", i);
#endif
    // for last layer
    DenseNorm best_out = -1024;
    ap_int<8> prediction = -1;
    Address kh_addr = 0;

    // read words from dmem and the wt store, dot them
    // o is the output bit, i is the input bit
    LOOP_DENSE_O:
    for (Address o = 0; o < (n_outputs + WORD_SIZE - 1) / WORD_SIZE; ++o) {
      Word o_word;
      for (Address w = 0; w < (n_outputs < WORD_SIZE ? n_outputs : (Address) WORD_SIZE); ++w) {

        DenseSum sum = 0;

        LOOP_DENSE_I:
        for (Address i = 0; i < n_inputs; i+=WORD_SIZE) {
#pragma HLS PIPELINE
          const Word in_wrd = dmem[i/WORD_SIZE];
          const Word wt_wrd = (Input_2.read(), Input_3.read());
#ifdef PROFILE
  bin_dense_in_2++;
  bin_dense_in_3++;
#endif
          input_2_cnt++;

          Word x = wt_wrd ^ in_wrd;

          // count_set bit for 64 bits, returns 2*cnt
          x -= (x >> 1) & m1;
          x = (x & m2) + ((x >> 2) & m2);
          x = (x + (x >> 4)) & m4;
          x += x >> 8;
          x += x >> 16;
          x += x >> 32;
          x = x & 0x7f;

          sum += WORD_SIZE - (DenseSum)(x<<1);
        } // n_inputs

        // not last layer -> biniarize,
        // otherwise just store the value as a 64bit word
        if (layer < 2) {
          Word kh_word = read_kh(layer, kh_addr / KH_PER_WORD);

          NormComp nc;
          IdxType kh_off = kh_addr % KH_PER_WORD;
          if (kh_off == 0)
            nc(15,0) = kh_word(15, 0);
          else if (kh_off == 1)
            nc(15,0) = kh_word(31,16);
          else if (kh_off == 2)
            nc(15,0) = kh_word(47,32);
          else
            nc(15,0) = kh_word(63,48);
          kh_addr++;

          o_word[w] = (sum >= nc) ? 0 : 1;
        } else {
          Word kh_word = read_kh(layer, kh_addr / 2);

          KType ki;  HType hi;
          IdxType kh_off = kh_addr % 2;
          if (kh_off == 0) {
            ki(15,0) = kh_word(15, 0);
            hi(15,0) = kh_word(31,16);
          } else {
            ki(15,0) = kh_word(47,32);
            hi(15,0) = kh_word(63,48);
          }
          kh_addr++;

          //printf (" >> %d * %f + %f\n", sum.to_int(), ki.to_float(), hi.to_float());
          ap_fixed<20,10> out = ap_fixed<20,10>(sum)*ki + hi;

          if (w == 0 || out > best_out) {
            prediction = w;
            best_out = out;
          }
        }
      }

      if (layer < 2) {
#ifdef CHECK_OUTPUT
        printf("bd: %li\n", o_word.to_long());
#endif
        dmem_tmp[o] = o_word;
#ifdef PROFILE
  bin_dense_out_1++;
  bin_dense_out_2++;
#endif
        output_1_cnt++;
        output_2_cnt++;
      }
    } // n_outputs

    //printf("bin_dense: layer = %d\n", layer);
    // Here we are using o_index as a bit index, not a word index!
    if (layer == 2) {
      Word o_word;
      o_word(7,0) = prediction(7,0);
      o_word(WORD_SIZE-1, 8) = 0;
#ifdef CHECK_OUTPUT
      printf("bdl: %li\n", o_word.to_long());
#endif
      Output_1.write(32);
      Output_1.write(o_word(31, 0));
      for(int out_i=0; out_i<30; out_i++)
      {
    	  Output_1.write(32);
      }
      output_1_cnt++;
#ifdef PROFILE
  bin_dense_out_1++;
#endif
    }

    layer++;
    if(layer == 3){
    	layer = 0;
    }
#ifdef YLXIAO
    printf("Ben_dense should get %d data from Input 2\n", input_2_cnt);
    printf("Ben_dense should get %d data from Output 1\n", output_1_cnt);
    printf("Ben_dense should get %d data from Output 2\n", output_2_cnt);
#endif
}


// -----------------------------------------------------------------------
// Module to do the first conv layer
// -----------------------------------------------------------------------
void fp_conv
(
  hls::stream< bit128  > & Input_1,
  hls::stream<bft_word> & Output_1,
  hls::stream<bft_word> & Output_2
  )
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2

  const unsigned M = 3;
  const unsigned S = 32;
  const unsigned N = 128;
  const unsigned OUTWORDS = 16; // words per output image

#include "fp_conv_wt.h"
#pragma HLS ARRAY_PARTITION variable=wt_mem complete dim=1

  C1InputType win[M][K][K];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0
  C1InputType lbuf[M][K-1][S];
//#pragma HLS ARRAY_PARTITION variable=lbuf complete dim=0
  Word outwords[OUTWORDS];
#pragma HLS ARRAY_PARTITION variable=outwords complete dim=0
  WtType wtbuf[M];
#pragma HLS ARRAY_PARTITION variable=wtbuf complete dim=0

  Word dmem[S * S];
  for (unsigned i = 0; i < S * S / 2; i++)
  {
#pragma HLS PIPELINE
	bit128 Input_tmp = Input_1.read();
    dmem[2*i](63, 0)   = Input_tmp(63,   0);
    dmem[2*i+1](63, 0) = Input_tmp(127, 64);
    if(i==2)
    {
    	Output_2.write(1);
    }
#ifdef PROFILE
  fp_conv_in_1++;
#endif
  }




  // Parallelized across m, better for HLS
  LOOP_FP_CONV_O:
  for (IdxType n = 0; n < N; ++n) {

    // The weights for the 1st conv layer are just laid out
    // linearly across wt_mem, 3 weights per 64-bit word
    Word wt_word = wt_mem[0][n];
    LOOP_LOAD_WTS:
    for (ap_uint<2> m = 0; m < M; ++m) {
#pragma HLS UNROLL
      wtbuf[m] = wt_word((m+1)*WT_SIZE-1, m*WT_SIZE);
    }

    // load batch norm params
    C1Comp nc;
    load_kh(nc, kh_mem, n);
    //printf ("  n=%3d, nc=%6.3f\n", n.to_int(), nc.to_float());

    // begin convolution
    LOOP_CONV_ROWS: for (IdxType r = 0; r < S+1; ++r) {
      LOOP_CONV_COLS: for (IdxType c = 0; c < S+1; ++c) {
#pragma HLS PIPELINE
        // load input word
        Word inword = 0;
        if (r < S && c < S) {
          inword = dmem[r * S + c];
        }

        C1InputType pix[M];
        for (ap_uint<2> m = 0; m < M; ++m) {
          // load data: the value of pix is either the pixel at [r,c]
          // 0 -> +1, -1 -> -1
          // or -> 0 for padding around the boundaries

          const unsigned W = pix[m].length();
          pix[m](W-1,0) = inword(W-1+m*W, m*W);

          // window: shift left, leaving rightmost col for new data
          for (IdxType wr = 0; wr < K; ++wr) {
            for (IdxType wc = 0; wc < K-1; ++wc) {
              win[m][wr][wc] = win[m][wr][wc+1];
          } }

          // window: fill top K-1 pixels of rightmost column from lbuf
          for (IdxType wr = 0; wr < K-1; ++wr) {
//            C1InputType val = (c != S) ? lbuf[m][wr][c] : C1InputType(0);
            C1InputType val = (c != S && r + wr >= K - 1) ? lbuf[m][wr][c] : C1InputType(0);
            win[m][wr][K-1] = val;
          }

          // window: fill bottom right with new input pixel
          win[m][K-1][K-1] = pix[m];

          // lbuf: shift up column c
          if (c != S) {
            for (IdxType lr = 0; lr < K-2; ++lr) {
              lbuf[m][lr][c] = lbuf[m][lr+1][c];
            }
            lbuf[m][K-2][c] = pix[m];
          }
        } // m

        // only perform the conv and store if legal position
        if (r > 0 && c > 0) {
          C1ConvType res = 0;
          for (ap_uint<2> m = 0; m < M; ++m) {
            for (ap_uint<2> wr = 0; wr < K; ++wr) {
              for (ap_uint<2> wc = 0; wc < K; ++wc) {
                const C1InputType& pix = win[m][wr][wc];
                const Bit& b = wtbuf[m][8-(wr*K+wc)];
                res += (b==0) ? pix : (C1InputType)(-pix);
            } }
          }

          // perform normalization right here
          outwords[(r-1)/2][((r-1)%2)*S + (c-1)] =
            (res >= nc) ? Bit(0) : Bit(-1);
        }

      } // CONV_COLS
    } // CONV_ROWS

    // Here i is the word offset within the outwords buffer
    LOOP_OUTPUT:
    for (IdxType i = 0; i < OUTWORDS; ++i) {
#pragma HLS PIPELINE
#ifdef CHECK_OUTPUT
      printf("fc: %li\n", outwords[i].to_long());
#endif
      Output_1.write(outwords[i](63, 32));
      Output_1.write(outwords[i](31, 0));
#ifdef PROFILE
  fp_conv_out_1++;
  fp_conv_out_2++;
#endif
    }
  } // n
}

#define INPUT_IMG_SIZE 32 * 32


void mem
(
  hls::stream<bft_word> & Input_6, // bin_dense
  hls::stream<bft_word> & Input_7, // bin_dense
  hls::stream< bit128 > & Output_4, // Main output
  hls::stream<bft_word> & Output_5, //
  hls::stream<bft_word> & Output_6, //
  hls::stream<bft_word> & Output_7 //
)
{
#pragma HLS INTERFACE ap_hs port=Input_6
#pragma HLS INTERFACE ap_hs port=Input_7
#pragma HLS INTERFACE ap_hs port=Output_4
#pragma HLS INTERFACE ap_hs port=Output_5
#pragma HLS INTERFACE ap_hs port=Output_6
#pragma HLS INTERFACE ap_hs port=Output_7

Output_5.write(1);
Output_6.write(1);
Output_7.write(1);


}

#define IMAGE_SIZE (32 * 32)


void data_gen
(
  hls::stream< bit128 > & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Output_1
	int i;
#include "input_data.h"
	for (int i = 0; i < IMAGE_SIZE/2; i++) {
	    Word w1 = input_data[2*i];
	    Word w2 = input_data[2*i+1];
	    bit128 Input_tmp;
	    Input_tmp(63,   0) = w1;
	    Input_tmp(127, 64) = w2;
	    Output_1.write(Input_tmp);
	  }
}

void data_gen256
(
  hls::stream< bit32 > & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Output_1
	int i;
	for (int i = 0; i < 256; i++) {
	    Output_1.write(i+1);
	  }
}

void data_source_dyn
(
  int num,
  hls::stream< bit32 > & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Output_1
	int i;
	for (int i = 0; i < num; i++) {
	    Output_1.write(i+1);
	  }
}

void data_sink_dyn
(
  int num,
  hls::stream< bit32 > & Input_1
)
{
#pragma HLS INTERFACE ap_hs port=Output_1
	int i;
	for (int i = 0; i < num; i++) {
	    Input_1.read();
	  }
}


void print_profile()
{
	#ifdef PROFILE
	printf("mem_in_1=%d\n", mem_in_1);
	printf("mem_in_2=%d\n", mem_in_2);
	printf("mem_in_3=%d\n", mem_in_3);
	printf("mem_in_4=%d\n", mem_in_4);
	printf("mem_in_5=%d\n", mem_in_5);
	printf("mem_in_6=%d\n", mem_in_6);
	printf("mem_in_7=%d\n", mem_in_7);
	printf("mem_out_1=%d\n", mem_out_1);
	printf("mem_out_2=%d\n", mem_out_2);
	printf("mem_out_3=%d\n", mem_out_3);
	printf("mem_out_4=%d\n", mem_out_4);
	printf("mem_out_5=%d\n", mem_out_5);
	printf("mem_out_6=%d\n", mem_out_6);
	printf("mem_out_7=%d\n", mem_out_7);
	printf("fp_conv_in_1=%d\n", fp_conv_in_1);
	printf("fp_conv_out_1=%d\n", fp_conv_out_1);
	printf("fp_conv_out_2=%d\n", fp_conv_out_2);
	printf("bin_conv_in_1=%d\n", bin_conv_in_1);
	printf("bin_conv_in_2=%d\n", bin_conv_in_2);
	printf("bin_conv_out_1=%d\n", bin_conv_out_1);
	printf("bin_conv_out_2=%d\n", bin_conv_out_2);
	printf("bin_dense_in_1=%d\n", bin_dense_in_1);
	printf("bin_dense_in_2=%d\n", bin_dense_in_2);
	printf("bin_dense_in_3=%d\n", bin_dense_in_3);
	printf("bin_dense_out_1=%d\n", bin_dense_out_1);
	printf("bin_dense_out_2=%d\n", bin_dense_out_2);
	printf("bin_conv_wt_0_in_1=%d\n", bin_conv_wt_0_in_1);
	printf("bin_conv_wt_0_out_1=%d\n", bin_conv_wt_0_out_1);
	printf("bin_conv_wt_1_in_1=%d\n", bin_conv_wt_1_in_1);
	printf("bin_conv_wt_1_out_1=%d\n", bin_conv_wt_1_out_1);
	printf("bin_conv_wt_2_in_1=%d\n", bin_conv_wt_2_in_1);
	printf("bin_conv_wt_2_out_1=%d\n", bin_conv_wt_2_out_1);
	printf("bin_conv_wt_3_in_1=%d\n", bin_conv_wt_3_in_1);
	printf("bin_conv_wt_3_out_1=%d\n", bin_conv_wt_3_out_1);
	printf("bin_conv_wt_4_in_1=%d\n", bin_conv_wt_4_in_1);
	printf("bin_conv_wt_4_out_1=%d\n", bin_conv_wt_4_out_1);
	printf("bin_conv_wt_5_in_1=%d\n", bin_conv_wt_5_in_1);
	printf("bin_conv_wt_5_out_1=%d\n", bin_conv_wt_5_out_1);
	printf("bin_conv_wt_6_in_1=%d\n", bin_conv_wt_6_in_1);
	printf("bin_conv_wt_6_out_1=%d\n", bin_conv_wt_6_out_1);
	printf("bin_conv_wt_7_in_1=%d\n", bin_conv_wt_7_in_1);
	printf("bin_conv_wt_7_out_1=%d\n", bin_conv_wt_7_out_1);
	printf("bin_conv_wt_8_in_1=%d\n", bin_conv_wt_8_in_1);
	printf("bin_conv_wt_8_out_1=%d\n", bin_conv_wt_8_out_1);
	printf("bin_dense_wt_0_in_1=%d\n", bin_dense_wt_0_in_1);
	printf("bin_dense_wt_0_out_1=%d\n", bin_dense_wt_0_out_1);
	printf("bin_dense_wt_1_in_1=%d\n", bin_dense_wt_1_in_1);
	printf("bin_dense_wt_1_out_1=%d\n", bin_dense_wt_1_out_1);
	printf("bin_dense_wt_2_in_1=%d\n", bin_dense_wt_2_in_1);
	printf("bin_dense_wt_2_out_1=%d\n", bin_dense_wt_2_out_1);
	printf("bin_dense_wt_3_in_1=%d\n", bin_dense_wt_3_in_1);
	printf("bin_dense_wt_3_out_1=%d\n", bin_dense_wt_3_out_1);
	printf("bin_dense_wt_4_in_1=%d\n", bin_dense_wt_4_in_1);
	printf("bin_dense_wt_4_out_1=%d\n", bin_dense_wt_4_out_1);
	printf("bin_dense_wt_5_in_1=%d\n", bin_dense_wt_5_in_1);
	printf("bin_dense_wt_5_out_1=%d\n", bin_dense_wt_5_out_1);
	printf("bin_dense_wt_6_in_1=%d\n", bin_dense_wt_6_in_1);
	printf("bin_dense_wt_6_out_1=%d\n", bin_dense_wt_6_out_1);
	printf("bin_dense_wt_7_in_1=%d\n", bin_dense_wt_7_in_1);
	printf("bin_dense_wt_7_out_1=%d\n", bin_dense_wt_7_out_1);
	printf("bin_dense_wt_8_in_1=%d\n", bin_dense_wt_8_in_1);
	printf("bin_dense_wt_8_out_1=%d\n", bin_dense_wt_8_out_1);
	printf("bin_dense_wt_9_in_1=%d\n", bin_dense_wt_9_in_1);
	printf("bin_dense_wt_9_out_1=%d\n", bin_dense_wt_9_out_1);
	printf("bin_dense_wt_10_in_1=%d\n", bin_dense_wt_10_in_1);
	printf("bin_dense_wt_10_out_1=%d\n", bin_dense_wt_10_out_1);
	printf("bin_dense_wt_11_in_1=%d\n", bin_dense_wt_11_in_1);
	printf("bin_dense_wt_11_out_1=%d\n", bin_dense_wt_11_out_1);
	printf("bin_dense_wt_12_in_1=%d\n", bin_dense_wt_12_in_1);
	printf("bin_dense_wt_12_out_1=%d\n", bin_dense_wt_12_out_1);
	printf("bin_dense_wt_13_in_1=%d\n", bin_dense_wt_13_in_1);
	printf("bin_dense_wt_13_out_1=%d\n", bin_dense_wt_13_out_1);
	printf("bin_dense_wt_14_in_1=%d\n", bin_dense_wt_14_in_1);
	printf("bin_dense_wt_14_out_1=%d\n", bin_dense_wt_14_out_1);
	printf("bin_dense_wt_15_in_1=%d\n", bin_dense_wt_15_in_1);
	printf("bin_dense_wt_15_out_1=%d\n", bin_dense_wt_15_out_1);
	#endif
}

/*
void bin_conv_wt
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

#include "bin_conv_para.h"

  if (Input_1.read() == 1)
  {
    for (Address i = 0; i < 146784; i++)
    {
      #pragma HLS pipeline
      Output_1.write(bin_conv_par[i]);
    }
  }
}

void bin_dense_wt_top
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

#include "bin_dense_para_0.h"

  if (Input_1.read() == 1)
  {
    for (Address i = 0; i < 147616; i++)
    {
      #pragma HLS pipeline
      Output_1.write(bin_dense_par_0[i]);
    }
  }
}

void bin_dense_wt_bot
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

#include "bin_dense_para_1.h"

  if (Input_1.read() == 1)
  {
    for (Address i = 0; i < 147616; i++)
    {
      #pragma HLS pipeline
      Output_1.write(bin_dense_par_1[i]);
    }
  }
}

*/


#define RAM36_36 36864
#define RAM36_30 30720
#define RAM36_44 45056
#define RAM36_40 40960
#define RAM36_20 20480
#define RAM36_32 32768
#define RAM36_23 23552
#define RAM36_22 22528
#define RAM36_21 21504
#define RAM36_11 11264
#define RAM36_10 10240
#define RAM36_16 16384
#define RAM36_8  8192
#define RAM36_6  6144
#define RAM36_4  4096
#define RAM36_2  2048
#define RAM36_1  1024

#include "wt_48.h"
#include "wt_24.h"
#include "wt_16.h"
#include "wt_8.h"
#include "wt_7.h"
#include "wt_4.h"
#include "wt_3.h"
#include "wt_2.h"
#include "wt_1.h"

//RAM36_10
void wt_36_0_start
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1,
  hls::stream<bft_word> & Output_2
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2

  if (Input_1.read() == 1)
  {
	Output_2.write(1);
    for (Address i = 0; i < RAM36_32; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_0_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_0_1[i]);
    }

  }
}


//RAM36_11
void wt_36_1
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

    for (Address i = 0; i < RAM36_36; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_32; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_1_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_1_1[i]);
    }
}

//RAM36_11
void wt_36_2
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

    for (Address i = 0; i < RAM36_36*2; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_32; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_2_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_2_1[i]);
    }

}

//RAM36_11
void wt_36_3
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

    for (Address i = 0; i < RAM36_36*3; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }


    for (Address i = 0; i < RAM36_32; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_3_0[i]);
    }


    for (Address i = 0; i < 3424; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_conv_par_0_3_1[i]);
    }


}


//////////////////////////////////////////////////////////////////////////////

void wt_32_4_start
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1,
  hls::stream<bft_word> & Output_2

)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2

  if(Input_1.read()==1)
  {
	Output_2.write(1);

    for (Address i = 0; i < RAM36_32; i++)
    {
#pragma HLS pipeline
      Output_1.write(bin_dense_par_0_0_0[i]);
    }

  }
}


void wt_32_5
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

    for (Address i = 0; i < RAM36_32; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }


    for (Address i = 0; i < RAM36_32; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_0_1_0[i]);
    }

}


void wt_36_6
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

    for (Address i = 0; i < RAM36_32*2; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_32; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_0_2_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_0_2_1[i]);
    }

}


void wt_36_7
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

    for (Address i = 0; i < RAM36_32*2+RAM36_36; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_32; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_0_3_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_0_3_0[i]);
    }

}


void wt_10_8
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

    for (Address i = 0; i < RAM36_32*2+RAM36_36*2; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_8; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_0_4_0[i]);
    }

    for (Address i = 0; i < 160; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_0_4_1[i]);
    }

}
///////////////////////////////////////////////////
void wt_20_9_start
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

	if(Input_1.read()==1)
	{

		for (Address i = 0; i < RAM36_16; i++)
		{
	  #pragma HLS pipeline
		  Output_1.write(bin_dense_par_1_0_0[i]);
		}

		for (Address i = 0; i < RAM36_4; i++)
		{
	  #pragma HLS pipeline
		  Output_1.write(bin_dense_par_1_0_1[i]);
		}

	}

}



void wt_20_10
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1


    for (Address i = 0; i < RAM36_20; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_16; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_1_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_1_1[i]);
    }




}



void wt_20_11
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1


    for (Address i = 0; i < RAM36_20*2; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }


    for (Address i = 0; i < RAM36_16; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_2_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_2_1[i]);
    }


}



void wt_36_12
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1


    for (Address i = 0; i < RAM36_20*3; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_32; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_3_0[i]);
    }

    for (Address i = 0; i < RAM36_4; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_3_1[i]);
    }


}



void wt_36_13
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1


    for (Address i = 0; i < RAM36_20*3+RAM36_36; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < RAM36_32; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_4_0[i]);
    }


    for (Address i = 0; i < RAM36_4; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_4_1[i]);
    }

}



void wt_12_14
(
  hls::stream<bft_word> & Input_1,
  hls::stream<bft_word> & Output_1
)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1


    for (Address i = 0; i < RAM36_20*3+RAM36_36*2; i++)
    {
#pragma HLS pipeline
      Output_1.write(Input_1.read());
    }

    for (Address i = 0; i < 12448; i++)
    {
  #pragma HLS pipeline
      Output_1.write(bin_dense_par_1_5_0[i]);
    }


}





