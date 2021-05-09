#include "optical_flow.h"
// use HLS video library
// use HLS fixed point
#include "ap_fixed.h"

typedef ap_uint<32> databus_t;
typedef ap_uint<128> bit128;
typedef ap_uint<64> bit64;
typedef ap_uint<160> bit160;

typedef ap_uint<288> widebus_t;
// define these constants so they can be used in pragma
const int max_width = MAX_WIDTH;
const int default_depth = MAX_WIDTH;

void gradient_xyz_calc(//input_t frame[MAX_HEIGHT][MAX_WIDTH],
    hls::stream< ap_uint<128> > &Input_1,
    hls::stream<databus_t> &Output_1,
    hls::stream<databus_t> &Output_2,
    hls::stream<databus_t> &Output_3)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Output_2
#pragma HLS interface ap_hs port=Output_3
  // our own line buffer
  static pixel_t buf[5][MAX_WIDTH];
  #pragma HLS array_partition variable=buf complete dim=1

  // small buffer
  pixel_t smallbuf[5];
  #pragma HLS array_partition variable=smallbuf complete dim=0

  // window buffer
  hls::Window<5,5,input_t> window;

  const int GRAD_WEIGHTS[] =  {1,-8,0,8,-1};
  hls::stream<databus_t> gradient_z;
  #pragma HLS STREAM variable=gradient_z depth=3*MAX_WIDTH
  databus_t temp3;
  databus_t grad_z;
  GRAD_XY_OUTER: for(int r=0; r<MAX_HEIGHT+2; r++)
  {
    GRAD_XY_INNER: for(int c=0; c<MAX_WIDTH+2; c++)
    {
      #pragma HLS pipeline II=1
      // read out values from current line buffer
      for (int i = 0; i < 4; i ++ )
        smallbuf[i] = buf[i+1][c];
      // the new value is either 0 or read from frame
      if (r<MAX_HEIGHT && c<MAX_WIDTH){
    	databus_t pixel1,pixel2,pixel3,pixel4,pixel5;
    	ap_uint<128> buf = Input_1.read();
    	pixel1 = 0;
    	pixel2 = 0;
    	pixel3 = 0;
    	pixel4 = 0;
    	pixel5 = 0;
        pixel1.range(7,0) = buf.range(7,0);
        pixel2.range(7,0) = buf.range(15,8);
        pixel3.range(7,0) = buf.range(23,16);
        pixel4.range(7,0) = buf.range(31,24);
        pixel5.range(7,0) = buf.range(39,32);

    	databus_t tmpread = pixel3;
        input_t tmpin = 0;
        tmpin(16,0) = tmpread(16,0);
        smallbuf[4] = (pixel_t)(tmpin);

        input_t frame1_tmp,frame2_tmp,frame3_tmp,frame4_tmp,frame5_tmp;
	frame1_tmp = 0;
	frame2_tmp = 0;
	frame3_tmp = 0;
	frame4_tmp = 0;
	frame5_tmp = 0;
        databus_t data = 0;
        pixel_t temp_z = 0;
        data = pixel1;
        frame1_tmp(16,0) = data(16,0);
        data = pixel2;;
        frame2_tmp(16,0) = data(16,0);
        data = pixel3;;
        frame3_tmp(16,0) = data(16,0);
        data = pixel4;
        frame4_tmp(16,0) = data(16,0);
        data = pixel5;
        frame5_tmp(16,0) = data(16,0);
        temp_z =((pixel_t)(frame1_tmp*GRAD_WEIGHTS[0]
                         + frame2_tmp*GRAD_WEIGHTS[1]
                         + frame3_tmp*GRAD_WEIGHTS[2]
                         + frame4_tmp*GRAD_WEIGHTS[3]
                         + frame5_tmp*GRAD_WEIGHTS[4]))/12;
        grad_z.range(31,0) = temp_z.range(31,0);
	gradient_z.write(grad_z);
      }
      else if (c < MAX_WIDTH)
        smallbuf[4] = 0;
      // update line buffer
      if(r<MAX_HEIGHT && c<MAX_WIDTH)
      {
        for (int i = 0; i < 4; i ++ )
          buf[i][c] = smallbuf[i];
        buf[4][c] = smallbuf[4];
      }
      else if(c<MAX_WIDTH)
      {
        for (int i = 0; i < 4; i ++ )
          buf[i][c] = smallbuf[i];
          buf[4][c] = smallbuf[4];
      }

      // manage window buffer
      if(r<MAX_HEIGHT && c<MAX_WIDTH)
      {
        window.shift_pixels_left();

        for (int i = 0; i < 5; i ++ )
          window.insert_pixel(smallbuf[i],i,4);
      }
      else
      {
        window.shift_pixels_left();
        window.insert_pixel(0,0,4);
        window.insert_pixel(0,1,4);
        window.insert_pixel(0,2,4);
        window.insert_pixel(0,3,4);
        window.insert_pixel(0,4,4);
      }

      // compute gradient
      pixel_t x_grad = 0;
      databus_t temp1 = 0;
      databus_t temp2 = 0;
      databus_t temp3 = 0;
      pixel_t y_grad = 0;
      if(r>=4 && r<MAX_HEIGHT && c>=4 && c<MAX_WIDTH)
      {
        GRAD_XY_XYGRAD: for(int i=0; i<5; i++)
        {
          x_grad += window.getval(2,i)*GRAD_WEIGHTS[i];
          y_grad += window.getval(i,2)*GRAD_WEIGHTS[i];
        }
		x_grad = x_grad/12;
		temp1.range(31,0) = x_grad.range(31,0);
		Output_1.write(temp1);
		y_grad = y_grad/12;
		temp2.range(31,0) = y_grad.range(31,0);
		Output_2.write(temp2);
		temp3 = gradient_z.read();
		Output_3.write(temp3);
      }
      else if(r>=2 && c>=2)
      {
		Output_1.write(0);
		Output_2.write(0);
		temp3 = gradient_z.read();
		Output_3.write(temp3);
      }
    }
  }
}

void gradient_weight_y_1(//pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &Input_1,
    hls::stream<databus_t> &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
  hls::LineBuffer<7,MAX_WIDTH,pixel_t> buf;

  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_Y_OUTER: for(int r=0; r<MAX_HEIGHT+3; r++)
  {
    GRAD_WEIGHT_Y_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      #pragma HLS dependence variable=buf inter false

      if(r<MAX_HEIGHT)
      {
        buf.shift_pixels_up(c);
        pixel_t tmp = 0;
	databus_t temp;
	temp = Input_1.read();
	tmp(31,0) = temp(31,0);

        buf.insert_bottom_row(tmp,c);
      }
      else
      {
        buf.shift_pixels_up(c);
        pixel_t tmp;
        tmp = 0;
        buf.insert_bottom_row(tmp,c);
      }

      pixel_t acc;
      databus_t temp1 = 0;
      databus_t temp2 = 0;
      acc = 0;
      if(r >= 6 && r<MAX_HEIGHT)
      {
        GRAD_WEIGHT_Y_ACC: for(int i=0; i<7; i++)
        {
          acc += buf.getval(i,c)*GRAD_FILTER[i];
        }
		temp1.range(31,0) = acc.range(31,0);
		Output_1.write(temp1);
      }
      else if(r>=3)
      {
		temp2.range(31,0) = acc.range(31,0);
		Output_1.write(temp2);
      }
    }
  }
}

void gradient_weight_y_2(//pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &Input_1,
    hls::stream<databus_t> &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
  hls::LineBuffer<7,MAX_WIDTH,pixel_t> buf;

  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_Y_OUTER: for(int r=0; r<MAX_HEIGHT+3; r++)
  {
    GRAD_WEIGHT_Y_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      #pragma HLS dependence variable=buf inter false

      if(r<MAX_HEIGHT)
      {
        buf.shift_pixels_up(c);
        pixel_t tmp = 0;
	databus_t temp;
	temp = Input_1.read();
	tmp(31,0) = temp(31,0);
        buf.insert_bottom_row(tmp,c);
      }
      else
      {
        buf.shift_pixels_up(c);
        pixel_t tmp;
        tmp = 0;
        buf.insert_bottom_row(tmp,c);
      }

      pixel_t acc;
      databus_t temp = 0;
      acc = 0;
      if(r >= 6 && r<MAX_HEIGHT)
      {
        GRAD_WEIGHT_Y_ACC: for(int i=0; i<7; i++)
        {
          acc+= buf.getval(i,c)*GRAD_FILTER[i];
        }
        //filt_grad[r-3][c] = acc;
		temp.range(31,0) = acc.range(31,0);
		Output_1.write(temp);
      }
      else if(r>=3)
      {
        //filt_grad[r-3][c] = acc;
		temp.range(31,0) = acc.range(31,0);
		Output_1.write(temp);
      }
    }
  }
}
// average the gradient in y direction
void gradient_weight_y_3(//pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &Input_1,
    hls::stream<databus_t> &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
  hls::LineBuffer<7,MAX_WIDTH,pixel_t> buf;

  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_Y_OUTER: for(int r=0; r<MAX_HEIGHT+3; r++)
  {
    GRAD_WEIGHT_Y_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      #pragma HLS dependence variable=buf inter false

      if(r<MAX_HEIGHT)
      {
        buf.shift_pixels_up(c);
        pixel_t tmp = 0;
	databus_t temp;
	temp = Input_1.read();
	tmp(31,0) = temp(31,0);
        buf.insert_bottom_row(tmp,c);
      }
      else
      {
        buf.shift_pixels_up(c);
        pixel_t tmp;
        tmp = 0;
        buf.insert_bottom_row(tmp,c);
      }

      pixel_t acc;
      databus_t temp = 0;
      acc = 0;
      if(r >= 6 && r<MAX_HEIGHT)
      {
        GRAD_WEIGHT_Y_ACC: for(int i=0; i<7; i++)
        {
          acc+= buf.getval(i,c)*GRAD_FILTER[i];
        }
		temp.range(31,0) = acc.range(31,0);
		Output_1.write(temp);
      }
      else if(r>=3)
      {
		temp.range(31,0) = acc.range(31,0);
		Output_1.write(temp);
      }
    }
  }
}

