#include <gtest/gtest.h>
#include <meshinfo.h>
#include <string>
#include <objloader.h>
#include <embree_raytracer.h>
#include <robin_hood.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <view_analysis.h>

#include "RayRequest.h"

#include "performance_testing.h"

using namespace HF::Geometry;
using namespace HF::RayTracer;
using std::vector;
using std::array;
using std::string;
using std::cerr;
using std::endl;

/// <summary>
/// Create a new raytracer from a basic 10x10 plane centered on the origin.
/// </summary>
inline EmbreeRayTracer CreateRTWithPlane() {
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};

	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };
	return EmbreeRayTracer(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});
}

/// <summary>
/// Calc distance between two arrays.
/// </summary>
inline float Distance(const array<float, 3>& p1, const array<float, 3>& p2) {
	return sqrt(
		pow(p1[0] - p2[0], 2)
		+ pow(p1[1] - p2[1], 2)
		+ pow(p1[2] - p2[2], 2)
	);
}

TEST(_EmbreeRayTracer, HashAlgorithm) {
	robin_hood::unordered_map<std::array<float, 3>, int> floatmap;

	std::array<float, 3> key1 = { 1,2,3 };
	std::array<float, 3> key2 = { 1.01,2.01,3.01 };

	floatmap[key1] = 1;
	floatmap[key2] = 2;

	EXPECT_EQ(floatmap[key1], 1);
	EXPECT_EQ(floatmap[key2], 2);
}

TEST(_EmbreeRayTracer, Init) {
	std::string teapot_path = "teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

	auto k = HF::RayTracer::EmbreeRayTracer(geom);
}

TEST(_EmbreeRayTracer, Copy) {
	std::string teapot_path = "teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// If the copy didn't work, then this operation would throw
	auto rt2 = k;
	rt2.FireOcclusionRay(std::array<float, 3>{1, 1, 1}, std::array<float, 3>{1, 1, 1});
}

TEST(_EmbreeRayTracer, EmbreeGarbageCollectCorrect) {

	// Load teapot
	std::string teapot_path = "teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

	// Construct a raytracer
	HF::RayTracer::EmbreeRayTracer * ERT = new HF::RayTracer::EmbreeRayTracer(geom);
	
	// Call copy constructor to create a new raytracer
	HF::RayTracer::EmbreeRayTracer ERT2 = *ERT;

	// Delete the original raytracer
	delete ERT;

	// Try to fire a ray. If this crashes, then it means the copy constructor isn't correctly incrementing the reference counter.
	ERT2.FireOcclusionRay(std::array<float, 3>{1, 1, 1}, std::array<float, 3>{1, 1, 1});
}


TEST(_EmbreeRayTracer, OcclusionRays) {
	std::string teapot_path = "big_teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE, true);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// All of these rays should hit since the origin is inside of the teapot
	std::array<float, 3> origin{ 0,0,1 };
	std::vector<std::array<float, 3>> directions = {
		{0,0,1},
		{0,1,0},
		{1,0,0},
		{-1,0,0},
		{0,-1,0},
		{0,0,-1},
	};
	for (auto& dir : directions)
		EXPECT_TRUE(k.FireOcclusionRay(origin, dir));
}

TEST(_EmbreeRayTracer, StandardRays) {
	std::string teapot_path = "big_teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE, true);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// All of these rays should hit since the origin is inside of the teapot
	const std::vector<std::array<float, 3>> directions = {
		{0,0,1},
		{0,1,0},
		{1,0,0},
		{-1,0,0},
		{0,-1,0},
		{0,0,-1},
	};
	for (auto& dir : directions) {
		std::array<float, 3> origin{ 0,0,1 };
		std::cerr << "(" << dir[0] << "," << dir[1] << "," << dir[2] << ")" << std::endl;
		EXPECT_TRUE(k.FireRay(origin, dir));
	}
}

