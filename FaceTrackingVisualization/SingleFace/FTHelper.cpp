﻿//------------------------------------------------------------------------------
// <copyright file="FTHelper.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "FTHelper.h"
#include "Visualize.h"

#include <cmath>

#ifdef _DEBUG
#include <iostream>
#endif

#ifdef SAMPLE_OPTIONS
#include "Options.h"
#else
PVOID _opt = NULL;
#endif

#include "utilVector.h"

#define FACETRIANGLESINDEXARRAY {69,53,70,70,73,69,74,73,70,74,56,73,68,20,67,68,67,72,72,67,71,71,23,72}
//#define FACETRIANGLESINDEXARRAY {70,54,71,71,74,70,75,74,71,75,57,74,69,21,68,69,68,73,73,68,72,72,24,73}
#define ISZERO(x) (fabsf((x))<0.00001)
#define ISFTVECTOR3DZERO(v) (ISZERO(((FT_VECTOR3D)(v)).x) && ISZERO(((FT_VECTOR3D)(v)).y) && ISZERO(((FT_VECTOR3D)(v)).z))

namespace util{

	FT_VECTOR3D PLUS(const FT_VECTOR3D& a, const FT_VECTOR3D& b)
	{
		return FT_VECTOR3D(a.x+b.x, a.y+b.y, a.z+b.z);
	}
	FT_VECTOR3D MINUS(const FT_VECTOR3D& a, const FT_VECTOR3D& b)
	{
		return FT_VECTOR3D(a.x-b.x, a.y-b.y, a.z-b.z);
	}
	FT_VECTOR3D TIMES(const FT_VECTOR3D& a, float t)
	{
		return FT_VECTOR3D(a.x*t, a.y*t, a.z*t);
	}

	FT_VECTOR3D triangleMap(const POINT& p, const POINT& a, const POINT& b, const POINT& c, const FT_VECTOR3D& d, const FT_VECTOR3D& e, const FT_VECTOR3D& f, float& x, float& y)
	{
		x = ((p.x-a.x)*(b.y-a.y)-(p.y-a.y)*(b.x-a.x))*1.0/((c.x-a.x)*(b.y-a.y)-(c.y-a.y)*(b.x-a.x));
		y = ((p.x-a.x)*(c.y-a.y)-(p.y-a.y)*(c.x-a.x))*1.0/((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x));
		return FT_VECTOR3D(x*(e.x-d.x)+y*(f.x-d.x)+d.x, x*(e.y-d.y)+y*(f.y-d.y)+d.y, x*(e.z-d.z)+y*(f.z-d.z)+d.z);
	}

	FT_VECTOR3D triangleMap(const POINT& p, const POINT P[3], const FT_VECTOR3D V[3], float& x, float& y)
	{
		return triangleMap(p, P[0], P[1], P[2], V[0], V[1], V[2], x ,y);
	}

	bool PointinTriangle(const POINT &A, const POINT &B, const POINT &C, const POINT &P, float tol = 0.0)
	{
		POINT v0 = {C.x-A.x, C.y-A.y};
		POINT v1 = {B.x-A.x, B.y-A.y};
		POINT v2 = {P.x-A.x, P.y-A.y};

		LONG dot00 = v0.x*v0.x+v0.y*v0.y ;
		LONG dot01 = v0.x*v1.x+v0.y*v1.y ;
		LONG dot02 = v0.x*v2.x+v0.y*v2.y ;
		LONG dot11 = v1.x*v1.x+v1.y*v1.y ;
		LONG dot12 = v1.x*v2.x+v1.y*v2.y ;

		double inverDeno = 1.0 / (dot00 * dot11 - dot01 * dot01) ;

		double u = (dot11 * dot02 - dot01 * dot12) * inverDeno ;
		if (u+tol < 0 || u-tol > 1) // if u out of range, return directly
		{
			return false ;
		}

		double v = (dot00 * dot12 - dot01 * dot02) * inverDeno ;
		if (v+tol < 0 || v-tol > 1) // if v out of range, return directly
		{
			return false ;
		}

		return u + v + tol <= 1 ;
	}

	bool PointinTriangle(const POINT &A, const POINT P[3], float tol = 0.0)
	{
		return PointinTriangle(P[0], P[1], P[2], A, tol);
	}

	POINT IntToPOINT(int x, int y)
	{
		POINT t;
		t.x = x;
		t.y = y;
		return t;
	}

	POINT LONGToPOINT(LONG x, LONG y)
	{
		POINT t;
		t.x = x;
		t.y = y;
		return t;
	}

	POINT FloatToPOINT(float x, float y)
	{
		POINT t;
		t.x = LONG(x + 0.5f);
		t.y = LONG(y + 0.5f);
		return t;
	}

