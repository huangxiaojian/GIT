﻿//------------------------------------------------------------------------------
// <copyright file="FTHelper.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once
#include <FaceTrackLib.h>
#include "KinectSensor.h"

#include "gazeTracking.h"

#define VERTEXCOUNT 121
#define TRIANGLECOUNT 206

#define GAZE_TRACKING

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

typedef void (*FTHelperCallBack)(PVOID lpParam);

struct GaseState{
	inline void set(float ox, float oy, int t0, int t1, int t2)
	{
		x = ox;
		y = oy;
		triangle[0] = t0;
		triangle[1] = t1;
		triangle[2] = t2;
	}
	float x;
	float y;
	int triangle[3];
};

class FTHelper
{
public:
    FTHelper();
    ~FTHelper();

    HRESULT Init(HWND hWnd, FTHelperCallBack callBack, PVOID callBackParam, 
        NUI_IMAGE_TYPE depthType, NUI_IMAGE_RESOLUTION depthRes, BOOL bNearMode, BOOL bFallbackToDefault, NUI_IMAGE_TYPE colorType, NUI_IMAGE_RESOLUTION colorRes, BOOL bSeatedSkeletonMode);
    HRESULT Stop();
    IFTResult* GetResult()      { return(m_pFTResult);}
    BOOL IsKinectPresent()      { return(m_KinectSensorPresent);}
    IFTImage* GetColorImage()   { return(m_colorImage);}
    float GetXCenterFace()      { return(m_XCenterFace);}
    float GetYCenterFace()      { return(m_YCenterFace);}
    void SetDrawMask(BOOL drawMask) { m_DrawMask = drawMask;}
    BOOL GetDrawMask()          { return(m_DrawMask);}
    IFTFaceTracker* GetTracker() { return(m_pFaceTracker);}
    HRESULT GetCameraConfig(FT_CAMERA_CONFIG* cameraConfig);

	RECT& GetFaceRect()			{return m_faceRect;}

	FT_VECTOR3D* GetVertices()	{return m_pPts3D;}
	FT_TRIANGLE* GetTriangles() {return m_pTriangles;}
	int GetVertexNum()			{return VERTEXCOUNT;}
	int GetTriangleNum()		{return TRIANGLECOUNT;}

	bool isSuccessful()			{return m_LastTrackSucceeded;}
	float GetPupilR()			{return m_pupilR;}

	FT_VECTOR3D& GetLeftPupil()	{return m_leftPupil;}
	FT_VECTOR3D& GetRightPupil(){return m_rightPupil;}

private:
    KinectSensor                m_KinectSensor;
    BOOL                        m_KinectSensorPresent;
    IFTFaceTracker*             m_pFaceTracker;
    HWND                        m_hWnd;
    IFTResult*                  m_pFTResult;
    IFTImage*                   m_colorImage;
    IFTImage*                   m_depthImage;
    FT_VECTOR3D                 m_hint3D[2];
    bool                        m_LastTrackSucceeded;
    bool                        m_ApplicationIsRunning;
    FTHelperCallBack            m_CallBack;
    LPVOID                      m_CallBackParam;
    float                       m_XCenterFace;
    float                       m_YCenterFace;
    HANDLE                      m_hFaceTrackingThread;
    BOOL                        m_DrawMask;
    NUI_IMAGE_TYPE              m_depthType;
    NUI_IMAGE_RESOLUTION        m_depthRes;
    BOOL                        m_bNearMode;
    BOOL                        m_bFallbackToDefault;
    BOOL                        m_bSeatedSkeletonMode;
    NUI_IMAGE_TYPE              m_colorType;
    NUI_IMAGE_RESOLUTION        m_colorRes;

#ifdef GAZE_TRACKING
	GazeTracking*				m_gazeTrack;
#endif
	RECT						m_faceRect;
	FT_VECTOR3D					m_pPts3D[VERTEXCOUNT];
	FT_VECTOR2D					m_pPts2D[VERTEXCOUNT];
	FT_TRIANGLE*				m_pTriangles;
	UINT						m_TriangleCount;
	FT_VECTOR3D					m_leftPupil;
	FT_VECTOR3D					m_rightPupil;
	float						m_pupilR;
	GaseState					m_gazeLastState[2];

    BOOL SubmitFraceTrackingResult(IFTResult* pResult);
    void SetCenterOfImage(IFTResult* pResult);
    void CheckCameraInput();
    DWORD WINAPI FaceTrackingThread();
    static DWORD WINAPI FaceTrackingStaticThread(PVOID lpParam);

	void GetVerticesPosition(IFTModel* ftModel);

	//visualization
	HRESULT VisualizeFacetracker(UINT32 color);
	HRESULT VisualizeFaceModel(IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, FLOAT zoomFactor, POINT viewOffset, UINT32 color);
	void DrawGazeInImage(POINT pos, int radius, UINT32 color);
	void SaveModel(IFTModel* model, const float* pSUs, UINT32 suCount, const float* pAUs, UINT32 auCount, float scale, const float* rotationXYZ, const float* translationXYZ, int count);

	void Map2Dto3D();
	float PointDis(int n, int m);
	void GetPupilFromLastState(FT_VECTOR3D& pupil, GaseState& gazeState);
};
