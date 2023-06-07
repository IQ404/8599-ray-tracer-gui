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

//float tr_accurate_fresnel_reflectance(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal, const float& entity_refraction_index)
//// The parameters has the same assumptions as for snell_refraction_direction().
//{
//	// Trial settings:
//	float eta_in = 1.0f;
//	float eta_out = entity_refraction_index;
//	// Trial settings correction:
//	float cos_incident = glm::dot(incident_ray_direction, surface_normal);
//	if (cos_incident < 0)		// case 1: ray is coming from the outside
//	{
//		cos_incident = -cos_incident;
//	}
//	else	// case 2: ray is coming from the inside
//			// Note that if cos_incident = 0 we treat the incident ray as coming from the inside, thus we return 1
//	{
//		std::swap(eta_in, eta_out);
//	}
//	float sin_refract = eta_in / eta_out * std::sqrtf(std::max(0.0f, 1 - cos_incident * cos_incident));		// Snell's law
//	if (sin_refract > 1.0f)		// Total internal reflection
//	{
//		return 1.0f;
//	}
//	else	// Note: this is NOT Schlick¡¯s approximation, we DO consider polarization here
//	{
//		float cos_refract = std::sqrtf(std::max(0.0f, 1 - sin_refract * sin_refract));
//		float R_s_sqrt = (eta_in * cos_incident - eta_out * cos_refract) / (eta_in * cos_incident + eta_out * cos_refract);
//		float R_p_sqrt = (eta_in * cos_refract - eta_out * cos_incident) / (eta_in * cos_refract + eta_out * cos_incident);
//		return (R_s_sqrt * R_s_sqrt + R_p_sqrt * R_p_sqrt) / 2;
//		// See https://en.wikipedia.org/wiki/Fresnel_equations
//	}
//}
//
//glm::vec3 snell_refraction_direction(const glm::vec3& incident_ray_direction, const glm::vec3& surface_normal, const float& entity_refraction_index)
//// Assume incident_ray_direction and surface_normal are unit vectors where the incident_ray_direction is toward to the
//// surface evaluation point and the surface_nornal is pointing outwards.
//// Assume we want the normal used in computation to always towards the incident ray, and want the incident vector used in
//// computation to always originate from the surface evaluation point.
//// The snell_refraction_direction we are returning is also a unit vector.
//{
//	// Trial settings:
//	float eta_in = 1.0f;
//	float eta_out = entity_refraction_index;
//	glm::vec3 normal = surface_normal;
//	// Trial settings correction:
//	float cos_incident = glm::dot(incident_ray_direction, surface_normal);
//	if (cos_incident < 0)		// case 1: ray is coming from the outside
//	{
//		cos_incident = -cos_incident;
//	}
//	else	// case 2: ray is coming from the inside
//			// Note that if cos_incident = 0, we treat the incident ray as coming from the inside, thus we return 0
//	{
//		std::swap(eta_in, eta_out);
//		normal = -normal;
//	}
//	float eta_ratio = eta_in / eta_out;
//	float cos_refract_squared = 1 - eta_ratio * eta_ratio * (1 - cos_incident * cos_incident);
//	return (cos_refract_squared < 0) ?
//		(glm::vec3{0.0f, 0.0f, 0.0f}) :
//		(eta_ratio * incident_ray_direction + (eta_ratio * cos_incident - std::sqrtf(cos_refract_squared)) * normal);
//	// if not total internal reflection, we have:
//	//		refraction_direction = component_parallel_to_the_normal + component_perpendicular_to_the_normal
//}




class CSC8599Layer : public Walnut::Layer
{
	float duration_per_frame = 0.0f;
	bool real_time = false;
	Renderer renderer;
	Camera camera{ 35.0f, 0.1f, 100.0f };
	uint32_t viewport_width = 0;
	uint32_t viewport_height = 0;
	Scene scene;

public:
	CSC8599Layer()
	{
		Material& sphere_0_material = scene.materials.emplace_back();
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
		}
		
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

		ImGui::End();

//----------------------------------------------------------------------------------------------------------------------------------------
		
// Control Panel:

		ImGui::Begin("Control Panel");

		/*glm::vec3 inci = glm::normalize(glm::vec3{2.0f, -1.0f, 0.0f});
		glm::vec3 inci_re = glm::normalize(-snell_refraction_direction(inci, glm::vec3{0.0f, 1.0f, 0.0f}, 1.5f));
		float T_in = 1 - tr_accurate_fresnel_reflectance(inci, glm::vec3{0.0f, 1.0f, 0.0f}, 1.5f);
		float T_out = 1 - tr_accurate_fresnel_reflectance(inci_re, glm::vec3{0.0f, 1.0f, 0.0f}, 1.5f);
		std::cout << (T_in) << std::endl;
		std::cout << (T_out) << std::endl;*/

		ImGui::Text("%.0f FPS", 1000.0f/duration_per_frame);	// Note that this will print inf if duration_per_frame == 0
		ImGui::Text("%.0f ms", duration_per_frame);

		ImGui::Separator();

		if (ImGui::Button("Render in Real-Time"))
		{
			real_time = true;
		}

		ImGui::Checkbox("Temporal Accumulation", &renderer.GetSettings().accumulating);

		if (ImGui::Button("Denoise"))
		{
			// TODO
		}

		ImGui::Separator();

		if (ImGui::Button("Render Offline"))
		{
			real_time = false;
			renderer.Reaccumulate();
			Render();
		}

		ImGui::Separator();

		//for (size_t i = 0; i < scene.materials.size(); i++)
		//{
		//	ImGui::PushID(i);	// Let ImGui know that the following controls (until ImGui::PopID) are exclusive for the ith sphere

		//	ImGui::Text("Sphere %i: ", i);

		//	Material& material = scene.materials[i];

		//	ImGui::DragFloat("Metallic", &material.metallic, 0.001f, 0.0f, 1.0f);
		//	ImGui::DragFloat("Roughness", &material.roughness, 0.001f, 0.0f, 1.0f);

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