// average gradient in the x direction
void gradient_weight_x1(//gradient_t y_filt[MAX_HEIGHT][MAX_WIDTH],
		       hls::stream<databus_t> &Input_1,
    		       hls::stream<databus_t> &Output_1,
    		       hls::stream<databus_t> &Output_2)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Output_2
  hls::Window<1,7,gradient_t> buf;
  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_X_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    GRAD_WEIGHT_X_INNER: for(int c=0; c<MAX_WIDTH+3; c++)
    {
      #pragma HLS pipeline II=1
      buf.shift_pixels_left();
      gradient_t tmp;
      tmp.x = 0;
      databus_t temp;
      if(c<MAX_WIDTH)
      {
		temp = Input_1.read();
		tmp.x.range(31,0) = temp.range(31,0);
      }
      else
      {
        tmp.x = 0;
      }
      buf.insert_pixel(tmp,0,6);

      gradient_t acc;
      acc.x = 0;
      if(c >= 6 && c<MAX_WIDTH)
      {
        GRAD_WEIGHT_X_ACC: for(int i=0; i<7; i++)
        {
          acc.x += buf.getval(0,i).x*GRAD_FILTER[i];
        }
        //filt_grad[r][c-3] = acc;
		temp.range(31,0) = acc.x.range(31,0);
		Output_1.write(temp);
		Output_2.write(temp);
      }
      else if(c>=3)
      {
		temp.range(31,0) = acc.x.range(31,0);
		Output_1.write(temp);
		Output_2.write(temp);
      }
    }
  }
}

void gradient_weight_x2(//gradient_t y_filt[MAX_HEIGHT][MAX_WIDTH],
		       hls::stream<databus_t> &Input_1,
		       hls::stream<databus_t> &Output_1,
		       hls::stream<databus_t> &Output_2)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Output_2
  hls::Window<1,7,gradient_t> buf;
  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_X_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    GRAD_WEIGHT_X_INNER: for(int c=0; c<MAX_WIDTH+3; c++)
    {
      #pragma HLS pipeline II=1
      buf.shift_pixels_left();
      gradient_t tmp;
      tmp.y = 0;
      databus_t temp;
      if(c<MAX_WIDTH)
      {
        //tmp = y_filt[r][c];
		temp = Input_1.read();
		tmp.y.range(31,0) = temp.range(31,0);
      }
      else
      {
        tmp.y = 0;
      }
      buf.insert_pixel(tmp,0,6);

      gradient_t acc;
      acc.y = 0;
      if(c >= 6 && c<MAX_WIDTH)
      {
        GRAD_WEIGHT_X_ACC: for(int i=0; i<7; i++)
        {
          acc.y += buf.getval(0,i).y*GRAD_FILTER[i];
        }
        //filt_grad[r][c-3] = acc;
		temp.range(31,0) = acc.y.range(31,0);
		Output_1.write(temp);
		Output_2.write(temp);
      }
      else if(c>=3)
      {
		temp.range(31,0) = acc.y.range(31,0);
		Output_1.write(temp);
		Output_2.write(temp);
      }
    }
  }
}
void gradient_weight_x3(//gradient_t y_filt[MAX_HEIGHT][MAX_WIDTH],
		       hls::stream<databus_t> &Input_1,
		       hls::stream<databus_t> &Output_1,
		       hls::stream<databus_t> &Output_2)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Output_2
  hls::Window<1,7,gradient_t> buf;
  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_X_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    GRAD_WEIGHT_X_INNER: for(int c=0; c<MAX_WIDTH+3; c++)
    {
      #pragma HLS pipeline II=1
      buf.shift_pixels_left();
      gradient_t tmp;
      tmp.z = 0;
      databus_t temp;
      if(c<MAX_WIDTH)
      {
		temp = Input_1.read();
		tmp.z.range(31,0) = temp.range(31,0);
      }
      else
      {
        tmp.z = 0;
      }
      buf.insert_pixel(tmp,0,6);

      gradient_t acc;
      acc.z = 0;
      if(c >= 6 && c<MAX_WIDTH)
      {
        GRAD_WEIGHT_X_ACC: for(int i=0; i<7; i++)
        {
          acc.z += buf.getval(0,i).z*GRAD_FILTER[i];
        }
        //filt_grad[r][c-3] = acc;
		temp.range(31,0) = acc.z.range(31,0);
		Output_1.write(temp);
		Output_2.write(temp);
      }
      else if(c>=3)
      {
       // filt_grad[r][c-3] = acc;
		temp.range(31,0) = acc.z.range(31,0);
		Output_1.write(temp);
		Output_2.write(temp);
      }
    }
  }
}
// outer product
void outer_product1(//gradient_t gradient[MAX_HEIGHT][MAX_WIDTH],
    		       hls::stream<databus_t> &Input_1,
		       hls::stream<databus_t> &Input_2,
		       hls::stream<databus_t> &Input_3,
		   hls::stream<bit160> &Output_1
		)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Input_2
#pragma HLS interface ap_hs port=Input_3
#pragma HLS interface ap_hs port=Output_1
  OUTER_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    OUTER_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      //gradient_t grad = gradient[r][c];
      gradient_t grad;
      databus_t temp_x,temp_y,temp_z;
      temp_x = Input_1.read();
      temp_y = Input_2.read();
      temp_z = Input_3.read();
      grad.x.range(31,0) = temp_x.range(31,0);
      grad.y.range(31,0) = temp_y.range(31,0);
      grad.z.range(31,0) = temp_z.range(31,0);
      outer_pixel_t x = (outer_pixel_t) grad.x;
      outer_pixel_t y = (outer_pixel_t) grad.y;
      outer_pixel_t z = (outer_pixel_t) grad.z;
      outer_t out;
      out.val[0] = (x*x);
      out.val[1] = (y*y);
      out.val[2] = (x*y);
      //Output_1[r][c] = out;
      widebus_t widetemp;
      widetemp.range(47,0) = out.val[0].range(47,0);
      widetemp.range(95,48) = out.val[1].range(47,0);
      widetemp.range(143,96) = out.val[2].range(47,0);
      widetemp.range(287,144) = 0;
      //Output_1.write(widetemp);

      bit160 out_tmp;
      out_tmp(159,0) = widetemp.range(159,0);
      Output_1.write(out_tmp);
    }
  }
}

void outer_product2(//gradient_t gradient[MAX_HEIGHT][MAX_WIDTH],
    		       hls::stream<databus_t> &Input_1,
		       hls::stream<databus_t> &Input_2,
		       hls::stream<databus_t> &Input_3,
		   hls::stream<bit160> &Output_1
			)

