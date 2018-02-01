using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class ItemManager : MonoBehaviour
{
    [Tooltip("Shows the attribute values when Debug mode is enabled.")]
    public Text AttributeText;

    public int MouthOpenBlendshapeIdx = 0;
    public float MouthOpenBlendshapeThreshold = 0.40f;
    public int SmileBlendshapeIdx = 1;
    public float SmileBlendshapeThreshold = 0.45f;
    public int EyeBrowRaiseBlendshapeIdx = 2;
    public float EyeBrowRaiseBlendshapeThreshold = 0.20f;

    public GameObject FaceMesh;
    public GameObject[] MouthOpenActivations;
    public GameObject[] SmileActivations;
    public GameObject[] EyeBrowsRaisedActivations;

    bool debugMode = false;
    bool showingFace = false;
    public bool ShowingFace
    {
        get
        {
            return showingFace;
        }
    }

    private UnityEngine.XR.WSA.Input.GestureRecognizer gestureRecognizer;
    private List<Transform> transforms;
    private int currentItem = 0;

    void Awake()
    {
        transforms = new List<Transform>();
        Transform[] tempTransforms = FaceMesh.GetComponentsInChildren<Transform>();
        for (int i = 0; i < tempTransforms.Length; i++)
        {
            if (tempTransforms[i].parent == FaceMesh.transform)
                transforms.Add(tempTransforms[i]);
        }
#if WINDOWS_UWP
        for (int i = 0; i < transforms.Count; i++)
            transforms[i].gameObject.SetActive(false);
#endif
        gestureRecognizer = new UnityEngine.XR.WSA.Input.GestureRecognizer();
        gestureRecognizer.SetRecognizableGestures(UnityEngine.XR.WSA.Input.GestureSettings.Tap);
        gestureRecognizer.TappedEvent += NextItem;
        gestureRecognizer.StartCapturingGestures();

#if WINDOWS_UWP
        HideFace();
#else
        ShowFace();
#endif
        transforms[currentItem].gameObject.SetActive(true);
    }
    

    public void ProcessFaceExpressions(float[] blendshapeWeights)
    {
        bool mouthOpen = false;
        bool smile = false;
        bool eyeBrowsRaised = false;
        if (blendshapeWeights[MouthOpenBlendshapeIdx] > MouthOpenBlendshapeThreshold)
        {
            mouthOpen = true;
        }
        if (blendshapeWeights[SmileBlendshapeIdx] > SmileBlendshapeThreshold)
        {
            smile = true;
        }
        //this is negative as the blendshape actually lowers the eyebrows
        if (-blendshapeWeights[EyeBrowRaiseBlendshapeIdx] > EyeBrowRaiseBlendshapeThreshold)
        {
            eyeBrowsRaised = true;
        }

        for (int i = 0; i < MouthOpenActivations.Length; i++)
        {
            IAnimationTrigger trigger = MouthOpenActivations[i].GetComponent<IAnimationTrigger>();

            if (trigger == null)
                continue;
            if (mouthOpen)
                trigger.AttributePresent();
            else
                trigger.AttributeNotPresent();
        }

        for (int i = 0; i < SmileActivations.Length; i++)
        {
            IAnimationTrigger trigger = SmileActivations[i].GetComponent<IAnimationTrigger>();

            if (trigger == null)
                continue;
            if (smile)
                trigger.AttributePresent();
            else
                trigger.AttributeNotPresent();
        }

        for (int i = 0; i < EyeBrowsRaisedActivations.Length; i++)
        {
            IAnimationTrigger trigger = EyeBrowsRaisedActivations[i].GetComponent<IAnimationTrigger>();

            if (trigger == null)
                continue;
            if (eyeBrowsRaised)
                trigger.AttributePresent();
            else
                trigger.AttributeNotPresent();
        }

        if (debugMode && AttributeText != null)
        {
            AttributeText.text = "Mouth open: " + mouthOpen.ToString() + ", val: " + blendshapeWeights[MouthOpenBlendshapeIdx].ToString("0.00") + "\n";
            AttributeText.text += "Smile: " + smile.ToString() + ", val: " + blendshapeWeights[SmileBlendshapeIdx].ToString("0.00") + "\n";
            AttributeText.text += "Eyebrows Raised: " + eyeBrowsRaised.ToString() + ", val: " + (-blendshapeWeights[EyeBrowRaiseBlendshapeIdx]).ToString("0.00") + "\n";
        }
    }

    public void ShowFace()
    {
        Renderer renderer = FaceMesh.GetComponent<Renderer>();
        renderer.enabled = true;
        showingFace = true;
    }

    public void HideFace()
    {
        Renderer renderer = FaceMesh.GetComponent<Renderer>();
        renderer.enabled = false;
        showingFace = false;
    }

    public void ShowDebug()
    {
        debugMode = true;
    }

    public void HideDebug()
    {
        debugMode = false;
        if (AttributeText != null)
        {
            AttributeText.text = "";
        }
    }

    void NextItem(UnityEngine.XR.WSA.Input.InteractionSourceKind source, int tapCount, Ray headRay)
    {
        if (currentItem != -1)
            transforms[currentItem].gameObject.SetActive(false);
        currentItem++;
        if (currentItem >= transforms.Count)
        {
            currentItem = -1;
            return;
        }
        transforms[currentItem].gameObject.SetActive(true);
    }
}
