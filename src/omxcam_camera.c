#include "omxcam_camera.h"

int OMXCAM_loadCameraDrivers (){
  /*
  This is a specific behaviour of the Broadcom's Raspberry Pi OpenMAX IL
  implementation module because the OMX_SetConfig() and OMX_SetParameter() are
  blocking functions but the drivers are loaded asynchronously, that is, an
  event is fired to signal the completion. Basically, what you're saying is:
  
  "When the parameter with index OMX_IndexParamCameraDeviceNumber is set, load
  the camera drivers and emit an OMX_EventParamOrConfigChanged event"
  
  The red LED of the camera will be turned on after this call.
  */
  
  OMXCAM_trace ("Loading '%s' drivers", OMXCAM_ctx.camera.name);
  
  OMX_ERRORTYPE error;

  OMX_CONFIG_REQUESTCALLBACKTYPE cbs;
  OMX_INIT_STRUCTURE (cbs);
  cbs.nPortIndex = OMX_ALL;
  cbs.nIndex = OMX_IndexParamCameraDeviceNumber;
  cbs.bEnable = OMX_TRUE;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigRequestCallback, &cbs))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigRequestCallback: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  OMX_PARAM_U32TYPE device;
  OMX_INIT_STRUCTURE (device);
  device.nPortIndex = OMX_ALL;
  //ID for the camera device
  device.nU32 = 0;
  if ((error = OMX_SetParameter (OMXCAM_ctx.camera.handle,
      OMX_IndexParamCameraDeviceNumber, &device))){
    OMXCAM_error ("OMX_SetParameter - OMX_IndexParamCameraDeviceNumber: "
        "%s", OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  if (OMXCAM_wait (&OMXCAM_ctx.camera, OMXCAM_EventParamOrConfigChanged, 0)){
    return -1;
  }
  
  //The LED is on
  
  return 0;
}

int OMXCAM_setCapturePort (OMX_U32 port){
  //The camera needs to know which output port is going to be used to consume
  //the data. Still: 72, Video: 71
  OMXCAM_trace ("Setting '%s' capture port", OMXCAM_ctx.camera.name);
  
  OMX_ERRORTYPE error;
  
  OMX_CONFIG_PORTBOOLEANTYPE cameraCapturePort;
  OMX_INIT_STRUCTURE (cameraCapturePort);
  cameraCapturePort.nPortIndex = port;
  cameraCapturePort.bEnabled = OMX_TRUE;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigPortCapturing, &cameraCapturePort))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigPortCapturing: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  return 0;
}

int OMXCAM_resetCapturePort (OMX_U32 port){
  OMXCAM_trace ("Resetting '%s' capture port", OMXCAM_ctx.camera.name);
  
  OMX_ERRORTYPE error;
  
  OMX_CONFIG_PORTBOOLEANTYPE cameraCapturePort;
  OMX_INIT_STRUCTURE (cameraCapturePort);
  cameraCapturePort.nPortIndex = port;
  cameraCapturePort.bEnabled = OMX_FALSE;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigPortCapturing, &cameraCapturePort))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigPortCapturing: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  return 0;
}

void OMXCAM_initCameraSettings (
    uint32_t width,
    uint32_t height,
    OMXCAM_CAMERA_SETTINGS* settings){
  settings->width = width;
  settings->height = height;
  settings->sharpness = 0;
  settings->contrast = 0;
  settings->brightness = 50;
  settings->saturation = 0;
  settings->shutterSpeedAuto = OMXCAM_TRUE;
  //1/8 seconds in milliseconds
  settings->shutterSpeed = 125;
  settings->isoAuto = OMXCAM_TRUE;
  settings->iso = 100;
  settings->exposure = OMXCAM_ExposureAuto;
  settings->exposureCompensation = 0;
  settings->mirror = OMXCAM_MirrorNone;
  settings->rotation = OMXCAM_RotationNone;
  settings->colorEnable = OMXCAM_FALSE;
  settings->colorU = 128;
  settings->colorV = 128;
  settings->noiseReduction = OMXCAM_TRUE;
  settings->frameStabilisation = OMXCAM_FALSE;
  settings->metering = OMXCAM_MeteringAverage;
  settings->whiteBalance = OMXCAM_WhiteBalanceAuto;
  settings->whiteBalanceRedGain = 0.1;
  settings->whiteBalanceBlueGain = 0.1;
  settings->imageFilter = OMXCAM_ImageFilterNone;
  settings->framerate = 30;
}