{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Input_2
#pragma HLS interface ap_hs port=Input_3
#pragma HLS interface ap_hs port=Output_1
  OUTER_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    OUTER_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      //gradient_t grad = gradient[r][c];
      gradient_t grad;
      databus_t temp_x,temp_y,temp_z;
      temp_x = Input_1.read();
      temp_y = Input_2.read();
      temp_z = Input_3.read();
      grad.x.range(31,0) = temp_x.range(31,0);
      grad.y.range(31,0) = temp_y.range(31,0);
      grad.z.range(31,0) = temp_z.range(31,0);
      outer_pixel_t x = (outer_pixel_t) grad.x;
      outer_pixel_t y = (outer_pixel_t) grad.y;
      outer_pixel_t z = (outer_pixel_t) grad.z;
      outer_t out;
      out.val[3] = (z*z);
      out.val[4] = (x*z);
      out.val[5] = (y*z);
      //Output_1[r][c] = out;
      widebus_t widetemp;
      widetemp.range(47,0) = out.val[3].range(47,0);
      widetemp.range(95,48) = out.val[4].range(47,0);
      widetemp.range(143,96) = out.val[5].range(47,0);
      widetemp.range(287,144) = 0;
      //Output_1.write(widetemp);

      bit160 out_tmp;
      out_tmp(159,0) = widetemp.range(159,0);
      Output_1.write(out_tmp);
    }
  }
}
// tensor weight
void tensor_weight_y1(
		hls::stream<bit160> &Input_1,
		hls::stream<bit160> &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
  hls::LineBuffer<3,MAX_WIDTH,outer_half_t> buf;
  const pixel_t TENSOR_FILTER[] = {0.3243, 0.3513, 0.3243};
  TENSOR_WEIGHT_Y_OUTER: for(int r=0; r<MAX_HEIGHT+1; r++)
  {
    TENSOR_WEIGHT_Y_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1

      outer_half_t tmp;
      #pragma HLS data_pack variable=tmp
      #pragma HLS data_pack variable=buf.val[0]
      buf.shift_pixels_up(c);
      if(r<MAX_HEIGHT)
      {
        bit160 widetemp;
        widetemp = Input_1.read();

        tmp.val[0].range(47,0) = widetemp.range(47,0);
        tmp.val[1].range(47,0) = widetemp.range(95,48);
        tmp.val[2].range(47,0) = widetemp.range(143,96);
      }
      else
      {
        TENSOR_WEIGHT_Y_TMP_INIT: for(int i=0; i<3; i++)
          tmp.val[i] = 0;
      }
      buf.insert_bottom_row(tmp,c);

      tensor_half_t acc;
      TENSOR_WEIGHT_Y_ACC_INIT: for(int k =0; k<3; k++)
        acc.val[k] = 0;

      if (r >= 2 && r < MAX_HEIGHT)
      {
        TENSOR_WEIGHT_Y_TMP_OUTER: for(int i=0; i<3; i++)
        {
          tmp = buf.getval(i,c);
          pixel_t k = TENSOR_FILTER[i];
          TENSOR_WEIGHT_Y_TMP_INNER: for(int component=0; component<3; component++)
          {
            acc.val[component] += tmp.val[component]*k;
          }
        }
      }
      if(r >= 1)
      {
      	bit160 widetemp;
        widetemp.range(47,0) = acc.val[0].range(47,0);
        widetemp.range(95,48) = acc.val[1].range(47,0);
        widetemp.range(143,96) = acc.val[2].range(47,0);
        widetemp.range(159,144) = 0;
        Output_1.write(widetemp);
      }
    }
  }
}

void tensor_weight_y2(
		hls::stream<bit160> &Input_1,
		hls::stream<bit160> &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
  hls::LineBuffer<3,MAX_WIDTH,outer_half_t> buf;
  const pixel_t TENSOR_FILTER[] = {0.3243, 0.3513, 0.3243};
  TENSOR_WEIGHT_Y_OUTER: for(int r=0; r<MAX_HEIGHT+1; r++)
  {
    TENSOR_WEIGHT_Y_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1

      outer_half_t tmp;
      #pragma HLS data_pack variable=tmp
      #pragma HLS data_pack variable=buf.val[0]
      buf.shift_pixels_up(c);
      if(r<MAX_HEIGHT)
      {
        widebus_t widetemp = Input_1.read();
        tmp.val[0].range(47,0) = widetemp.range(47,0);
        tmp.val[1].range(47,0) = widetemp.range(95,48);
        tmp.val[2].range(47,0) = widetemp.range(143,96);
      }
      else
      {
        TENSOR_WEIGHT_Y_TMP_INIT: for(int i=0; i<3; i++)
          tmp.val[i] = 0;
      }
      buf.insert_bottom_row(tmp,c);

      tensor_half_t acc;
      TENSOR_WEIGHT_Y_ACC_INIT: for(int k =0; k<3; k++)
        acc.val[k] = 0;

      if (r >= 2 && r < MAX_HEIGHT)
      {
        TENSOR_WEIGHT_Y_TMP_OUTER: for(int i=0; i<3; i++)
        {
          tmp = buf.getval(i,c);
          pixel_t k = TENSOR_FILTER[i];
          TENSOR_WEIGHT_Y_TMP_INNER: for(int component=0; component<3; component++)
          {
            acc.val[component] += tmp.val[component]*k;
          }
        }
      }
      if(r >= 1)
      {
      	bit160 widetemp;
        widetemp.range(47,0) = acc.val[0].range(47,0);
        widetemp.range(95,48) = acc.val[1].range(47,0);
        widetemp.range(143,96) = acc.val[2].range(47,0);
        widetemp.range(159,144) = 0;
        Output_1.write(widetemp);
      }
    }
  }
}
void tensor_weight_x1(hls::stream<bit160> &Input_1,
		     hls::stream<bit160> &Output_1,
		     hls::stream<bit160> &Output_2)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Output_2
  hls::Window<1,3,tensor_half_t> buf;
  const pixel_t TENSOR_FILTER[] = {0.3243, 0.3513, 0.3243};
  TENSOR_WEIGHT_X_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    TENSOR_WEIGHT_X_INNER: for(int c=0; c<MAX_WIDTH+1; c++)
    {
      #pragma HLS pipeline II=1
      buf.shift_pixels_left();
      tensor_half_t tmp;
      if(c<MAX_WIDTH)
      {
        //widebus_t widetemp = Input_1.read();
    	bit160 widetemp = Input_1.read();
        tmp.val[0].range(47,0) = widetemp.range(47,0);
        tmp.val[1].range(47,0) = widetemp.range(95,48);
        tmp.val[2].range(47,0) = widetemp.range(143,96);
      }
      else
      {
        TENSOR_WEIGHT_X_TMP_INIT: for(int i=0; i<3; i++)
          tmp.val[i] = 0;
      }
      buf.insert_pixel(tmp,0,2);

      tensor_half_t acc;
      TENSOR_WEIGHT_X_ACC_INIT: for(int k =0; k<3; k++)
        acc.val[k] = 0;
      if (c >= 2 && c < MAX_WIDTH)
      {
        TENSOR_WEIGHT_X_TMP_OUTER: for(int i=0; i<3; i++)
        {
          tmp = buf.getval(0,i);
          TENSOR_WEIGHT_X_TMP_INNER: for(int component=0; component<3; component++)
          {
            acc.val[component] += tmp.val[component]*TENSOR_FILTER[i];
          }
        }
      }
      if(c>=1)
      {
      	bit160 widetemp;
        widetemp.range(47,0) = acc.val[0].range(47,0);
        widetemp.range(95,48) = acc.val[1].range(47,0);
        widetemp.range(143,96) = acc.val[2].range(47,0);
        widetemp.range(159,144) = 0;
        Output_1.write(widetemp);
        Output_2.write(widetemp);
      }
    }
  }
}

