/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#include <cstdio>
#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <pnmfile.h>
#include "segment-image.h"
#include <opencv2/core/core.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <ctime>
//#include "types_c.h"
using namespace cv;
using namespace std;

void usage(char** argv,int argc){

  if (argc != 6) {
  
    fprintf(stderr, "usage: %s sigma k min input(ppm) output(ppm)\n", argv[0]);
    exit(0);
  }

}


cv::Mat imgToMat( image<rgb> *input){
  int width  = input->width();
  int height = input->height();

  cv::Mat imgMat(height,width,CV_8UC3);
 
  for(int i = 0; i < height; i++){  
    for(int j=0; j < width; j++){
     imgMat.at<Vec3b>(i,j)[0] = input->data[i*width + j].b;
     imgMat.at<Vec3b>(i,j)[1] = input->data[i*width + j].g;
     imgMat.at<Vec3b>(i,j)[2] = input->data[i*width + j].r;
    }
  }
    return imgMat;
}


// helper to change the form of the image container
image<rgb>* matToimg(cv::Mat input ){


  int width  = input.cols; 
  int height = input.rows;

  image<rgb> *imPtr = new image<rgb>(input.cols, input.rows);


  for(int i = 0; i < height; i++){
  
    for(int j=0; j < width; j++){
   

       imPtr->data[ i*width + j].b = input.at<Vec3b>(i,j)[0]; 
       imPtr->data[ i*width + j].g = input.at<Vec3b>(i,j)[1]; 
       imPtr->data[ i*width + j].r = input.at<Vec3b>(i,j)[2]; 

    }
  
  }
  
  return imPtr;

}

typedef struct maskImg{
    int x;
    int y;
    int val;
    int label;
}maskImg;

int main(int argc, char **argv) {

  // sanity check for correct input
  usage(argv,argc);

  float sigma = atof(argv[1]);
  float k = atof(argv[2]);
  int min_size = atoi(argv[3]);
  universe* u;
	
  printf("loading input image.\n");

  cv::Mat inp = imread(argv[4]);
  resize(inp, inp, Size(0,0),0.25,0.25);  
  imshow("input",inp);

  image<rgb> *input = matToimg(inp); 

  printf("processing\n");

  clock_t startTime = clock();
  int num_ccs; 
  image<rgb> *seg = segment_image(input, sigma, k, min_size, &num_ccs,u); 
  clock_t endTime = clock();

  cout << "Time taken: " << (float)(endTime - startTime)/CLOCKS_PER_SEC << endl;
  Mat outImg  = imgToMat(seg);
  resize(outImg, outImg, Size(0,0),2.0,2.0, INTER_LANCZOS4);  
  erode(outImg, outImg, getStructuringElement(MORPH_ELLIPSE, Size(50,50), Point(0,0)));
 // dilate(outImg, outImg, Mat(), Point(-1, -1), 2, 1, 1);
  dilate(outImg, outImg, getStructuringElement(MORPH_ELLIPSE, Size(50,50), Point(0,0)));
  Mat grayImg(outImg.rows,outImg.cols, CV_8UC1);

  imshow("colour_output", outImg);
  waitKey(); 
  cvtColor(outImg,grayImg,CV_BGR2GRAY);

  imshow("output", grayImg);
  waitKey();

  int outWidth = grayImg.cols;
  int outHeight = grayImg.rows;
 
  vector<maskImg> maskPoints;
  int label_=0;
  for (int i = 0; i < outWidth; i++) {
      for (int j = 0; j < outHeight; j++) {
          if (j < 500 && j > 300 && i > 300 && i < 600) {
              maskImg tmp;
              tmp.x = i;
              tmp.y = j;
              tmp.val = grayImg.at<uchar>(i,j);
              tmp.label = label_;
              label_++;
              maskPoints.push_back(tmp);
          }
      }
  }
  clock_t endTime2 = clock();

  cout << "Time taken2: " << (float)(endTime2 - startTime)/CLOCKS_PER_SEC << endl;
  
  printf("got %d components\n", num_ccs);
  printf("done! uff...thats hard work.\n");

  return 0;

}