TEST(_EmbreeRayTracer, HitPointsAreAccurate) {
	std::string plane_path = "plane.obj";
	auto geom = HF::Geometry::LoadMeshObjects(plane_path, HF::Geometry::ONLY_FILE, true);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// All of these rays should hit since the origin is inside of the teapot
	std::vector<std::array<float, 3>> origins = {
		{0,0,1},
		{0,1,1},
		{1,0,1},
		{-1,0,1},
		{0,-1,1},
		{0,0,1},
	};

	const std::array<float, 3> direction{ 0,0,-1 };
	float height = NAN;
	for (auto& origin : origins) {
		std::cerr << "(" << origin[0] << "," << origin[1] << "," << origin[2] << ")" << std::endl;
		EXPECT_TRUE(k.FireRay(origin, direction));

		if (isnan(height))
			height = origin[2];
		else
			EXPECT_NEAR(height, origin[2], 0.001);
	}
}

// Fire a large volume of rays to assert that we don't have any issues with race conditions.
TEST(_EmbreeRayTracer, DeterministicResults) {
	// Create plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});
	
	const int num_trials = 150;
	const int num_rays = 1000;
	int fails = 0;
	int successes = 0;

	// Iterate through number of trials
	for (int k = 0; k < num_trials; k++) {

		// Go Create direction/origin arrays
		std::vector<std::array<float, 3>> directions(num_rays, std::array<float, 3>{0, 0, -1});
		std::vector<std::array<float, 3>> origins(num_rays, std::array<float, 3>{0, 0, 1});
		
		// Fire rays in parallel
		auto results = ert.FireRays(origins, directions);

		// Check the result of each ray
		for (int i = 0; i < num_rays; i++) {
			// Mark this in test explorer
			float dist = Distance(origins[i], std::array<float, 3>{0, 0, 0});

			// This ray is incorrect if it's distance is greater than our threshold or it 
			// doesn't intersect the ground. 
			if (!results[i] || Distance(origins[i], std::array<float, 3>{0, 0, 0}) > 0.0001)
			{
				std::cerr << "FAILED] Trial: " << k << " Ray: " << i 
				<< " Result: "  << results[i] << " Distance: " << dist << std::endl;
				fails++;
			}
			else
				successes++;
		}
	}

	// Print total number of fails / succeses if we fail
	if (fails > 0) {
		double fail_percent = static_cast<double>(fails) / static_cast<double>(successes);
		std::cerr << "END RESULTS -  FAILURES:" << fails << ", SUCCESSES: " << successes
			<< " RATE: " << fail_percent << "\%" << std::endl;
	}
	ASSERT_EQ(fails, 0);
}

// TODO: Add a distance check to this?
TEST(_EmbreeRayTracer, FireRays) {
	// Create plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Create an array of directions all containing {0,0,-1}
	std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{0, 0, -1});

	// Create an array of origin points moving further to the left with each point
	std::vector<std::array<float, 3>> origins(10);
	for (int i = 0; i < 10; i++) origins[i] = std::array<float, 3>{static_cast<float>(1.99 * i), 0, 1};

	// Fire every ray. Results should all be true and be within a certain distance of zero;
	auto results = ert.FireRays(origins, directions);

	// Print results
	std::cerr << "[";
	for (int i = 0; i < 10; i++) {
		if (results[i])
			std::cerr << "(" << origins[i][0] << ", " << origins[i][1] << ", " << origins[i][2] << ")";
		else
			std::cerr << "Miss";

		if (i != 9) std::cerr << ", ";

		// Test that the ray intersected, and it isn't far from where it should have hit.
		if (i < 6) {
			ASSERT_TRUE(results[i]);

			float dist = Distance(origins[i], std::array<float, 3>{static_cast<float>(i) * 1.99f, 0, 0});
			ASSERT_NEAR(dist, 0, 0.0001);
		}
		else ASSERT_FALSE(results[i]);
	}
	std::cerr << "]" << std::endl;
}

