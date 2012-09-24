Metal Planet
=============

Hacked together to learn DX, rushed towards the end. Around 30fps at 720p on stock 5770.

![](https://github.com/hdemmer/metal-planet/raw/master/screenshots/final.JPG)

Features
--------

- Dynamic tiling with tiles generated on GPU.
- Sort-of deferred shading, but G-Buffer is way too fat. Forward-shading would have been much more appropriate for this.

![](https://github.com/hdemmer/metal-planet/raw/master/screenshots/tiling.JPG)

Lessons learned
---------------

 - You don't need classes. Data Oriented Design is clearly the way forward. But requires more planning.
 - A planet made of metal without an atmosphere was not a good choice: The only depth cues are basically the artifacts from the undersampled normal mapping in the distance -__-
 - A procedural normal map (combined octaves) is not a good idea. Gave me a lot of headaches.
 - Wasted a lot of time tweaking and fiddling with the lighting values.
 - Good assets are everything.

 Nice to haves
 -------------

 - Cull off-screen tiles
 - Slim down G-Buffer, e.g. position from depth
 - Profile and slim down pixel shaders
- Cleaner tear down


