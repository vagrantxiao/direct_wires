/*===============================================================*/
/*                                                               */
/*                          sgd.cpp                              */
/*                                                               */
/*             Hardware function for spam filtering.             */
/*                                                               */
/*===============================================================*/

#include "sgd.h"
#include "lut.h"
#include <hls_stream.h>

// wrapper for the streamin part
// prevent incorrect flattening


// Function to compute the dot product of data (feature) vector and parameter vector
FeatureType dotProduct(FeatureType param[NUM_FEATURES],
                       DataType    feature[NUM_FEATURES])
{
  #pragma HLS INLINE
  const int unroll_factor = PAR_FACTOR;
  
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor

  FeatureType result = 0;
  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR; i++)
  {
    #pragma HLS PIPELINE II=1
    DOT_INNER: for(int j = 0; j < PAR_FACTOR; j++) 
    {
      FeatureType term = param[i*PAR_FACTOR+j] * feature[i*PAR_FACTOR+j];
      result += term;
    }
  }
  return result;
}

// values of sigmoid function stored in a look-up table
FeatureType useLUT(FeatureType in) 
{ 
  #pragma HLS INLINE
  IdxFixed index;
  if (in < 0)
  {
    in = -in;
    index = (IdxFixed)LUT_SIZE - (IdxFixed)(((TmpFixed)in) << (LUTIN_TWIDTH - LUTIN_IWIDTH));  
  } 
  else 
    index = ((TmpFixed)in) << (LUTIN_TWIDTH - LUTIN_IWIDTH);  
  return lut[index];
}

// Function to compute the probability. The Sigmoid function is implemented using BRAMS 
// to store the value of Sigmoid function for values relevant to this application
FeatureType Sigmoid(FeatureType exponent) 
{
  #pragma HLS INLINE
  if (exponent > 4) 
    return 1.0;
  else if (exponent < -4) 
    return 0.0;
  else 
  {
    LutInFixed inLut = (LutInFixed)exponent;
    return useLUT(inLut);
  } 
}

// Compute the gradient of the cost function
void computeGradient(
    FeatureType grad[NUM_FEATURES],
    DataType    feature[NUM_FEATURES],
    FeatureType scale)
{
  #pragma HLS INLINE
  const int unroll_factor = PAR_FACTOR;
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR; i++)
  {
    #pragma HLS PIPELINE II=1
    GRAD_INNER: for (int j = 0; j < PAR_FACTOR; j++)
      grad[i*PAR_FACTOR+j] = (scale * feature[i*PAR_FACTOR+j]);
  }
}

// Update the parameter vector
void updateParameter(
    FeatureType param[NUM_FEATURES],
    FeatureType grad[NUM_FEATURES],
    FeatureType scale)
{
  #pragma HLS INLINE
  const int unroll_factor = PAR_FACTOR;
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor

  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR; i++)
  {
    #pragma HLS PIPELINE II=1
    UPDATE_INNER: for (int j = 0; j < PAR_FACTOR; j++)
      param[i*PAR_FACTOR+j] += scale * grad[i*PAR_FACTOR+j];
  }
}


// wrapper, wraps the compute part for dataflow
void compute(FeatureType theta_local[NUM_FEATURES], 
             LabelType training_label,
             DataType training_instance[NUM_FEATURES])
{
  // array for storing gradient
  FeatureType gradient[NUM_FEATURES];

  // step size
  FeatureType step = STEP_SIZE;

  // do dot product with the parameter vector
  FeatureType dot = dotProduct(theta_local, training_instance);
  // do sigmoid function
  FeatureType prob = Sigmoid(dot);
  // compute gradient
  computeGradient(gradient, training_instance, (prob-training_label));
  // update the param vector
  updateParameter(theta_local, gradient, -step);
}


// stream out the data
void streamOut(FeatureType theta_local[NUM_FEATURES], hls::stream<ap_uint<32> > & Output_1)
{
  #pragma HLS INLINE
  // stream out the result
  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE; i ++ )
  {
    #pragma HLS pipeline II=2
    VectorFeatureType tmp_theta = 0;
    STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
      tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = theta_local[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
    Output_1.write(tmp_theta.range(31,0));
    Output_1.write(tmp_theta.range(63,32));
  }

}