	Vector3 FT_VECTOR3DtoVector3(const FT_VECTOR3D& p)
	{
		return Vector3(p.x, p.y, p.z);
	}

	FT_VECTOR3D Vector3ToFT_VECTOR3D(const Vector3& p)
	{
		return FT_VECTOR3D(p[0], p[1], p[2]);
	}

	Vector3 Normal(const Vector3& a, const Vector3& b, const Vector3& c)
	{
		Vector3 p1 = b-a;
		Vector3 p2 = c-b;
		return (p1%p2).normalize();
	}

	FT_VECTOR3D Normal(const FT_VECTOR3D &A, const FT_VECTOR3D &B, const FT_VECTOR3D& C)
	{
		return Vector3ToFT_VECTOR3D(Normal(FT_VECTOR3DtoVector3(A), FT_VECTOR3DtoVector3(B), FT_VECTOR3DtoVector3(C)));
	}

	FT_VECTOR3D Normal(FT_VECTOR3D P[3])
	{
		return Normal(P[0], P[1], P[2]);
	}
};


FTHelper::FTHelper()
{
    m_pFaceTracker = 0;
    m_hWnd = NULL;
    m_pFTResult = NULL;
    m_colorImage = NULL;
    m_depthImage = NULL;
    m_ApplicationIsRunning = false;
    m_LastTrackSucceeded = false;
    m_CallBack = NULL;
    m_XCenterFace = 0;
    m_YCenterFace = 0;
    m_hFaceTrackingThread = NULL;
    m_DrawMask = TRUE;
    m_depthType = NUI_IMAGE_TYPE_DEPTH;
    m_depthRes = NUI_IMAGE_RESOLUTION_INVALID;
    m_bNearMode = FALSE;
    m_bFallbackToDefault = FALSE;
    m_colorType = NUI_IMAGE_TYPE_COLOR;
    m_colorRes = NUI_IMAGE_RESOLUTION_INVALID;

	memset(m_pPts3D, 0, VERTEXCOUNT*sizeof(FT_VECTOR3D));
	memset(m_pPts2D, 0, VERTEXCOUNT*sizeof(FT_VECTOR2D));
	m_pTriangles = NULL;
	m_TriangleCount = 0;

	memset(&m_leftPupil, 0, sizeof(FT_VECTOR3D));
	memset(&m_rightPupil, 0, sizeof(FT_VECTOR3D));
	m_pupilR = 0;

	m_gazeLastState[0].set(0.5,0,69,74,73);
	m_gazeLastState[1].set(0.5,0,67,72,71);
}

FTHelper::~FTHelper()
{
    Stop();
}

HRESULT FTHelper::Init(HWND hWnd, FTHelperCallBack callBack, PVOID callBackParam, 
                       NUI_IMAGE_TYPE depthType, NUI_IMAGE_RESOLUTION depthRes, BOOL bNearMode, BOOL bFallbackToDefault, NUI_IMAGE_TYPE colorType, NUI_IMAGE_RESOLUTION colorRes, BOOL bSeatedSkeletonMode)
{
    if (!hWnd || !callBack)
    {
        return E_INVALIDARG;
    }
    m_hWnd = hWnd;
    m_CallBack = callBack;
    m_CallBackParam = callBackParam;
    m_ApplicationIsRunning = true;
    m_depthType = depthType;
    m_depthRes = depthRes;
    m_bNearMode = bNearMode;
    m_bFallbackToDefault = bFallbackToDefault;
    m_bSeatedSkeletonMode = bSeatedSkeletonMode;
    m_colorType = colorType;
    m_colorRes = colorRes;
    m_hFaceTrackingThread = CreateThread(NULL, 0, FaceTrackingStaticThread, (PVOID)this, 0, 0);

#ifdef GAZE_TRACKING
	m_gazeTrack = new GazeTracking();
	m_gazeTrack->initialize("res/haarcascade_frontalface_alt.xml");
#endif
    return S_OK;
}

HRESULT FTHelper::Stop()
{
    m_ApplicationIsRunning = false;
    if (m_hFaceTrackingThread)
    {
        WaitForSingleObject(m_hFaceTrackingThread, 1000);
    }
    m_hFaceTrackingThread = 0;
    return S_OK;
}

