using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

#if WINDOWS_UWP
using FaceProcessing;
#endif


public class FaceRenderer : MonoBehaviour
{
    [Tooltip("Distance between the pupils of the subject's face, 63mm is the human average.")]
    public float InterPupilDistance = 0.063f; 
    [Tooltip("Specifies how far into the future the Kalman Filter should predict the head pose, seconds.")]
    public float KalmanPredictionTime = 0.12f;
    [Tooltip("A threshold, in meters, if a head moves by more than this much between two frames smoothing is not applied.")]
    public float MaxDisplacementForSmoothing = 0.005f; 
    [Tooltip("A threshold, in degrees, if a head rotates by more than this much between two frames smoothing is not applied.")]
    public float MaxRotationForSmoothing = 4.0f;
    [Tooltip("Kalman filter parameter, see article for details.")]
    public float SigmaA = 10.0f;
    [Tooltip("Kalman filter parameter, see article for details.")]
    public float SigmaM = 0.01f;
    [Tooltip("Kalman filter parameter, see article for details.")]
    public float DecayRate = 0.1f;
    [Tooltip("Number of landmarks returned by the face tracker.")]
    public int NumberOfLandmarks = 51;

    public Mesh NeutralFace;
    public Mesh[] Blendshapes;
    public GameObject FaceMeshGameObject;


    SkinnedMeshRenderer skinnedRenderer;
    Mesh skinnedMesh;
    List<GameObject> cubes;
    Vector3[] landmarkPositions;
    bool modelFitterInitialized = false;
    Matrix4x4 K;
    HololensCameraUWP webcam;
    float[] blendshapeWeights = new float[0];
    float lastUpdateTimestamp;
    
    bool bShowDebug = false;

    float meshScale;
#if WINDOWS_UWP
    ModelFitter modelFitter;
    PositionFilter xFilter, yFilter, zFilter;
#endif

    //indices of the 2D landmarks and corresponding indices of vertices in the candide face model
    int[] idxs2D = { 18, 14, 13, 16, 11, 17,
                    15, 19, 20, 21, 22, 23,
                    24, 25, 26, 27, 28, 29,
                    30,  0,  2,  4,  5,  7,
                    9, 31, 34, 37, 40, 36,
                    32, 38, 42, 43, 45, 47,
                    49 };
    int[] idxs3D = { 25, 58,  4,  5, 93, 110,
                    111, 52, 97, 103, 55,
                    109, 99, 22, 102,  96,
                    19,  98, 108,  47,  48,
                    49,  16,  15,  14,  63,
                    6,  30,   7,  78,  79,
                    84,  85,  88,  86,  87,
                    39 };


    public float[] BlendshapeWeights
    {
        get
        {
            return blendshapeWeights;
        }
    }

    void Start ()
    {
        meshScale = GetMeshScale();

        skinnedRenderer = FaceMeshGameObject.GetComponent<SkinnedMeshRenderer>();
#if WINDOWS_UWP
        xFilter = new PositionFilter(SigmaA, SigmaM, DecayRate);
        yFilter = new PositionFilter(SigmaA, SigmaM, DecayRate);
        zFilter = new PositionFilter(SigmaA, SigmaM, DecayRate);
#else
        BuildMesh();
        skinnedRenderer.sharedMesh = skinnedMesh;
#endif

        cubes = new List<GameObject>();
        for (int i = 0; i < NumberOfLandmarks; i++)
        {
            GameObject cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
            cube.transform.localScale = new Vector3(0.005f, 0.005f, 0.005f);

            cubes.Add(cube);
        }

        landmarkPositions = new Vector3[NumberOfLandmarks];
        HideDebug();
    }
	
	void Update ()
    {
        if (webcam.Initialized && !modelFitterInitialized)
        {
            InitializeModelFitter();
            BuildMesh();
            skinnedRenderer.sharedMesh = skinnedMesh;
            modelFitterInitialized = true;
        }
    }

    public void SetWebcam(HololensCameraUWP webcam)
    {
        this.webcam = webcam;
    }

    public void UpdateHeadPose(float[] landmarks, Matrix4x4 webcamToWorldTransform)
    {
        if (!modelFitterInitialized)
            return;

        float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
        float tX = 0.0f, tY = 0.0f, tZ = 0.0f;

#if WINDOWS_UWP
        modelFitter.Fit3D(landmarks, out rotX, out rotY, out rotZ, out tX, out tY, out tZ, out blendshapeWeights, 2);
#endif
        Vector3 rotVector = new Vector3(rotX, rotY, rotZ);
        Quaternion modelRotation = Quaternion.AngleAxis(180 * rotVector.magnitude / 3.14f, rotVector / rotVector.magnitude);
        modelRotation = new Quaternion(-modelRotation.x, -modelRotation.y, modelRotation.z, modelRotation.w);
        modelRotation = modelRotation * Quaternion.Euler(0, 0, 180.0f);
        Vector3 modelPosition = new Vector3(tX, tY, tZ);
        modelPosition = GetModelPosition(modelPosition, webcamToWorldTransform, K);

        UpdateModelPose(modelPosition, modelRotation, webcamToWorldTransform);

        for (int i = 0; i < blendshapeWeights.Length; i++)
        {
            skinnedRenderer.SetBlendShapeWeight(i, blendshapeWeights[i]);
        }

        UpdateLandamarkPositions(landmarks, webcamToWorldTransform, (modelPosition - (Vector3)webcamToWorldTransform.GetColumn(3)).magnitude);

        if (bShowDebug)
            DrawLandmarks();
        skinnedRenderer.gameObject.SetActive(true);
        lastUpdateTimestamp = Time.realtimeSinceStartup;
    }

