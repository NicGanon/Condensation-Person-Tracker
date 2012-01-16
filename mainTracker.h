  // int stopCount = 0;
  // char frameName[100] = "heat/heat";
  // char frameCount[100];
  // char frameNameTemp[200];


/* Variables that affect program operation */

  int debug = 0;             // shouldn't be needed really
  int shadowRemove = 0;     // remove shadows by ignoring 
  int stateDraw = 0;    
  int enable3D;

  /* best for recorded video */
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /* best for project video set 1 */
  int minWidth = 100;  int maxWidth = 180;
  int minHeight = 250; int maxHeight = 400;
  
  /* best for project video set 2 */
  // int minWidth = 90;  int maxWidth = 180;
  // int minHeight = 200; int maxHeight = 400;
  
  
  /* best(?) for live video */
  // int minWidth = 120;  int maxWidth = 150;
  // int minHeight = 250; int maxHeight = 340;
  
  int footHigh = 480, footLow = 520;
  
  // int whiteFloor, whiteMove, whiteIntersection, whiteMotionModel, whiteMotionGuard;
  // int redFloor, redMove, redIntersection, redMotionModel, redMotionGuard;
  
  int white[5];
  int red[5];
  
  /* Deals with depth scaling factors */
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //! 3dset1
  // int depthScaling = 1;
  // float scalingX = 0.3125;
  // float scalingY = 0.588235;
  // const int addScalingX = 100;
  // const int addScalingY = 250;
  // int addStartingX = 100; // to deal with case when 3d map doesn't start at 0
  // int addStartingY = 250; // to deal with case when 3d map doesn't start at 0
  

  //! 3dset2 and 3dset3
  int depthScaling = 1;
  float scalingX = 0.470588;
  float scalingY = 0.882353;
  int addScalingX = 60;
  int addScalingY = 174;
  int addStartingX = 100;
  int addStartingY = 250;
  int minDepth = 90;
  
  
  //! VIDEO 2 3dset1
  // int depthScaling = 1;
  // float scalingX = 0.9;
  // float scalingY = 1.8333;
  // int addScalingX = 50;
  // int addScalingY = 122;
  // int addStartingX = 94;
  // int addStartingY = 214;
  // int minDepth = 50;
  
  //! VIDEO 2 3dset2
  // int depthScaling = 1;
  // float scalingX = 0.786;
  // float scalingY = 2.222;
  // int addScalingX = 30;
  // int addScalingY = 68;
  // int addStartingX = 100;
  // int addStartingY = 260;
  // int minDepth = 90;

  
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
                            0,0,0,0,0,0,0,0,0,0,
                            0,-3,-3,-3,-3,-3,-3,-3,-3,0,
                            0,-3,1,1,1,1,1,1,-3,0,
                            0,-3,1,3,3,3,3,1,-3,0,
                            0,-3,1,3,7,7,3,1,-3,0,
                            0,-3,1,3,7,7,3,1,-3,0,
                            0,-3,1,3,3,3,3,1,-3,0,
                            0,-3,1,1,1,1,1,1,-3,0,
                            0,-3,-3,-3,-3,-3,-3,-3,-3,0,
                            0,0,0,0,0,0,0,0,0,0
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

  
  
  
  

  
  IplImage* yImg  = NULL;
  IplImage* img  = NULL;
  
  IplImage  *frame = 0;
  IplImage  *stereoFrame = NULL;
  IplImage  *positionFrame = NULL;

  CvRNG rng_state;
  CvMat* A = NULL;