//data_input_redirection
void data_in_redir( hls::stream<ap_uint<128> > & Input_1,
			hls::stream<ap_uint<64> > & Output_1,
			hls::stream<ap_uint<64> > & Output_2
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2

  // intermediate variables
  // local buffer of labels
  static LabelType   label_local[4512];
  #pragma HLS array_partition variable=label_local cyclic factor=8
  // array for storing one training instance
  static DataType training_instance[NUM_FEATURES];
  static int epoch = 0;
  bit128 dump = 0;

  if (epoch == 0)
  {
	DUM_CP: for(int i = 0; i<486; i++){
#pragma HLS PIPELINE II=1
		dump = Input_1.read();
	}

// copy in labels
	//LABEL_CP: for (int i = 0; i < NUM_TRAINING / L_VECTOR_SIZE; i ++ )
	LABEL_CP: for (int i = 0; i < 282; i ++ )
	{
#pragma HLS PIPELINE II=1
	  bit128 tmp_label = Input_1.read();
	  LABEL_CP_INNER: for (int j = 0; j < 16; j ++ )
		label_local[i * 16 + j].range(LTYPE_WIDTH-1, 0) = tmp_label.range((j+1)*LTYPE_WIDTH-1, j*LTYPE_WIDTH);
	}
  }

  // main loop
  // in each epoch, go through each training instance in sequence
  TRAINING_INST: for( int training_id = 0; training_id < NUM_TRAINING; training_id ++ )
  {
    // get the label
    LabelType training_label;
    training_label.range(LTYPE_WIDTH-1, 0)= label_local[training_id].range(LTYPE_WIDTH-1, 0);
    // first reads in the training instance

    Output_1.write(training_label.range(7, 0));
    READ_TRAINING_DATA_top: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 4; i ++ )
    {
#pragma HLS PIPELINE II=2
      bit128 tmp_data;
      tmp_data= Input_1.read();
      Output_1.write(tmp_data(63, 0));
      Output_1.write(tmp_data(127, 64));
    }

    Output_2.write(training_label.range(7, 0));
    READ_TRAINING_DATA_bot: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 4; i ++ )
    {
#pragma HLS PIPELINE II=2
      bit128 tmp_data;
      tmp_data= Input_1.read();
      Output_2.write(tmp_data(63, 0));
      Output_2.write(tmp_data(127, 64));
    }

  }
  epoch = epoch+1;
}

//data_input_redirection
void data_1_4_1(
		    hls::stream<ap_uint<64> > & Input_1,
			hls::stream<ap_uint<32> > & Input_2,
			hls::stream<ap_uint<32> > & Input_3,
			hls::stream<ap_uint<32> > & Input_4,
			hls::stream<ap_uint<32> > & Input_5,

			hls::stream<ap_uint<64> > & Output_1,
			hls::stream<ap_uint<64> > & Output_2,
			hls::stream<ap_uint<64> > & Output_3,
			hls::stream<ap_uint<64> > & Output_4,
			hls::stream<ap_uint<32> > & Output_5
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Input_3
#pragma HLS INTERFACE ap_hs port=Input_4
#pragma HLS INTERFACE ap_hs port=Input_5
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
#pragma HLS INTERFACE ap_hs port=Output_3
#pragma HLS INTERFACE ap_hs port=Output_4
#pragma HLS INTERFACE ap_hs port=Output_5

	LabelType training_label;
	static int epoch = 0;


  if(epoch < 5)
  {
	  // main loop
	  // in each epoch, go through each training instance in sequence
	  TRAINING_INST: for( int training_id = 0; training_id < NUM_TRAINING; training_id ++ )
	  {
		training_label.range(7,0) = Input_1.read();
		Output_1.write(training_label.range(7,0));
		// first reads in the training instance
		READ_TRAINING_DATA_1: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_1.write(tmp_data);
		}

		Output_2.write(training_label.range(7,0));
		READ_TRAINING_DATA_2: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_2.write(tmp_data.range(63, 0));
		}

		Output_3.write(training_label.range(7,0));
		READ_TRAINING_DATA_3: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_3.write(tmp_data.range(63, 0));
		}

		Output_4.write(training_label.range(7,0));
		READ_TRAINING_DATA_4: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_4.write(tmp_data.range(63, 0));
		}
	  }
	  epoch++;
  }else{

	  STREAM_OUT_1: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_2.read();
	    tmp_theta.range(63, 32) = Input_2.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  STREAM_OUT_2: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_3.read();
	    tmp_theta.range(63, 32) = Input_3.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  STREAM_OUT_3: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_4.read();
	    tmp_theta.range(63, 32) = Input_4.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  STREAM_OUT_4: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_5.read();
	    tmp_theta.range(63, 32) = Input_5.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  epoch = 0;
  }
}