void tensor_weight_x2(hls::stream<bit160> &Input_1,
		     hls::stream<bit160> &Output_1,
		     hls::stream<bit160> &Output_2)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Output_2
  hls::Window<1,3,tensor_half_t> buf;
  const pixel_t TENSOR_FILTER[] = {0.3243, 0.3513, 0.3243};
  TENSOR_WEIGHT_X_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    TENSOR_WEIGHT_X_INNER: for(int c=0; c<MAX_WIDTH+1; c++)
    {
      #pragma HLS pipeline II=1
      buf.shift_pixels_left();
      tensor_half_t tmp;
      if(c<MAX_WIDTH)
      {
       // widebus_t widetemp = Input_1.read();
    	bit160 widetemp = Input_1.read();
        tmp.val[0].range(47,0) = widetemp.range(47,0);
        tmp.val[1].range(47,0) = widetemp.range(95,48);
        tmp.val[2].range(47,0) = widetemp.range(143,96);
      }
      else
      {
        TENSOR_WEIGHT_X_TMP_INIT: for(int i=0; i<3; i++)
          tmp.val[i] = 0;
      }
      buf.insert_pixel(tmp,0,2);

      tensor_half_t acc;
      TENSOR_WEIGHT_X_ACC_INIT: for(int k =0; k<3; k++)
        acc.val[k] = 0;
      if (c >= 2 && c < MAX_WIDTH)
      {
        TENSOR_WEIGHT_X_TMP_OUTER: for(int i=0; i<3; i++)
        {
          tmp = buf.getval(0,i);
          TENSOR_WEIGHT_X_TMP_INNER: for(int component=0; component<3; component++)
          {
            acc.val[component] += tmp.val[component]*TENSOR_FILTER[i];
          }
        }
      }
      if(c>=1)
      {
      	bit160 widetemp;
        widetemp.range(47,0) = acc.val[0].range(47,0);
        widetemp.range(95,48) = acc.val[1].range(47,0);
        widetemp.range(143,96) = acc.val[2].range(47,0);
        widetemp.range(159,144) = 0;
        Output_1.write(widetemp);
        Output_2.write(widetemp);
      }
    }
  }
}
void flow_calc_1(
		hls::stream<bit160> &Input_1,
		hls::stream<bit160> &Input_2,
		hls::stream<stdio_t> &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Input_2
  static float_t buf;
  FLOW_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    FLOW_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      tensor_t tmp_tensor;
      bit160 widetemp = Input_1.read();
      tmp_tensor.val[0].range(47,0) = widetemp.range(47,0);
      tmp_tensor.val[1].range(47,0) = widetemp.range(95,48);
      tmp_tensor.val[2].range(47,0) = widetemp.range(143,96);

      widetemp = Input_2.read();
      tmp_tensor.val[3].range(47,0) = widetemp.range(47,0);
      tmp_tensor.val[4].range(47,0) = widetemp.range(95,48);
      tmp_tensor.val[5].range(47,0) = widetemp.range(143,96);

      if(r>=2 && r<MAX_HEIGHT-2 && c>=2 && c<MAX_WIDTH-2)
      {
	      calc_pixel_t t1 = (calc_pixel_t) tmp_tensor.val[0];
	      calc_pixel_t t2 = (calc_pixel_t) tmp_tensor.val[1];
	      calc_pixel_t t4 = (calc_pixel_t) tmp_tensor.val[2];
	      calc_pixel_t t5 = (calc_pixel_t) tmp_tensor.val[4];
	      calc_pixel_t t6 = (calc_pixel_t) tmp_tensor.val[5];
	      calc_pixel_t denom = t1*t2-t4*t4;
	      calc_pixel_t numer0 = t6*t4-t5*t2;

	      if(denom != 0)
              {
	          buf =(float) numer0 / (float) denom;
        	  //buf = (ap_fixed<64,56>) numer0 / (ap_fixed<64,56>) denom;
	      }
	      else
	      {
		      buf = 0;
	      }
      }
      else
      {
        buf = 0;
      }
      stdio_t tmpframe;
      vel_pixel_t tmpvel;
      tmpvel = (vel_pixel_t)buf;
      tmpframe(31,0) = tmpvel(31,0);
      Output_1.write(tmpframe);
    }
  }
}
void flow_calc_2(
		hls::stream<bit160> &Input_1,
		hls::stream<bit160> &Input_2,
		hls::stream<stdio_t> &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Input_2
  static float buf;
  FLOW_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    FLOW_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      tensor_t tmp_tensor;
      bit160 widetemp = Input_1.read();
      tmp_tensor.val[0].range(47,0) = widetemp.range(47,0);
      tmp_tensor.val[1].range(47,0) = widetemp.range(95,48);
      tmp_tensor.val[2].range(47,0) = widetemp.range(143,96);

      widetemp = Input_2.read();
      tmp_tensor.val[3].range(47,0) = widetemp.range(47,0);
      tmp_tensor.val[4].range(47,0) = widetemp.range(95,48);
      tmp_tensor.val[5].range(47,0) = widetemp.range(143,96);

      if(r>=2 && r<MAX_HEIGHT-2 && c>=2 && c<MAX_WIDTH-2)
      {
	      calc_pixel_t t1 = (calc_pixel_t) tmp_tensor.val[0];
	      calc_pixel_t t2 = (calc_pixel_t) tmp_tensor.val[1];
	      calc_pixel_t t4 = (calc_pixel_t) tmp_tensor.val[2];
	      calc_pixel_t t5 = (calc_pixel_t) tmp_tensor.val[4];
	      calc_pixel_t t6 = (calc_pixel_t) tmp_tensor.val[5];
	      calc_pixel_t denom = t1*t2-t4*t4;
	      calc_pixel_t numer1 = t5*t4-t6*t1;

	      if(denom != 0)
              {
        	  buf = (float) numer1 / (float) denom;
        	  //buf = (ap_fixed<64,56>) numer1 / (ap_fixed<64,56>) denom;
	      }
	      else
	      {
		      buf = 0;
	      }
      }
      else
      {
        buf = 0;
      }
      stdio_t tmpframe;
      vel_pixel_t tmpvel;
      tmpvel = (vel_pixel_t)buf;
      tmpframe(31,0) = tmpvel(31,0);
      Output_1.write(tmpframe);
    }
  }
}
void output_fun(hls::stream<stdio_t> &Input_1,
		hls::stream<stdio_t> &Input_2,
		hls::stream< ap_uint<128> > &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
#pragma HLS interface ap_hs port=Input_2
	while (Input_1.empty());
	ap_uint<128> tmp =10;
	tmp(31,0) = Input_1.read();
	if (tmp !=10) Output_1.write(MAX_HEIGHT*MAX_WIDTH/2+1);

	tmp(63, 32)  = Input_2.read();
	tmp(95, 64)  = Input_1.read();
	tmp(127, 96) = Input_2.read();
	Output_1.write(tmp);


	OUT_CONVERT: for (int i = 0; i < MAX_HEIGHT*MAX_WIDTH/2-1; i++)
	{
		#pragma HLS pipeline II = 2
		tmp(31,   0) = Input_1.read();
		tmp(63,  32) = Input_2.read();
		tmp(95,  64) = Input_1.read();
		tmp(127, 96) = Input_2.read();
		Output_1.write(tmp);
	}
}
void optical_flow(//frames_t   frames[MAX_HEIGHT][MAX_WIDTH],
		 hls::stream< ap_uint<128> > &Input_1,
                  //velocity_t outputs[MAX_HEIGHT][MAX_WIDTH])
		 hls::stream< ap_uint<128> > &Output_1)
{

#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
  #pragma HLS DATAFLOW

  // FIFOs connecting the stages

  //static pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH];
  static hls::stream<databus_t> gradient_x;
  #pragma HLS STREAM variable=gradient_x depth=default_depth

  //static pixel_t gradient_y[MAX_HEIGHT][MAX_WIDTH];
  static hls::stream<databus_t> gradient_y;
  #pragma HLS STREAM variable=gradient_y depth=default_depth

  //static pixel_t gradient_z[MAX_HEIGHT][MAX_WIDTH];
  static hls::stream<databus_t> gradient_z;
  #pragma HLS STREAM variable=gradient_z depth=max_width*4

  //static gradient_t y_filtered[MAX_HEIGHT][MAX_WIDTH];
  //#pragma HLS STREAM variable=y_filtered depth=default_depth
  static hls::stream<databus_t> y_filtered_x;
  static hls::stream<databus_t> y_filtered_y;
  static hls::stream<databus_t> y_filtered_z;
  #pragma HLS STREAM variable=y_filtered_x depth=default_depth
  #pragma HLS STREAM variable=y_filtered_y depth=default_depth
  #pragma HLS STREAM variable=y_filtered_z depth=default_depth

  //static gradient_t filtered_gradient[MAX_HEIGHT][MAX_WIDTH];
  //#pragma HLS STREAM variable=filtered_gradient depth=default_depth
  static hls::stream<databus_t> filtered_gradient_x1;
  static hls::stream<databus_t> filtered_gradient_y1;
  static hls::stream<databus_t> filtered_gradient_z1;
  #pragma HLS STREAM variable=filtered_gradient_x1 depth=default_depth
  #pragma HLS STREAM variable=filtered_gradient_y1 depth=default_depth
  #pragma HLS STREAM variable=filtered_gradient_z1 depth=default_depth

  static hls::stream<databus_t> filtered_gradient_x2;
  static hls::stream<databus_t> filtered_gradient_y2;
  static hls::stream<databus_t> filtered_gradient_z2;
  #pragma HLS STREAM variable=filtered_gradient_x2 depth=default_depth
  #pragma HLS STREAM variable=filtered_gradient_y2 depth=default_depth
  #pragma HLS STREAM variable=filtered_gradient_z2 depth=default_depth

  static hls::stream<bit160> out_product1;
  #pragma HLS STREAM variable=out_product1 depth=default_depth
  static hls::stream<bit160> out_product2;
  #pragma HLS STREAM variable=out_product2 depth=default_depth
  static hls::stream<bit160> tensor_y1;
  #pragma HLS STREAM variable=tensor_y1 depth=default_depth
  static hls::stream<bit160> tensor_y2;
  #pragma HLS STREAM variable=tensor_y2 depth=default_depth
  static hls::stream<bit160> tx1_out;
  #pragma HLS STREAM variable=tx1_out depth=default_depth
  static hls::stream<bit160> tx2_out;
  #pragma HLS STREAM variable=tx2_out depth=default_depth
  static hls::stream<bit160> tx1_out1;
  #pragma HLS STREAM variable=tx1_out1 depth=default_depth
  static hls::stream<bit160> tx2_out1;
  #pragma HLS STREAM variable=tx2_out1 depth=default_depth

  static hls::stream<databus_t> out_product1a;
  #pragma HLS STREAM variable=out_product1a depth=default_depth
  static hls::stream<databus_t> out_product2a;
  #pragma HLS STREAM variable=out_product2a depth=default_depth
  static hls::stream<databus_t> tensor_y1a;
  #pragma HLS STREAM variable=tensor_y1a depth=default_depth
  static hls::stream<databus_t> tensor_y2a;
  #pragma HLS STREAM variable=tensor_y2a depth=default_depth
  static hls::stream<databus_t> tx1_outa;
  #pragma HLS STREAM variable=tx1_outa depth=default_depth
  static hls::stream<databus_t> tx2_outa;
  #pragma HLS STREAM variable=tx2_outa depth=default_depth
  static hls::stream<databus_t> tx1_out1a;
  #pragma HLS STREAM variable=tx1_out1a depth=default_depth
  static hls::stream<databus_t> tx2_out1a;
  #pragma HLS STREAM variable=tx2_out1a depth=default_depth

  static hls::stream<databus_t> frame3_a;
  #pragma HLS STREAM variable=frame3_a depth=default_depth
  static hls::stream<databus_t> frame1_a;
  #pragma HLS STREAM variable=frame1_a depth=default_depth
  static hls::stream<databus_t> frame2_a;
  #pragma HLS STREAM variable=frame2_a depth=default_depth
  static hls::stream<databus_t> frame3_b;
  #pragma HLS STREAM variable=frame3_b depth=default_depth
  static hls::stream<databus_t> frame4_a;
  #pragma HLS STREAM variable=frame4_a depth=default_depth
  static hls::stream<databus_t> frame5_a;
  #pragma HLS STREAM variable=frame5_a depth=default_depth
//  static input_t frame3_b[MAX_HEIGHT][MAX_WIDTH];
//  #pragma HLS STREAM variable=frame3_b depth=default_depth
  static hls::stream<stdio_t> in;
  #pragma HLS STREAM variable=in depth=default_depth
  static hls::stream<stdio_t> in1;
  #pragma HLS STREAM variable=in1 depth=default_depth
  // stream in and organize the inputs
  //
  // compute
  //input_unpack(Input_1,frame1_a,frame2_a,frame3_a,frame3_b,frame4_a,frame5_a);
  //gradient_xy_calc(frame3_a, gradient_x, gradient_y); //17 + 32 + 32
  //gradient_z_calc(frame1_a, frame2_a, frame3_b, frame4_a, frame5_a, gradient_z); // 17 * 5 + 32
  gradient_xyz_calc(Input_1, gradient_x, gradient_y,gradient_z);
  gradient_weight_y_1(gradient_x, y_filtered_x); //y_filtered); // 32 * 3 + 32 * 3
  gradient_weight_y_2(gradient_y, y_filtered_y); //y_filtered); // 32 * 3 + 32 * 3
  gradient_weight_y_3(gradient_z, y_filtered_z); //y_filtered); // 32 * 3 + 32 * 3
  gradient_weight_x1(y_filtered_x,
		    filtered_gradient_x1,
	            filtered_gradient_x2); // 32 * 3 + 32 * 3
  gradient_weight_x2(y_filtered_y,
		     filtered_gradient_y1,
	             filtered_gradient_y2); // 32 * 3 + 32 * 3
  gradient_weight_x3(y_filtered_z,
		     filtered_gradient_z1,
	            filtered_gradient_z2); // 32 * 3 + 32 * 3
  outer_product1(filtered_gradient_x1, filtered_gradient_y1, filtered_gradient_z1, out_product1); // 32 * 3 + 3 * 48
  outer_product2(filtered_gradient_x2, filtered_gradient_y2, filtered_gradient_z2, out_product2); // 32 * 3 + 3 * 48
  tensor_weight_y1(out_product1,tensor_y1); // 6 * 48 + 6 * 48
  tensor_weight_y2(out_product2,tensor_y2); // 6 * 48 + 6 * 48
  tensor_weight_x1(tensor_y1,
		   tx1_out,tx1_out1); // 6 * 48 + 6 * 48
  tensor_weight_x2(tensor_y2,
		   tx2_out,tx2_out1); // 6 * 48 + 6 * 48
  flow_calc_1(tx1_out, tx2_out, in); // 6 * 48 + 32 * 2
  flow_calc_2(tx1_out1, tx2_out1,in1); // 6 * 48 + 32 * 2
  output_fun(in,in1,Output_1);
  printf("sbbbbb\n");
}

