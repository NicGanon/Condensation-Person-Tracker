/*
  imageLikelihood.c 
  
  Takes an input image, along with backMean and backCov
  Returns a 48x64 likelihood matrix of each patch in the image being background
  
  
  Contained functions:
    likelihood()      - classifies an image, given a background model
    patchLikelihood() - calculates likelihood of patch belonging to the background
    responsePatch()   - given a patch, calculates the response vector
    getP()            - gets the value of a pixel
    

  2010-2011  Will Dollman <willdollman@gmail.com>
  
*/


/*
  Given an input patch, calculates a response vector the Y,Cb,Cr components
    on average value
    response to a mexican hat function
    response to a vertical line detection function
*/
void responsePatch( IplImage* img, int xPatch, int yPatch, float response[9] ) {

  /*
      Run 9 filters on the 100px and record responses
  */

  
  int x, y, xPos, yPos;
  response[0] = 0; response[1] = 0; response[2] = 0;
  response[3] = 0; response[4] = 0; response[5] = 0;
  response[6] = 0; response[7] = 0; response[8] = 0; 
  
  
  for( x = 0, xPos = xPatch * 10; x < 10; x++, xPos++ ) {
    for( y = 0, yPos = yPatch * 10; y < 10; y++, yPos++ ) {
    
      /*
        Average response
      */
      response[2] = (float)response[2] + (int)getP( img, xPos, yPos, 0 );
      response[1] = (float)response[1] + (int)getP( img, xPos, yPos, 1 );
      response[0] = (float)response[0] + (int)getP( img, xPos, yPos, 2 );
      
      /*
        Mexican response
      */
      // response[5] = ( (int)getP( img, xPos, yPos, 0 ) * mexican[y][x] ) + response[5];
      // response[4] = ( (int)getP( img, xPos, yPos, 1 ) * mexican[y][x] ) + response[4];
      // response[3] = ( (int)getP( img, xPos, yPos, 2 ) * mexican[y][x] ) + response[3];
      
      
      /*
        Horizontal response
      */
      if( y < 5 ) {
        response[5] = (float)response[5] - (int)getP( img, xPos, yPos, 0 );
        response[4] = (float)response[4] - (int)getP( img, xPos, yPos, 1 );
        response[3] = (float)response[3] - (int)getP( img, xPos, yPos, 2 );
      } else if( y >= 5 ) {
        response[5] = (float)response[5] + (int)getP( img, xPos, yPos, 0 );
        response[4] = (float)response[4] + (int)getP( img, xPos, yPos, 1 );
        response[3] = (float)response[3] + (int)getP( img, xPos, yPos, 2 );
      }
      
      /*
        Vertical response
      */
      if( x < 5 ) {
        response[8] = (float)response[8] - (int)getP( img, xPos, yPos, 0 );
        response[7] = (float)response[7] - (int)getP( img, xPos, yPos, 1 );
        response[6] = (float)response[6] - (int)getP( img, xPos, yPos, 2 );
      } else if( x >= 5 ) {
        response[8] = (float)response[8] + (int)getP( img, xPos, yPos, 0 );
        response[7] = (float)response[7] + (int)getP( img, xPos, yPos, 1 );
        response[6] = (float)response[6] + (int)getP( img, xPos, yPos, 2 );
      }
      
    }
  }

  // Average the first 3 responses
  response[0] = response[0] / 100.0;
  response[1] = response[1] / 100.0;
  response[2] = response[2] / 100.0;  
  
 
  
} 




/*
  Return the value of a pixel
*/
int getP( IplImage* img, int x, int y, int chan ) {
  return CV_IMAGE_ELEM( img, uchar, y, x*3+chan);
}