TEST(_EmbreeRayTracer, FireOcclusionRays) {
	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Create an array of directions all containing {0,0,-1}
	std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{0, 0, -1});

	// Create an array of origins with the first 5 values being above the plane and the last
	// five values being under it.
	std::vector<std::array<float, 3>> origins(10);
	for (int i = 0; i < 5; i++) origins[i] = std::array<float, 3>{0.0f, 0.0f, 1.0f};
	for (int i = 5; i < 10; i++) origins[i] = std::array<float, 3>{0.0f, 0.0f, -1.0f};

	// Fire every ray.
	std::vector<char> results = ert.FireOcclusionRays(origins, directions);

	// Iterate through all results to print them
	std::cerr << "[";
	for (int i = 0; i < 10; i++) {
		// Print true if the ray intersected, false otherwise
		if (results[i]) std::cout << "True";
		else std::cerr << "False";

		// Add a comma if it's not the last member
		if (i != 9) std::cerr << ", ";

		if (i < 5) ASSERT_TRUE(results[i]);
		else ASSERT_FALSE(results[i]);
	}
	std::cerr << "]" << std::endl;
}

TEST(_EmbreeRayTracer, FireRay) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	float x = 0; float y = 0; float z = 1;
	bool res;

	// Fire a ray straight down and ensure it connects with a distance of 1 (within a certain tolerance)
	res = ert.FireRay(x, y, z, 0, 0, -1);
	if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_TRUE(res);
	ASSERT_NEAR(Distance(std::array<float, 3>{x, y, z}, std::array<float, 3>{0, 0, 0}), 0, 0.0001);

	x = 0; y = 0; z = 1;
	// Fire a ray straight up and ensure it misses
	res = ert.FireRay(x, y, z, 0, 0, 1);
	if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_FALSE(res);
}

TEST(_EmbreeRayTracer, FireRayArrayOverload) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Fire a ray straight down and ensure it connects with a distance of 1 (within a certain tolerance)
	std::array<float, 3> origin{ 0,0,1 };
	bool res = ert.FireRay(
		origin,
		std::array<float, 3>{0, 0, -1}
	);

	// Print Results
	if (res) std::cerr << "(" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_TRUE(res);
	ASSERT_NEAR(Distance(origin, std::array<float, 3>{0, 0, 0}), 0, 0.0001);

	// Fire a ray straight up and ensure it misses
	origin = std::array<float, 3>{ 0, 0, 1 };
	res = ert.FireRay(
		origin,
		std::array<float, 3>{0, 0, 1}
	);

	// Print Results
	if (res) std::cerr << "(" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_FALSE(res);
}

/*
#undef max
TEST(Sanity, Precision) {

	auto rt = CreateRTWithPlane();

	int num_rays = 10000;
	float max_height = 1000.0f;
	auto obj_path = GetTestOBJPath("energy blob");
	auto ray_tracer = EmbreeRayTracer(HF::Geometry::LoadMeshObjects(obj_path));

	std::array<float, 3> start_point = { 7.587f, 3.890f, 12.276f };

	float increment = max_height / static_cast<float>(num_rays);
	std::vector<float> distances(num_rays);

	const std::array<float, 3> down = { 0,0,-1 };

	int out_mesh_id = -1;

	std::ofstream myfile;
	myfile.open("EmbreeRayTracerResults2.csv");
	myfile << "trial" << "," << "height" << "," << "distance" << "," << "difference" << "," << "moved" << std::endl;

	for (int i = 0; i < num_rays; i++) {
		const float current_z_value = (increment * static_cast<float>(i)) + start_point[2];

		std::array<float, 3> origin = { start_point[0], start_point[1], current_z_value };
		ray_tracer.FireAnyRay(
			origin,
			down,
			distances[i],
			out_mesh_id
		);

		myfile << i << ","
			<< current_z_value - start_point[2] << ","
			<< distances[i] << ","
			<< current_z_value - start_point[2] - distances[i] << ","
			<< origin[2] + (down[2] * distances[i])
			<< std::endl;
	}
	myfile.close();
}
*/
TEST(_EmbreeRayTracer, Intersect) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	HitStruct res;

	// Fire a ray straight down
	res = ert.Intersect(0, 0, 1, 0, 0, -1);

	// Print distance if it connected
	if (res.DidHit()) std::cerr << res.distance << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_TRUE(res.DidHit());
	ASSERT_NEAR(res.distance, 1, 0.0001);

	// Fire a ray straight up and ensure it misses
	res = ert.Intersect(0, 0, 1, 0, 0, 1);
	if (res.DidHit()) std::cerr << res.distance << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_FALSE(res.DidHit());
}