/*
void data_gen(
		 hls::stream< ap_uint<128> > &Output_1)
{
#pragma HLS interface ap_hs port=Output_1


#include "/home/ylxiao/ws_182/F200525_git/optical_hls/optical_hls/input_data.h"
#pragma HLS ARRAY_PARTITION variable=input_data cyclic factor=2 dim=1

	int i;
	for (i=0; i<446464; i++)
	{
#pragma HLS pipeline II=1
		bit128 tmp;
		tmp(127,96) = input_data[i*4];
		tmp(95,64) = input_data[i*4+1];
		tmp(63,32) = input_data[i*4+2];
		tmp(31,0) = input_data[i*4+3];

		Output_1.write(tmp);
	}
}
*/
void optical_flow_pseudo(
		hls::stream< ap_uint<128> > &Input_1,
		hls::stream< ap_uint<128> > &Output_1)
{
#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1


	int sum = 0;
	bit128 in_tmp1 = 0;
	bit128 in_tmp2 = 0;
	int i=0;

	PIPE_IN: for(i=0; i<6220; i++)
	{
#pragma HLS PIPELINE II=1
		in_tmp1 = Input_1.read();
		sum+=(unsigned int)in_tmp1;
	}

	if(i==6220)
	{
		Output_1.write(MAX_HEIGHT*MAX_WIDTH/2+1);
	}

	PIPE_II2: for(i=0; i<(446464-6220)/2; i++)
	{
#pragma HLS PIPELINE II=2
		in_tmp1 = Input_1.read();
		in_tmp2 = Input_1.read();
		sum+=(unsigned int)in_tmp1;
		Output_1.write(sum);
	}

	PIPE_OUT: for(i=0; i<6220/2; i++)
	{
#pragma HLS PIPELINE II=2
		sum = sum << 1;
		sum = sum << 1;
		Output_1.write(sum);
	}



}

void gradient_xy_calc(//input_t frame[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &frame,
    //pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &gradient_x,
    //pixel_t gradient_y[MAX_HEIGHT][MAX_WIDTH])
    hls::stream<databus_t> &gradient_y)
{
  // our own line buffer
  static pixel_t buf[5][MAX_WIDTH];
  #pragma HLS array_partition variable=buf complete dim=1

  // small buffer
  pixel_t smallbuf[5];
  #pragma HLS array_partition variable=smallbuf complete dim=0

  // window buffer
  hls::Window<5,5,input_t> window;

  const int GRAD_WEIGHTS[] =  {1,-8,0,8,-1};

  GRAD_XY_OUTER: for(int r=0; r<MAX_HEIGHT+2; r++)
  {
    GRAD_XY_INNER: for(int c=0; c<MAX_WIDTH+2; c++)
    {
      #pragma HLS pipeline II=1
      // read out values from current line buffer
      for (int i = 0; i < 4; i ++ )
        smallbuf[i] = buf[i+1][c];
      // the new value is either 0 or read from frame
      if (r<MAX_HEIGHT && c<MAX_WIDTH){
        //smallbuf[4] = (pixel_t)(frame[r][c]);
	databus_t tmpread = frame.read();
        input_t tmpin;
	tmpin(16,0) = tmpread(16,0);
        smallbuf[4] = (pixel_t)(tmpin);
      }
      else if (c < MAX_WIDTH)
        smallbuf[4] = 0;
      // update line buffer
      if(r<MAX_HEIGHT && c<MAX_WIDTH)
      {
        for (int i = 0; i < 4; i ++ )
          buf[i][c] = smallbuf[i];
        buf[4][c] = smallbuf[4];
      }
      else if(c<MAX_WIDTH)
      {
        for (int i = 0; i < 4; i ++ )
          buf[i][c] = smallbuf[i];
        buf[4][c] = smallbuf[4];
      }

      // manage window buffer
      if(r<MAX_HEIGHT && c<MAX_WIDTH)
      {
        window.shift_pixels_left();

        for (int i = 0; i < 5; i ++ )
          window.insert_pixel(smallbuf[i],i,4);
      }
      else
      {
        window.shift_pixels_left();
        window.insert_pixel(0,0,4);
        window.insert_pixel(0,1,4);
        window.insert_pixel(0,2,4);
        window.insert_pixel(0,3,4);
        window.insert_pixel(0,4,4);
      }

      // compute gradient
      pixel_t x_grad = 0;
      databus_t temp = 0;
      pixel_t y_grad = 0;
      if(r>=4 && r<MAX_HEIGHT && c>=4 && c<MAX_WIDTH)
      {
        GRAD_XY_XYGRAD: for(int i=0; i<5; i++)
        {
          x_grad += window.getval(2,i)*GRAD_WEIGHTS[i];
          y_grad += window.getval(i,2)*GRAD_WEIGHTS[i];
        }
        //gradient_x[r-2][c-2] = x_grad/12;
	x_grad = x_grad/12;
	temp.range(31,0) = x_grad.range(31,0);
	gradient_x.write(temp);
        //gradient_y[r-2][c-2] = y_grad/12;
	y_grad = y_grad/12;
	temp.range(31,0) = y_grad.range(31,0);
	gradient_y.write(temp);
      }
      else if(r>=2 && c>=2)
      {
        //gradient_x[r-2][c-2] = 0;
	gradient_x.write(0);
        //gradient_y[r-2][c-2] = 0;
	gradient_y.write(0);
      }
    }
  }
}