//data_input_redirection
void data_1_4_2(
		    hls::stream<ap_uint<64> > & Input_1,
			hls::stream<ap_uint<32> > & Input_2,
			hls::stream<ap_uint<32> > & Input_3,
			hls::stream<ap_uint<32> > & Input_4,
			hls::stream<ap_uint<32> > & Input_5,
			hls::stream<ap_uint<64> > & Output_1,
			hls::stream<ap_uint<64> > & Output_2,
			hls::stream<ap_uint<64> > & Output_3,
			hls::stream<ap_uint<64> > & Output_4,
			hls::stream<ap_uint<32> > & Output_5
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Input_3
#pragma HLS INTERFACE ap_hs port=Input_4
#pragma HLS INTERFACE ap_hs port=Input_5
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
#pragma HLS INTERFACE ap_hs port=Output_3
#pragma HLS INTERFACE ap_hs port=Output_4
#pragma HLS INTERFACE ap_hs port=Output_5

	static int epoch = 0;



  if(epoch < 5)
  {
		LabelType training_label;
	  // main loop
	  // in each epoch, go through each training instance in sequence
	  TRAINING_INST: for( int training_id = 0; training_id < NUM_TRAINING; training_id ++ )
	  {
		  training_label.range(7,0) = Input_1.read();
		Output_1.write(training_label.range(7,0));
		// first reads in the training instance
		READ_TRAINING_DATA_1: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_1.write(tmp_data.range(63, 0));
		}

		Output_2.write(training_label.range(7,0));
		READ_TRAINING_DATA_2: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_2.write(tmp_data.range(63, 0));
		}

		Output_3.write(training_label.range(7,0));
		READ_TRAINING_DATA_3: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_3.write(tmp_data.range(63, 0));
		}

		Output_4.write(training_label.range(7,0));
		READ_TRAINING_DATA_4: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
		{
		  VectorFeatureType tmp_data;
		  tmp_data.range(63, 0)= Input_1.read();
		  Output_4.write(tmp_data.range(63, 0));
		}
	  }
	  epoch++;
  }else{
	  STREAM_OUT_1: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_2.read();
	    tmp_theta.range(63, 32) = Input_2.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  STREAM_OUT_2: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_3.read();
	    tmp_theta.range(63, 32) = Input_3.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  STREAM_OUT_3: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_4.read();
	    tmp_theta.range(63, 32) = Input_4.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  STREAM_OUT_4: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE /8; i ++ )
	  {
	    #pragma HLS pipeline II=1
	    VectorFeatureType tmp_theta = 0;
	    tmp_theta.range(31, 0) = Input_5.read();
	    tmp_theta.range(63, 32) = Input_5.read();
	    Output_5.write(tmp_theta.range(31,0));
	    Output_5.write(tmp_theta.range(63,32));
	  }
	  epoch = 0;
  }
}

void data_1_4_3(
			hls::stream<ap_uint<32> > & Input_1,
			hls::stream<ap_uint<32> > & Input_2,
			hls::stream<ap_uint<32> > & Input_3,
			hls::stream<ap_uint<32> > & Input_4,
			hls::stream<ap_uint<32> > & Input_5,
			hls::stream<ap_uint<32> > & Output_1,
			hls::stream<ap_uint<32> > & Output_2,
			hls::stream<ap_uint<32> > & Output_3,
			hls::stream<ap_uint<32> > & Output_4,
			hls::stream<ap_uint<32> > & Output_5
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Input_3
#pragma HLS INTERFACE ap_hs port=Input_4
#pragma HLS INTERFACE ap_hs port=Input_5
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
#pragma HLS INTERFACE ap_hs port=Output_3
#pragma HLS INTERFACE ap_hs port=Output_4
#pragma HLS INTERFACE ap_hs port=Output_5

	static char ExecuteAdd = 1;

	if(ExecuteAdd == 1)
	{
		FeatureType a1;
		FeatureType a2;
		FeatureType a3;
		FeatureType a4;
		FeatureType c;
		a1.range(31,0) = Input_1.read();
		a2.range(31,0) = Input_2.read();
		a3.range(31,0) = Input_3.read();
		a4.range(31,0) = Input_4.read();
		c = a1+a2+a3+a4;
		Output_1.write(c.range(31,0));
		ExecuteAdd = 0;
	}else{
		FeatureType tmp;
		tmp.range(31, 0) = Input_5.read();
		Output_2.write(tmp(31, 0));
		Output_3.write(tmp(31, 0));
		Output_4.write(tmp(31, 0));
		Output_5.write(tmp(31, 0));
		ExecuteAdd = 1;
	}
}