TEST(_EmbreeRayTracer, FireAnyRay) {
	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Create a vector of direction and origin arrays.
	std::array<float, 3> origin{ 0,0.5,1 };
	std::array<float, 3> direction{ 0,0,-1 };

	bool res = false; float out_dist = -1; int out_id = -1;

	// Fire a ray straight down
	res = ert.FireAnyRay(origin, direction, out_dist, out_id);
	ASSERT_TRUE(res);
	ASSERT_NEAR(out_dist, 1, 0.0001);

	// Print its distance if it connected
	if (res) std::cerr << out_dist << std::endl;
	else std::cerr << "Miss" << std::endl;

	// Fire a ray straight up and ensure it misses
	res = ert.FireAnyRay(origin, origin, out_dist, out_id);
	ASSERT_FALSE(res);

	// Print its distance if it connected
	if (res) std::cerr << out_dist << std::endl;
	else std::cerr << "Miss" << std::endl;
}

TEST(_EmbreeRayTracer, FireAnyOcclusionRay) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Fire a ray straight down
	bool res = ert.FireAnyOcclusionRay(
		std::array<float, 3>{0, 0, 1},
		std::array<float, 3>{0, 0, -1}
	);

	ASSERT_TRUE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;

	// Fire a ray straight up
	res = ert.FireAnyOcclusionRay(
		std::array<float, 3>{0, 0, 1},
		std::array<float, 3>{0, 0, 1}
	);

	ASSERT_FALSE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;
}

TEST(_EmbreeRayTracer, FireOcclusionRayArray) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Fire a ray straight down
	bool res = ert.FireOcclusionRay(
		std::array<float, 3>{0, 0, 1},
		std::array<float, 3>{0, 0, -1}
	);

	ASSERT_TRUE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;

	// Fire a ray straight up
	res = ert.FireOcclusionRay(
		std::array<float, 3>{0, 0, 1},
		std::array<float, 3>{0, 0, 1}
	);

	ASSERT_FALSE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;
}

TEST(_EmbreeRayTracer, FireOcclusionRay) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Fire a ray straight down
	bool res = ert.FireOcclusionRay(0, 0, 1, 0, 0, -1);
	ASSERT_TRUE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;

	// Fire a ray straight up
	res = ert.FireOcclusionRay(0, 0, 1, 0, 0, 1);
	ASSERT_FALSE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;
}

TEST(_EmbreeRayTracer, InsertNewMesh) {
	// Requires #include "embree_raytracer.h", #include "objloader.h"

	// Create a container of coordinates
	std::vector<std::array<float, 3>> directions = {
		{0, 0, 1},
		{0, 1, 0},
		{1, 0, 0},
		{-1, 0, 0},
		{0, -1, 0},
		{0, 0, -1},
	};

	// Create the EmbreeRayTracer
	auto ert = HF::RayTracer::EmbreeRayTracer(directions);

	// Prepare the mesh ID
	const int id = 214;

	// Insert the mesh, Commit parameter defaults to false
	bool status = ert.InsertNewMesh(directions, id);

	std::string result = status ? "ok" : "not ok";
	std::cout << result << std::endl;
}