// calculate gradient in the z direction
void gradient_z_calc(//input_t frame1[MAX_HEIGHT][MAX_WIDTH],
   // input_t frame2[MAX_HEIGHT][MAX_WIDTH],
    //input_t frame3[MAX_HEIGHT][MAX_WIDTH],
    //input_t frame4[MAX_HEIGHT][MAX_WIDTH],
    //input_t frame5[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &frame1,
    hls::stream<databus_t> &frame2,
    hls::stream<databus_t> &frame3,
    hls::stream<databus_t> &frame4,
    hls::stream<databus_t> &frame5,
    //pixel_t gradient_z[MAX_HEIGHT][MAX_WIDTH])
    hls::stream<databus_t> &gradient_z)
{
  const int GRAD_WEIGHTS[] =  {1,-8,0,8,-1};
  GRAD_Z_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    GRAD_Z_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      pixel_t temp;
      input_t frame1_tmp,frame2_tmp,frame3_tmp,frame4_tmp,frame5_tmp;
      databus_t data;
      data = frame1.read();
      frame1_tmp(16,0) = data(16,0);
      data = frame2.read();
      frame2_tmp(16,0) = data(16,0);
      data = frame3.read();
      frame3_tmp(16,0) = data(16,0);
      data = frame4.read();
      frame4_tmp(16,0) = data(16,0);
      data = frame5.read();
      frame5_tmp(16,0) = data(16,0);
      temp =((pixel_t)(frame1_tmp*GRAD_WEIGHTS[0]
                        + frame2_tmp*GRAD_WEIGHTS[1]
                        + frame3_tmp*GRAD_WEIGHTS[2]
                        + frame4_tmp*GRAD_WEIGHTS[3]
                        + frame5_tmp*GRAD_WEIGHTS[4]))/12;
      data.range(31,0) = temp.range(31,0);
      gradient_z.write(data);

    }
  }
}

// average the gradient in y direction
void gradient_weight_y(//pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &gradient_x,
    //pixel_t gradient_y[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &gradient_y,
    //pixel_t gradient_z[MAX_HEIGHT][MAX_WIDTH],
    hls::stream<databus_t> &gradient_z,
    //gradient_t filt_grad[MAX_HEIGHT][MAX_WIDTH])
    hls::stream<databus_t> &filt_grad_x,
    hls::stream<databus_t> &filt_grad_y,
    hls::stream<databus_t> &filt_grad_z)
{
  hls::LineBuffer<7,MAX_WIDTH,gradient_t> buf;

  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_Y_OUTER: for(int r=0; r<MAX_HEIGHT+3; r++)
  {
    GRAD_WEIGHT_Y_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      #pragma HLS dependence variable=buf inter false

      if(r<MAX_HEIGHT)
      {
        buf.shift_pixels_up(c);
        gradient_t tmp;
	databus_t temp;
        //tmp.x = gradient_x[r][c];
	temp = gradient_x.read();
	//tmp.x.range(31,0) = temp.range(31,0);
	tmp.x(31,0) = temp(31,0);

        //tmp.y = gradient_y[r][c];
	temp = gradient_y.read();
	tmp.y(31,0) = temp(31,0);
        //tmp.z = gradient_z[r][c];
	temp = gradient_z.read();
	tmp.z(31,0) = temp(31,0);
        buf.insert_bottom_row(tmp,c);
      }
      else
      {
        buf.shift_pixels_up(c);
        gradient_t tmp;
        tmp.x = 0;
        tmp.y = 0;
        tmp.z = 0;
        buf.insert_bottom_row(tmp,c);
      }

      gradient_t acc;
      databus_t temp;
      acc.x = 0;
      acc.y = 0;
      acc.z = 0;
      if(r >= 6 && r<MAX_HEIGHT)
      {
        GRAD_WEIGHT_Y_ACC: for(int i=0; i<7; i++)
        {
          acc.x += buf.getval(i,c).x*GRAD_FILTER[i];
          acc.y += buf.getval(i,c).y*GRAD_FILTER[i];
          acc.z += buf.getval(i,c).z*GRAD_FILTER[i];
        }
        //filt_grad[r-3][c] = acc;
	temp.range(31,0) = acc.x.range(31,0);
	filt_grad_x.write(temp);
	temp.range(31,0) = acc.y.range(31,0);
	filt_grad_y.write(temp);
	temp.range(31,0) = acc.z.range(31,0);
	filt_grad_z.write(temp);
      }
      else if(r>=3)
      {
        //filt_grad[r-3][c] = acc;
	temp.range(31,0) = acc.x.range(31,0);
	filt_grad_x.write(temp);
	temp.range(31,0) = acc.y.range(31,0);
	filt_grad_y.write(temp);
	temp.range(31,0) = acc.z.range(31,0);
	filt_grad_z.write(temp);
      }
    }
  }
}

// average gradient in the x direction
void gradient_weight_x(//gradient_t y_filt[MAX_HEIGHT][MAX_WIDTH],
		       hls::stream<databus_t> &y_filt_x,
		       hls::stream<databus_t> &y_filt_y,
		       hls::stream<databus_t> &y_filt_z,
                       //gradient_t filt_grad[MAX_HEIGHT][MAX_WIDTH])
    		       hls::stream<databus_t> &filt_grad_x,
		       hls::stream<databus_t> &filt_grad_y,
		       hls::stream<databus_t> &filt_grad_z)
{
  hls::Window<1,7,gradient_t> buf;
  const pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903, 0.1869, 0.133, 0.0755};
  GRAD_WEIGHT_X_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    GRAD_WEIGHT_X_INNER: for(int c=0; c<MAX_WIDTH+3; c++)
    {
      #pragma HLS pipeline II=1
      buf.shift_pixels_left();
      gradient_t tmp;
      databus_t temp;
      if(c<MAX_WIDTH)
      {
        //tmp = y_filt[r][c];
	temp = y_filt_x.read();
	tmp.x.range(31,0) = temp.range(31,0);
	temp = y_filt_y.read();
	tmp.y.range(31,0) = temp.range(31,0);
	temp = y_filt_z.read();
	tmp.z.range(31,0) = temp.range(31,0);
      }
      else
      {
        tmp.x = 0;
        tmp.y = 0;
        tmp.z = 0;
      }
      buf.insert_pixel(tmp,0,6);

      gradient_t acc;
      acc.x = 0;
      acc.y = 0;
      acc.z = 0;
      if(c >= 6 && c<MAX_WIDTH)
      {
        GRAD_WEIGHT_X_ACC: for(int i=0; i<7; i++)
        {
          acc.x += buf.getval(0,i).x*GRAD_FILTER[i];
          acc.y += buf.getval(0,i).y*GRAD_FILTER[i];
          acc.z += buf.getval(0,i).z*GRAD_FILTER[i];
        }
        //filt_grad[r][c-3] = acc;
	temp.range(31,0) = acc.x.range(31,0);
	filt_grad_x.write(temp);
	temp.range(31,0) = acc.y.range(31,0);
	filt_grad_y.write(temp);
	temp.range(31,0) = acc.z.range(31,0);
	filt_grad_z.write(temp);
      }
      else if(c>=3)
      {
       // filt_grad[r][c-3] = acc;
	temp.range(31,0) = acc.x.range(31,0);
	filt_grad_x.write(temp);
	temp.range(31,0) = acc.y.range(31,0);
	filt_grad_y.write(temp);
	temp.range(31,0) = acc.z.range(31,0);
	filt_grad_z.write(temp);
      }
    }
  }
}

