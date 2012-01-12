/*
  particleFilter.c
  
  Uses the previously computed likelihood matrix to run a particle filter
  Finds and tracks an unknown number of people in the video feed
  
  
  Contained functions:
    initializeCondensation()  - define parameters and setup Condensation data structure
    drawParticles()           - draws particles in position, with size proportional to weight
    updateCondensation()      - reweights every particle and updates Condensation structure
    boxMethod()               - calculates average likelihood within a box
    reinitialiseTracker()     - reinitialises part or all of the Condensation state vector
    motionModel()             - implements our motion model

    
  2010-2011  Will Dollman <willdollman@gmail.com>
*/



void motionModel( float p[2], int i, int prevX[2], int prevSpeed[2], int tempPerson[2] );
void reinitialiseTracker( int person[2] );
void boxMethod( float p[2], int personNum, int i, int lastPerson[2], int depth[2], CvMat* A1 );



/*
  Sets up the Condensation data structure
*/
void initializeCondensation() {

  int op;
  CvMat* low    = cvCreateMat(DP,1,CV_32F);
  CvMat* up     = cvCreateMat(DP,1,CV_32F);

  //! Set starting position for tracker (not actually used by me)
  ConDens->State[0] = 320; ConDens->State[1] = 240;
  
  /*
    This transition matrix defines what is affected by what when Condensation updates
    An identity matrix means the only value that affects a particle's new value is itself
  */
  A = cvCreateMat(8,8,CV_32F);
  
  //! create identity matrix
  for( op = 0; op < 64; op++ ) 
  {
    if( op % 9 == 0 )
      A->data.fl[op] =  1;
    else
      A->data.fl[op] =  0;
  }

  //! copy  transition matrix to the Condensation structure
  memcpy(ConDens->DynamMatr, A->data.fl, sizeof(float)*8*8);
  

  //! Set minima & maxima values to generate samples within
  
  low->data.fl[0]   = 0;            up->data.fl[0]  = img->width;
  low->data.fl[1]   = 0;            up->data.fl[1]  = img->height/2;
  low->data.fl[2]   = minWidth;     up->data.fl[2]  = maxWidth;
  low->data.fl[3]   = minHeight;    up->data.fl[3]  = maxHeight;
  
  low->data.fl[4]   = 0;            up->data.fl[4]  = img->width;
  low->data.fl[5]   = 0;            up->data.fl[5]  = img->height/2;
  low->data.fl[6]   = minWidth;     up->data.fl[6]  = maxWidth;
  low->data.fl[7]   = minHeight;    up->data.fl[7]  = maxHeight;
  
  
  //! Initialise Condensation!
  cvConDensInitSampleSet( ConDens, low, up );

}



/*
  Draws the particles that Condensation is currently using
  Currently disabled for release version
*/
void drawParticles( ) {

  int i;
  for( i = 0; i < ConDens->SamplesNum && stateDraw; i++ ) {
    
    cvCircle( frame, cvPoint( (int) ConDens->flSamples[i][0], (int) ConDens->flSamples[i][1] ), ConDens->flConfidence[i]/1000, CV_RGB ( 255, 250, 135 ) , 1, 8, 0 );
    cvCircle( frame, cvPoint( (int) ConDens->flSamples[i][4], (int) ConDens->flSamples[i][5] ), ConDens->flConfidence[i]/1000, CV_RGB ( 255,   0,   0 ) , 1, 8, 0 );
  
  }

}



