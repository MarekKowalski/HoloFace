# HoloFace
HoloFace is a framework for facial augmented reality on HoloLens, it allows for augmenting the faces of people seen through HoloLens with various items and visual effects. It is described in detail in an article published at the 2018 IEEE Winter Conference on Applications of Computer Vision (WACV'2018). 

Look the video below to see HoloFace in action.

[![YouTube link](http://img.youtube.com/vi/Zexjx9VWkSU/0.jpg)](https://www.youtube.com/watch?v=Zexjx9VWkSU)

## Getting started
In order to run HoloFace you will need Unity (we used Unity 2017.3.0f3) and Visual Studio 2017. To run the application on HoloLens take the following steps:
 1. Clone or download this repository.
 2. Open the HoloFace subdirectory with Unity.
 3. Got to File->Build Settings and select Universal Windows Platform.
 4. Change the Target Device to HoloLens and click Build. Unity will likely throw several errors regarding System.Numerics.Vector2 (we are looking into getting rid of these), the project should however still work fine.
 5. Open the project you just created using Visual Studio 2017, change Solution Configuration to Release and Solution Platform to x86.
 6. Change the target device from Local Machine to Remote Machine and input the IP number of the HoloLens.
 7. Press Ctrl+F5 to build and run on HoloLens.
 
If everything worked fine the application should now be running on your HoloLens. Now, look at someone's face, if the application works correctly, you should see fire going out of that person's mouth. You can also look at an image of a face on a screen, but keep in mind that if the size of the face on the screen does not match a real life face size, the augmented elements will not be correctly aligned.

For more information about how to use the application, see the next paragraphs.

If you are experiencing issues, please let us know in the Issues section above, all feedback is very valuable.

## Voice commands and gestures

## Enabling the remote face alignment method

## Adding new effects and items

## Known issues

## Third party libraries and assets
HoloFace uses the [HoloLensForCV library](https://github.com/Microsoft/HoloLensForCV) from Microsoft. The library is however included in the HoloFace repo, since we have made minor modifications to its code. Because of that there is no need to download HoloLensForCV separately.

The Unity project includes the following third party assets:
 - Japanese Mask by [Hideout Studio](http://www.hideout-studio.com/), [asset store link](https://assetstore.unity.com/packages/3d/props/free-japanese-mask-66432)
 - Starter Particle Pack by [Full Tilt Boogie](http://www.fulltiltboogie.ca/), [asset store link](https://assetstore.unity.com/packages/vfx/particles/starter-particle-pack-83179)
 
All of the assets above were included in the projection with permission from the authors

## Citation
If you use this software in your research, then please cite the following paper:

Kowalski, M.; Nasarzewski Z.; Galinski G., Garbat P.: " HoloFace: Augmenting Human-to-Human Interactions on HoloLens ", WACV 2018

## Contact
If you have any questions or suggestions feel free to contact me at <m.kowalski@ire.pw.edu.pl>.