// outer product
void outer_product(//gradient_t gradient[MAX_HEIGHT][MAX_WIDTH],
    		       hls::stream<databus_t> &gradient_x,
		       hls::stream<databus_t> &gradient_y,
		       hls::stream<databus_t> &gradient_z,
     //outer_t outer_product[MAX_HEIGHT][MAX_WIDTH])
		   hls::stream<widebus_t> &outer_product)
{
  OUTER_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    OUTER_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      //gradient_t grad = gradient[r][c];
      gradient_t grad;
      databus_t temp_x,temp_y,temp_z;
      temp_x = gradient_x.read();
      temp_y = gradient_y.read();
      temp_z = gradient_z.read();
      grad.x.range(31,0) = temp_x.range(31,0);
      grad.y.range(31,0) = temp_y.range(31,0);
      grad.z.range(31,0) = temp_z.range(31,0);
      outer_pixel_t x = (outer_pixel_t) grad.x;
      outer_pixel_t y = (outer_pixel_t) grad.y;
      outer_pixel_t z = (outer_pixel_t) grad.z;
      outer_t out;
      out.val[0] = (x*x);
      out.val[1] = (y*y);
      out.val[2] = (z*z);
      out.val[3] = (x*y);
      out.val[4] = (x*z);
      out.val[5] = (y*z);
      //outer_product[r][c] = out;
      widebus_t widetemp;
      widetemp.range(47,0) = out.val[0].range(47,0);
      widetemp.range(95,48) = out.val[1].range(47,0);
      widetemp.range(143,96) = out.val[2].range(47,0);
      widetemp.range(191,144) = out.val[3].range(47,0);
      widetemp.range(239,192) = out.val[4].range(47,0);
      widetemp.range(287,240) = out.val[5].range(47,0);
      outer_product.write(widetemp);
    }
  }
}

// tensor weight
void tensor_weight_y(//outer_t outer[MAX_HEIGHT][MAX_WIDTH],
		hls::stream<widebus_t> &outer,
                //     tensor_t tensor_y[MAX_HEIGHT][MAX_WIDTH])
		hls::stream<widebus_t> &tensor_y)
{
  hls::LineBuffer<3,MAX_WIDTH,outer_t> buf;
  const pixel_t TENSOR_FILTER[] = {0.3243, 0.3513, 0.3243};
  TENSOR_WEIGHT_Y_OUTER: for(int r=0; r<MAX_HEIGHT+1; r++)
  {
    TENSOR_WEIGHT_Y_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1

      outer_t tmp;
      #pragma HLS data_pack variable=tmp
      #pragma HLS data_pack variable=buf.val[0]
      buf.shift_pixels_up(c);
      if(r<MAX_HEIGHT)
      {
        widebus_t widetemp = outer.read();
	//tmp = outer[r][c];
        tmp.val[0].range(47,0) = widetemp.range(47,0);
        tmp.val[1].range(47,0) = widetemp.range(95,48);
        tmp.val[2].range(47,0) = widetemp.range(143,96);
        tmp.val[3].range(47,0) = widetemp.range(191,144);
        tmp.val[4].range(47,0) = widetemp.range(239,192);
        tmp.val[5].range(47,0) = widetemp.range(287,240);
      }
      else
      {
        TENSOR_WEIGHT_Y_TMP_INIT: for(int i=0; i<6; i++)
          tmp.val[i] = 0;
      }
      buf.insert_bottom_row(tmp,c);

      tensor_t acc;
      TENSOR_WEIGHT_Y_ACC_INIT: for(int k =0; k<6; k++)
        acc.val[k] = 0;

      if (r >= 2 && r < MAX_HEIGHT)
      {
        TENSOR_WEIGHT_Y_TMP_OUTER: for(int i=0; i<3; i++)
        {
          tmp = buf.getval(i,c);
          pixel_t k = TENSOR_FILTER[i];
          TENSOR_WEIGHT_Y_TMP_INNER: for(int component=0; component<6; component++)
          {
            acc.val[component] += tmp.val[component]*k;
          }
        }
      }
      if(r >= 1)
      {
        //tensor_y[r-1][c] = acc;
      	widebus_t widetemp;
        widetemp.range(47,0) = acc.val[0].range(47,0);
        widetemp.range(95,48) = acc.val[1].range(47,0);
        widetemp.range(143,96) = acc.val[2].range(47,0);
        widetemp.range(191,144) = acc.val[3].range(47,0);
        widetemp.range(239,192) = acc.val[4].range(47,0);
        widetemp.range(287,240) = acc.val[5].range(47,0);
        tensor_y.write(widetemp);
      }
    }
  }
}

void tensor_weight_x(//tensor_t tensor_y[MAX_HEIGHT][MAX_WIDTH],
		hls::stream<widebus_t> &tensor_y,
              //       tensor_t tensor[MAX_HEIGHT][MAX_WIDTH])
		hls::stream<widebus_t> &tensor)
{
  hls::Window<1,3,tensor_t> buf;
  const pixel_t TENSOR_FILTER[] = {0.3243, 0.3513, 0.3243};
  //const float TENSOR_FILTER[] = {0.3243, 0.3513, 0.3243};
  TENSOR_WEIGHT_X_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    TENSOR_WEIGHT_X_INNER: for(int c=0; c<MAX_WIDTH+1; c++)
    {
      #pragma HLS pipeline II=1
      buf.shift_pixels_left();
      tensor_t tmp;
      if(c<MAX_WIDTH)
      {
        widebus_t widetemp = tensor_y.read();
        //tmp = tensor_y[r][c];
        tmp.val[0].range(47,0) = widetemp.range(47,0);
        tmp.val[1].range(47,0) = widetemp.range(95,48);
        tmp.val[2].range(47,0) = widetemp.range(143,96);
        tmp.val[3].range(47,0) = widetemp.range(191,144);
        tmp.val[4].range(47,0) = widetemp.range(239,192);
        tmp.val[5].range(47,0) = widetemp.range(287,240);
      }
      else
      {
        TENSOR_WEIGHT_X_TMP_INIT: for(int i=0; i<6; i++)
          tmp.val[i] = 0;
      }
      buf.insert_pixel(tmp,0,2);

      tensor_t acc;
      TENSOR_WEIGHT_X_ACC_INIT: for(int k =0; k<6; k++)
        acc.val[k] = 0;
      if (c >= 2 && c < MAX_WIDTH)
      {
        TENSOR_WEIGHT_X_TMP_OUTER: for(int i=0; i<3; i++)
        {
          tmp = buf.getval(0,i);
          TENSOR_WEIGHT_X_TMP_INNER: for(int component=0; component<6; component++)
          {
            acc.val[component] += tmp.val[component]*TENSOR_FILTER[i];
          }
        }
      }
      if(c>=1)
      {
        //tensor[r][c-1] = acc;
      	widebus_t widetemp;
        widetemp.range(47,0) = acc.val[0].range(47,0);
        widetemp.range(95,48) = acc.val[1].range(47,0);
        widetemp.range(143,96) = acc.val[2].range(47,0);
        widetemp.range(191,144) = acc.val[3].range(47,0);
        widetemp.range(239,192) = acc.val[4].range(47,0);
        widetemp.range(287,240) = acc.val[5].range(47,0);
        tensor.write(widetemp);
      }
    }
  }
}

