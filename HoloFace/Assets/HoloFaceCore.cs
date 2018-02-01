using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System;
using System.Linq;
using FaceProcessing;
using UnityEngine.Windows.Speech;

#if WINDOWS_UWP
using System.Threading.Tasks;
using Windows.Graphics.Imaging;
#endif

public class HoloFaceCore : MonoBehaviour
{
    [Tooltip("Shows FPS when Debug mode is enabled.")]
    public Text FPSText;
    public Text BackendFaceTrackerTipText;
    public float LocalTrackerConfidenceThreshold = 900.0f;
    public float BackendTrackerConfidenceThreshold = 0.1f;
    public int LocalTrackerNumberOfIters = 3;
    public int nLandmarks = 51;

    HololensCameraUWP webcam;
    LocalFaceTracker localFaceTracker;
    BackendFaceTracker backendFaceTracker;
    FaceRenderer faceRenderer;
    ItemManager itemManager;
    
    int nProcessing = 0;
    float imageProcessingStartTime = 0.0f;
    float elapsedTime = 0.0f;
    float lastTipTextUpdateTime = 0.0f;
    Queue<Action> executeOnMainThread = new Queue<Action>();

    KeywordRecognizer keywordRecognizer;  
    delegate void KeywordAction();
    Dictionary<string, KeywordAction> keywordCollection;

    bool showDebug = false;
    bool useBackendManager = false;
    bool backendConnected = false;
    

    int frameReportPeriod = 10;
    int frameCounter = 0;

    void Start()
    {
        itemManager = GetComponent<ItemManager>();

        webcam = GetComponent<HololensCameraUWP>();

        faceRenderer = GetComponent<FaceRenderer>();
        faceRenderer.SetWebcam(webcam);

        localFaceTracker = new LocalFaceTracker(LocalTrackerNumberOfIters, LocalTrackerConfidenceThreshold);
        backendFaceTracker = new BackendFaceTracker(nLandmarks, BackendTrackerConfidenceThreshold, 43002, localFaceTracker);

        if (PhraseRecognitionSystem.isSupported)
        {
            keywordCollection = new Dictionary<string, KeywordAction>();

            keywordCollection.Add("Show debug", ShowDebug);
            keywordCollection.Add("Hide debug", HideDebug);
            keywordCollection.Add("Computer", BackendProcessing);
            keywordCollection.Add("Local", LocalProcessing);
            keywordCollection.Add("Show face", ShowFace);
            keywordCollection.Add("Hide face", HideFace);

            keywordRecognizer = new KeywordRecognizer(keywordCollection.Keys.ToArray());
            keywordRecognizer.OnPhraseRecognized += KeywordRecognizer_OnPhraseRecognized;
            keywordRecognizer.Start();
        }
        if (FPSText != null)
            FPSText.text = "";
    }

#if WINDOWS_UWP
    void Update ()
    {
        while (executeOnMainThread.Count > 0)
        {
            Action nextAction = executeOnMainThread.Dequeue();
            nextAction.Invoke();

            if (executeOnMainThread.Count == 0)
            {
                elapsedTime += Time.realtimeSinceStartup - imageProcessingStartTime;
                if (frameCounter % frameReportPeriod == 0)
                {
                    if (showDebug && FPSText != null)
                        FPSText.text = (1 / (elapsedTime / frameReportPeriod)).ToString();
                    elapsedTime = 0;
                }
                imageProcessingStartTime = Time.realtimeSinceStartup;
                frameCounter++;
            }
        }

        if (nProcessing < 1)
        {
            SoftwareBitmap image = webcam.GetImage();
            if (image != null)
            {
                Matrix4x4 webcamToWorldTransform = webcam.WebcamToWorldMatrix;
                float[] landmarkProjections = faceRenderer.GetLandmarkProjections(webcamToWorldTransform);

                nProcessing++;
                Task.Run(() => ProcessFrame(image, landmarkProjections, webcamToWorldTransform));
            }
        }

        if (Time.time > lastTipTextUpdateTime + 3.0f)
            BackendFaceTrackerTipText.text = "";
    }

    private async Task ProcessFrame(SoftwareBitmap image, float[] landmarkInits, Matrix4x4 webcamToWorldTransform)
    {
        bool resetModelFitter = true;
        float[] landmarks = null;
        if (useBackendManager)
        {
            if (backendFaceTracker.Connected)
            {
                if (!backendConnected)
                {
                    backendConnected = true;
                    executeOnMainThread.Enqueue(() =>
                    {
                        BackendFaceTrackerTipText.text = "Backend connected";
                        lastTipTextUpdateTime = Time.time;
                    });
                }

                landmarks = await backendFaceTracker.GetLandmarks(image, landmarkInits);
                resetModelFitter = backendFaceTracker.ResetModelFitter;
                backendFaceTracker.ModelFitterReset();
            }
            else
            {
                if (backendConnected)
                {
                    backendConnected = false;
                    executeOnMainThread.Enqueue(() =>
                    {
                        BackendFaceTrackerTipText.text = "Backend disconnected";
                        lastTipTextUpdateTime = Time.time;
                    });
                }

                landmarks = await localFaceTracker.GetLandmarks(image, landmarkInits);
                resetModelFitter = localFaceTracker.ResetModelFitter;
                localFaceTracker.ResetModelFitter = false;
            }
        }
        else if (!useBackendManager)
        {
            landmarks = await localFaceTracker.GetLandmarks(image, landmarkInits);
            resetModelFitter = localFaceTracker.ResetModelFitter;
            localFaceTracker.ResetModelFitter = false;
        }

        if (landmarks != null)
        {
            executeOnMainThread.Enqueue(() => 
            {
                FrameProcessed(landmarks, webcamToWorldTransform, resetModelFitter);
            });
        }
        else
        {
            executeOnMainThread.Enqueue(() =>
            {
                faceRenderer.ResetFitter();
            });
        }

        nProcessing--;
    }
#endif

    private void FrameProcessed(float[] landmarks, Matrix4x4 webcamToWorldTransform, bool resetModelFitter)
    {
        if (resetModelFitter)
        {
            faceRenderer.ResetFitter();
        }
        faceRenderer.UpdateHeadPose(landmarks, webcamToWorldTransform);
        itemManager.ProcessFaceExpressions(faceRenderer.BlendshapeWeights);
    }

    private void KeywordRecognizer_OnPhraseRecognized(PhraseRecognizedEventArgs args)
    {
        KeywordAction keywordAction;

        if (keywordCollection.TryGetValue(args.text, out keywordAction))
        {
            keywordAction.Invoke();
        }
    }

    void ShowDebug()
    {
        showDebug = true;
        faceRenderer.ShowDebug();
        itemManager.ShowDebug();
    }

    void HideDebug()
    {
        showDebug = false;
        faceRenderer.HideDebug();
        itemManager.HideDebug();
        if (FPSText != null)
            FPSText.text = "";
    }

    void BackendProcessing()
    {
        if (!backendFaceTracker.Initialized)
        {
            backendFaceTracker.Initialize();
        }
        if (!backendFaceTracker.Connected)
        {
            BackendFaceTrackerTipText.text = "Connect the backend processing client";
            lastTipTextUpdateTime = Time.time;
        }
        useBackendManager = true;
    }

    void LocalProcessing()
    {
        if (backendFaceTracker.Initialized)
        {
            backendFaceTracker.Close();
        }
        BackendFaceTrackerTipText.text = "";
        useBackendManager = false;
    }

    void ShowFace()
    {
        itemManager.ShowFace();
    }

    void HideFace()
    {
        itemManager.HideFace();
    }
}
