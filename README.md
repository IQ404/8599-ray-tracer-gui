# 8599 Ray Tracer Integrated with Walnut GUI

## Housekeeping

- To just use the application, run the executable (which I will distribute after I have finished implementing this software ray tracer).

- To run from the source code, we need:

  - [Visual Studio 2022](https://visualstudio.com)
  - [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
  
  Then, execute `scripts/Setup.bat` after cloning the repository, this will generate the Visual Studio 2022 solution for us. Note that current implementation requires C++20 or above.

## Progression

### May 12-19th 2023

Constructing the [initial framework of the GUI](https://github.com/IQ404/8599-ray-tracer-gui/tree/initial_framework).

### May 20th 2023

I was playing games on this day.

### May 21st 2023

I was travelled to Durham on this day.

### May 22-30th 2023

To have an optimized ray tracer, I decided to relearn / learn more about the theory behind ray-tracing and then add more logic to my offline ray tracer before I proceed to merge it with this GUI framework. More specifically, I am learning radiometry, rendering equation, global illumination, Monte Carlo path tracing, BSDF/BRDF/BTDF/BSSRDF and microfacet models during those days.

### May 31st - June 7th 2023

Implementing the Whitted Style ray tracing in the GUI renderer framework. The sample video is as follows:

<img src="https://github.com/IQ404/8599-ray-tracer-gui/blob/whitted/SampleImages/WhittedStyle.gif"></a>

For source code, please see [this branch](https://github.com/IQ404/8599-ray-tracer-gui/tree/whitted).

### June 8th - 17th 2023

Before I move to physically-based path tracing, I would like to add acceleration structures into the ray tracer, and use it to render a Stanford bunny and a Utah teapot.

The results are as follows:

<img src="https://github.com/IQ404/8599-ray-tracer-gui/blob/stanford_bunny/SampleImages/stanford_bunny.jpg"></a>

<img src="https://github.com/IQ404/8599-ray-tracer-gui/blob/stanford_bunny/SampleImages/utah_teapot.jpg"></a>

The source code has been pushed to [this branch](https://github.com/IQ404/8599-ray-tracer-gui/tree/stanford_bunny).

### June 18th - 23rd 2023

I will be writing an interim report on what I have done so far for the module (CSC8599).

### June 24th - July 3rd 2023

I was travelling to London during this period.

### July 4th - 14th 2023

Creating the [8599 Non Physical Path Tracer](https://github.com/IQ404/8599-ray-tracer-gui/tree/non_physical_path_tracer) by integrating the [prototype](https://github.com/IQ404/8599-ray-tracer-prototype) into the [initial framework](https://github.com/IQ404/8599-ray-tracer-gui/tree/initial_framework).

The results (using temporal accumulation) for the three diffuse models are as follows:

<img src="https://github.com/IQ404/8599-ray-tracer-gui/blob/non_physical_path_tracer/SampleImages/8599NonPhysicalPathTracer.gif"></a>