BOOL FTHelper::SubmitFraceTrackingResult(IFTResult* pResult)
{
    if (pResult != NULL && SUCCEEDED(pResult->GetStatus()))
    {
        if (m_CallBack)
        {
            (*m_CallBack)(m_CallBackParam);
        }

        if (m_DrawMask)
        {
            FLOAT* pSU = NULL;
            UINT numSU;
            BOOL suConverged;
            m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);
            POINT viewOffset = {0, 0};
            FT_CAMERA_CONFIG cameraConfig;
            if (m_KinectSensorPresent)
            {
                m_KinectSensor.GetVideoConfiguration(&cameraConfig);
            }
            else
            {
                cameraConfig.Width = 640;
                cameraConfig.Height = 480;
                cameraConfig.FocalLength = 500.0f;
            }
            IFTModel* ftModel;
            HRESULT hr = m_pFaceTracker->GetFaceModel(&ftModel);
            if (SUCCEEDED(hr))
            {
				//////////////////////////////////////////////////////////////////////////
#ifndef ASSOCIATE			
				IplImage *img = cvCreateImage(cvSize(m_colorImage->GetWidth(), m_colorImage->GetHeight()), IPL_DEPTH_8U, 4);
				memcpy(img->imageData, m_colorImage->GetBuffer(), m_colorImage->GetBufferSize());
				cv::Mat frame(img, true);
#else
				cv::Mat frame(m_KinectSensor.GetImage(), true);
				//cv::flip(frame, frame, 1);
#endif
				if(!frame.empty())
				{
					m_gazeTrack->process(frame);
				}
				//////////////////////////////////////////////////////////////////////////
            }

			FLOAT *pAUs;
			UINT auCount;
			hr = m_pFTResult->GetAUCoefficients(&pAUs, &auCount);
			FLOAT scale, rotationXYZ[3], translationXYZ[3];
			m_pFTResult->Get3DPose(&scale, rotationXYZ, translationXYZ);
			ftModel->Get3DShape(pSU, ftModel->GetSUCount(), pAUs, ftModel->GetAUCount(), scale, rotationXYZ, translationXYZ, m_pPts3D, VERTEXCOUNT);
			ftModel->GetProjectedShape(&cameraConfig, 1.0, viewOffset, pSU, ftModel->GetSUCount(), pAUs, auCount, 
				scale, rotationXYZ, translationXYZ, m_pPts2D, VERTEXCOUNT);
			ftModel->GetTriangles(&m_pTriangles, &m_TriangleCount);

			//hr = VisualizeFaceModel(m_colorImage, ftModel, &cameraConfig, pSU, 1.0, viewOffset, pResult, 0x00FFFF00);
			//VisualizeFaceModel(ftModel, &cameraConfig, pSU, 1.0, viewOffset, 0x00ff0000);
			m_pupilR = (PointDis(69, 74)+PointDis(70,73)+PointDis(67,72)+PointDis(68,71))/16;

			if(m_gazeTrack->isFindFace())
			{	
				Map2Dto3D();
				
				//static int count = 0;
				//SaveModel(ftModel, pSU, ftModel->GetSUCount(), pAUs, ftModel->GetAUCount(), 1.0, rotationXYZ, translationXYZ, count++);
			}
			ftModel->Release();
        }
    }
    return TRUE;
}

// We compute here the nominal "center of attention" that is used when zooming the presented image.
void FTHelper::SetCenterOfImage(IFTResult* pResult)
{
    float centerX = ((float)m_colorImage->GetWidth())/2.0f;
    float centerY = ((float)m_colorImage->GetHeight())/2.0f;
    if (pResult)
    {
        if (SUCCEEDED(pResult->GetStatus()))
        {
            RECT faceRect;
            pResult->GetFaceRect(&faceRect);
            centerX = (faceRect.left+faceRect.right)/2.0f;
            centerY = (faceRect.top+faceRect.bottom)/2.0f;
        }
        m_XCenterFace += 0.02f*(centerX-m_XCenterFace);
        m_YCenterFace += 0.02f*(centerY-m_YCenterFace);
    }
    else
    {
        m_XCenterFace = centerX;
        m_YCenterFace = centerY;
    }
}

// Get a video image and process it.
void FTHelper::CheckCameraInput()
{
    HRESULT hrFT = E_FAIL;

    if (m_KinectSensorPresent && m_KinectSensor.GetVideoBuffer())
    {
        HRESULT hrCopy = m_KinectSensor.GetVideoBuffer()->CopyTo(m_colorImage, NULL, 0, 0);
        if (SUCCEEDED(hrCopy) && m_KinectSensor.GetDepthBuffer())
        {
            hrCopy = m_KinectSensor.GetDepthBuffer()->CopyTo(m_depthImage, NULL, 0, 0);
        }
        // Do face tracking
        if (SUCCEEDED(hrCopy))
        {
            FT_SENSOR_DATA sensorData(m_colorImage, m_depthImage, m_KinectSensor.GetZoomFactor(), m_KinectSensor.GetViewOffSet());

            FT_VECTOR3D* hint = NULL;
            if (SUCCEEDED(m_KinectSensor.GetClosestHint(m_hint3D)))
            {
                hint = m_hint3D;
            }
            if (m_LastTrackSucceeded)
            {
                hrFT = m_pFaceTracker->ContinueTracking(&sensorData, hint, m_pFTResult);
            }
            else
            {
                hrFT = m_pFaceTracker->StartTracking(&sensorData, NULL, hint, m_pFTResult);
            }
        }
    }

    m_LastTrackSucceeded = SUCCEEDED(hrFT) && SUCCEEDED(m_pFTResult->GetStatus());
    if (m_LastTrackSucceeded)
    {
        SubmitFraceTrackingResult(m_pFTResult);
    }
    else
    {
        m_pFTResult->Reset();
    }
    SetCenterOfImage(m_pFTResult);
}