TEST(_EmbreeRayTracer, InsertNewMeshOneMesh) {
	// Requires #include "embree_raytracer.h", #include "objloader.h"

	// Create a container of coordinates
	std::vector<std::array<float, 3>> directions = {
		{0, 0, 1},
		{0, 1, 0},
		{1, 0, 0}
	};


	// Create the EmbreeRayTracer
	auto ert = HF::RayTracer::EmbreeRayTracer(directions);

	// Prepare coordinates to create a mesh
	std::vector<std::array<float, 3>> mesh_coords = { {-1, 0, 0},
		{0, -1, 0},
		{0, 0, -1} };

	// Create a mesh
	const int id = 325;
	const std::string mesh_name = "my mesh";
	HF::Geometry::MeshInfo mesh(mesh_coords, id, mesh_name);

	// Determine if mesh insertion successful
	if (ert.InsertNewMesh(mesh, false)) {
		std::cout << "Mesh insertion okay" << std::endl;
	}
	else {
		std::cout << "Mesh insertion error" << std::endl;
	}
}

TEST(_EmbreeRayTracer, InsertNewMeshVecMesh) {
	// Requires #include "embree_raytracer.h", #include "objloader.h"

	// For brevity
	using HF::Geometry::MeshInfo;
	using HF::RayTracer::EmbreeRayTracer;

	// Prepare the obj file path
	std::string teapot_path = "teapot.obj";
	std::vector<MeshInfo> geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

	// Create the EmbreeRayTracer
	auto ert = EmbreeRayTracer(geom);

	// Prepare coordinates to create a mesh
	std::vector<std::array<float, 3>> mesh_coords_0 = {
		{0, 0, 1},
		{0, 1, 0},
		{1, 0, 0}
	};

	std::vector<std::array<float, 3>> mesh_coords_1 = {
		{-1, 0, 0},
		{0, -1, 0},
		{0, 0, -1}
	};

	// Prepare mesh IDs and names
	const int mesh_id_0 = 241;
	const int mesh_id_1 = 363;
	const std::string mesh_name_0 = "this mesh";
	const std::string mesh_name_1 = "that mesh";

	// Create each MeshInfo
	MeshInfo mesh_0(mesh_coords_0, mesh_id_0, mesh_name_0);
	MeshInfo mesh_1(mesh_coords_1, mesh_id_1, mesh_name_1);

	// Create a container of MeshInfo
	std::vector<MeshInfo> mesh_vec = { mesh_0, mesh_1 };

	// Determine if mesh insertion successful
	if (ert.InsertNewMesh(mesh_vec, false)) {
		std::cout << "Mesh insertion okay" << std::endl;
	}
	else {
		std::cout << "Mesh insertion error" << std::endl;
	}
}

TEST(_EmbreeRayTracer, OperatorAssignment) {
	// Requires #include "embree_raytracer.h"

	// Create a container of coordinates
	std::vector<std::array<float, 3>> directions = {
		{0, 0, 1},
		{0, 1, 0},
		{1, 0, 0},
		{-1, 0, 0},
		{0, -1, 0},
		{0, 0, -1},
	};

	// Create the EmbreeRayTracer
	HF::RayTracer::EmbreeRayTracer ert_0(directions);
	
	// Create an EmbreeRayTracer, no arguments
	HF::RayTracer::EmbreeRayTracer ert_1;

	// If and when ert_0 goes out of scope,
	// data within ert_0 will be retained inside of ert_1.
	ert_1 = ert_0;
}

TEST(_FullRayRequest, ConstructorArgs) {
	// Requires #include "RayRequest.h"

	// For brevity
	using HF::RayTracer::FullRayRequest;

	// Prepare FullRayRequest's parameters
	const float x_in = 0.0;
	const float y_in = 0.0;
	const float z_in = 0.0;
	const float dx_in = 1.0;
	const float dy_in = 1.0;
	const float dz_in = 2.0;
	const float distance_in = 10.0;

	// Create the FullRayRequest 
	FullRayRequest request(x_in, y_in, z_in, dx_in, dy_in, dz_in, distance_in);
}