int OMXCAM_setCameraSettings (OMXCAM_CAMERA_SETTINGS* settings){
  OMXCAM_trace ("Configuring '%s' settings", OMXCAM_ctx.camera.name);

  OMX_ERRORTYPE error;
  
  //Sharpness
  OMX_CONFIG_SHARPNESSTYPE sharpness;
  OMX_INIT_STRUCTURE (sharpness);
  sharpness.nPortIndex = OMX_ALL;
  sharpness.nSharpness = settings->sharpness;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonSharpness, &sharpness))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonSharpness: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Contrast
  OMX_CONFIG_CONTRASTTYPE contrast;
  OMX_INIT_STRUCTURE (contrast);
  contrast.nPortIndex = OMX_ALL;
  contrast.nContrast = settings->contrast;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonContrast, &contrast))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonContrast: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Saturation
  OMX_CONFIG_SATURATIONTYPE saturation;
  OMX_INIT_STRUCTURE (saturation);
  saturation.nPortIndex = OMX_ALL;
  saturation.nSaturation = settings->saturation;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonSaturation, &saturation))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonSaturation: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Brightness
  OMX_CONFIG_BRIGHTNESSTYPE brightness;
  OMX_INIT_STRUCTURE (brightness);
  brightness.nPortIndex = OMX_ALL;
  brightness.nBrightness = settings->brightness;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonBrightness, &brightness))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonBrightness: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Exposure value
  OMX_CONFIG_EXPOSUREVALUETYPE exposureValue;
  OMX_INIT_STRUCTURE (exposureValue);
  exposureValue.nPortIndex = OMX_ALL;
  exposureValue.eMetering = settings->metering;
  exposureValue.xEVCompensation =
      (OMX_S32)((settings->exposureCompensation<<16)/6.0);
  exposureValue.nShutterSpeedMsec = settings->shutterSpeed*1000;
  exposureValue.bAutoShutterSpeed = settings->shutterSpeedAuto;
  exposureValue.nSensitivity = settings->iso;
  exposureValue.bAutoSensitivity = settings->isoAuto;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonExposureValue, &exposureValue))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonExposureValue: "
        "%s", OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Exposure control
  OMX_CONFIG_EXPOSURECONTROLTYPE exposureControl;
  OMX_INIT_STRUCTURE (exposureControl);
  exposureControl.nPortIndex = OMX_ALL;
  exposureControl.eExposureControl = settings->exposure;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonExposure, &exposureControl))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonExposure: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Frame stabilisation
  OMX_CONFIG_FRAMESTABTYPE frameStabilisation;
  OMX_INIT_STRUCTURE (frameStabilisation);
  frameStabilisation.nPortIndex = OMX_ALL;
  frameStabilisation.bStab = settings->frameStabilisation;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonFrameStabilisation, &frameStabilisation))){
    OMXCAM_error ("OMX_SetConfig - "
        "OMX_IndexConfigCommonFrameStabilisation: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //White balance
  OMX_CONFIG_WHITEBALCONTROLTYPE whiteBalance;
  OMX_INIT_STRUCTURE (whiteBalance);
  whiteBalance.nPortIndex = OMX_ALL;
  whiteBalance.eWhiteBalControl = settings->whiteBalance;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonWhiteBalance, &whiteBalance))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonWhiteBalance: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //White balance gains (if white balance is set to off)
  if (!settings->whiteBalance){
    OMX_CONFIG_CUSTOMAWBGAINSTYPE whiteBalanceGains;
    OMX_INIT_STRUCTURE (whiteBalanceGains);
    whiteBalanceGains.xGainR = (OMX_U32)(settings->whiteBalanceRedGain*65536);
    whiteBalanceGains.xGainB = (OMX_U32)(settings->whiteBalanceBlueGain*65536);
    if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
        OMX_IndexConfigCustomAwbGains, &whiteBalanceGains))){
      OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCustomAwbGains: %s",
          OMXCAM_dump_OMX_ERRORTYPE (error));
      return -1;
    }
  }
  
  //Image filter
  OMX_CONFIG_IMAGEFILTERTYPE imageFilter;
  OMX_INIT_STRUCTURE (imageFilter);
  imageFilter.nPortIndex = OMX_ALL;
  imageFilter.eImageFilter = settings->imageFilter;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonImageFilter, &imageFilter))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonImageFilter: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Mirror
  OMX_CONFIG_MIRRORTYPE mirror;
  OMX_INIT_STRUCTURE (mirror);
  mirror.nPortIndex = 72;
  mirror.eMirror = settings->mirror;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonMirror, &mirror))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonMirror: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Rotation
  OMX_CONFIG_ROTATIONTYPE rotation;
  OMX_INIT_STRUCTURE (rotation);
  rotation.nPortIndex = 72;
  rotation.nRotation = settings->rotation;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonRotate, &rotation))){
    OMXCAM_error ("OMX_SetConfig - OMX_IndexConfigCommonRotate: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Color enhancement
  OMX_CONFIG_COLORENHANCEMENTTYPE colorEnhancement;
  OMX_INIT_STRUCTURE (colorEnhancement);
  colorEnhancement.nPortIndex = OMX_ALL;
  colorEnhancement.bColorEnhancement = settings->colorEnable;
  colorEnhancement.nCustomizedU = settings->colorU;
  colorEnhancement.nCustomizedV = settings->colorV;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigCommonColorEnhancement, &colorEnhancement))){
    OMXCAM_error ("OMX_SetConfig - "
        "OMX_IndexConfigCommonColorEnhancement: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  //Denoise
  OMX_CONFIG_BOOLEANTYPE denoise;
  OMX_INIT_STRUCTURE (denoise);
  denoise.bEnabled = settings->noiseReduction;
  if ((error = OMX_SetConfig (OMXCAM_ctx.camera.handle,
      OMX_IndexConfigStillColourDenoiseEnable, &denoise))){
    OMXCAM_error ("OMX_SetConfig - "
        "OMX_IndexConfigStillColourDenoiseEnable: %s",
        OMXCAM_dump_OMX_ERRORTYPE (error));
    return -1;
  }
  
  return 0;
}