DWORD WINAPI FTHelper::FaceTrackingStaticThread(PVOID lpParam)
{
    FTHelper* context = static_cast<FTHelper*>(lpParam);
    if (context)
    {
        return context->FaceTrackingThread();
    }
    return 0;
}

DWORD WINAPI FTHelper::FaceTrackingThread()
{
    FT_CAMERA_CONFIG videoConfig;
    FT_CAMERA_CONFIG depthConfig;
    FT_CAMERA_CONFIG* pDepthConfig = NULL;

    // Try to get the Kinect camera to work
    HRESULT hr = m_KinectSensor.Init(m_depthType, m_depthRes, m_bNearMode, m_bFallbackToDefault, m_colorType, m_colorRes, m_bSeatedSkeletonMode);
    if (SUCCEEDED(hr))
    {
        m_KinectSensorPresent = TRUE;
        m_KinectSensor.GetVideoConfiguration(&videoConfig);
        m_KinectSensor.GetDepthConfiguration(&depthConfig);
        pDepthConfig = &depthConfig;
        m_hint3D[0] = m_hint3D[1] = FT_VECTOR3D(0, 0, 0);
    }
    else
    {
        m_KinectSensorPresent = FALSE;
        WCHAR errorText[MAX_PATH];
        ZeroMemory(errorText, sizeof(WCHAR) * MAX_PATH);
        wsprintf(errorText, L"Could not initialize the Kinect sensor. hr=0x%x\n", hr);
        MessageBoxW(m_hWnd, errorText, L"Face Tracker Initialization Error\n", MB_OK);
        return 1;
    }

    // Try to start the face tracker.
    m_pFaceTracker = FTCreateFaceTracker(_opt);
    if (!m_pFaceTracker)
    {
        MessageBoxW(m_hWnd, L"Could not create the face tracker.\n", L"Face Tracker Initialization Error\n", MB_OK);
        return 2;
    }

    hr = m_pFaceTracker->Initialize(&videoConfig, pDepthConfig, NULL, NULL); 
    if (FAILED(hr))
    {
        WCHAR path[512], buffer[1024];
        GetCurrentDirectoryW(ARRAYSIZE(path), path);
        wsprintf(buffer, L"Could not initialize face tracker (%s). hr=0x%x", path, hr);

        MessageBoxW(m_hWnd, /*L"Could not initialize the face tracker.\n"*/ buffer, L"Face Tracker Initialization Error\n", MB_OK);

        return 3;
    }

    hr = m_pFaceTracker->CreateFTResult(&m_pFTResult);
    if (FAILED(hr) || !m_pFTResult)
    {
        MessageBoxW(m_hWnd, L"Could not initialize the face tracker result.\n", L"Face Tracker Initialization Error\n", MB_OK);
        return 4;
    }

    // Initialize the RGB image.
    m_colorImage = FTCreateImage();
    if (!m_colorImage || FAILED(hr = m_colorImage->Allocate(videoConfig.Width, videoConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8)))
    {
        return 5;
    }

    if (pDepthConfig)
    {
        m_depthImage = FTCreateImage();
        if (!m_depthImage || FAILED(hr = m_depthImage->Allocate(depthConfig.Width, depthConfig.Height, FTIMAGEFORMAT_UINT16_D13P3)))
        {
            return 6;
        }
    }

    SetCenterOfImage(NULL);
    m_LastTrackSucceeded = false;

    while (m_ApplicationIsRunning)
    {
        CheckCameraInput();
        InvalidateRect(m_hWnd, NULL, FALSE);
        UpdateWindow(m_hWnd);
        Sleep(16);
    }

    m_pFaceTracker->Release();
    m_pFaceTracker = NULL;

    if(m_colorImage)
    {
        m_colorImage->Release();
        m_colorImage = NULL;
    }

    if(m_depthImage) 
    {
        m_depthImage->Release();
        m_depthImage = NULL;
    }

    if(m_pFTResult)
    {
        m_pFTResult->Release();
        m_pFTResult = NULL;
    }
    m_KinectSensor.Release();
    return 0;
}