void data_1_4_4(
			hls::stream<ap_uint<32> > & Input_1,
			hls::stream<ap_uint<32> > & Input_2,
			hls::stream<ap_uint<32> > & Input_3,
			hls::stream<ap_uint<32> > & Input_4,
			hls::stream<ap_uint<32> > & Input_5,
			hls::stream<ap_uint<32> > & Output_1,
			hls::stream<ap_uint<32> > & Output_2,
			hls::stream<ap_uint<32> > & Output_3,
			hls::stream<ap_uint<32> > & Output_4,
			hls::stream<ap_uint<32> > & Output_5
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Input_3
#pragma HLS INTERFACE ap_hs port=Input_4
#pragma HLS INTERFACE ap_hs port=Input_5
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
#pragma HLS INTERFACE ap_hs port=Output_3
#pragma HLS INTERFACE ap_hs port=Output_4
#pragma HLS INTERFACE ap_hs port=Output_5

	static char ExecuteAdd = 1;

	if(ExecuteAdd == 1)
	{
		FeatureType a1;
		FeatureType a2;
		FeatureType a3;
		FeatureType a4;
		FeatureType c;
		a1.range(31,0) = Input_1.read();
		a2.range(31,0) = Input_2.read();
		a3.range(31,0) = Input_3.read();
		a4.range(31,0) = Input_4.read();
		c = a1+a2+a3+a4;
		Output_1.write(c.range(31,0));
		ExecuteAdd = 0;
	}else{
		FeatureType tmp;
		tmp.range(31, 0) = Input_5.read();
		Output_2.write(tmp(31, 0));
		Output_3.write(tmp(31, 0));
		Output_4.write(tmp(31, 0));
		Output_5.write(tmp(31, 0));
		ExecuteAdd = 1;
	}
}



void dotProduct_1(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];

  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];

  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor dim=1
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor dim=1
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor dim=1
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data = Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		{
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);

		}

	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}

void dotProduct_2(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];
  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];
  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data= Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		//                                            4
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
		//                                               16                                        16
	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}

void dotProduct_3(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];
  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];
  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data.range(63, 0)= Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		//                                            4
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
		//                                               16                                        16
	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}

void dotProduct_4(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];
  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];
  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data.range(63, 0)= Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		//                                            4
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
		//                                               16                                        16
	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}

void dotProduct_5(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];
  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];
  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data.range(63, 0)= Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		//                                            4
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
		//                                               16                                        16
	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}

void dotProduct_6(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];
  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];
  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data.range(63, 0)= Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		//                                            4
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
		//                                               16                                        16
	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}

void dotProduct_7(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];
  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];
  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data.range(63, 0)= Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		//                                            4
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
		//                                               16                                        16
	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}

void dotProduct_8(hls::stream<ap_uint<64> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2
)
{

#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  const int unroll_factor = PAR_FACTOR_DEC;
  static FeatureType param[NUM_FEATURES/8];
  FeatureType grad[NUM_FEATURES/8];
  static DataType feature[NUM_FEATURES/8];
  FeatureType scale;
  FeatureType prob;
  #pragma HLS array_partition variable=param cyclic factor=unroll_factor
  #pragma HLS array_partition variable=feature cyclic factor=unroll_factor
  #pragma HLS array_partition variable=grad cyclic factor=unroll_factor
  static int odd_even = 0;
  static int num_train = 0;
  static int epoch;
  static int sb = 0;
  static LabelType training_label;

  if(odd_even == 0){
	  training_label.range(7,0) = Input_1.read();

	  READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE / 8; i ++ )
	  //                                      1024           4
	  {
#pragma HLS PIPELINE II=1
		VectorFeatureType tmp_data;
		tmp_data.range(63, 0)= Input_1.read();
		READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
		//                                            4
			feature[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
		//                                               16                                        16
	  }


	  FeatureType result = 0;
	  DOT: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		DOT_INNER: for(int j = 0; j < PAR_FACTOR_DEC; j++)
		{
		  FeatureType term = param[i*PAR_FACTOR_DEC+j] * feature[i*PAR_FACTOR_DEC+j];
		  result += term;
		}
	  }
	  Output_1.write(result.range(31,0));
	  odd_even = 1;
	  return;
  }else{
	  prob.range(31,0) = Input_2.read();
	  scale = prob - training_label;

	  GRAD: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC / 8; i++)
	  {
		#pragma HLS PIPELINE II=1
		GRAD_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  grad[i*PAR_FACTOR_DEC+j] = (scale * feature[i*PAR_FACTOR_DEC+j]);
	  }

	  FeatureType step = STEP_SIZE;
	  UPDATE: for (int i = 0; i < NUM_FEATURES / PAR_FACTOR_DEC/8; i++)
	  {
		#pragma HLS PIPELINE II=1
		UPDATE_INNER: for (int j = 0; j < PAR_FACTOR_DEC; j++)
		  param[i*PAR_FACTOR_DEC+j] += (-step) * grad[i*PAR_FACTOR_DEC+j];
	  }

	  num_train++;
	  if(num_train==NUM_TRAINING){
		  num_train = 0;
		  epoch++;
	  }
	  if(epoch==5){
		  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE / 8; i ++ )
		  {
			#pragma HLS pipeline II=1
			VectorFeatureType tmp_theta = 0;
			STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
			  tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = param[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
			Output_2.write(tmp_theta.range(31,0));
			Output_2.write(tmp_theta.range(63,32));
		  }
	  }
	  odd_even = 0;
	  return;
  }
}