    public void ResetFitter()
    {
#if WINDOWS_UWP
        modelFitter.ResetPose();
        xFilter.Reset();
        yFilter.Reset();
        zFilter.Reset();
#endif
        skinnedRenderer.gameObject.SetActive(false);
        lastUpdateTimestamp = -1.0f;
    }

    public float[] GetLandmarkProjections(Matrix4x4 webcamToWorldMatrix)
    {
        float[] landmarkProjections = new float[landmarkPositions.Length * 2];

        Matrix4x4 worldToWebcamMatrix = webcamToWorldMatrix.inverse;
        for (int i = 0; i < landmarkPositions.Length; i++)
        {
            Vector2 proj = ProjectVector(worldToWebcamMatrix.MultiplyPoint3x4(landmarkPositions[i]));
            landmarkProjections[2 * i] = proj.x;
            landmarkProjections[2 * i + 1] = proj.y;
        }

        return landmarkProjections;
    }


    private Vector3 GetModelPosition(Vector3 vertex, Matrix4x4 webcamToWorldTransform, Matrix4x4 K)
    {
        Vector3 projectedHomo = K * vertex;
        Vector2 projected = new Vector3(projectedHomo.x / projectedHomo.z, projectedHomo.y / projectedHomo.z);

        var webcamSpacePosition = vertex.magnitude * Pix2WebcamSpacePos(projected.x, projected.y).normalized;
        var cameraPosition = webcamToWorldTransform * (new Vector4(0, 0, 0, 1));

        return webcamToWorldTransform * webcamSpacePosition + cameraPosition;
    }

    float GetMeshScale()
    {
        //indices of the corners of the left eye in the Candide mesh
        int lEyelCornerIdx = 52;
        int lEyeRCornerIdx = 55;
        //indices of the corners of the right eye in the Candide mesh
        int rEyelCornerIdx = 19;
        int rEyeRCornerIdx = 22;

        Vector3 leftEyeCenter = (NeutralFace.vertices[lEyelCornerIdx] + NeutralFace.vertices[lEyeRCornerIdx]) / 2;
        Vector3 rightEyeCenter = (NeutralFace.vertices[rEyelCornerIdx] + NeutralFace.vertices[rEyeRCornerIdx]) / 2;
        float curDist = (leftEyeCenter - rightEyeCenter).magnitude;

        return InterPupilDistance / curDist;
    }

    private Vector2 ProjectVector(Vector3 vec)
    {
        Vector4 vec4 = new Vector4(vec.x, vec.y, vec.z, 1);
        Vector3 projectedHomo = K * vec4;
        Vector2 projected = new Vector2(webcam.Width - projectedHomo.x / projectedHomo.z, projectedHomo.y / projectedHomo.z);

        return projected;
    }

    private Vector3 Pix2WebcamSpacePos(float x, float y)
    {
        Vector3 res = new Vector3(0.0f, 0.0f, 0.0f);
        res.z = 1.0f;
        res.x = ((webcam.Width - x) - K.m02) / K.m00;
        res.y = (y - K.m12) / K.m11;

        return res * -1.0f;
    }

    private void UpdateLandamarkPositions(float[] landmarks, Matrix4x4 webcamToWorldTransform, float distToFace)
    {
        var WorldSpaceRayPoint1 = webcamToWorldTransform * (new Vector4(0, 0, 0, 1));

        for (int i = 0; i < landmarks.Length / 2; i++)
        {
            float posX = landmarks[i * 2];
            float posY = landmarks[i * 2 + 1];

            var webcamSpacePos = Pix2WebcamSpacePos(posX, posY);
            var WorldSpaceRayPoint2 = webcamToWorldTransform * webcamSpacePos;

            landmarkPositions[i] = WorldSpaceRayPoint1 + distToFace * WorldSpaceRayPoint2.normalized;
        }
    }

    private void DrawLandmarks()
    {
        for (int i = 0; i < landmarkPositions.Length; i++)
        {
            cubes[i].transform.position = landmarkPositions[i];
        }
    }

