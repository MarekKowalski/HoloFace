# HoloFace
HoloFace is a framework for facial augmented reality on HoloLens, it allows for augmenting the faces of people seen through HoloLens with various items and visual effects. It is described in detail in an article published at the 2018 IEEE Winter Conference on Applications of Computer Vision (WACV'2018). A preprint of the article is [available on arXiv](https://arxiv.org/abs/1802.00278).

Look the video below to see HoloFace in action.

[![YouTube link](http://img.youtube.com/vi/Zexjx9VWkSU/0.jpg)](https://www.youtube.com/watch?v=Zexjx9VWkSU)

## Getting started
In order to run HoloFace you will need Unity (we used Unity 2017.3.0f3) and Visual Studio 2017. To run the application on HoloLens take the following steps:
 1. Clone or download this repository.
 2. Open the HoloFace subdirectory with Unity.
 3. Got to File->Build Settings and select Universal Windows Platform.
 4. Change the Target Device to HoloLens and click Build. Choose a directory where Unity will generate a Visual Studio solution. Unity will likely throw several errors regarding System.Numerics.Vector2 (we are looking into getting rid of these), the project should however still work fine.
 5. Open the solution you just created using Visual Studio 2017, change Solution Configuration to Release and Solution Platform to x86.
 6. Change the target device from Local Machine to Remote Machine and input the IP number of the HoloLens.
 7. Press Ctrl+F5 to build and run on HoloLens.
 8. Try not to move your head till the Unity logo disappears (more details in the Known issues section below).
 
If everything worked fine the application should now be running on your HoloLens. Now, look at someone's face, if the application works correctly, you should see fire going out of that person's mouth. You can also look at an image of a face on a screen, but keep in mind that if the size of the face on the screen does not match a real life face size, the augmented elements will not be correctly aligned.

For more information about how to use the application, see the next paragraphs.

If you are experiencing issues, please let us know in the Issues section above, all feedback is very valuable.

## Voice commands and gestures
When the app is running you can use the following commands:
 - tap gesture: changes the effects currently being displayed, in the version that is currently on the repo it cycles the following effects: fire breathing, wolf mask, fire breathing triggered by mouth opening, no effect,
 - "show debug" voice command: shows additional information such as the framerate and landmark locations,
 - "show face" voice command: shows the face mesh, nice for debugging issue swith the mesh placement,
 - "computer" voice command: enables the alternative face alignment mode, which runs on a backend, more details below,
 - "local" voice command: enables the local face alignment mode (set by default).

## Enabling the remote face alignment method
As explained in the article, HoloFace has supports two methods for facial landmark localization (face alignment). The default method runs locally on the device. The alternative method runs on a remote backend but is significantly more accurate and stable. 

To run the remote method, first clone the [Deep Alignment Network respoitory](https://github.com/MarekKowalski/DeepAlignmentNetwork) and follow the setup instruction there. Once everything is configured you can run the remote backend by executing the following command:

```
python HoloFaceBackend.py IP_OF_THE_HOLOLENS
```

## Adding new effects and items
You can configure HoloFace to render any Game Object on top of a face seen through the headset. In order to add a new object take the following steps:
 1. Add the new object under the FaceMesh game object, which is under the HoloFace object in the default scene.
 2. Start the application in Unity Editor and disable all other objects under FaceMesh, you should now see a 3D shape resembling a face.
 3. Position you new object relative to the face.
 4. Build the Unity project as explained in the Getting started section above.
 
If you want your object to be activated by a face expression you need to do two things. First add a script containing a MonoBehavious that inherits from IAnimationTrigger to your object (see SparksTrigger for an example). Second, add your object to the appropriate list in HoloFace->ItemManager. For example, if you want the action to be activated when the person opens their mouth, add the object to the MouthOpenActivations list. Note that the mouth opening activation is more robust than the other two and usually gives the best results. 

## Known issues
There are currently two known issues, if you know how to solve them please let us know.
 1. Errors during the Unity build. The UWP project is generated fine but Unity throws errors regarding System.Numerics.Vector2.
 2. If the headset moves before the app fully launches (when the Unity logo is displayed) the displayed content will not be aligned correctly. It appears that the ```FrameToOrigin``` matrix that is a part of ```SensorFrame``` returned by HoloLensForCV is off if the headset moves during app initialization.

## Third party libraries and assets
HoloFace uses the [HoloLensForCV library](https://github.com/Microsoft/HoloLensForCV) from Microsoft. The library is however included in the HoloFace repo since we have made minor modifications to its code. Because of that there is no need to download HoloLensForCV separately.

The Unity project includes the following third party assets:
 - Japanese Mask by [Hideout Studio](http://www.hideout-studio.com/), [asset store link](https://assetstore.unity.com/packages/3d/props/free-japanese-mask-66432)
 - Starter Particle Pack by [Full Tilt Boogie](http://www.fulltiltboogie.ca/), [asset store link](https://assetstore.unity.com/packages/vfx/particles/starter-particle-pack-83179)
 
All of the assets above were included in the projection with permission from the authors

## Citation
If you use this software in your research, then please cite the following paper:

Kowalski, M.; Nasarzewski Z.; Galinski G., Garbat P.: " HoloFace: Augmenting Human-to-Human Interactions on HoloLens ", WACV 2018

## Commercial use
The landmarks used for training the face alignment models in HoloFace are part of the 300-W and Menpo datasets. Note that the licenses for those datasets exclude commercial use. You should contact s.zafeiriou@imperial.ac.uk to find out if it's OK for you to use the model files in a commercial product.

## Contact
If you have any questions or suggestions feel free to contact me at <m.kowalski@ire.pw.edu.pl>.

