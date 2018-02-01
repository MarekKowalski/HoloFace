using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class SparksTrigger : MonoBehaviour, IAnimationTrigger
{
    public Text TipText;

    ParticleSystem sparks;
    float enableTime = 0.0f;

    void Start ()
    {
        sparks = GetComponent<ParticleSystem>();
	}

    public void Update()
    {
        if (Time.time > enableTime + 3.0f && TipText != null)
            TipText.text = "";
    }

    public void OnEnable()
    {
        if (TipText != null)
            TipText.text = "Open mouth to enable the effect";
        enableTime = Time.time;
    }

    public void OnDisable()
    {

        if (TipText != null)
            TipText.text = "";
    }

    public void AttributePresent()
    {
        if (sparks != null && !sparks.isPlaying)
            sparks.Play();
        if (TipText != null)
            TipText.text = "";
    }

    public void AttributeNotPresent()
    {
        if (sparks != null && sparks.isPlaying)
            sparks.Stop();
    }
}