/*
  For every particle, it looks at the positions of the tracking boxes
  Then uses the imgLikelihood to calculate the value within the box
  It reweights these values using a large number of metrics 
    - do the boxes overlap, are they too large/small, have they moved too much
    - also does some occlusion reasoning
  If necessary, reinitialises box positions
  Finally, it updates the condensation structure and returns control to mainTracker
*/
void updateCondensation( int x, int y, CvMat* A1, int maxBox[10] ) {

		
    int i, j;
    int high = 0;
    int tempPerson[2];
    int isIntersection;
    int sampleX,     sampleY;
    int sampleWidth, sampleHeight;
    
    int occlusionCalc;
    int occlusionDetected   = 0;
    int footP               = 0, 
    int footQ               = 0;
		
    float p[2];
    float one, two;
    
    int xx[2], yy[2];
    int width[2], height[2];
    
    ConDens->flConfidence[high] = -600;
    
    boxSizeD = 0; footzoneD = 0; intersectionD = 0;
      
    white[0] = 0;
    white[1] = 0;
    white[2] = 0;
    white[3] = 0;
    white[4] = 0;
    
    red[0]   = 0;
    red[1]   = 0;
    red[2]   = 0;
    red[3]   = 0;
    red[4]   = 0;
    
    /*
      For every particle, re-weight new position
    */
		for(i = 0; i < ConDens->SamplesNum; i++ ) {
      
      p[0] = 1.0f;
      p[1] = 1.0f;
      
            
      /* 
        Big chunk of code that calculates the value inside the box
          does head and body boxes and then averages them out.
        Also deals with box sizes while depth scaling
      */
      boxMethod( p, 0, i, lastPerson, depth, A1 );
      boxMethod( p, 1, i, lastPerson, depth, A1 );
      
      xx[0]     = ConDens->flSamples[i][0];
      width[0]  = ConDens->flSamples[i][2];
      xx[1]     = ConDens->flSamples[i][4];
      width[1]  = ConDens->flSamples[i][6];
      
      yy[0]     = ConDens->flSamples[i][1];
      height[0] = ConDens->flSamples[i][3];
      yy[1]     = ConDens->flSamples[i][5];
      height[1] = ConDens->flSamples[i][7];
      
      
      /* Check whether boxes are in valid footzone */
      if( yy[0] + height[0] >= footHigh && yy[0] + height[0] <= footLow ) { } else {
        p[0] = 0;
        white[0]++;
        footP++;
        footzoneD++;
      }
      
      if( yy[1] + height[1] >= footHigh && yy[1] + height[1] <= footLow ) { } else {
        p[1] = 0;
        red[0]++;
        footQ++;
      }
      
      
      /* Check that the boxes haven't moved too much */
      if( abs( prevX[0] - ConDens->flSamples[i][0] ) > 200 ) {
        white[1]++;
        p[0] = p[0] * 0.3;
      }
      if( abs( prevX[1] - ConDens->flSamples[i][4] ) > 200 ) {
        red[1]++;
        p[1] = p[1] * 0.3;
      }
      
      
      /* Calculates the minimal distance between the 2 boxes */
      if( prevX[0] < prevX[1] ) {
        occlusionCalc = abs( prevX[0] + width[0] - prevX[1] );
      } else {
        occlusionCalc = abs( prevX[1] + width[1] - prevX[0] );
      }
            
      
      /*
        do occlusion reasoning
        if you had 2 people in close proximity and now there's just one
        the one closest to the camera is the one we're most likely to be looking for
      */
      if( occlusionCalc < 20 &&
          lastPerson[0] && lastPerson[1] &&
          ( ( person[0] && !person[1] ) | ( !person[0] && person[1] ) ) )
      {
        if( !occlusionDetected ) printf("\n\n\nOcclusion code in use\n");
        if( depth[0] > depth[1] ) {
          p[0] = p[0] * 2;
          p[1] = p[1] * 0.5;
          if( !occlusionDetected ) printf("White wins");
        } else if( depth[0] <= depth[1] ) {
          p[0] = p[0] * 0.5;
          p[1] = p[1] * 2;
          if( !occlusionDetected ) printf("Red wins");
        }
        occlusionDetected = 1;
        if( !occlusionDetected ) printf("\n\n");
      }
         
      
      /* Looks for intersection between the boxes */
      if( ( xx[0] <= xx[1] && xx[1] <= xx[0] + width[0] ) // x1 < x2 < x1+width1
        | ( xx[0] <= xx[1] + width[1] && xx[1] + width[1] <= xx[0] + width[0] ) // x1 < x2+width2 < x1+width1
        
        | ( xx[1] <= xx[0] && xx[0] <= xx[1] + width[1] ) // oh yes, this is necessary
        | ( xx[1] <= xx[0] + width[0] && xx[0] + width[0] <= xx[1] + width[1] )
        ) {
          
          if( ( yy[0] <= yy[1] && yy[1] <= yy[0] + height[0] ) // y1 < y2 < x2 + height1
            | ( yy[0] <= yy[1] + height[1] && yy[1] + height[1] <= yy[0] + height[0] ) // y1 < y2+height2 < y1+height1
            
            | ( yy[1] <= yy[0] && yy[0] <= yy[1] + height[1] )
            | ( yy[1] <= yy[0] + height[0] && yy[0] + height[0] <= yy[1] + height[1] )
            ) {
            
              ConDens->flConfidence[i] = 0;
              white[2]++;
              red[2]++;
              isIntersection = 1;
              intersectionD++;
            }
      } else {
        
          isIntersection = 0;
          
          /* A first-run of "how many people in frame?" to get a guess for the motion model */
          tempPerson[0] = 0; tempPerson[1] = 0;
          
          if( p[0] - 300 > 0 ) {
            tempPerson[0] = 1;
          }
          if( p[1] - 300 > 0 ) {
            tempPerson[1] = 1;
          }
          
      }
        

      /* Models the motion of each person and 'guards' their territory */
      motionModel( p, i, prevX, prevSpeed, tempPerson );
       
        
      ConDens->flConfidence[i] = 0; tempPerson[0] = 0; tempPerson[1] = 0; numTempPeople = 0;
      /* If no intersection, then make sure prob > 300, and then calculate flConfidence */
      if( !isIntersection ) {
      
        if( p[0] - 300 > 0 ) {
          // ConDens->flConfidence[i] = ConDens->flConfidence[i] + p[0] - 300;
          tempPerson[0] = 1;
          numTempPeople++;
        }
        if( p[1] - 300 > 0 ) {
          // ConDens->flConfidence[i] = ConDens->flConfidence[i] + p[1] - 300;
          tempPerson[1] = 1;
          numTempPeople++;
        }
        
        ConDens->flConfidence[i] = p[0] + p[1] - 600;
        
      } else if ( isIntersection ) {
        //ConDens->flConfidence[i] = -600;
      }

          
      /* Find the maximum confidence value */
      if (ConDens->flConfidence[high] <= ConDens->flConfidence[i]) {
        high = i;
        one = p[0] - 300;
        two = p[1] - 300;
        person[0] = tempPerson[0];
        person[1] = tempPerson[1];
        numPeople = numTempPeople;
        if(!lastPerson[0] && lastPerson[1] && person[0] && !person[1] && debug == 1) printf("\nGot best %f vs %f with %f\n", one, two, ConDens->flConfidence[i] );
      }

        
      
    }
    
    // this is it.
      
    /* Set the maxBox values - these are returned to mainTracker */
    
    maxBox[0] = (int)ConDens-> flSamples[high][0];
    maxBox[1] = (int)ConDens-> flSamples[high][1];
    maxBox[2] = (int)ConDens-> flSamples[high][2];
    maxBox[3] = (int)ConDens-> flSamples[high][3];
    
    maxBox[4] = (int)ConDens-> flSamples[high][4];
    maxBox[5] = (int)ConDens-> flSamples[high][5];
    maxBox[6] = (int)ConDens-> flSamples[high][6];
    maxBox[7] = (int)ConDens-> flSamples[high][7];
    
    prevSpeed[0] = (int)ConDens->flSamples[high][0] - prevX[0];
    prevSpeed[1] = (int)ConDens->flSamples[high][4] - prevX[1];

    maxBox[8] = (int)ConDens->flSamples[high][0] - prevX[0];
    maxBox[9] = (int)ConDens->flSamples[high][4] - prevX[1];
    
    printf("\n\n\n\n\np is (%d,%d) to (%d,%d) with %d, %d", maxBox[0], maxBox[1], maxBox[0] + maxBox[2], maxBox[1] + maxBox[3], maxBox[2], maxBox[3]);
    printf("\nq is (%d,%d) to (%d,%d) with %d, %d", maxBox[4], maxBox[5], maxBox[4] + maxBox[6], maxBox[5] + maxBox[7], maxBox[6], maxBox[7]);
    printf( "\nlikelihoods are %f and %f\n", one, two );
    
    
    /* Bounds checking */
    
    if( maxBox[0] < -maxBox[2] )
      maxBox[0] = 0;
    else if( maxBox[0] > 640 )
      maxBox[0] = 640;
      
    if( maxBox[1] < -maxBox[3] )
      maxBox[1] = 0;
    else if( maxBox[1] > 480 )
      maxBox[1] = 480;
      
    if( maxBox[4] < -maxBox[6] )
      maxBox[4] = 0;
    else if( maxBox[4] > 640 )
      maxBox[4] = 640;
      
    if( maxBox[5] < -maxBox[7] )
      maxBox[5] = 0;
    else if( maxBox[5] > 480 )
      maxBox[5] = 480;
    
    
    printf( "\nPeople (%d | %d)", person[0], person[1] );
    if(high == 0) {
      printf(" - but no responses - fail loudly!\n");
      person[0] = 0;
      person[1] = 0;
    }
    
    
    /* 
      If the max for p or q falls below given value, we need to re-init
      Requires going through all particles and setting to random values
    */
    if( person[0] == 0 | person[1] == 0 ) {
      reinitialiseTracker( person );
    }
    printf("\n\n");
      
      
    // update the previously guessed x and y positions
    prevX[0] = maxBox[0]; 
    prevX[1] = maxBox[4]; 
    prevY[0] = maxBox[1];
    prevY[1] = maxBox[5];
    
    lastPerson[0] = person[0];
    lastPerson[1] = person[1];
    
    printf("\nWhi: Floor %d, move %d, inter %d, motion %d, guard %d, total %d", white[0], white[1], white[2], white[3], white[4], white[0]+white[1]+white[2]+white[3]+white[4]);
    printf("\nRed: Floor %d, move %d, inter %d, motion %d, guard %d, total %d\n\n", red[0], red[1], red[2], red[3], red[4], red[0]+red[1]+red[2]+red[3]+red[4]);
				
    //! For each particle, resample and update new positions
		cvConDensUpdateByTime( ConDens);
	
}