void Sigmoid_axi(
		hls::stream<ap_uint<32> > & Input_1,
		hls::stream<ap_uint<32> > & Input_2,
		hls::stream<ap_uint<32> > & Output_1,
		hls::stream<ap_uint<32> > & Output_2)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1
#pragma HLS INTERFACE ap_hs port=Output_2
  #pragma HLS INLINE
	FeatureType exponent;

	FeatureType a1;
	FeatureType a2;
	FeatureType c;
	a1.range(31,0) = Input_1.read();
	a2.range(31,0) = Input_2.read();
	exponent = a1+a2;

	FeatureType result;
  if (exponent > 4)
    result =  1.0;
  else if (exponent < -4)
    result =  0.0;
  else
  {
    LutInFixed inLut = (LutInFixed)exponent;
    result = useLUT(inLut);
  }
  Output_1.write(result.range(31, 0));
  Output_2.write(result.range(31, 0));
}



void data_2_1(
			hls::stream<ap_uint<32> > & Input_1,
			hls::stream<ap_uint<32> > & Input_2,
			hls::stream<ap_uint<128> > & Output_1
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Input_2
#pragma HLS INTERFACE ap_hs port=Output_1


	static unsigned int theta[NUM_FEATURES / F_VECTOR_SIZE * 2];

	  STREAM_IN_1: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE; i ++ )
	  {
	    #pragma HLS pipeline II=1
		theta[i] = Input_1.read();
	  }
	  STREAM_IN_2: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE; i ++ )
	  {
	    #pragma HLS pipeline II=1
		theta[i+NUM_FEATURES / F_VECTOR_SIZE] = Input_2.read();
	  }

	  Output_1.write(257);

	  STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE*2/4; i ++ )
	  {
	    #pragma HLS pipeline II=1
		bit128 Output_tmp;
		Output_tmp(31, 0) = theta[4*i];
		Output_tmp(63, 32) = theta[4*i+1];
		Output_tmp(95, 64) = theta[4*i+2];
		Output_tmp(127, 96) = theta[4*i+3];
		Output_1.write(Output_tmp);
	  }




}


// top-level function
void SgdLR_old( hls::stream<ap_uint<32> > & Input_1,
			hls::stream<ap_uint<32> > & Output_1
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

  // intermediate variables
  // local version of the parameters
  static FeatureType theta_local[NUM_FEATURES];
  // local buffer of labels
  static LabelType   label_local[NUM_TRAINING];
  #pragma HLS array_partition variable=label_local cyclic factor=L_VECTOR_SIZE
  // array for storing one training instance
  static DataType training_instance[NUM_FEATURES];
  static int epoch = 0;
  //epoch = Input_1.read();

  if (epoch == 0)
  {
    // copy in labels
    LABEL_CP: for (int i = 0; i < NUM_TRAINING / L_VECTOR_SIZE; i ++ )
    {
      #pragma HLS pipeline
      VectorLabelType tmp_label = Input_1.read();
      LABEL_CP_INNER: for (int j = 0; j < L_VECTOR_SIZE; j ++ )
        label_local[i * L_VECTOR_SIZE + j].range(LTYPE_WIDTH-1, 0) = tmp_label.range((j+1)*LTYPE_WIDTH-1, j*LTYPE_WIDTH);
    }
  }


  // main loop
  // in each epoch, go through each training instance in sequence
  TRAINING_INST: for( int training_id = 0; training_id < NUM_TRAINING; training_id ++ )
  {
    // get the label
    LabelType training_label = label_local[training_id];
    // first reads in the training instance

    READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE; i ++ )
    //                                      1024           4
    {
      VectorFeatureType tmp_data;
      tmp_data.range(31, 0)= Input_1.read();
      tmp_data.range(63, 32)= Input_1.read();
      READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
      //                                            4
        training_instance[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
      //                                               16                                        16
    }



    // compute part
    compute(theta_local, training_label, training_instance);
  }

  if (epoch == 4)
  {
     streamOut(theta_local, Output_1);
  }
  epoch++;
  if(epoch == 5){epoch = 0;}

}



