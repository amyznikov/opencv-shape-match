/*
 * main.cc
 *
 *  Created on: Oct 15, 2016
 *      Author: amyznikov
 */

#include <opencv2/opencv.hpp>

static const char MainWindow[] = "MainWindow";

using namespace cv;


static Mat inputFrame;

int main(int argc, char *argv[])
{

  // input video stream URL
  const char * input_stream_url = NULL;
  VideoCapture inputVideo;

  // Optional output video stream URL (for live broadcasting)
  const char * output_stream_url = NULL;
  VideoWriter outputVideo;

  // prepared shape image
  const char * shape_file_name = NULL;
  Mat shape;

  int matchMethod = TM_CCORR_NORMED;


  char c;
  double minVal;
  double maxVal;
  Point minLoc;
  Point maxLoc;
  Point matchLoc;
  Mat display;


  // parse command line

  for ( int i = 1; i < argc; ++i ) {

    if ( strcmp(argv[i], "--help") == 0 ) {
      fprintf(stdout, "Usage:\n");
      fprintf(stdout, "  shape-match -shape <shape-image-file-name> [input-stream-url] [-o <output-stream-url>] \n");
      return 0;
    }

    if ( strcmp(argv[i], "-shape") == 0 ) {

      if ( ++i >= argc ) {
        fprintf(stderr, "Missing argument after %s\n", argv[i]);
        return 1;
      }

      shape_file_name = argv[i];
    }
    else if ( strcmp(argv[i], "-o") == 0 ) {
      if ( ++i >= argc ) {
        fprintf(stderr, "Missing argument after %s\n", argv[i]);
        return 1;
      }
      output_stream_url = argv[i];
    }
    else if ( !input_stream_url ) {
      input_stream_url = argv[i];
    }
    else {
      fprintf(stderr, "Invalid argument %s", argv[i]);
      return 1;
    }
  }

  if ( !shape_file_name ) {
    fprintf(stderr, "No shape file name provided\n");
    return 1;
  }



  ///////////////////////////////////////////////////
  // Read prepared shape image

  shape = imread(shape_file_name, IMREAD_GRAYSCALE);
  if ( !shape.data ) {
    fprintf(stderr, "imread(%s) fails\n", shape_file_name);
    return 1;
  }




  ///////////////////////////////////////////////////
  //Open Input Video

  if ( input_stream_url ) {
    inputVideo.open(input_stream_url);
    if ( !inputVideo.isOpened() ) {
      fprintf(stderr, "Can not open %s\n", input_stream_url);
      return 1;
    }
  }
  else {
    fprintf(stderr, "Input file name not provided, trying to open camera\n");
    inputVideo.open(0);
    if ( !inputVideo.isOpened() ) {
      fprintf(stderr, "Can not open camera\n");
      return 1;
    }
  }





  //////////////////////////////////////////////////////////////////////////////////////////////
  //Open Output Video Stream (usualy points to ffsrv live input for live broadcasting )

  if ( output_stream_url ) {

    if ( !inputVideo.read(inputFrame) ) {
      fprintf(stderr, "inputVideo.read() fails: Can not determite input frame size\n");
      return 1;
    }

    if ( !outputVideo.open(output_stream_url, CV_FOURCC('M', 'J', 'P', 'G'), 30,
        Size(inputFrame.cols, inputFrame.rows), false) ) {
      fprintf(stderr, "outputVideo.open(%s) fails\n", output_stream_url);
      return 1;
    }
  }






  ///////////////////////////////////////////////////

  namedWindow(MainWindow, WINDOW_AUTOSIZE );


  while ( 42 ) {



    if ( !inputVideo.read(inputFrame) ) {
      fprintf(stderr, "inputVideo.read() fails\n");
      break;
    }

    cvtColor(inputFrame, inputFrame, CV_BGR2GRAY);


    /// Do the Matching and Normalize
    Mat result;
    matchTemplate( inputFrame, shape, result, matchMethod );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Localizing the best match with minMaxLoc
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());


    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if ( matchMethod == CV_TM_SQDIFF || matchMethod == CV_TM_SQDIFF_NORMED ) {
      matchLoc = minLoc;
    }
    else {
      matchLoc = maxLoc;
    }


    /// display image
    display = inputFrame;
    rectangle(display, matchLoc, Point(matchLoc.x + shape.cols, matchLoc.y + shape.rows), Scalar::all(0), 2, 8, 0);


    imshow(MainWindow, display);
    if ( output_stream_url ) {
      outputVideo.write(display);
      if ( !outputVideo.isOpened() ) {
        fprintf(stderr, "outputVideo.write() fails\n");
        break;
      }
    }

    if ( (c = (char) waitKey(5)) == 'q' ) {
      break;
    }


    switch ( c ) {
    case '0' :
      matchMethod = TM_SQDIFF;
      break;

    case '1' :
      matchMethod = TM_SQDIFF_NORMED;
      break;

    case '2' :
      matchMethod = TM_CCORR;
      break;

    case '3' :
      matchMethod = TM_CCORR_NORMED;
      break;

    case '4' :
      matchMethod = TM_CCOEFF;
      break;

    case '5' :
      matchMethod = TM_CCOEFF_NORMED;
      break;

    default :
      break;
    }

  }

  return 0;
}