// compute output flow
void flow_calc(//tensor_t tensors[MAX_HEIGHT][MAX_WIDTH],
		hls::stream<widebus_t> &tensor,
               //velocity_t outputs[MAX_HEIGHT][MAX_WIDTH])
		hls::stream<bit64> &flo)
{
  static outer_pixel_t buf[2];
  FLOW_OUTER: for(int r=0; r<MAX_HEIGHT; r++)
  {
    FLOW_INNER: for(int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1
      //tensor_t tmp_tensor = tensors[r][c];
      tensor_t tmp_tensor;
      widebus_t widetemp = tensor.read();
      tmp_tensor.val[0].range(47,0) = widetemp.range(47,0);
      tmp_tensor.val[1].range(47,0) = widetemp.range(95,48);
      tmp_tensor.val[2].range(47,0) = widetemp.range(143,96);
      tmp_tensor.val[3].range(47,0) = widetemp.range(191,144);
      tmp_tensor.val[4].range(47,0) = widetemp.range(239,192);
      tmp_tensor.val[5].range(47,0) = widetemp.range(287,240);
      if(r>=2 && r<MAX_HEIGHT-2 && c>=2 && c<MAX_WIDTH-2)
      {
	      calc_pixel_t t1 = (calc_pixel_t) tmp_tensor.val[0];
	      calc_pixel_t t2 = (calc_pixel_t) tmp_tensor.val[1];
	      calc_pixel_t t3 = (calc_pixel_t) tmp_tensor.val[2];
	      calc_pixel_t t4 = (calc_pixel_t) tmp_tensor.val[3];
	      calc_pixel_t t5 = (calc_pixel_t) tmp_tensor.val[4];
	      calc_pixel_t t6 = (calc_pixel_t) tmp_tensor.val[5];

	      calc_pixel_t denom = t1*t2-t4*t4;
	      calc_pixel_t numer0 = t6*t4-t5*t2;
	      calc_pixel_t numer1 = t5*t4-t6*t1;

	      if(denom != 0)
        {
          buf[0] = numer0 / denom;
          buf[1] = numer1 / denom;
	      }
	      else
	      {
		      buf[0] = 0;
		      buf[1] = 0;
	      }
      }
      else
      {
        buf[0] = buf[1] = 0;
      }

      //outputs[r][c].x = (vel_pixel_t)buf[0];
      //outputs[r][c].y = (vel_pixel_t)buf[1];
      bit64 tmpframe;
      vel_pixel_t tmpvel;
      tmpvel = (vel_pixel_t)buf[0];
      tmpframe(31,0) = tmpvel(31,0);
      tmpvel = (vel_pixel_t)buf[1];
      tmpframe(63,32) = tmpvel(31,0);
      flo.write(tmpframe);
    }
  }
}
void output_fun2(hls::stream<bit64> &in,
		hls::stream<bit128> &out)
{
	bit128 out_tmp = 0;
	out_tmp(63,0) = in.read();
	out_tmp(127,64) = in.read();

	if(out_tmp!=10)
	{
		out.write(MAX_HEIGHT*MAX_WIDTH/2+1);
	}
	out.write(out_tmp);
	for (int i = 0; i < MAX_HEIGHT*MAX_WIDTH/2-1; i++)
	{
		#pragma HLS pipeline II = 2
		out_tmp = 0;
		out_tmp(63,0) = in.read();
		out_tmp(127,64) = in.read();
		out.write(out_tmp);
	}
}
// top-level kernel function
void optical_flow_mono(//frames_t   frames[MAX_HEIGHT][MAX_WIDTH],
		 hls::stream<bit128> &Input_1,
                  //velocity_t outputs[MAX_HEIGHT][MAX_WIDTH])
		 hls::stream<bit128> &Output_1)
{

#pragma HLS interface ap_hs port=Input_1
#pragma HLS interface ap_hs port=Output_1
  #pragma HLS DATAFLOW

  // FIFOs connecting the stages

  //static pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH];
  static hls::stream<databus_t> gradient_x;
  #pragma HLS STREAM variable=gradient_x depth=default_depth

  //static pixel_t gradient_y[MAX_HEIGHT][MAX_WIDTH];
  static hls::stream<databus_t> gradient_y;
  #pragma HLS STREAM variable=gradient_y depth=default_depth

  //static pixel_t gradient_z[MAX_HEIGHT][MAX_WIDTH];
  static hls::stream<databus_t> gradient_z;
  #pragma HLS STREAM variable=gradient_z depth=max_width*4

  //static gradient_t y_filtered[MAX_HEIGHT][MAX_WIDTH];
  //#pragma HLS STREAM variable=y_filtered depth=default_depth
  static hls::stream<databus_t> y_filtered_x;
  static hls::stream<databus_t> y_filtered_y;
  static hls::stream<databus_t> y_filtered_z;
  #pragma HLS STREAM variable=y_filtered_x depth=default_depth
  #pragma HLS STREAM variable=y_filtered_y depth=default_depth
  #pragma HLS STREAM variable=y_filtered_z depth=default_depth

  //static gradient_t filtered_gradient[MAX_HEIGHT][MAX_WIDTH];
  //#pragma HLS STREAM variable=filtered_gradient depth=default_depth
  static hls::stream<databus_t> filtered_gradient_x;
  static hls::stream<databus_t> filtered_gradient_y;
  static hls::stream<databus_t> filtered_gradient_z;
  #pragma HLS STREAM variable=filtered_gradient_x depth=default_depth
  #pragma HLS STREAM variable=filtered_gradient_y depth=default_depth
  #pragma HLS STREAM variable=filtered_gradient_z depth=default_depth

  // static outer_t out_product[MAX_HEIGHT][MAX_WIDTH];
  //#pragma HLS STREAM variable=out_product depth=default_depth
  //#pragma HLS data_pack variable=out_product
  static hls::stream<widebus_t> out_product;
  #pragma HLS STREAM variable=out_product depth=default_depth


  //static tensor_t tensor_y[MAX_HEIGHT][MAX_WIDTH];
  //#pragma HLS STREAM variable=tensor_y depth=default_depth
  //#pragma HLS data_pack variable=tensor_y
  static hls::stream<widebus_t> tensor_y;
  #pragma HLS STREAM variable=tensor_y depth=default_depth

  //static tensor_t tensor[MAX_HEIGHT][MAX_WIDTH];
  //#pragma HLS STREAM variable=tensor depth=default_depth
  //#pragma HLS data_pack variable=tensor
  static hls::stream<widebus_t> tensor;
  #pragma HLS STREAM variable=tensor depth=default_depth

  // FIFOs for streaming in, just for clarity
//  static input_t frame1_a[MAX_HEIGHT][MAX_WIDTH];
 // #pragma HLS STREAM variable=frame1_a depth=default_depth
//  static input_t frame2_a[MAX_HEIGHT][MAX_WIDTH];
//  #pragma HLS STREAM variable=frame2_a depth=default_depth
 // static input_t frame4_a[MAX_HEIGHT][MAX_WIDTH];
 // #pragma HLS STREAM variable=frame4_a depth=default_depth
 // static input_t frame5_a[MAX_HEIGHT][MAX_WIDTH];
//  #pragma HLS STREAM variable=frame5_a depth=default_depth

  //Need to duplicate frame3 for the two calculations
  //static input_t frame3_a[MAX_HEIGHT][MAX_WIDTH];
  static hls::stream<databus_t> frame3_a;
  #pragma HLS STREAM variable=frame3_a depth=default_depth
  static hls::stream<databus_t> frame1_a;
  #pragma HLS STREAM variable=frame1_a depth=default_depth
  static hls::stream<databus_t> frame2_a;
  #pragma HLS STREAM variable=frame2_a depth=default_depth
  static hls::stream<databus_t> frame3_b;
  #pragma HLS STREAM variable=frame3_b depth=default_depth
  static hls::stream<databus_t> frame4_a;
  #pragma HLS STREAM variable=frame4_a depth=default_depth
  static hls::stream<databus_t> frame5_a;
  #pragma HLS STREAM variable=frame5_a depth=default_depth
//  static input_t frame3_b[MAX_HEIGHT][MAX_WIDTH];
//  #pragma HLS STREAM variable=frame3_b depth=default_depth
  static hls::stream<bit64> in;
  #pragma HLS STREAM variable=in depth=default_depth
  // stream in and organize the inputs
  stdio_t buf;
  FRAMES_CP_OUTER: for (int r=0; r<MAX_HEIGHT; r++)
  {
    FRAMES_CP_INNER: for (int c=0; c<MAX_WIDTH; c++)
    {
      #pragma HLS pipeline II=1

      // one wide read
      //buf = frames[r][c];
      bit128 in_tmp = Input_1.read();

      buf = in_tmp(31,0);
      // assign values to the FIFOs
      //frame1_a[r][c] = ((input_t)(buf(7 ,  0)) >> 8);
      input_t temp = ((input_t)(buf(7 ,  0)) >> 8);
      databus_t datatmp;
      datatmp(16,0) = temp(16,0);
      frame1_a.write(datatmp);
      //frame2_a[r][c] = ((input_t)(buf(15,  8)) >> 8);
      temp = ((input_t)(buf(15,  8)) >> 8);
      datatmp(16,0) = temp(16,0);
      frame2_a.write(datatmp);
      //frame3_a[r][c] = ((input_t)(buf(23, 16)) >> 8);
      temp = ((input_t)(buf(23, 16)) >> 8);
      datatmp(16,0) = temp(16,0);
      frame3_a.write(datatmp);
      //frame3_b[r][c] = ((input_t)(buf(23, 16)) >> 8);
      temp = ((input_t)(buf(23, 16)) >> 8);
      datatmp(16,0) = temp(16,0);
      frame3_b.write(datatmp);
      //frame4_a[r][c] = ((input_t)(buf(31, 24)) >> 8);
      temp = ((input_t)(buf(31, 24)) >> 8);
      datatmp(16,0) = temp(16,0);
      frame4_a.write(datatmp);
      //frame5_a[r][c] = ((input_t)(buf(39, 32)) >> 8);
      buf = in_tmp(63,32);
      temp = ((input_t)(buf(7, 0)) >> 8);
      datatmp(16,0) = temp(16,0);
      frame5_a.write(datatmp);
    }
  }
  //
  // compute
  gradient_xy_calc(frame3_a, gradient_x, gradient_y); //17 + 32 + 32
  gradient_z_calc(frame1_a, frame2_a, frame3_b, frame4_a, frame5_a, gradient_z); // 17 * 5 + 32
  gradient_weight_y(gradient_x, gradient_y, gradient_z, y_filtered_x, y_filtered_y, y_filtered_z); //y_filtered); // 32 * 3 + 32 * 3
  gradient_weight_x(/*y_filtered,*/ y_filtered_x, y_filtered_y, y_filtered_z, filtered_gradient_x, filtered_gradient_y, filtered_gradient_z);
													//filtered_gradient); // 32 * 3 + 32 * 3
  outer_product(/*filtered_gradient,*/filtered_gradient_x, filtered_gradient_y, filtered_gradient_z, out_product); // 32 * 3 + 6 * 48
  tensor_weight_y(out_product, tensor_y); // 6 * 48 + 6 * 48
  tensor_weight_x(tensor_y, tensor); // 6 * 48 + 6 * 48
  flow_calc(tensor, in); // 6 * 48 + 32 * 2
  output_fun2(in,Output_1);

}
