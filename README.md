# 8599 Ray Tracer Integrated with Walnut GUI

## Housekeeping

- To just use the application, run the executable (which I will distribute after I have finished implementing this software ray tracer).

- To run from the source code, we need:

  - [Visual Studio 2022](https://visualstudio.com)
  - [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
  
  Then, execute `scripts/Setup.bat` after <ins>recursively</ins> clone the repository, this will generate the Visual Studio 2022 solution for us. Note that current implementation of multithreading requires C++17 more above.

## Progression

- For progression from May 12-19th 2023 where the [initial framework of the GUI](https://github.com/IQ404/8599-ray-tracer-gui/tree/initial_framework) was established, please see [here](https://github.com/IQ404/8599-ray-tracer-gui/blob/initial_framework/README.md).

### May 20th 2023

I was playing games on this day.

### May 21st 2023

I was travelled to Durham on this day.

### May 22-28th 2023

To have an optimized ray tracer, I decided to relearn / learn more about ray-tracing and then add more logic to my offline ray tracer before I proceed to merge it with this GUI framework. More specifically, I am learning radiometry, rendering equation, global illumination and Monte Carlo path tracing during those days.