TEST(_FullRayRequest, DidHit) {
	// Requires #include "RayRequest.h"

	// For brevity
	using HF::RayTracer::FullRayRequest;

	// Prepare FullRayRequest's parameters
	const float x_in = 0.0;
	const float y_in = 0.0;
	const float z_in = 0.0;
	const float dx_in = 1.0;
	const float dy_in = 1.0;
	const float dz_in = 2.0;
	const float distance_in = 10.0;

	// Create the FullRayRequest 
	FullRayRequest request(x_in, y_in, z_in, dx_in, dy_in, dz_in, distance_in);

	// Use didHit
	if (request.didHit()) {
		std::cout << "Hit" << std::endl;
	}
	else {
		std::cout << "Miss" << std::endl;
	}
}

/*!
	\brief How quickly the raytracer can fire rays directly at a simple plane.

	\details
	Likely isn't truly indicative of the raytracer's performance, but provides a starting point
	for other more in depth performance tests. Variations on model complexity and ray direction
	are suggested.
*/
TEST(Performance, EmbreeRaytracer) {
	
	// Number of trials is based on number of elements here
	const vector<int> raycount = {
		100,
		1000,
		10000,
		100000,
		1000000
	};
	const int num_trials = raycount.size();

	// Create Watches
	std::vector<StopWatch> watches(num_trials);

	// Setup raytracer
	string model_key = "plane";
	string model_path = GetTestOBJPath(model_key);
	auto meshes = LoadMeshObjects(model_path);
	EmbreeRayTracer ert(meshes);

	// Ray settings
	array<float, 3> origin{ 0,0,1 };
	array<float, 3> direction{ 0,0,-1};

	for (int i = 0; i < num_trials; i++) {
		const int num_rays = raycount[i];
		auto& watch = watches[i];

		// Create arrays of origins and directions
		vector<array<float, 3>> origins(num_rays, origin);
		vector<array<float, 3>> directions(num_rays, direction);
		
		watch.StartClock();
		auto results = ert.FireRays(origins, directions);
		watch.StopClock();
	}
	
	PrintTrials(watches, raycount, "rays");
}

inline void WriteToCSV(std::ofstream &  file, const std::vector<string> & strings_to_write) {
	const int n = strings_to_write.size();
	for (int i = 0; i < strings_to_write.size(); i++) {
		file << strings_to_write[i] << ( (i != n - 1) ? "," : "\n");
	}
}

struct ModelAndStart {
	std::array<float, 3> start;
	EmbreeRayTracer PreciseERT;
	EmbreeRayTracer StandardERT;

	int verts = 0;
	int triangles = 0;
	string model_name;

	ModelAndStart(std::array<float, 3> start_point, string model, bool flip_z = false) {

		std::cout << "Loading " << model << std::endl;

		std::vector<MeshInfo> MI = HF::Geometry::LoadMeshObjects(model, ONLY_FILE, flip_z);
		for (auto& m : MI)
		{
			verts += m.NumVerts();
			triangles += m.NumTris();
		}

		StandardERT = EmbreeRayTracer(MI, false);
		PreciseERT = EmbreeRayTracer(MI,  true);
		start = start_point;
		model_name = model;
	};
};

inline int count_hits(vector<HitStruct>& results) {
	int hits = 0;
	for (const auto& result : results)
		if (result.DidHit())
			++hits;

	return hits;
}

