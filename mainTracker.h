/* Variables that affect program operation */

  int debug         = 0;             // shouldn't be needed really
  int shadowRemove  = 0;     // remove shadows by ignoring 
  int stateDraw     = 0;    
  int enable3D;
  
  /* 
    If this is set to 1, then we expect a left+right video file
      and we resize to get normal video, & process stereo ourselves 
  */
  int processStereo = 0;
  IplImage  *leftFrame  = 0;
  IplImage  *rightFrame = 0;

  /* best for project video set 1 */
  int minWidth = 100;  int maxWidth = 180;
  int minHeight = 250; int maxHeight = 400;
  
  /* best for project video set 2 */
  // int minWidth = 90;  int maxWidth = 180;
  // int minHeight = 200; int maxHeight = 400;
  
  int footHigh = 480, footLow = 520;
  
  int white[5];
  int red[5];
  

  /* Deals with depth scaling factors */

  //! 3dset2 and 3dset3
  int depthScaling = 1;
  float scalingX = 0.470588;
  float scalingY = 0.882353;
  int addScalingX = 60;
  int addScalingY = 174;
  int addStartingX = 100;
  int addStartingY = 250;
  int minDepth = 90;
  

  /* What should be displayed? */
  int displayHeatFrame = 1;
  int displayStereoFrame = 1;
  int displayPositionFrame = 1;
  int displayParticles = 1;
  
  
  /* Variables that shouldn't be varied */
  int _numImages = 18;
  int _sizeX = 64;
  int _sizeY = 48;
  int _numResponses = 9;

  

  /* Variables that just need initialising */

  int mexican[10][10] = { 
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0,-3,-3,-3,-3,-3,-3,-3,-3, 0,
                            0,-3, 1, 1, 1, 1, 1, 1,-3, 0,
                            0,-3, 1, 3, 3, 3, 3, 1,-3, 0,
                            0,-3, 1, 3, 7, 7, 3, 1,-3, 0,
                            0,-3, 1, 3, 7, 7, 3, 1,-3, 0,
                            0,-3, 1, 3, 3, 3, 3, 1,-3, 0,
                            0,-3, 1, 1, 1, 1, 1, 1,-3, 0,
                            0,-3,-3,-3,-3,-3,-3,-3,-3, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                          };

                          
  #define DP 8
  #define MP 2
  #define TRUE 1
  #define FALSE 0
  #define nSamples 3000
  #define CV_RGB( r, g, b )  cvScalar( (b), (g), (r), 0 )


  CvConDensation* ConDens = NULL;
  

  int person[2];
  int prevSpeed[2];
  int depth[2];
  int prevX[2], prevY[2];
  int lastPerson[2] = {0,0};
  
  int bx, by;
  int numPeople, numTempPeople;
  int boxSizeD, footzoneD, intersectionD;


  CvMat*    temp1;
  CvMat*    temp2;
  CvMat*    temp3;
  CvMat*    temp4;
  CvMat*    temp5;
  CvMat*    covTemp;
  CvMat*    Z;
  CvMat*    meanTemp;
            
  IplImage* img             = NULL;
  IplImage* yImg            = NULL;
            
  IplImage* frame = 0;
  IplImage* stereoFrame     = NULL;
  IplImage* positionFrame   = NULL;

  CvRNG     rng_state;
  CvMat*    A               = NULL;