HRESULT FTHelper::GetCameraConfig(FT_CAMERA_CONFIG* cameraConfig)
{
    return m_KinectSensorPresent ? m_KinectSensor.GetVideoConfiguration(cameraConfig) : E_FAIL;
}

HRESULT FTHelper::VisualizeFacetracker(UINT32 color)
{
	if (!m_colorImage->GetBuffer() || !m_pFTResult)
	{
		return E_POINTER;
	}

	// Insufficient data points to render face data
	FT_VECTOR2D* pPts2D;
	UINT pts2DCount;
	HRESULT hr = m_pFTResult->Get2DShapePoints(&pPts2D, &pts2DCount);
	if (FAILED(hr))
	{
		return hr;
	}

	if (pts2DCount < 86)
	{
		return E_INVALIDARG;
	}


	POINT* pFaceModel2DPoint = reinterpret_cast<POINT*>(_malloca(sizeof(POINT) * pts2DCount));
	if (!pFaceModel2DPoint)
	{
		return E_OUTOFMEMORY;
	}


	for (UINT ipt = 0; ipt < pts2DCount; ++ipt)
	{
		pFaceModel2DPoint[ipt].x = LONG(pPts2D[ipt].x + 0.5f);
		pFaceModel2DPoint[ipt].y = LONG(pPts2D[ipt].y + 0.5f);
	}

	for (UINT ipt = 0; ipt < 8; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[(ipt+1)%8];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}

	for (UINT ipt = 8; ipt < 16; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[(ipt-8+1)%8+8];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}

	for (UINT ipt = 16; ipt < 26; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[(ipt-16+1)%10+16];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}

	for (UINT ipt = 26; ipt < 36; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[(ipt-26+1)%10+26];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}

	for (UINT ipt = 36; ipt < 47; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[ipt+1];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}

	for (UINT ipt = 48; ipt < 60; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[(ipt-48+1)%12+48];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}

	for (UINT ipt = 60; ipt < 68; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[(ipt-60+1)%8+60];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}

	for (UINT ipt = 68; ipt < 86; ++ipt)
	{
		POINT ptStart = pFaceModel2DPoint[ipt];
		POINT ptEnd = pFaceModel2DPoint[ipt+1];
		m_colorImage->DrawLine(ptStart, ptEnd, color, 1);
	}
	_freea(pFaceModel2DPoint);

	return hr;
}

HRESULT FTHelper::VisualizeFaceModel(IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, FLOAT zoomFactor, POINT viewOffset, UINT32 color)

{
	if (!m_colorImage || !pModel || !pCameraConfig || !pSUCoef || !m_pFTResult)
	{
		return E_POINTER;
	}


	HRESULT hr = S_OK;
	UINT vertexCount = pModel->GetVertexCount();
	FT_VECTOR2D* pPts2D = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D) * vertexCount));
#ifdef _DEBUG	
	FT_VECTOR3D* pPts3D = reinterpret_cast<FT_VECTOR3D*>(_malloca(sizeof(FT_VECTOR3D) * vertexCount));
