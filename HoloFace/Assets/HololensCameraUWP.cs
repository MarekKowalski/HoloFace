using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.XR.WSA.WebCam;

using System.Linq;

#if WINDOWS_UWP
using Windows.Graphics.Imaging;
using Windows.Media.MediaProperties;
using System.Threading.Tasks;
using System.Runtime.InteropServices.WindowsRuntime;
using HoloLensForCV;
#endif

public class HololensCameraUWP : MonoBehaviour
{
    bool webcamInitialized = false;
    bool imageInitialized = false;

    int width;
    int height;

    Matrix4x4 webcamToWorldMatrix;
    Matrix4x4 projectionMatrix;

    DateTimeOffset lastFrameTimestamp;

    public bool Initialized
    {
        get
        {
            return webcamInitialized && imageInitialized;
        }
    }

    public Matrix4x4 WebcamToWorldMatrix
    {
        get
        {
            if (imageInitialized)
                return webcamToWorldMatrix;
            else
                return Matrix4x4.zero;
        }
    }

    public Matrix4x4 ProjectionMatrix
    {
        get
        {
            if (imageInitialized)
                return projectionMatrix;
            else
                return Matrix4x4.zero;
        }
    }

    public int Width
    {
        get
        {
            if (imageInitialized)
                return width;
            else
                return 0;
        }
    }

    public int Height
    {
        get
        {
            if (imageInitialized)
                return height;
            else
                return 0;
        }
    }

#if WINDOWS_UWP && !UNITY_EDITOR
    MediaFrameSourceGroup _holoLensMediaFrameSourceGroup = new MediaFrameSourceGroup(MediaFrameSourceGroupType.PhotoVideoCamera, new SpatialPerception(), null);

    public void Start()
    {
        InitializeMediaCapture();
    }

    public void Update()
    {
    }

    public void OnApplicationFocus(bool hasFocus)
    {
        if (hasFocus)
            InitializeMediaCapture();
        else
        {
            _holoLensMediaFrameSourceGroup.StopAsync();
            webcamInitialized = false;
        }

    }

    public SoftwareBitmap GetImage()
    {
        if (!webcamInitialized)
            return null;

        SensorFrame latestFrame;
        latestFrame = _holoLensMediaFrameSourceGroup.GetLatestSensorFrame(SensorType.PhotoVideo);

        if (latestFrame == null || latestFrame.Timestamp == lastFrameTimestamp)
            return null;

        lastFrameTimestamp = latestFrame.Timestamp;

        webcamToWorldMatrix.m00 = latestFrame.FrameToOrigin.M11;
        webcamToWorldMatrix.m01 = latestFrame.FrameToOrigin.M21;
        webcamToWorldMatrix.m02 = latestFrame.FrameToOrigin.M31;

        webcamToWorldMatrix.m10 = latestFrame.FrameToOrigin.M12;
        webcamToWorldMatrix.m11 = latestFrame.FrameToOrigin.M22;
        webcamToWorldMatrix.m12 = latestFrame.FrameToOrigin.M32;

        webcamToWorldMatrix.m20 = -latestFrame.FrameToOrigin.M13;
        webcamToWorldMatrix.m21 = -latestFrame.FrameToOrigin.M23;
        webcamToWorldMatrix.m22 = -latestFrame.FrameToOrigin.M33;

        webcamToWorldMatrix.m03 = latestFrame.FrameToOrigin.Translation.X;
        webcamToWorldMatrix.m13 = latestFrame.FrameToOrigin.Translation.Y;
        webcamToWorldMatrix.m23 = -latestFrame.FrameToOrigin.Translation.Z;
        webcamToWorldMatrix.m33 = 1;


        if (imageInitialized == false)
        {
            height = latestFrame.SoftwareBitmap.PixelHeight;
            width = latestFrame.SoftwareBitmap.PixelWidth;

            projectionMatrix = new Matrix4x4();
            projectionMatrix.m00 = 2 * latestFrame.CameraIntrinsics.FocalLength.X / width;
            projectionMatrix.m11 = 2 * latestFrame.CameraIntrinsics.FocalLength.Y / height;
            projectionMatrix.m02 = -2 * (latestFrame.CameraIntrinsics.PrincipalPoint.X - width / 2) / width;
            projectionMatrix.m12 = 2 * (latestFrame.CameraIntrinsics.PrincipalPoint.Y - height / 2) / height;
            projectionMatrix.m22 = -1;
            projectionMatrix.m33 = -1;

            imageInitialized = true;
        }


        return latestFrame.SoftwareBitmap;
    }

    async Task InitializeMediaCapture()
    { 
        await _holoLensMediaFrameSourceGroup.StartAsync();

        webcamInitialized = true;
    }
#endif
}