/* 
  Given a box that a particle represents, calculates the sum of likelihoods inside that box (body + head box)
*/
void boxMethod( float p[2], int personNum, int i, int lastPerson[2], int depth[2], CvMat* A1 ) {

  int bx, by, nX, nY;
  int nXConst, nYConst;

  int imgSizeX = 64;
  int imgSizeY = 48;

  int base = personNum * 4;
  
  int sampleX, sampleY, sampleWidth, sampleHeight;
  
  
  /* If depth scaling enabled, set width and height to mx+c, where x is depth */
  
  if( depthScaling && lastPerson[personNum] ) {
    ConDens-> flSamples[i][base+2] = ( depth[personNum] * scalingX ) + addScalingX;
    ConDens-> flSamples[i][base+3] = ( depth[personNum] * scalingY ) + addScalingY;
    ConDens->flSamples[i][base+1] = 2 + footHigh - ConDens->flSamples[i][base+3]; // also change y so it doesn't get exluded
  }

  sampleX = (int)ConDens-> flSamples[i][base+0] / 10;
  sampleY = (int)ConDens-> flSamples[i][base+1] / 10;

  sampleWidth  = (int)ConDens-> flSamples[i][base+2];
  sampleHeight = (int)ConDens-> flSamples[i][base+3];


  /* Sum inside body box */
  for( by = sampleY; by < sampleY + (sampleHeight/10); by++ ) {
         for( bx = sampleX; bx < sampleX + ( sampleWidth / 10 ); bx++ ) {
                if( bx < 1 | bx > 64 | by < 1 | by > 48 ) {
                  p[personNum] = p[personNum]; 
                } else {
                  p[personNum] = p[personNum] + (float)cvGetReal2D( A1, by-1, bx-1 );
                }
         }            
  }

  nXConst = sampleX + ( sampleWidth / 30 );
  nYConst = sampleY - ( sampleWidth / 20 );

  /* Sum inside head box */
  for( nY = nYConst; nY < nYConst + ( sampleHeight / 40 ); nY++ ) {
    for( nX = nXConst; nX < nXConst + ( sampleWidth / 30 ); nX++ ) {
      if( nX < 1 | nX > 64 | nY < 1 | nY > 48 ) {
        p[personNum] = p[personNum];
      } else {
        p[personNum] = p[personNum] + (float)cvGetReal2D( A1, nY - 1, nX - 1 );
      }              
    }
  }

  /* Check that box size is within bounds */
  if( sampleWidth >= minWidth && sampleWidth <= maxWidth && sampleHeight >= minHeight && sampleHeight <= maxHeight) {
      p[personNum] = (float) 100* p[personNum] / (sampleWidth*sampleHeight);
      //printf("\nBox at %d,%d with size %d,%d - preserved at %f", sampleX,sampleY,sampleWidth,sampleHeight, p);
  } else {
      p[personNum] = (float) 0;
      boxSizeD++;
      //printf(" - removed");
  }

  
  
}