    private void UpdateModelPose(Vector3 position, Quaternion rotation, Matrix4x4 webcamToWorldTransform)
    {        
        Quaternion oldRotation = FaceMeshGameObject.transform.rotation;
        Vector3 oldPosition = FaceMeshGameObject.transform.position;

        float timeSinceLastUpdate;
        if (lastUpdateTimestamp < 0.0f)
            timeSinceLastUpdate = 0.0f;
        else
            timeSinceLastUpdate = Time.realtimeSinceStartup - lastUpdateTimestamp;

#if WINDOWS_UWP
        xFilter.Step(position.x, timeSinceLastUpdate);
        yFilter.Step(position.y, timeSinceLastUpdate);
        zFilter.Step(position.z, timeSinceLastUpdate);

        Vector3 predictedPosition = new Vector3(xFilter.GetFuturePosition(KalmanPredictionTime), yFilter.GetFuturePosition(KalmanPredictionTime), zFilter.GetFuturePosition(KalmanPredictionTime));


        if ((oldPosition - predictedPosition).magnitude < MaxDisplacementForSmoothing)
        {
            FaceMeshGameObject.transform.position = Vector3.Lerp(oldPosition, predictedPosition, 0.5f);
        }
        else
        {
            FaceMeshGameObject.transform.position = predictedPosition;
        }
#else
        FaceMeshGameObject.transform.position = position;
#endif
        FaceMeshGameObject.transform.LookAt(webcamToWorldTransform.GetColumn(3), webcamToWorldTransform.GetColumn(1));
        FaceMeshGameObject.transform.rotation *= rotation;

        if (Quaternion.Angle(oldRotation, FaceMeshGameObject.transform.rotation) < MaxRotationForSmoothing)
        {
            FaceMeshGameObject.transform.rotation = Quaternion.Lerp(FaceMeshGameObject.transform.rotation, oldRotation, 0.5f);
        }
    }


    void InitializeModelFitter()
    {
        float[] floatMean3DShape = new float[NeutralFace.vertices.Length * 3];
        for (int i = 0; i < NeutralFace.vertices.Length; i++)
        {
            floatMean3DShape[3 * i + 0] = meshScale * NeutralFace.vertices[i].x;
            floatMean3DShape[3 * i + 1] = meshScale * NeutralFace.vertices[i].y;
            floatMean3DShape[3 * i + 2] = meshScale * NeutralFace.vertices[i].z;
        }

        List<IList<float>> listBlendshapes = new List<IList<float>>();

        for (int i = 0; i < Blendshapes.Length; i++)
        {
            float[] blendshape = new float[NeutralFace.vertices.Length * 3];
            for (int j = 0; j < Blendshapes[i].vertices.Length; j++)
            {
                blendshape[3 * j + 0] = meshScale * (Blendshapes[i].vertices[j].x - NeutralFace.vertices[j].x);
                blendshape[3 * j + 1] = meshScale * (Blendshapes[i].vertices[j].y - NeutralFace.vertices[j].y);
                blendshape[3 * j + 2] = meshScale * (Blendshapes[i].vertices[j].z - NeutralFace.vertices[j].z);
            }

            listBlendshapes.Add(new List<float>(blendshape));
        }


#if WINDOWS_UWP
        int w = webcam.Width;
        int h = webcam.Height;
        Matrix4x4 normK = webcam.ProjectionMatrix;
        float[] arrayK = new float[9];
        arrayK[0 * 3 + 0] = w * normK.m00 / 2;
        arrayK[1 * 3 + 1] = h * normK.m11 / 2;
        arrayK[0 * 3 + 2] = w * (normK.m02 + 1) / 2;
        arrayK[1 * 3 + 2] = h * (normK.m12 + 1) / 2;
        arrayK[2 * 3 + 2] = 1;
        K = webcam.ProjectionMatrix;
        K.m00 = w * normK.m00 / 2;
        K.m11 = h * normK.m11 / 2;
        K.m02 = w * (normK.m02 + 1) / 2;
        K.m12 = h * (normK.m12 + 1) / 2;
        K.m22 = 1;

        modelFitter = new ModelFitter(floatMean3DShape, listBlendshapes, idxs2D, idxs3D, arrayK);
#endif
    }

    void BuildMesh()
    {
        int nVertices = NeutralFace.vertices.Length;
        Vector3[] vertices = new Vector3[nVertices];
        for (int i = 0; i < nVertices; i++)
        {
            vertices[i] = meshScale * (NeutralFace.vertices[i]);
        }

        skinnedMesh = new Mesh();
        skinnedMesh.vertices = vertices;
        skinnedMesh.SetIndices(NeutralFace.GetIndices(0), MeshTopology.Triangles, 0);

        Vector3[] dummyDeltas = new Vector3[nVertices];

        for (int i = 0; i < Blendshapes.Length; i++)
        {
            Vector3[] deltaVertices = new Vector3[nVertices];

            for (int j = 0; j < nVertices; j++)
            {
                deltaVertices[j] = meshScale * (Blendshapes[i].vertices[j] - NeutralFace.vertices[j]);
            }

            skinnedMesh.AddBlendShapeFrame("blendshape" + i, 1.0f, deltaVertices, dummyDeltas, dummyDeltas);
        }

        skinnedMesh.RecalculateNormals();
        skinnedMesh.RecalculateBounds();
    }

    public void ShowDebug()
    {
        for (int i = 0; i < cubes.Count; i++)
        {
            cubes[i].SetActive(true);
        }
        bShowDebug = true;
    }

    public void HideDebug()
    {
        for (int i = 0; i < cubes.Count; i++)
        {
            cubes[i].SetActive(false);
        }
        bShowDebug = false;
    }
}