// top-level function
void SgdLR( hls::stream<ap_uint<128> > & Input_1,
			hls::stream<ap_uint<128> > & Output_1
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1
	static int epoch = 0;

	hls::stream<ap_uint<64> >  Output_1_redir("sb1");
	hls::stream<ap_uint<64> >  Output_2_redir("sb2");
	hls::stream<ap_uint<64> >  Output_1_1_4_1("sb3");
	hls::stream<ap_uint<64> >  Output_2_1_4_1("sb4");
	hls::stream<ap_uint<64> >  Output_3_1_4_1("sb5");
	hls::stream<ap_uint<64> >  Output_4_1_4_1("sb6");
	hls::stream<ap_uint<64> >  Output_1_1_4_2("sb7");
	hls::stream<ap_uint<64> >  Output_2_1_4_2("sb8");
	hls::stream<ap_uint<64> >  Output_3_1_4_2("sb9");
	hls::stream<ap_uint<64> >  Output_4_1_4_2("sb10");
	hls::stream<ap_uint<32> >  Output_1_dot_1("sb12");
	hls::stream<ap_uint<32> >  Output_2_dot_1("sb13");
	hls::stream<ap_uint<32> >  Output_1_dot_2("sb14");
	hls::stream<ap_uint<32> >  Output_2_dot_2("sb15");
	hls::stream<ap_uint<32> >  Output_1_dot_3("sb16");
	hls::stream<ap_uint<32> >  Output_2_dot_3("sb17");
	hls::stream<ap_uint<32> >  Output_1_dot_4("sb18");
	hls::stream<ap_uint<32> >  Output_2_dot_4("sb19");
	hls::stream<ap_uint<32> >  Output_1_dot_5("sb20");
	hls::stream<ap_uint<32> >  Output_2_dot_5("sb21");
	hls::stream<ap_uint<32> >  Output_1_dot_6("sb22");
	hls::stream<ap_uint<32> >  Output_2_dot_6("sb23");
	hls::stream<ap_uint<32> >  Output_1_dot_7("sb24");
	hls::stream<ap_uint<32> >  Output_2_dot_7("sb25");
	hls::stream<ap_uint<32> >  Output_1_dot_8("sb26");
	hls::stream<ap_uint<32> >  Output_2_dot_8("sb27");
	hls::stream<ap_uint<32> >  Output_1_add_4_1_1("sb28");
	hls::stream<ap_uint<32> >  Output_1_add_4_1_2("sb29");
	hls::stream<ap_uint<32> >  Output_1_add_2_1("sb30");
	hls::stream<ap_uint<32> >  Output_1_sig("sb31");
	hls::stream<ap_uint<32> >  Output_2_sig("sb32");
	hls::stream<ap_uint<32> >  Output_1_1_4_3("sb33");
	hls::stream<ap_uint<32> >  Output_2_1_4_3("sb34");
	hls::stream<ap_uint<32> >  Output_3_1_4_3("sb35");
	hls::stream<ap_uint<32> >  Output_4_1_4_3("sb36");
	hls::stream<ap_uint<32> >  Output_1_1_4_4("sb37");
	hls::stream<ap_uint<32> >  Output_2_1_4_4("sb38");
	hls::stream<ap_uint<32> >  Output_3_1_4_4("sb39");
	hls::stream<ap_uint<32> >  Output_4_1_4_4("sb40");
	hls::stream<ap_uint<32> >  Output_1_4_1_1("sb41");
	hls::stream<ap_uint<32> >  Output_1_4_1_2("sb42");

	data_in_redir(Input_1, Output_1_redir, Output_2_redir);
	data_1_4_1(Output_1_redir, Output_2_dot_1, Output_2_dot_2, Output_2_dot_3, Output_2_dot_4,
			   Output_1_1_4_1, Output_2_1_4_1, Output_3_1_4_1, Output_4_1_4_1, Output_1_4_1_1);
	data_1_4_2(Output_2_redir, Output_2_dot_5, Output_2_dot_6, Output_2_dot_7, Output_2_dot_8,
			   Output_1_1_4_2, Output_2_1_4_2, Output_3_1_4_2, Output_4_1_4_2, Output_1_4_1_2);


	TRAINING_INST: for( int training_id = 0; training_id < NUM_TRAINING; training_id ++ ){
		dotProduct_1(Output_1_1_4_1, Output_1_1_4_3, Output_1_dot_1, Output_2_dot_1);
		dotProduct_2(Output_2_1_4_1, Output_2_1_4_3, Output_1_dot_2, Output_2_dot_2);
		dotProduct_3(Output_3_1_4_1, Output_3_1_4_3, Output_1_dot_3, Output_2_dot_3);
		dotProduct_4(Output_4_1_4_1, Output_4_1_4_3, Output_1_dot_4, Output_2_dot_4);
		dotProduct_5(Output_1_1_4_2, Output_1_1_4_4, Output_1_dot_5, Output_2_dot_5);
		dotProduct_6(Output_2_1_4_2, Output_2_1_4_4, Output_1_dot_6, Output_2_dot_6);
		dotProduct_7(Output_3_1_4_2, Output_3_1_4_4, Output_1_dot_7, Output_2_dot_7);
		dotProduct_8(Output_4_1_4_2, Output_4_1_4_4, Output_1_dot_8, Output_2_dot_8);

		data_1_4_3(Output_1_dot_1,  Output_1_dot_2, Output_1_dot_3, Output_1_dot_4, Output_1_sig,
				Output_1_add_4_1_1, Output_1_1_4_3, Output_2_1_4_3, Output_3_1_4_3, Output_4_1_4_3);
		data_1_4_4(Output_1_dot_5, Output_1_dot_6, Output_1_dot_7, Output_1_dot_8, Output_2_sig,
				Output_1_add_4_1_2, Output_1_1_4_4, Output_2_1_4_4, Output_3_1_4_4, Output_4_1_4_4);

		Sigmoid_axi(Output_1_add_4_1_1, Output_1_add_4_1_2, Output_1_sig, Output_2_sig);

		data_1_4_3(Output_1_dot_1,  Output_1_dot_2, Output_1_dot_3, Output_1_dot_4, Output_1_sig,
				Output_1_add_4_1_1, Output_1_1_4_3, Output_2_1_4_3, Output_3_1_4_3, Output_4_1_4_3);
		data_1_4_4(Output_1_dot_5, Output_1_dot_6, Output_1_dot_7, Output_1_dot_8, Output_2_sig,
				Output_1_add_4_1_2, Output_1_1_4_4, Output_2_1_4_4, Output_3_1_4_4, Output_4_1_4_4);
		dotProduct_1(Output_1_1_4_1, Output_1_1_4_3, Output_1_dot_1, Output_2_dot_1);
		dotProduct_2(Output_2_1_4_1, Output_2_1_4_3, Output_1_dot_2, Output_2_dot_2);
		dotProduct_3(Output_3_1_4_1, Output_3_1_4_3, Output_1_dot_3, Output_2_dot_3);
		dotProduct_4(Output_4_1_4_1, Output_4_1_4_3, Output_1_dot_4, Output_2_dot_4);
		dotProduct_5(Output_1_1_4_2, Output_1_1_4_4, Output_1_dot_5, Output_2_dot_5);
		dotProduct_6(Output_2_1_4_2, Output_2_1_4_4, Output_1_dot_6, Output_2_dot_6);
		dotProduct_7(Output_3_1_4_2, Output_3_1_4_4, Output_1_dot_7, Output_2_dot_7);
		dotProduct_8(Output_4_1_4_2, Output_4_1_4_4, Output_1_dot_8, Output_2_dot_8);
	}
	if(epoch==4){
		data_1_4_1(Output_1_redir, Output_2_dot_1, Output_2_dot_2, Output_2_dot_3, Output_2_dot_4,
				   Output_1_1_4_1, Output_2_1_4_1, Output_3_1_4_1, Output_4_1_4_1, Output_1_4_1_1);
		data_1_4_2(Output_2_redir, Output_2_dot_5, Output_2_dot_6, Output_2_dot_7, Output_2_dot_8,
				   Output_1_1_4_2, Output_2_1_4_2, Output_3_1_4_2, Output_4_1_4_2, Output_1_4_1_2);

		data_2_1(Output_1_4_1_1, Output_1_4_1_2, Output_1);
	}
	epoch++;
}

