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
	Camera camera{ 35.0f, 0.1f, 100.0f };
	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;
	Scene scene;
	NP_PathTracing::CompositeHittable world;

public:
	CSC8599Layer()
	{
		/*Material& sphere_0_material = scene.materials.emplace_back();
		sphere_0_material.albedo = glm::vec3{ 1.0f,0.0f,1.0f };
		sphere_0_material.roughness = 0.0f;

		Material& sphere_1_material = scene.materials.emplace_back();
		sphere_1_material.albedo = glm::vec3{ 0.2f,0.3f,1.0f };
		sphere_1_material.roughness = 0.1f;

		{
			Sphere sphere;
			sphere.center = glm::vec3{ 0.0f,0.0f,0.0f };
			sphere.radius = 1.0f;
			sphere.material_index = 0;
			scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = glm::vec3{ 0.0f,-101.0f,0.0f };
			sphere.radius = 100.0f;
			sphere.material_index = 1;
			scene.spheres.push_back(sphere);
		}*/
		
		//-------------------------------

		auto material_ground = std::make_shared<NP_PathTracing::Diffuse>(ColorRGB(0.8, 0.8, 0.0));
		auto material_back = std::make_shared<NP_PathTracing::Diffuse>(ColorRGB(0.1, 0.2, 0.5));
		auto material_up = std::make_shared<NP_PathTracing::Dielectric>(1.5);
		auto material_left = std::make_shared<NP_PathTracing::Metal>(ColorRGB(0.8, 0.6, 0.2), 0.0);
		auto material_right = std::make_shared<NP_PathTracing::Metal>(ColorRGB(0.8, 0.6, 0.2), 0.5);
		world.add(std::make_shared<NP_PathTracing::Sphere>(Point3D(0.0, -100.5, -1.0), 100.0, material_ground));
		world.add(std::make_shared<NP_PathTracing::Sphere>(Point3D(0.0, 0.0, -3.0), 0.5, material_back));
		world.add(std::make_shared<NP_PathTracing::Sphere>(Point3D(-1.0, 0.0, -1.0), 0.5, material_left));
		world.add(std::make_shared<NP_PathTracing::Sphere>(Point3D(0.0, 2.0, -2.0), 0.5, material_up));
		world.add(std::make_shared<NP_PathTracing::Sphere>(Point3D(0.0, 2.0, -2.0), -0.05, material_up));
		world.add(std::make_shared<NP_PathTracing::Sphere>(Point3D(1.0, 0.0, -1.0), 0.5, material_right));
	}

	virtual void OnUpdate(float dt) override
	{
		if (real_time)
		{
			if (camera.UpdateCamera(dt))
			{
				renderer.Reaccumulate();
			}
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

		/*
		TODO:
			If we call ImGui::GetContentRegionAvail() after the above ImGui::Image, ImGui::GetContentRegionAvail().y will return incorrect value (-4).
			Explain why...
		*/
		//// dimension (in number of pixels):
		//viewport_width = ImGui::GetContentRegionAvail().x;
		//viewport_height = ImGui::GetContentRegionAvail().y;

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------
		
// Control Panel:

		ImGui::Begin("Control Panel");

		ImGui::Text("%.0f FPS", (float)1000.0f / duration_per_frame);	// Note that this will print inf if duration_per_frame == 0
		ImGui::Text("%.0f ms", duration_per_frame);

		ImGui::Separator();

		if (ImGui::Button("Render in Real-Time"))
		{
			real_time = true;
		}

		ImGui::Separator();

		ImGui::Checkbox("Temporal Accumulation", &renderer.GetSettings().accumulating);

		ImGui::Separator();

		ImGui::Text("Diffuse Model:");

		if (ImGui::Button("IN-Sphere"))
		{
			renderer.Reaccumulate();
			NP_PathTracing::GetActiveDiffuseModel() = NP_PathTracing::IN_Sphere;
		}

		if (ImGui::Button("ON-Sphere (Lambertian)"))
		{
			renderer.Reaccumulate();
			NP_PathTracing::GetActiveDiffuseModel() = NP_PathTracing::ON_Sphere;
		}

		if (ImGui::Button("IN-Hemisphere (Uniform)"))
		{
			renderer.Reaccumulate();
			NP_PathTracing::GetActiveDiffuseModel() = NP_PathTracing::IN_Hemisphere;
		}

		ImGui::Separator();

		if (ImGui::Button("Denoise"))
		{
			// TODO
		}

		ImGui::Separator();

		if (ImGui::Button("Render Offline"))
		{
			real_time = false;
			renderer.Reaccumulate();	// Does not accumlate temporally whenever we are just rendering one frame.
			Render();
		}

		ImGui::Separator();

		//for (size_t i = 0; i < scene.materials.size(); i++)
		//{
		//	ImGui::PushID(i);	// Let ImGui know that the following controls (until ImGui::PopID) are exclusive for the ith sphere
		//
		//	ImGui::Text("Sphere %i: ", i);
		//
		//	Material& material = scene.materials[i];
		//
		//	ImGui::DragFloat("Metallic", &material.metallic, 0.001f, 0.0f, 1.0f);
		//	ImGui::DragFloat("Roughness", &material.roughness, 0.001f, 0.0f, 1.0f);
		//
		//	ImGui::PopID();
		//}

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
		Walnut::Timer frame_timer;	// this timer starts counting from right here

		renderer.ResizeViewport(viewport_width, viewport_height);
		camera.ResizeViewport(viewport_width, viewport_height);
		renderer.Render(scene, camera, world);

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