/*
  Calculates probability of a patch belonging to the background
  (given mean and covariance)
*/
float patchLikelihood( int xPatch, int yPatch, float response[9], CvMatND *backMean, CvMatND *backCov ) {

  /*
   if all the means for a patch are 0, then skip
   else
    temp1 = (Z - meanTemp(i,:));
    temp2 = inv(diag(diag(selectCov(covTemp,i))));
    temp3 = transpose(Z - meanTemp(i,:));
    temp4 = temp1 * temp2 * temp3;
    temp4 = temp4/100;
  */
  
  
  float returnVal = 0;
  int i, j, k, clusterNum;
  int meanIndex[] = { yPatch, xPatch, 0, 0 };
  int covIndex[] = { yPatch, xPatch, 0, 0, 0 };
  
  
  CvMat* temp1 = cvCreateMat( 9, 1, CV_32F );
  CvMat* temp2 = cvCreateMat( 1, 9, CV_32F );
  CvMat* temp3 = cvCreateMat( 1, 9, CV_32F );
  CvMat* temp4 = cvCreateMat( 1, 1, CV_32F );
  CvMat* temp5 = cvCreateMat( 9, 9, CV_32F );
  CvMat* covTemp = cvCreateMat( 9, 9, CV_32F );
  CvMat *Z = cvCreateMat( 9, 1, CV_32F );
  CvMat *meanTemp = cvCreateMat( 9, 1, CV_32F );
  
  cvZero( covTemp );
  
  
  // There are 4 clusters, run for each
  for( clusterNum = 0; clusterNum < 4; clusterNum++ ) {
  
    meanIndex[2] = clusterNum;
    covIndex[2]  = clusterNum;
      
      // There are 9 responses in each cluster, run for each
      for( i = 0; i < 9; i++ ) {
      
        // set position
          meanIndex[3] = i;
        // set meanTemp
          cvSetReal2D( meanTemp, i, 0, (float)cvGetRealND( backMean, meanIndex ) );
        // set Z
          cvSetReal2D( Z, i, 0, (float)response[i] );
        
        // Attempt at removing shadows - ignore the Y components
        if( shadowRemove == 1 ) {
          cvSetReal2D( Z, 2, 0, cvGetReal2D( meanTemp, 2, 0 ) + 0 );
          cvSetReal2D( Z, 5, 0, cvGetReal2D( meanTemp, 5, 0 ) + 00 );
          cvSetReal2D( Z, 8, 0, cvGetReal2D( meanTemp, 8, 0 ) + 00 );
        }
        
        // set position
          covIndex[3] = i;
          covIndex[4] = i;
        // update matrix
          cvSetReal2D( covTemp, i, i, (float)cvGetRealND( backCov, covIndex ) );
        
      }
      
      /*
        Detect empty clusters
        These can occur when are area is overexposed in every training frame
      */
      for( i = 0, j = 0; i < 9; i++ ) {
        if( cvGetReal2D( meanTemp, i, 0 ) == 0 )
          j++;
      }
      // If the mean is all 0's, draw a square and quit
      if( j == 9 ) {
        cvRectangle(frame, cvPoint( (xPatch*10), (yPatch*10) ), cvPoint( (xPatch*10)+10, (yPatch*10)+10 ), CV_RGB ( 255, 0, 0 ),1, 8, 0);
        return 0;
      }
      
      // This can also occur in bad areas - just quit
      if( (int)cvGetReal2D( meanTemp, 0, 0 ) == 126 &&
          (int)cvGetReal2D( meanTemp, 1, 0 ) == 125 &&
          (int)cvGetReal2D( meanTemp, 2, 0 ) == 232 ) 
      { 
          cvRectangle(frame, cvPoint( (xPatch*10), (yPatch*10) ), cvPoint( (xPatch*10)+10, (yPatch*10)+10 ), CV_RGB ( 0, 255, 0 ),1, 8, 0);
          return 0;
      }
      // Otherwise we're fine!
      
    
      //temp1 = Z - meanTemp;
        cvSub( Z, meanTemp, temp1, NULL );
        
      // temp2 = temp1 transposed (actually the other way around...)
        cvTranspose( temp1, temp2 );
        
      // invert the matrix
        cvInvert( covTemp, temp5, CV_LU );
        
      // temp3 = temp2 * covTemp
        cvMatMul( temp2, temp5, temp3 );
        
      // temp4 = temp3 * temp1
        cvMatMul( temp3, temp1, temp4 );
        
      // sum the responses for all 4 clusters
        returnVal = (cvGetReal2D( temp4, 0,0 ) / 100.0) + returnVal;
      
 
  }
  
  
  
  return returnVal;

}








/*
  For each patch (64x48), calculate the response vector
  Then calculate and store the likelihood of that patch being background
  (given the mean and covariance)
*/

int likelihood( IplImage *img, CvMatND *backMean, CvMatND *backCov, CvMat *imgLikelihood ) {


  int i, j;
  int xPatch = 2, yPatch = 2;
  float classImg[_sizeY][_sizeX][_numResponses];
  float patchVal;
  
  // Convert frame to YCrCb
  /* Leave in if using YCrCb, comment out otherwise */
  cvCvtColor( img, yImg, CV_RGB2YCrCb );
  

  // for( yPatch = 0; yPatch < _sizeY; yPatch++,yPatch++ ) {
    // for( xPatch = 0; xPatch < _sizeX; xPatch++,xPatch++ ) {
  for( yPatch = 0; yPatch < _sizeY; yPatch++ ) {
    for( xPatch = 0; xPatch < _sizeX; xPatch++ ) {

        
        // Calculates the response vector
        /* Leave in if using YCrCb, comment out otherwise */
        responsePatch( yImg, xPatch, yPatch, classImg[yPatch][xPatch] );
        /* Leave in if using RGB, comment out otherwise */
        // responsePatch( img, xPatch, yPatch, classImg[yPatch][xPatch] );

        // patchLikelihood calcs the probability a patch belongs to the background
        // reduces "hot pixels" - caps the max value a pixel can be

        patchVal = patchLikelihood( xPatch, yPatch, classImg[yPatch][xPatch], backMean, backCov );
        
        if( patchVal >= 10000 )
          patchVal = 10000;

        cvSetReal2D( imgLikelihood, yPatch, xPatch, patchVal );
        
        
        // cvSetReal2D( imgLikelihood, yPatch, xPatch+1, patchVal );
        // cvSetReal2D( imgLikelihood, yPatch+1, xPatch, patchVal );
        // cvSetReal2D( imgLikelihood, yPatch+1, xPatch+1, patchVal );
      
    }
  }

  
}