// top-level function 
void SgdLR_mono( hls::stream<ap_uint<32> > & Input_1,
			hls::stream<ap_uint<32> > & Output_1
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1

  // intermediate variables 
  // local version of the parameters
  static FeatureType theta_local[NUM_FEATURES];
  // local buffer of labels
  static LabelType   label_local[NUM_TRAINING];
  #pragma HLS array_partition variable=label_local cyclic factor=L_VECTOR_SIZE
  // array for storing one training instance
  static DataType training_instance[NUM_FEATURES];
  static int epoch = 0;
  static int dump = 0;
  //epoch = Input_1.read();

  if (epoch == 0)
  {
	    for(int i = 0; i<1947; i++){
	    	dump = Input_1.read();
	    }

    // copy in labels
    LABEL_CP: for (int i = 0; i < NUM_TRAINING / L_VECTOR_SIZE; i ++ )
    {
     #pragma HLS PIPELINE II=1
      VectorLabelType tmp_label = Input_1.read();
      LABEL_CP_INNER: for (int j = 0; j < L_VECTOR_SIZE; j ++ )
        label_local[i * L_VECTOR_SIZE + j].range(LTYPE_WIDTH-1, 0) = tmp_label.range((j+1)*LTYPE_WIDTH-1, j*LTYPE_WIDTH);
    }


  }


  // main loop
  // in each epoch, go through each training instance in sequence
  TRAINING_INST: for( int training_id = 0; training_id < NUM_TRAINING; training_id ++ )
  {  
    // get the label
    LabelType training_label = label_local[training_id];
    // first reads in the training instance

    READ_TRAINING_DATA: for (int i = 0; i < NUM_FEATURES / D_VECTOR_SIZE; i ++ )
    {
#pragma HLS PIPELINE II=2
      VectorFeatureType tmp_data;
      tmp_data.range(31, 0)= Input_1.read();
      tmp_data.range(63, 32)= Input_1.read();
      READ_TRAINING_DATA_INNER: for (int j = 0; j < D_VECTOR_SIZE; j ++ )
        training_instance[i * D_VECTOR_SIZE + j].range(DTYPE_TWIDTH-1, 0) = tmp_data.range((j+1)*DTYPE_TWIDTH-1, j*DTYPE_TWIDTH);
    }



    // compute part
    compute(theta_local, training_label, training_instance);
  }


  if (epoch == 4)
  {
    //streamOut(theta_local, Output_1);
  // stream out the result
	Output_1.write(1025);
    STREAM_OUT: for (int i = 0; i < NUM_FEATURES / F_VECTOR_SIZE; i ++ )
    {
      #pragma HLS pipeline II=2
      VectorFeatureType tmp_theta = 0;
      STREAM_OUT_INNER: for (int j = 0; j < F_VECTOR_SIZE; j ++ )
        tmp_theta.range((j+1)*FTYPE_TWIDTH-1, j*FTYPE_TWIDTH) = theta_local[i * F_VECTOR_SIZE + j].range(FTYPE_TWIDTH-1, 0);
      Output_1.write(tmp_theta.range(31,0));
      Output_1.write(tmp_theta.range(63,32));
    }
  }
  epoch++;


}





// top-level function
void SgdLR_sudo( hls::stream<ap_uint<32> > & Input_1,
			hls::stream<ap_uint<32> > & Output_1
			)
{
#pragma HLS INTERFACE ap_hs port=Input_1
#pragma HLS INTERFACE ap_hs port=Output_1
 unsigned long i;
 unsigned int sum = 0;
 unsigned int tmp = 0;
 data: for (i=0; i<2307071; i++)
 {
#pragma HLS PIPELINE II=1
	 sum += Input_1.read();
 }
 tmp = Input_1.read();

 Output_1.write(402);

 out_stream: for (i=0; i<400; i++){
#pragma HLS PIPELINE II=1
	 Output_1.write(sum);
 }
 Output_1.write(tmp);
}