void PrintDirections(const std::vector < std::array<float,3>> directions) {
	for (const auto& direction : directions)
		printf("(%f,%f,%f)", directions[0], directions[1], directions[2]);

}
// This will run once for every model and every raycount
TEST(Performance, CustomTriangleIntersection) {

	// Number of trials is based on number of elements here
	const vector<int> raycount = {
		1000000,
		50000,
		50000,
		50000,
		100000,
		100000,
		100000,
		100000,
		500000,
		500000,
		500000,
		500000,
		1000000,
		1000000,
		1000000,
		5000000,
		5000000,
		5000000,
		10000000,
		10000000
	};

	printf("Loading Models...\n");
	vector<ModelAndStart> models = {
		ModelAndStart({0,0,1},  "plane.obj", true),
		ModelAndStart({-4.711,1.651,-14.300},  "sibenik.obj", true),
	//	ModelAndStart({-4.711,1.651,-14.300},  "sibenik_subdivided.obj", true),
		ModelAndStart({0.007,-0.001,0.093},  "sponza.obj", true),
		ModelAndStart({0,0,1},  "energy_blob_zup.obj"),
		ModelAndStart({833.093,546.809,288.125},  "Weston_Analysis.obj"),
	//	ModelAndStart({2532.320,-19.040,45.696},  "ButchersDenFinal.obj", true),
		//ModelAndStart({0,0,1},  "zs_abandonded_mall.obj", true),
	//	ModelAndStart({0,0,1},  "zs_amsterdam.obj", true),
	//	ModelAndStart({0,0,1},  "zs_comfy.obj", true),
	//	ModelAndStart({0,0,1},  "dragon.obj", true),
	//	ModelAndStart({44.218,-39.946,15.691},  "mountain.obj", true)
	};

	const int num_trials = raycount.size();

	// Create Watches
	std::vector<StopWatch> watches(num_trials);

	printf("GeneratingDirections...\n");
	vector < vector<array<float, 3>>> directions;
	vector < vector<array<float, 3>>> origins;
	for (int rc : raycount) {
		directions.push_back(HF::ViewAnalysis::FibbonacciDistributePoints(rc, 90, 90));
	}
	//PrintDirections(directions[0]);

	vector<std::string> RowHeaders = {
		"Trial Number",
		"Model",
		"Rays",
		"Standard Hits",
		"Precise Hits",
		"Time Standard (ms)",
		"Time Precise (ms)",
		"Vertices",
		"Triangles"
	};
	// Open CSV
	std::ofstream csv_output;
	csv_output.open("PreciseVSStandardRaysOut.csv");
	WriteToCSV(csv_output, RowHeaders);
	int k = 0;
	for (auto& mas : models) {
		// Get things that wil stay constant
		auto tris = mas.triangles;
		auto verts = mas.verts;
		const auto& origin = mas.start;
		
		std::vector<std::string> output = {
			"",
			mas.model_name,
			"",
			"",
			"",
			"",
			"",
			std::to_string(mas.verts),
			std::to_string(mas.triangles)
		};
		printf("Conducting Tests for ");
		std::cout << mas.model_name << std::endl;
		for (int i = 0; i < num_trials; i++) {
			StopWatch standard_watch;
			StopWatch precise_watch;

			const auto& dirs = directions[i];
			const auto rc = dirs.size();
			// Create arrays of origins and directions
			const vector<array<float, 3>> origins(rc, origin);

			printf("Firing %i Rays... \n", rc);

			// Conduct Precise Check
			precise_watch.StartClock();
			vector<HitStruct> precise_results = mas.PreciseERT.FireAnyRayParallel(origins, dirs, -1.0f, true, false);
			precise_watch.StopClock();

			// Conduct standard check
			standard_watch.StartClock();
			vector<HitStruct> results = mas.StandardERT.FireAnyRayParallel(origins, dirs, -1.0f, false, false);
			standard_watch.StopClock();

			
			// Update output
			output[0] = std::to_string(k++);
			output[2] = std::to_string(rc);
			output[3] = std::to_string(count_hits(results));
			output[4] = std::to_string(count_hits(precise_results));
			output[5] = std::to_string(static_cast<double>(standard_watch.GetDuration()) / 1000000.0);
			output[6] = std::to_string(static_cast<double>(precise_watch.GetDuration()) / 1000000.0);

			// Write row of CSV
			WriteToCSV(csv_output, output);
		}
	}
	csv_output.close();
}