#endif
	if (pPts2D)
	{
		FLOAT *pAUs;
		UINT auCount;
		hr = m_pFTResult->GetAUCoefficients(&pAUs, &auCount);
		if (SUCCEEDED(hr))
		{
			FLOAT scale, rotationXYZ[3], translationXYZ[3];
			hr = m_pFTResult->Get3DPose(&scale, rotationXYZ, translationXYZ);
#ifdef _DEBUG
			pModel->Get3DShape(pSUCoef, pModel->GetSUCount(), pAUs, pModel->GetAUCount(), scale, rotationXYZ, translationXYZ, pPts3D, vertexCount);
			std::cout << pPts3D[5].x << ' ' << pPts3D[5].y << ' ' << pPts3D[5].z << std::endl;
#endif

			if (SUCCEEDED(hr))
			{
				hr = pModel->GetProjectedShape(pCameraConfig, zoomFactor, viewOffset, pSUCoef, pModel->GetSUCount(), pAUs, auCount, 
					scale, rotationXYZ, translationXYZ, pPts2D, vertexCount);
				if (SUCCEEDED(hr))
				{
					POINT* p3DMdl   = reinterpret_cast<POINT*>(_malloca(sizeof(POINT) * vertexCount));
					if (p3DMdl)
					{
						for (UINT i = 0; i < vertexCount; ++i)
						{
							p3DMdl[i].x = LONG(pPts2D[i].x + 0.5f);
							p3DMdl[i].y = LONG(pPts2D[i].y + 0.5f);
						}

						FT_TRIANGLE* pTriangles;
						UINT triangleCount;
						hr = pModel->GetTriangles(&pTriangles, &triangleCount);
						if (SUCCEEDED(hr))
						{
							struct EdgeHashTable
							{
								UINT32* pEdges;
								UINT edgesAlloc;

								void Insert(int a, int b) 
								{
									UINT32 v = (min(a, b) << 16) | max(a, b);
									UINT32 index = (v + (v << 8)) * 49157, i;
									for (i = 0; i < edgesAlloc - 1 && pEdges[(index + i) & (edgesAlloc - 1)] && v != pEdges[(index + i) & (edgesAlloc - 1)]; ++i)
									{
									}
									pEdges[(index + i) & (edgesAlloc - 1)] = v;
								}
							} eht;

							eht.edgesAlloc = 1 << UINT(log(2.f * (1 + vertexCount + triangleCount)) / log(2.f));
							eht.pEdges = reinterpret_cast<UINT32*>(_malloca(sizeof(UINT32) * eht.edgesAlloc));
							if (eht.pEdges)
							{
								ZeroMemory(eht.pEdges, sizeof(UINT32) * eht.edgesAlloc);
								for (UINT i = 0; i < triangleCount; ++i)
								{ 
									eht.Insert(pTriangles[i].i, pTriangles[i].j);
									eht.Insert(pTriangles[i].j, pTriangles[i].k);
									eht.Insert(pTriangles[i].k, pTriangles[i].i);
								}
								for (UINT i = 0; i < eht.edgesAlloc; ++i)
								{
									if(eht.pEdges[i] != 0)
									{
										m_colorImage->DrawLine(p3DMdl[eht.pEdges[i] >> 16], p3DMdl[eht.pEdges[i] & 0xFFFF], color, 1);
									}
								}
								_freea(eht.pEdges);
							}

							// Render the face rect in magenta
							RECT rectFace;
							hr = m_pFTResult->GetFaceRect(&rectFace);
							if (SUCCEEDED(hr))
							{
								POINT leftTop = {rectFace.left, rectFace.top};
								POINT rightTop = {rectFace.right - 1, rectFace.top};
								POINT leftBottom = {rectFace.left, rectFace.bottom - 1};
								POINT rightBottom = {rectFace.right - 1, rectFace.bottom - 1};
								UINT32 nColor = 0xff00ff;
								SUCCEEDED(hr = m_colorImage->DrawLine(leftTop, rightTop, nColor, 1)) &&
									SUCCEEDED(hr = m_colorImage->DrawLine(rightTop, rightBottom, nColor, 1)) &&
									SUCCEEDED(hr = m_colorImage->DrawLine(rightBottom, leftBottom, nColor, 1)) &&
									SUCCEEDED(hr = m_colorImage->DrawLine(leftBottom, leftTop, nColor, 1));
							}
						}

						_freea(p3DMdl); 
					}
					else
					{
						hr = E_OUTOFMEMORY;
					}
				}
			}
		}
		_freea(pPts2D);
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}

void FTHelper::DrawGazeInImage(POINT pos, int radius, UINT32 color)
{
	POINT up, left, down, right;
	down.x = up.x = pos.x;
	left.y = right.y = pos.y;

	for(int i = 1; i < radius;i ++)
	{
		up.y = pos.y+i;
		left.x = pos.x-i;
		right.x = pos.x + i;
		down.y = pos.y-i;
		m_colorImage->DrawLine(up, left, color, 1);
		m_colorImage->DrawLine(left, down, color, 1);
		m_colorImage->DrawLine(down, right, color, 1);
		m_colorImage->DrawLine(right, up, color, 1);
	}
}

void FTHelper::SaveModel(IFTModel* model, const float* pSUs, UINT32 suCount, const float* pAUs, UINT32 auCount, float scale, const float* rotationXYZ, const float* translationXYZ, int count)
{
	UINT32 vertexCount = model->GetVertexCount();//model->GetVertexCount(); 
	FT_VECTOR3D* pVertices = new FT_VECTOR3D[vertexCount]; 
	model->Get3DShape(pSUs, suCount, pAUs, auCount, scale, rotationXYZ, translationXYZ, pVertices, vertexCount); 
	UINT32 triangleCount = 0; 
	FT_TRIANGLE* pTriangles = NULL; 
	model->GetTriangles(&pTriangles, &triangleCount); 
	FILE* fobj = NULL; 
	char filename[10];
	sprintf_s(filename, "%d", count);
	strcat(filename, ".obj");
	fopen_s(&fobj, filename, "w"); 
	fprintf(fobj, "# %u vertices, # %u faces\n", vertexCount, triangleCount); 
	for (UINT32 vi = 0; vi < vertexCount; ++vi) { 
		fprintf(fobj, "v %f %f %f\n", pVertices[vi].x, pVertices[vi].y, pVertices[vi].z);
	} 
	fprintf(fobj, "v %f %f %f\n", m_leftPupil.x+0.05, m_leftPupil.y, m_leftPupil.z);
	fprintf(fobj, "v %f %f %f\n", m_leftPupil.x-0.05, m_leftPupil.y, m_leftPupil.z);
	fprintf(fobj, "v %f %f %f\n", m_leftPupil.x, m_leftPupil.y+0.08, m_leftPupil.z);
	fprintf(fobj, "v %f %f %f\n", m_rightPupil.x+0.05, m_rightPupil.y, m_rightPupil.z);
	fprintf(fobj, "v %f %f %f\n", m_rightPupil.x-0.05, m_rightPupil.y, m_rightPupil.z);
	fprintf(fobj, "v %f %f %f\n", m_rightPupil.x, m_rightPupil.y+0.08, m_rightPupil.z);
	for (UINT32 ti = 0; ti < triangleCount; ++ti) { 
		fprintf(fobj, "f %d %d %d\n", pTriangles[ti].i+1, pTriangles[ti].j+1, pTriangles[ti].k+1); 
	} 
	fprintf(fobj, "f %d %d %d\n", vertexCount+1, vertexCount+2, vertexCount+3);
	fprintf(fobj, "f %d %d %d\n", vertexCount+4, vertexCount+5, vertexCount+6);
	fclose(fobj); 
	delete[] pVertices; 
	return; 
}

