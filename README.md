# 8599 Ray Tracer Integrated with Walnut GUI

## Housekeeping

- To just use the application, run the executable (which I will distribute after I have finished implementing this software ray tracer).

- To run from the source code, we need:

  - [Visual Studio 2022](https://visualstudio.com)
  - [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
  
  Then, execute `scripts/Setup.bat` after <ins>recursively</ins> clone the repository, this will generate the Visual Studio 2022 solution for us. Note that current implementation of multithreading requires C++17 more above.

## Progression

### May 12th 2023

Set up the scaffolding for rendering images into the GUI:

```cpp
/*****************************************************************//**
 * \file   mainloop.cpp
 * \brief  The main rendering function that is called for every frame
 * 
 * \author Xiaoyang Liu (built upon the Walnut GUI template: https://github.com/TheCherno/WalnutAppTemplate)
 * \date   May 2023
 *********************************************************************/

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

using namespace Walnut;

class CSC8599Layer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override		// this is called every frame
	{
// Viewport:

		ImGui::Begin("Viewport");

		// dimension (in number of pixels):
		viewport_width = ImGui::GetContentRegionAvail().x;
		viewport_height = ImGui::GetContentRegionAvail().y;

		if (frame_image)
		{
			ImGui::Image(frame_image->GetDescriptorSet(), { (float)frame_image->GetWidth(), (float)frame_image->GetHeight() });		// display the image
		}

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------
		
// Control Panel:

		ImGui::Begin("Control Panel");

		if (ImGui::Button("Render"))	// NOTE: currently Render() is only called once every time we hit "Render"
		{
			Render();
		}
		
		ImGui::Text("%.0f ms", duration_per_frame);

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------

// Miscellaneous:
		
		//ImGui::ShowDemoWindow();
		
//----------------------------------------------------------------------------------------------------------------------------------------

		//Render();
	}

	void Render()
	{
		Timer frame_timer;	// this timer starts counting from here

		if (!frame_image || viewport_width != frame_image->GetWidth() || viewport_height != frame_image->GetHeight())
		{
			frame_image = std::make_shared<Image>(viewport_width, viewport_height, ImageFormat::RGBA);
			delete[] frame_data;
			frame_data = new uint32_t[viewport_width * viewport_height];	// Note: we use 32 bits int to represent RGBA values each by 1 byte (2^8 = 256).
		}

		for (int i = 0; i < (viewport_width * viewport_height); i++)
		{
			frame_data[i] = 0xffff00ff;		// this will be interpreted as 0xAABBGGRR;
		}

		frame_image->SetData(frame_data);	// send the frame data to GPU

		duration_per_frame = frame_timer.ElapsedMillis();
	}

private:
	float duration_per_frame = 0.0f;

	std::shared_ptr<Image> frame_image;
	uint32_t* frame_data = nullptr;
	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "8599 Ray Tracer";

	Walnut::Application* app = new Walnut::Application(spec);	// Vulkan is initialized AFTER this point
	app->PushLayer<CSC8599Layer>();
	return app;
}
```

### May 13th 2023

Create the `Renderer` class:

```cpp
/*****************************************************************//**
 * \file   Renderer.h
 * \brief  The header file of the renderer for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RENDERER_H
#define RENDERER_H

#include <memory>	// to use std::shared_ptr
#include <glm/glm.hpp>		// to use glm vec
#include "Walnut/Image.h"

class Renderer
{
	std::shared_ptr<Walnut::Image> frame_image_final;
	uint32_t* frame_data = nullptr;

public:

	Renderer() = default;	// Defaulted default constructor: the compiler will define the implicit default constructor even if other constructors are present.

	void ResizeViewport(uint32_t width, uint32_t height);
	void Render();

	std::shared_ptr<Walnut::Image> GetFinalImage() const
	{
		return frame_image_final;
	}

private:

	uint32_t soft_FragmentShader(glm::vec2 coordinate);
	
};

#endif // !RENDERER_H
```

```cpp
/*****************************************************************//**
 * \file   Renderer.cpp
 * \brief  The definitions of the renderer class for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Renderer.h"

void Renderer::ResizeViewport(uint32_t width, uint32_t height)
{
	if (frame_image_final)
	{
		if ((frame_image_final->GetWidth() == width) && (frame_image_final->GetHeight() == height))
		{
			return;
		}
		frame_image_final->Resize(width, height);
	}
	else
	{
		frame_image_final = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] frame_data;
	frame_data = new uint32_t[width * height];
}

void Renderer::Render()
{
	for (uint32_t y = 0; y < frame_image_final->GetHeight(); y++)		// each column
	{
		for (uint32_t x = 0; x < frame_image_final->GetWidth(); x++)		// each row
		{
			glm::vec2 coordinate{ (float)x / frame_image_final->GetWidth(), (float)y / frame_image_final->GetHeight() };
			coordinate = coordinate * 2.0f - 1.0f;		// normalize to [-1,1)^2
			frame_data[(y * frame_image_final->GetWidth()) + x] = soft_FragmentShader(coordinate);
		}
	}

	frame_image_final->SetData(frame_data);	// send the frame data to GPU
}

uint32_t Renderer::soft_FragmentShader(glm::vec2 coordinate)
{
	glm::vec3 rayOrigin{ 0.0f,0.0f,2.0f };
	glm::vec3 rayDirection{ coordinate.x, coordinate.y, -1.0f };	// viewport at z = -1
	float radius = 0.5f;

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
	float discriminant = b * b - 4 * a * c;

	if (discriminant >= 0.0f)
	{
		return 0xffff00ff;
	}
	return 0xff000000;

	/*
	Side note:
		uint8_t a = 255;
		cout << (a << 8) << endl;	// this promotes a to an int since the literal 8 is an int
	*/
}
```

so that our main loop becomes:

```cpp
/*****************************************************************//**
 * \file   mainloop.cpp
 * \brief  The main rendering function that is called for every frame
 * 
 * \author Xiaoyang Liu (built upon the Walnut GUI template: https://github.com/TheCherno/WalnutAppTemplate)
 * \date   May 2023
 *********************************************************************/

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Timer.h"

#include "Renderer.h"

class CSC8599Layer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override		// this is called every frame
	{
// Viewport:

		ImGui::Begin("Viewport");

		// dimension (in number of pixels):
		viewport_width = ImGui::GetContentRegionAvail().x;
		viewport_height = ImGui::GetContentRegionAvail().y;

		auto final_image = renderer.GetFinalImage();
		if (final_image)
		{
			ImGui::Image(final_image->GetDescriptorSet(), { (float)final_image->GetWidth(), (float)final_image->GetHeight() }, { 0,1 }, { 1,0 });		// display the image
			/*
			Note for the last two parameters:
			change uv0 (0,0) which is originally at the top left to (0,1) i.e. bottom left; change uv1 (1,1) which is originally at the bottom right to (0,1) i.e. top right.
			*/
		}

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------
		
// Control Panel:

		ImGui::Begin("Control Panel");

		ImGui::Text("%.0f FPS", 1000.0f/duration_per_frame);	// Note that this will print inf if duration_per_frame == 0
		ImGui::Text("%.0f ms", duration_per_frame);

		if (ImGui::Button("Render"))	// NOTE: currently Render() is only called once every time we hit "Render"
		{
			rendering = !rendering;
		}

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------

// Miscellaneous:
		
		//ImGui::ShowDemoWindow();
		
//----------------------------------------------------------------------------------------------------------------------------------------

// Compute and Sending Data:

		if (rendering)
		{
			Render();
		}
	}

	void Render()
	{
		Walnut::Timer frame_timer;	// this timer starts counting from here

		renderer.ResizeViewport(viewport_width, viewport_height);
		renderer.Render();

		duration_per_frame = frame_timer.ElapsedMillis();
	}

private:
	float duration_per_frame = 0.0f;
	bool rendering = false;
	Renderer renderer;
	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "8599 Ray Tracer";

	Walnut::Application* app = new Walnut::Application(spec);	// Vulkan is initialized AFTER this point
	app->PushLayer<CSC8599Layer>();		// OnUIRender() will be called every loop after this layer has been pushed.
	return app;
}
```

### May 14th 2023

- Add a simplest lighting model into the renderer:

```cpp
/*****************************************************************//**
 * \file   Renderer.h
 * \brief  The header file of the renderer for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RENDERER_H
#define RENDERER_H

#include <memory>	// to use std::shared_ptr
#include <glm/glm.hpp>		// to use glm vec
#include "Walnut/Image.h"

class Renderer
{
	std::shared_ptr<Walnut::Image> frame_image_final;
	uint32_t* frame_data = nullptr;

public:

	Renderer() = default;	// Defaulted default constructor: the compiler will define the implicit default constructor even if other constructors are present.

	void ResizeViewport(uint32_t width, uint32_t height);
	void Render();

	std::shared_ptr<Walnut::Image> GetFinalImage() const
	{
		return frame_image_final;
	}

private:

	glm::vec4 soft_FragmentShader(glm::vec2 coordinate);
	
};

#endif // !RENDERER_H
```

```cpp
/*****************************************************************//**
 * \file   Renderer.cpp
 * \brief  The definitions of the renderer class for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Renderer.h"

namespace RTUtility
{
	uint32_t vecRGBA_to_0xABGR(const glm::vec4& colorRGBA)
	{
		uint8_t r = (uint8_t)(colorRGBA.r * 255.0f);
		uint8_t g = (uint8_t)(colorRGBA.g * 255.0f);
		uint8_t b = (uint8_t)(colorRGBA.b * 255.0f);
		uint8_t a = (uint8_t)(colorRGBA.a * 255.0f);

		return ((a << 24) | (b << 16) | (g << 8) | r);	// this automatically promotes 8 bits memory to 32 bits memory
	}
}

void Renderer::ResizeViewport(uint32_t width, uint32_t height)
{
	if (frame_image_final)
	{
		if ((frame_image_final->GetWidth() == width) && (frame_image_final->GetHeight() == height))
		{
			return;
		}
		frame_image_final->Resize(width, height);
	}
	else
	{
		frame_image_final = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] frame_data;
	frame_data = new uint32_t[width * height];
}

void Renderer::Render()
{
	for (uint32_t y = 0; y < frame_image_final->GetHeight(); y++)		// each column
	{
		for (uint32_t x = 0; x < frame_image_final->GetWidth(); x++)		// each row
		{
			glm::vec2 coordinate{ (float)x / frame_image_final->GetWidth(), (float)y / frame_image_final->GetHeight() };
			coordinate = coordinate * 2.0f - 1.0f;		// normalize to [-1,1)^2
			glm::vec4 colorRGBA = soft_FragmentShader(coordinate);
			colorRGBA = glm::clamp(colorRGBA, glm::vec4(0.0f), glm::vec4(1.0f));	// glm::clamp(value, min, max)
			frame_data[(y * frame_image_final->GetWidth()) + x] = RTUtility::vecRGBA_to_0xABGR(colorRGBA);
		}
	}

	frame_image_final->SetData(frame_data);	// send the frame data to GPU
}

glm::vec4 Renderer::soft_FragmentShader(glm::vec2 coordinate)
{
	glm::vec3 rayOrigin{ 0.0f,0.0f,1.0f };
	glm::vec3 rayDirection{ coordinate.x, coordinate.y, -1.0f };	// viewport at z = -1
	float radius = 0.5f;

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
	float discriminant = b * b - 4 * a * c;

	if (discriminant < 0.0f)
	{
		return glm::vec4{0,0,0,1};
	}
	float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	glm::vec3 hit_point = rayOrigin + rayDirection * closestT;
	glm::vec3 normal = glm::normalize(hit_point);
	glm::vec3 lightDirection = glm::normalize(glm::vec3{ -1.0,-1.0,-1.0 });
	float diffuseIntensity = glm::max(glm::dot(normal, -lightDirection), 0.0f);
	glm::vec3 sphereColor = glm::vec3{ 1,1,0 } * diffuseIntensity;
	return glm::vec4{ sphereColor, 1.0f };
}
```

I have also change a bit about the `mainloop.cpp` so that we can choose to render "in real-time" (i.e. compute and send the new data of one image to gpu for each loop) or render "offline" (i.e. only render one image in a loop, send the data to gpu, and use that data for every subsequent loop):

```cpp
/*****************************************************************//**
 * \file   mainloop.cpp
 * \brief  The main rendering function that is called for every frame
 * 
 * \author Xiaoyang Liu (built upon the Walnut GUI template: https://github.com/TheCherno/WalnutAppTemplate)
 * \date   May 2023
 *********************************************************************/

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Timer.h"

#include "Renderer.h"

class CSC8599Layer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override		// this is called every frame
	{
// Viewport:

		ImGui::Begin("Viewport");

		// dimension (in number of pixels):
		viewport_width = ImGui::GetContentRegionAvail().x;
		viewport_height = ImGui::GetContentRegionAvail().y;

		auto final_image = renderer.GetFinalImage();
		if (final_image)
		{
			ImGui::Image(final_image->GetDescriptorSet(), { (float)final_image->GetWidth(), (float)final_image->GetHeight() }, { 0,1 }, { 1,0 });		// display the image
			/*
			Note for the last two parameters:
			change uv0 (0,0) which is originally at the top left to (0,1) i.e. bottom left; change uv1 (1,1) which is originally at the bottom right to (0,1) i.e. top right.
			*/
		}

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------
		
// Control Panel:

		ImGui::Begin("Control Panel");

		ImGui::Text("%.0f FPS", 1000.0f/duration_per_frame);	// Note that this will print inf if duration_per_frame == 0
		ImGui::Text("%.0f ms", duration_per_frame);

		if (ImGui::Button("Render in Real-Time"))	// NOTE: currently Render() is only called once every time we hit "Render"
		{
			rendering = true;
		}
		if (ImGui::Button("Render Offline"))	// NOTE: currently Render() is only called once every time we hit "Render"
		{
			rendering = false;
			Render();
		}

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------

// Miscellaneous:
		
		//ImGui::ShowDemoWindow();
		
//----------------------------------------------------------------------------------------------------------------------------------------

// Compute and Sending Data:

		if (rendering)
		{
			Render();
		}
	}

	void Render()
	{
		Walnut::Timer frame_timer;	// this timer starts counting from here

		renderer.ResizeViewport(viewport_width, viewport_height);
		renderer.Render();

		duration_per_frame = frame_timer.ElapsedMillis();
	}

private:
	float duration_per_frame = 0.0f;
	bool rendering = false;
	Renderer renderer;
	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "8599 Ray Tracer";

	Walnut::Application* app = new Walnut::Application(spec);	// Vulkan is initialized AFTER this point
	app->PushLayer<CSC8599Layer>();		// OnUIRender() will be called every loop after this layer has been pushed.
	return app;
}
```

The resulting GUI is as follows:

<img src="https://github.com/IQ404/8599-ray-tracer-gui/blob/main/SampleImages/basic_lighting.jpg" width="700" height="400"></a>

### May 15th 2023

- Implement a movable camera

```cpp
/*****************************************************************//**
 * \file   Camera.h
 * \brief  The header of a movable camera
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>		// to use glm vec
#include <vector>

class Camera
{
// Member variables:
	glm::vec3 position{ 0.0f,0.0f,6.0f };
	glm::vec3 forward_direction{ 0.0f,0.0f,-1.0f };
	glm::vec3 up_direction{ 0.0f,1.0f,0.0f };

	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;

	glm::mat4 projection_matrix{ 1.0f };
	glm::mat4 inverse_projection_matrix{ 1.0f };
	
	/* Side - note: glm::mat4{1} creates a diagonal matrix with 1s on its diagonal. */

	glm::mat4 view_matrix{ 1.0f };
	glm::mat4 inverse_view_matrix{ 1.0f };

	float vertical_FOV = 45.0f;

	float near_clip_plane_distance = 0.1f;
	float far_clip_plane_distance = 100.0f;

	glm::vec2 mouse_was_at{ 0.0f,0.0f };

	std::vector<glm::vec3> ray_directions;

public:

// Constructors:
	Camera(float verticalFOV, float NearClipPlaneDistance, float FarClipPlaneDistance);

// Update Data related to the Camera:
	bool UpdateCamera(float dt);
	void ResizeViewport(uint32_t new_width, uint32_t new_height);

// Getters:
	float Sensitivity() const
	{
		return 0.0006f;
	}

	const glm::vec3& Position() const
	{
		return position;
	}
	const glm::vec3& ForwardDirection() const
	{
		return forward_direction;
	}

	const glm::mat4& ProjectionMatrix() const
	{
		return projection_matrix;
	}
	const glm::mat4& InverseProjectionMatrix() const
	{
		return inverse_projection_matrix;
	}

	const glm::mat4& ViewMatrix() const
	{
		return view_matrix;
	}
	const glm::mat4& InverseViewMatrix() const
	{
		return inverse_view_matrix;
	}

	const std::vector<glm::vec3>& RayDirections() const
	{
		return ray_directions;
	}

private:

	void RecomputeProjectionMatrix();
	void RecomputeViewMatrix();
	void RecomputeRayDirections();
};

#endif // !CAMERA_H
```

```cpp
/*****************************************************************//**
 * \file   Camera.cpp
 * \brief  The definitions of a movable camera
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>	// to use glm::quat
#include <glm/gtx/quaternion.hpp>	// to use glm::rotate

#include "Walnut/Input/Input.h"

Camera::Camera(float verticalFOV, float NearClipPlaneDistance, float FarClipPlaneDistance)
	: vertical_FOV{ verticalFOV }, near_clip_plane_distance{ NearClipPlaneDistance }, far_clip_plane_distance{ FarClipPlaneDistance }
{

}

bool Camera::UpdateCamera(float dt)
{

	glm::vec2 mouse_currently_at = Walnut::Input::GetMousePosition();
	glm::vec2 mouse_displacement = mouse_currently_at - mouse_was_at;
	mouse_was_at = mouse_currently_at;

	if (!Walnut::Input::IsMouseButtonDown(Walnut::MouseButton::Right))
	{
		Walnut::Input::SetCursorMode(Walnut::CursorMode::Normal);
		return false;
	}
	Walnut::Input::SetCursorMode(Walnut::CursorMode::Locked);

	bool is_moved{ false };
	float moving_speed{ 5.0f };
	glm::vec3 right_direction{ glm::cross(forward_direction, up_direction) };

	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::W))
	{
		position += moving_speed * dt * forward_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::S))
	{
		position -= moving_speed * dt * forward_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::D))
	{
		position += moving_speed * dt * right_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::A))
	{
		position -= moving_speed * dt * right_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::Space))
	{
		position += moving_speed * dt * up_direction;
		is_moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::LeftShift))
	{
		position -= moving_speed * dt * up_direction;
		is_moved = true;
	}
	if ((mouse_displacement.x != 0.0f) || (mouse_displacement.y != 0.0f))
	{
		float change_in_pitch{ mouse_displacement.y * Sensitivity() };
		float change_in_yaw{ mouse_displacement.x * Sensitivity() };
		glm::quat quaternion{ glm::normalize(glm::cross(glm::angleAxis(-change_in_pitch, right_direction), glm::angleAxis(-change_in_yaw, up_direction))) };
		forward_direction = glm::rotate(quaternion, forward_direction);
		is_moved = true;
	}
	
	if (is_moved)
	{
		RecomputeViewMatrix();		// Note: order matters!
		RecomputeRayDirections();
	}

	return is_moved;
}

void Camera::ResizeViewport(uint32_t new_width, uint32_t new_height)
{
	if ((viewport_width == new_width) && (viewport_height == new_height))
	{
		return;
	}
	viewport_width = new_width;
	viewport_height = new_height;

	RecomputeProjectionMatrix();		// Note: order matters!
	RecomputeRayDirections();
}

void Camera::RecomputeProjectionMatrix()
{
	projection_matrix = glm::perspectiveFov(glm::radians(vertical_FOV), (float)viewport_width, (float)viewport_height, near_clip_plane_distance, far_clip_plane_distance);
	/* passing in viewport_width and viewport_height is to calculate the aspect ratio */
	inverse_projection_matrix = glm::inverse(projection_matrix);
}

void Camera::RecomputeViewMatrix()
{
	view_matrix = glm::lookAt(position, position + forward_direction, up_direction);
	inverse_view_matrix = glm::inverse(view_matrix);
}

void Camera::RecomputeRayDirections()
{
	ray_directions.resize(viewport_width * viewport_height);
	for (uint32_t y = 0; y < viewport_height; y++)		// each column
	{
		for (uint32_t x = 0; x < viewport_width; x++)		// each row
		{
			glm::vec2 coordinate{ (float)x / viewport_width, (float)y / viewport_height };
			coordinate = coordinate * 2.0f - 1.0f;		// normalize to [-1,1)^2
			
			// Get the ray direction in world space (from the camera to the pixel on the near clip plane of the perspective projection):
			glm::vec4 target{ inverse_projection_matrix * glm::vec4{coordinate.x, coordinate.y, 1, 1}};
			glm::vec3 ray_direction{ glm::vec3{inverse_view_matrix * glm::vec4{glm::normalize(glm::vec3{target} / target.w),0}} };

			ray_directions[y * viewport_width + x] = ray_direction;
		}
	}
}
```

- Rendering two spheres

We first write some abstractions to clean up the renderer:

```cpp
/*****************************************************************//**
 * \file   Ray.h
 * \brief  Data structure representing the ray
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

#endif // !RAY_H
```

```cpp
/*****************************************************************//**
 * \file   Scene.h
 * \brief  Representation of data in the world
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <glm/glm.hpp>

struct Sphere
{
	glm::vec3 center{ 0.0f,0.0f,0.0f };
	float radius{ 0.5 };
	glm::vec3 albedo{ 1.0 };
};

struct Scene
{
	std::vector<Sphere> spheres;
};

#endif // !SCENE_H
```

Now, the renderer becomes:

```cpp
/*****************************************************************//**
 * \file   Renderer.h
 * \brief  The header file of the renderer for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RENDERER_H
#define RENDERER_H

#include <memory>	// to use std::shared_ptr
#include <glm/glm.hpp>		// to use glm vec
#include "Walnut/Image.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

class Renderer
{
	std::shared_ptr<Walnut::Image> frame_image_final;
	uint32_t* frame_data = nullptr;

public:

	Renderer() = default;	// Defaulted default constructor: the compiler will define the implicit default constructor even if other constructors are present.

	void ResizeViewport(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const
	{
		return frame_image_final;
	}

private:

	glm::vec4 soft_FragmentShader(const Scene& scene, const Ray& ray);
};

#endif // !RENDERER_H
```

```cpp
/*****************************************************************//**
 * \file   Renderer.cpp
 * \brief  The definitions of the renderer class for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Renderer.h"

namespace RTUtility
{
	uint32_t vecRGBA_to_0xABGR(const glm::vec4& colorRGBA)
	{
		uint8_t r = (uint8_t)(colorRGBA.r * 255.0f);
		uint8_t g = (uint8_t)(colorRGBA.g * 255.0f);
		uint8_t b = (uint8_t)(colorRGBA.b * 255.0f);
		uint8_t a = (uint8_t)(colorRGBA.a * 255.0f);

		return ((a << 24) | (b << 16) | (g << 8) | r);	// this automatically promotes 8 bits memory to 32 bits memory
	}
}

void Renderer::ResizeViewport(uint32_t width, uint32_t height)
{
	if (frame_image_final)
	{
		if ((frame_image_final->GetWidth() == width) && (frame_image_final->GetHeight() == height))
		{
			return;
		}
		frame_image_final->Resize(width, height);
	}
	else
	{
		frame_image_final = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] frame_data;
	frame_data = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	Ray ray;
	ray.origin = camera.Position();
	for (uint32_t y = 0; y < frame_image_final->GetHeight(); y++)		// each column
	{
		for (uint32_t x = 0; x < frame_image_final->GetWidth(); x++)		// each row
		{
			ray.direction = camera.RayDirections()[y * frame_image_final->GetWidth() + x];
			glm::vec4 colorRGBA = soft_FragmentShader(scene, ray);
			colorRGBA = glm::clamp(colorRGBA, glm::vec4(0.0f), glm::vec4(1.0f));	// glm::clamp(value, min, max)
			frame_data[(y * frame_image_final->GetWidth()) + x] = RTUtility::vecRGBA_to_0xABGR(colorRGBA);
		}
	}

	frame_image_final->SetData(frame_data);	// send the frame data to GPU
}

glm::vec4 Renderer::soft_FragmentShader(const Scene& scene, const Ray& ray)
{
	if (scene.spheres.size() == 0)
	{
		return { 0.0f,0.0f,0.0f,1.0f };
	}

	float closestT = std::numeric_limits<float>::max();
	const Sphere* closest_sphere = nullptr;
	for (const Sphere& sphere : scene.spheres)
	{
		glm::vec3 transformed_origin = ray.origin - sphere.center;	// Do the equivalent movement on camera rather than on sphere
		
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(transformed_origin, ray.direction);
		float c = glm::dot(transformed_origin, transformed_origin) - sphere.radius * sphere.radius;
		float discriminant = b * b - 4 * a * c;

		if (discriminant < 0.0f)
		{
			continue;
		}

		float closerT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closerT < closestT)
		{
			closestT = closerT;
			closest_sphere = &sphere;
		}
	}
	if (!closest_sphere)
	{
		return glm::vec4{ 0.0f,0.0f,0.0f,1.0f };
	}
	glm::vec3 transformed_origin = ray.origin - closest_sphere->center;	// Do the equivalent movement on camera rather than on sphere
	glm::vec3 hit_point = transformed_origin + ray.direction * closestT;
	glm::vec3 normal = glm::normalize(hit_point);
	glm::vec3 lightDirection = glm::normalize(glm::vec3{ -1.0,-1.0,-1.0 });
	float diffuseIntensity = glm::max(glm::dot(normal, -lightDirection), 0.0f);
	glm::vec3 sphereColor = closest_sphere->albedo * diffuseIntensity;	// light source emits white light
	return glm::vec4{ sphereColor, 1.0f };
}
```

We modify the main loop:

```cpp
/*****************************************************************//**
 * \file   mainloop.cpp
 * \brief  The main rendering function that is called for every frame
 * 
 * \author Xiaoyang Liu (built upon the Walnut GUI template: https://github.com/TheCherno/WalnutAppTemplate)
 * \date   May 2023
 *********************************************************************/

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"

class CSC8599Layer : public Walnut::Layer
{
	float duration_per_frame = 0.0f;
	bool real_time = false;
	Renderer renderer;
	Camera camera{ 45.0f, 0.1f, 100.0f };
	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;
	Scene scene;

public:
	CSC8599Layer()
	{
		{
			Sphere sphere;
			sphere.center = glm::vec3{ 0.0f,0.0f,0.0f };
			sphere.radius = 0.5;
			sphere.albedo = glm::vec3{ 1.0f,1.0f,0.0f };
			scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = glm::vec3{ 1.0f,0.0f,-5.0f };
			sphere.radius = 1.5;
			sphere.albedo = glm::vec3{ 0.2f,0.3f,1.0f };
			scene.spheres.push_back(sphere);
		}
		
	}

	virtual void OnUpdate(float dt) override
	{
		if (real_time)
		{
			camera.UpdateCamera(dt);
		}
	}

	virtual void OnUIRender() override
	{
//----------------------------------------------------------------------------------------------------------------------------------------

// Viewport:

		ImGui::Begin("Viewport");

		// dimension (in number of pixels):
		viewport_width = ImGui::GetContentRegionAvail().x;
		viewport_height = ImGui::GetContentRegionAvail().y;

		auto final_image = renderer.GetFinalImage();
		if (final_image)
		{
			ImGui::Image(final_image->GetDescriptorSet(), { (float)final_image->GetWidth(), (float)final_image->GetHeight() }, { 0,1 }, { 1,0 });		// display the image
			/*
			Note for the last two parameters:
			change uv0 (0,0) which is originally at the top left to (0,1) i.e. bottom left; change uv1 (1,1) which is originally at the bottom right to (0,1) i.e. top right.
			*/
		}

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------
		
// Control Panel:

		ImGui::Begin("Control Panel");

		ImGui::Text("%.0f FPS", 1000.0f/duration_per_frame);	// Note that this will print inf if duration_per_frame == 0
		ImGui::Text("%.0f ms", duration_per_frame);

		ImGui::Separator();

		if (ImGui::Button("Render in Real-Time"))	// NOTE: currently Render() is only called once every time we hit "Render"
		{
			real_time = true;
		}

		if (ImGui::Button("Denoise"))
		{
			// TODO
		}

		ImGui::Separator();

		if (ImGui::Button("Render Offline"))	// NOTE: currently Render() is only called once every time we hit "Render"
		{
			real_time = false;
			Render();
		}

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------

// Miscellaneous:
		
		//ImGui::ShowDemoWindow();
		
//----------------------------------------------------------------------------------------------------------------------------------------

		if (real_time)
		{
			Render();
		}
	}

	void Render()
	{
		Walnut::Timer frame_timer;	// this timer starts counting from here

		renderer.ResizeViewport(viewport_width, viewport_height);
		camera.ResizeViewport(viewport_width, viewport_height);
		renderer.Render(scene, camera);

		duration_per_frame = frame_timer.ElapsedMillis();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "8599 Ray Tracer";

	Walnut::Application* app = new Walnut::Application(spec);	// Vulkan is initialized AFTER this point
	app->PushLayer<CSC8599Layer>();		// OnUIRender() will be called every loop after this layer has been pushed.
	return app;
}
```

so that the current output is like follows:

<img src="https://github.com/IQ404/8599-ray-tracer-gui/blob/main/SampleImages/two_spheres.jpg" width="700" height="400"></a>

### May 16th 2023

- Restructure the renderer pipeline into a "software GPU ray-tracing pipeline"

```cpp
/*****************************************************************//**
 * \file   Renderer.h
 * \brief  The header file of the renderer for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#ifndef RENDERER_H
#define RENDERER_H

#include <memory>	// to use std::shared_ptr
#include <glm/glm.hpp>		// to use glm vec
#include "Walnut/Image.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

class Renderer
{
	std::shared_ptr<Walnut::Image> frame_image_final;
	uint32_t* frame_data = nullptr;
	const Scene* active_scene = nullptr;
	const Camera* active_camera = nullptr;

public:

	Renderer() = default;	// Defaulted default constructor: the compiler will define the implicit default constructor even if other constructors are present.

	void ResizeViewport(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const
	{
		return frame_image_final;
	}

private:

	struct HitRecord
	{
		float hit_Distance;
		glm::vec3 hit_WorldPosition;
		glm::vec3 hit_WorldNormal;

		int hit_ObjectIndex;
	};

	void RayGen_Shader(uint32_t x, uint32_t y);	// mimic one of the vulkan shaders which is called to cast ray(s) for every pixel
	HitRecord Intersection_Shader(const Ray& ray);		// Intersection_Shader: mimic one of the vulkan shaders which is called to determine whether we hit something
	HitRecord ClosestHit_Shader(const Ray& ray, float hit_Distance, int hit_ObjectIndex);	// mimic one of the vulkan shaders which is called when the ray hits something
	HitRecord Miss_Shader(const Ray& ray);	// mimic one of the vulkan shaders which is called when the ray does not hit anything
};

#endif // !RENDERER_H
```

```cpp
/*****************************************************************//**
 * \file   Renderer.cpp
 * \brief  The definitions of the renderer class for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Renderer.h"

namespace RTUtility
{
	uint32_t vecRGBA_to_0xABGR(const glm::vec4& colorRGBA)
	{
		uint8_t r = (uint8_t)(colorRGBA.r * 255.0f);
		uint8_t g = (uint8_t)(colorRGBA.g * 255.0f);
		uint8_t b = (uint8_t)(colorRGBA.b * 255.0f);
		uint8_t a = (uint8_t)(colorRGBA.a * 255.0f);

		return ((a << 24) | (b << 16) | (g << 8) | r);	// this automatically promotes 8 bits memory to 32 bits memory
	}
}

void Renderer::ResizeViewport(uint32_t width, uint32_t height)
{
	if (frame_image_final)
	{
		if ((frame_image_final->GetWidth() == width) && (frame_image_final->GetHeight() == height))
		{
			return;
		}
		frame_image_final->Resize(width, height);
	}
	else
	{
		frame_image_final = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] frame_data;
	frame_data = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	active_scene = &scene;
	active_camera = &camera;
	
	for (uint32_t y = 0; y < frame_image_final->GetHeight(); y++)		// each column
	{
		for (uint32_t x = 0; x < frame_image_final->GetWidth(); x++)		// each row
		{
			RayGen_Shader(x, y);
		}
	}

	frame_image_final->SetData(frame_data);	// send the frame data to GPU
}

void Renderer::RayGen_Shader(uint32_t x, uint32_t y)
{
	glm::vec4 colorRGBA;

	Ray ray;
	ray.origin = active_camera->Position();
	ray.direction = active_camera->RayDirections()[y * frame_image_final->GetWidth() + x];
	
	Renderer::HitRecord hit_record = Intersection_Shader(ray);

	if (hit_record.hit_Distance == -1.0f)
	{
		colorRGBA = glm::vec4{ 0.0f,0.0f,0.0f,1.0f };
	}
	else
	{
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ -1.0,-1.0,-1.0 });
		float diffuseIntensity = glm::max(glm::dot(hit_record.hit_WorldNormal, -lightDirection), 0.0f);
		glm::vec3 sphereColor = active_scene->spheres[hit_record.hit_ObjectIndex].albedo * diffuseIntensity;	// light source emits white light
		colorRGBA = glm::clamp(glm::vec4{ sphereColor,1.0f }, glm::vec4(0.0f), glm::vec4(1.0f));	// glm::clamp(value, min, max)
	}

	frame_data[(y * frame_image_final->GetWidth()) + x] = RTUtility::vecRGBA_to_0xABGR(colorRGBA);
}

Renderer::HitRecord Renderer::Intersection_Shader(const Ray& ray)
{
	float closestT = std::numeric_limits<float>::max();
	int closest_sphere_index = -1;
	for (size_t index = 0; index < active_scene->spheres.size(); index++)
	{
		const Sphere& sphere = active_scene->spheres[index];
		glm::vec3 transformed_origin = ray.origin - sphere.center;	// Do the equivalent movement on camera rather than on sphere

		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(transformed_origin, ray.direction);
		float c = glm::dot(transformed_origin, transformed_origin) - sphere.radius * sphere.radius;
		float discriminant = b * b - 4 * a * c;

		if (discriminant < 0.0f)
		{
			continue;
		}

		float closerT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closerT < closestT)
		{
			closestT = closerT;
			closest_sphere_index = (int)index;
		}
	}
	if (closest_sphere_index == -1)
	{
		return Miss_Shader(ray);
	}
	return ClosestHit_Shader(ray, closestT, closest_sphere_index);
}

Renderer::HitRecord Renderer::ClosestHit_Shader(const Ray& ray, float hit_Distance, int hit_ObjectIndex)
{
	Renderer::HitRecord hit_record;
	hit_record.hit_Distance = hit_Distance;
	hit_record.hit_ObjectIndex = hit_ObjectIndex;
	
	const Sphere& closest_sphere = active_scene->spheres[hit_ObjectIndex];
	
	glm::vec3 transformed_origin = ray.origin - closest_sphere.center;	// Do the equivalent movement on camera rather than on sphere
	glm::vec3 transformed_hit_point = transformed_origin + ray.direction * hit_Distance;

	hit_record.hit_WorldNormal = glm::normalize(transformed_hit_point);
	hit_record.hit_WorldPosition = transformed_hit_point + closest_sphere.center;

	return hit_record;
}

Renderer::HitRecord Renderer::Miss_Shader(const Ray& ray)
{
	Renderer::HitRecord hit_record;
	hit_record.hit_Distance = -1.0f;
	return hit_record;
}
```

### May 17th 2023

- A step towards "non-physical photorealistic rendering" (kidding...)

Apart from point light diffused from the sphere directly to the camera, we now also allow rays to bounce one more time according to mirror reflection on the sphere:

```cpp
/*****************************************************************//**
 * \file   Renderer.cpp
 * \brief  The definitions of the renderer class for the 8599 ray tracer
 * 
 * \author Xiaoyang Liu
 * \date   May 2023
 *********************************************************************/

#include "Renderer.h"

namespace RTUtility
{
	uint32_t vecRGBA_to_0xABGR(const glm::vec4& colorRGBA)
	{
		uint8_t r = (uint8_t)(colorRGBA.r * 255.0f);
		uint8_t g = (uint8_t)(colorRGBA.g * 255.0f);
		uint8_t b = (uint8_t)(colorRGBA.b * 255.0f);
		uint8_t a = (uint8_t)(colorRGBA.a * 255.0f);

		return ((a << 24) | (b << 16) | (g << 8) | r);	// this automatically promotes 8 bits memory to 32 bits memory
	}
}

void Renderer::ResizeViewport(uint32_t width, uint32_t height)
{
	if (frame_image_final)
	{
		if ((frame_image_final->GetWidth() == width) && (frame_image_final->GetHeight() == height))
		{
			return;
		}
		frame_image_final->Resize(width, height);
	}
	else
	{
		frame_image_final = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] frame_data;
	frame_data = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	active_scene = &scene;
	active_camera = &camera;
	
	for (uint32_t y = 0; y < frame_image_final->GetHeight(); y++)		// each column
	{
		for (uint32_t x = 0; x < frame_image_final->GetWidth(); x++)		// each row
		{
			RayGen_Shader(x, y);
		}
	}

	frame_image_final->SetData(frame_data);	// send the frame data to GPU
}

void Renderer::RayGen_Shader(uint32_t x, uint32_t y)
{
	glm::vec3 final_color(0.0f);

	Ray ray;
	ray.origin = active_camera->Position();
	ray.direction = active_camera->RayDirections()[y * frame_image_final->GetWidth() + x];
	
	int bounces = 2;
	float energy_remaining = 1.0f;
	glm::vec3 sky_color{ 0.0f,0.0f,0.0f };
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitRecord hit_record = Intersection_Shader(ray);

		if (hit_record.hit_Distance == -1.0f)
		{
			final_color += sky_color * energy_remaining;
			break;
		}
		else
		{
			glm::vec3 lightDirection = glm::normalize(glm::vec3{ -1.0,-1.0,-1.0 });
			float diffuseIntensity = glm::max(glm::dot(hit_record.hit_WorldNormal, -lightDirection), 0.0f);
			final_color += energy_remaining * active_scene->spheres[hit_record.hit_ObjectIndex].albedo * diffuseIntensity;	// light source emits white light
			energy_remaining *= 0.7f;
			ray.origin = hit_record.hit_WorldPosition + hit_record.hit_WorldNormal * 0.0001f;	// shadow acne elimination
			ray.direction = glm::reflect(ray.direction, hit_record.hit_WorldNormal);
		}
	}

	glm::vec4 final_color_RGBA = glm::clamp(glm::vec4{ final_color,1.0f }, glm::vec4(0.0f), glm::vec4(1.0f));	// glm::clamp(value, min, max)

	frame_data[(y * frame_image_final->GetWidth()) + x] = RTUtility::vecRGBA_to_0xABGR(final_color_RGBA);
}

Renderer::HitRecord Renderer::Intersection_Shader(const Ray& ray)
{
	float closestT = std::numeric_limits<float>::max();
	int closest_sphere_index = -1;
	for (size_t index = 0; index < active_scene->spheres.size(); index++)
	{
		const Sphere& sphere = active_scene->spheres[index];
		glm::vec3 transformed_origin = ray.origin - sphere.center;	// Do the equivalent movement on camera rather than on sphere

		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(transformed_origin, ray.direction);
		float c = glm::dot(transformed_origin, transformed_origin) - sphere.radius * sphere.radius;
		float discriminant = b * b - 4 * a * c;

		if (discriminant < 0.0f)
		{
			continue;
		}

		float closerT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closerT > 0.0f && closerT < closestT)	// note that here we are coding it so that when we are inside a sphere, that sphere will NOT be rendered.
		{
			closestT = closerT;
			closest_sphere_index = (int)index;
		}
	}
	if (closest_sphere_index == -1)
	{
		return Miss_Shader(ray);
	}
	return ClosestHit_Shader(ray, closestT, closest_sphere_index);
}

Renderer::HitRecord Renderer::ClosestHit_Shader(const Ray& ray, float hit_Distance, int hit_ObjectIndex)
{
	Renderer::HitRecord hit_record;
	hit_record.hit_Distance = hit_Distance;
	hit_record.hit_ObjectIndex = hit_ObjectIndex;
	
	const Sphere& closest_sphere = active_scene->spheres[hit_ObjectIndex];
	
	glm::vec3 transformed_origin = ray.origin - closest_sphere.center;	// Do the equivalent movement on camera rather than on sphere
	glm::vec3 transformed_hit_point = transformed_origin + ray.direction * hit_Distance;

	hit_record.hit_WorldNormal = glm::normalize(transformed_hit_point);
	hit_record.hit_WorldPosition = transformed_hit_point + closest_sphere.center;

	return hit_record;
}

Renderer::HitRecord Renderer::Miss_Shader(const Ray& ray)
{
	Renderer::HitRecord hit_record;
	hit_record.hit_Distance = -1.0f;
	return hit_record;
}
```

This results in the following effect: (note that this is not physically correct!)

<img src="https://github.com/IQ404/8599-ray-tracer-gui/blob/main/SampleImages/reflection_between_two_spheres.jpg" width="700" height="400"></a>

### May 18-19th 2023

- Separate material from geometry.
- Add roughness to reflection.
- Implement temporal accumulation sampling.
- Apply multithreading.

The framework of our ray-tracing GUI is more or less finished by far. The code up to this point is saved into [this branch](https://github.com/IQ404/8599-ray-tracer-gui/tree/initial_framework).

### May 20th 2023

I was playing games on this day.

### May 21st 2023

I was travelled to Durham on this day.

### May 22nd 2023

I decided to relearn / learn more about ray-tracing before I proceed, 