/* 
  When things don't work out and particles clustered in an area can't find anything good, it's time to start over.
  Particles are randomly distributed in areas where they will be valid (according to their size and the footzone)
*/
void reinitialiseTracker( int person[2] ) {

  int j;
  
  if( person[0] == 0 )
    printf("\nReinitialising p");
  if( person[1] == 0 )
    printf("\nReinitialising q");
  
  
  // Loop through all particles
  for( j = 0; j < ConDens->SamplesNum; j++ ) {
  
    /* If none of one type of box find anything, start them afresh */
    if( person[0] == 0 ) {
      /* If we're depth scaling, then the height is fixed as addStartingY */
      if( depthScaling ) {
        ConDens->flSamples[j][0] =   rand() % img->width;
        ConDens->flSamples[j][2] =   addStartingX;
        ConDens->flSamples[j][3] =   250; // change to addStartingY?
        ConDens->flSamples[j][1] = ( rand() % ( footLow - footHigh )) + footHigh - addStartingY;
      } else {
      /* Otherwise the height will be a bit random, so work accordingly */
        ConDens->flSamples[j][0] =   rand() % img->width;
        ConDens->flSamples[j][2] = ( rand() % ( maxWidth - minWidth ) ) + minWidth;
        ConDens->flSamples[j][3] = ( rand() % ( maxHeight - minHeight ) ) + minHeight;
        // rand mod 500-480 is 0 to 20. Then plus footHigh-yHeight = 480-200
        ConDens->flSamples[j][1] = ( rand() % ( footLow - footHigh )) + ( footHigh - ConDens->flSamples[j][3] );
      }
    }

    if( person[1] == 0 ) {
    
      if( depthScaling ) {
        ConDens->flSamples[j][4] =   rand() % img->width;
        ConDens->flSamples[j][6] =   addStartingX;
        ConDens->flSamples[j][7] =   250; // change to addStartingY?
        ConDens->flSamples[j][5] = ( rand() % ( footLow - footHigh )) + footHigh - addStartingY;
      } else {
        ConDens->flSamples[j][4] =   rand() % img->width;
        ConDens->flSamples[j][6] = ( rand() % ( maxWidth - minWidth ) ) + minWidth;
        ConDens->flSamples[j][7] = ( rand() % ( maxHeight - minHeight ) ) + minHeight;
        ConDens->flSamples[j][5] = ( rand() % ( footLow - footHigh )) + ( footHigh - ConDens->flSamples[j][7]); 
      }
    }

    /* Reset likelihood levels to a constant value */
    if( person[0] == 0 && person[1] == 0 ) {
      ConDens->flConfidence[j] = 1;
    }

  } // end loop of all particles

}