void FTHelper::Map2Dto3D()
{
	cv::Point left = m_gazeTrack->getLeftPupil();
	cv::Point right = m_gazeTrack->getRightPupil();
	//m_gazeTrack->getLeftPupilXY(leftPupil.x, leftPupil.y);
	//m_gazeTrack->getRightPupilXY(rightPupil.x, rightPupil.y);

	//int triangles[][3] = {{70,54,71}, {71,74,70}, {75,74,71}, {75,57,74}, {69,21,68}, {69,68,73}, {73,68,72}, {72,24,73}};
	int triangles[] = FACETRIANGLESINDEXARRAY;
	POINT pupil[2];
	pupil[0].x = left.x;
	pupil[0].y = left.y;
	pupil[1].x = right.x;
	pupil[1].y = right.y;
	
	bool flags[2] = {false, false};
	FT_VECTOR3D tmpPupil[2];

	int halflen = sizeof(triangles)/6/sizeof(triangles[0]);
	//m_pupilR = (PointDis(70, 75)+PointDis(71,74)+PointDis(68,73)+PointDis(69,72))/64;

	/*if(ISFTVECTOR3DZERO(m_leftPupil))
	{
		FT_VECTOR3D mapP = util::TIMES(util::PLUS(m_pPts3D[70], m_pPts3D[73]), 0.5);
		FT_VECTOR3D n = util::Normal(m_pPts3D[70], m_pPts3D[73], m_pPts3D[69]);
		m_leftPupil = util::PLUS(mapP, util::TIMES(n,-m_pupilR));
	}
	if(ISFTVECTOR3DZERO(m_rightPupil))
	{
		FT_VECTOR3D mapP = util::TIMES(util::PLUS(m_pPts3D[67], m_pPts3D[72]), 0.5);
		FT_VECTOR3D n = util::Normal(m_pPts3D[68], m_pPts3D[67], m_pPts3D[72]);
		m_rightPupil = util::PLUS(mapP, util::TIMES(n,-m_pupilR));
	}*/

	for(int index = 0; index < 2; index++)
	{
		for(int i = index*halflen; i < halflen*(index+1); i++)
		{
			POINT p[3];
			FT_VECTOR3D q[3];
			float x, y;
			for(int j = 0; j < 3; j++)
			{
				p[j] = util::FloatToPOINT(m_pPts2D[triangles[i*3+j]].x, m_pPts2D[triangles[i*3+j]].y);
				q[j] = m_pPts3D[triangles[i*3+j]];
			}
			//POINT a = util::FloatToPOINT(m_pPts2D[triangles[i*3]].x, m_pPts2D[triangles[i*3]].y);
			//POINT b = util::FloatToPOINT();
			//if(util::PointinTriangle())
			if(util::PointinTriangle(pupil[index], p, 0))
			{
				FT_VECTOR3D mapP = util::triangleMap(pupil[index], p, q, x, y);
				FT_VECTOR3D n = util::Normal(q);
				//FT_VECTOR3D center(mapP.x+n.x, mapP.y+n.y, mapP.z+n.z);
#ifdef _DEBUG
				std::cout << "x:" << x << "\ty:" << y << std::endl;
#endif // _DEBUG

				m_gazeLastState[index].set(x, y, triangles[i*3], triangles[i*3+1], triangles[i*3+2]);
				tmpPupil[index] = util::PLUS(mapP, util::TIMES(n, -m_pupilR));
				std::cout << "index:" << tmpPupil[index].x << ' ' << tmpPupil[index].y << ' ' << tmpPupil[2].z << std::endl;
 				/*tmpPupil[index].x = center.x+n.x*m_pupilR;
				tmpPupil[index].y = center.y+n.y*m_pupilR;
				tmpPupil[index].z = center.z+n.z*m_pupilR;*/
				flags[index] = true;
				break;
			}
		}
	}
	if(flags[0])
	{
#ifdef _DEBUG
		std::cout << "Map2Dto3D(): left is OK" << std::endl;
#endif
		m_leftPupil = tmpPupil[0];
	}
	else
	{
		//TODO
		/*m_leftPupil.x = tmpPupil[0].x;
		m_leftPupil.y = tmpPupil[0].y;
		m_leftPupil.z = tmpPupil[0].z;*/
		GetPupilFromLastState(m_leftPupil, m_gazeLastState[0]);
	}
	if(flags[1])
	{
#ifdef _DEBUG
		std::cout << "Map2Dto3D(): right is OK" << std::endl;
#endif
		m_rightPupil = tmpPupil[1];
	}
	else
	{
		//TODO
		/*m_rightPupil.x = tmpPupil[0].x;
		m_rightPupil.y = tmpPupil[0].y;
		m_rightPupil.z = tmpPupil[0].z;*/
		GetPupilFromLastState(m_rightPupil, m_gazeLastState[1]);
	}
#ifdef _DEBUG
	if(flags[0] || flags[1])
	{
		getchar();
		std::cout << "Map2Dto3D(): left gaze:" << m_gazeTrack->getLeftPupil().x << ' ' << m_gazeTrack->getLeftPupil().y << std::endl;
		std::cout << "Map2Dto3D(): right gaze:" << m_gazeTrack->getRightPupil().x << ' ' << m_gazeTrack->getRightPupil().y << std::endl;
		std::cout << "Map2Dto3D(): leftPupil:" << m_leftPupil.x << ' ' << m_leftPupil.y << ' ' << m_leftPupil.z << std::endl;
		std::cout << "Map2Dto3D(): rightPupil:" << m_rightPupil.x << ' ' << m_rightPupil.y << ' ' << m_rightPupil.z << std::endl;
		
		std::cout << m_pPts3D[70-1].x << ' ' << m_pPts3D[70-1].y << ' ' << m_pPts3D[70-1].z << std::endl;
		std::cout << m_pPts3D[75-1].x << ' ' << m_pPts3D[75-1].y << ' ' << m_pPts3D[75-1].z << std::endl;
		std::cout << m_pPts3D[74-1].x << ' ' << m_pPts3D[74-1].y << ' ' << m_pPts3D[74-1].z << std::endl;
		std::cout << m_pPts3D[68-1].x << ' ' << m_pPts3D[68-1].y << ' ' << m_pPts3D[68-1].z << std::endl;
		std::cout << m_pPts3D[73-1].x << ' ' << m_pPts3D[73-1].y << ' ' << m_pPts3D[73-1].z << std::endl;
		std::cout << m_pPts3D[72-1].x << ' ' << m_pPts3D[72-1].y << ' ' << m_pPts3D[72-1].z << std::endl;
		getchar();
	}
#endif
}