/*
  Uses previous positions to calculate speed, uses speed to estimate next position
  Also ensures that one box does not 'claim' the area that another box is currently in
*/
void motionModel( float p[2], int i, int prevX[2], int prevSpeed[2], int tempPerson[2] ) {


  if( tempPerson[0] == 1 ) {
    if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][0] ) <= 10 )
      p[0] = p[0];
    else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][0] ) <= 20 )
      p[0] = p[0] * 0.9;
    else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][0] ) <= 30 )
      p[0] = p[0] * 0.8;
    else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][0] ) <= 40 )
      p[0] = p[0] * 0.7;
    else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][0] ) <= 50 )
      p[0] = p[0] * 0.6;
    else {
      p[0] = p[0] * 0.5;
      white[3]++;
    }
  }

  if( tempPerson[1] == 1 ) {
    if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][4] ) <= 10 )
      p[1] = p[1];
    else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][4] ) <= 20 )
      p[1] = p[1] * 0.9;
    else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][4] ) <= 30 )
      p[1] = p[1] * 0.8;
    else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][4] ) <= 40 )
      p[1] = p[1] * 0.7;
    else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][4] ) <= 50 )
      p[1] = p[1] * 0.6;
    else {
      p[1] = p[1] * 0.5;
      red[3]++;
    }
  }
  
  
  // if this frame there's ( 1 | 0 ) and last frame it was ( 0 | 1 )
      // then take potential action
      // if p[0] position is close to p[1]'s last position...
  if( tempPerson[0] && !tempPerson[1] && !lastPerson[0] && lastPerson[1] ) {
    if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][0] ) <= 10 ) {
      p[0] = p[0] * 0.5;
      white[4]++;
    } else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][0] ) <= 20 )
      p[0] = p[0] * 0.6;
    else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][0] ) <= 30 )
      p[0] = p[0] * 0.7;
    else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][0] ) <= 40 )
      p[0] = p[0] * 0.8;
    else if( abs( prevX[1] + prevSpeed[1] - ConDens->flSamples[i][0] ) <= 50 )
      p[0] = p[0] * 0.9;
    else
      p[0] = p[0];
  }
      
      // other way around
      // if q's position is close to p's last position...
  if( !tempPerson[0] && tempPerson[1] && lastPerson[0] && !lastPerson[1] ) {
    if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][4] ) <= 10 ) {
      p[1] = p[1] * 0.5;
      red[4]++;
    } else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][4] ) <= 20 )
      p[1] = p[1] * 0.6;
    else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][4] ) <= 30 )
      p[1] = p[1] * 0.7;
    else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][4] ) <= 40 )
      p[1] = p[1] * 0.8;
    else if( abs( prevX[0] + prevSpeed[0] - ConDens->flSamples[i][4] ) <= 50 )
      p[1] = p[1] * 0.9;
    else
      p[1] = p[1];
  }

}