float FTHelper::PointDis(int n, int m)
{
	return sqrtf((m_pPts3D[n].x-m_pPts3D[m].x)*(m_pPts3D[n].x-m_pPts3D[m].x)+(m_pPts3D[n].y-m_pPts3D[m].y)*(m_pPts3D[n].y-m_pPts3D[m].y)+(m_pPts3D[n].z-m_pPts3D[m].z)*(m_pPts3D[n].z-m_pPts3D[m].z));
}

void FTHelper::GetPupilFromLastState(FT_VECTOR3D& pupil, GaseState& gazeState)
{
	pupil.x = m_pPts3D[gazeState.triangle[0]].x+(m_pPts3D[gazeState.triangle[1]].x-m_pPts3D[gazeState.triangle[0]].x)*gazeState.x+(m_pPts3D[gazeState.triangle[2]].x-m_pPts3D[gazeState.triangle[0]].x)*gazeState.y;
	pupil.y = m_pPts3D[gazeState.triangle[0]].y+(m_pPts3D[gazeState.triangle[1]].y-m_pPts3D[gazeState.triangle[0]].y)*gazeState.x+(m_pPts3D[gazeState.triangle[2]].y-m_pPts3D[gazeState.triangle[0]].y)*gazeState.y;
	pupil.z = m_pPts3D[gazeState.triangle[0]].z+(m_pPts3D[gazeState.triangle[1]].z-m_pPts3D[gazeState.triangle[0]].z)*gazeState.x+(m_pPts3D[gazeState.triangle[2]].z-m_pPts3D[gazeState.triangle[0]].z)*gazeState.y;
}