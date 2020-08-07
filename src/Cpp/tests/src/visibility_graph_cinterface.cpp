/*!
	\file		visibility_graph_cinterface.cpp
	\brief		Unit test source file for testing the functionality of visibility_graph_C

	\author		Gem Aludino
	\date		07 Aug 2020
*/

#include "gtest/gtest.h"
#include "objloader_C.h"
#include "spatialstructures_C.h"
#include "visibility_graph_C.h"

namespace CInterfaceTests {
	TEST(_visibility_graph_cinterface, CreateVisibilityGraphAllToAll) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// Size of obj file string (character count)
		const int obj_length = static_cast<int>(obj_path_str.size());

		// This will point to memory on free store.
		// The memory will be allocated inside the LoadOBJ function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;

		// Load mesh
		// The array rot will rotate the mesh 90 degrees with respect to the x-axis,
		// i.e. makes the mesh 'z-up'.
		//
		// Notice that we pass the address of the loaded_obj pointer
		// to LoadOBJ. We do not want to pass loaded_obj by value, but by address --
		// so that we can dereference it and assign it to the address of (pointer to)
		// the free store memory allocated within LoadOBJ.
		const float rot[] = { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_setup]
		// The model is a flat plane, so only nodes 0, 2 should connect.

		// Every three floats should represent a single (x, y, z) point.
		const float points[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -10.0f, 0.0f, 2.0f, 0.0f };

		// This should be the array element count within points.
		const int points_size = 9;

		// Represents how far to offset nodes from the ground.
		const float height = 1.7f;

		// Total amount of nodes to use in the visibility graph.
		const int points_count = points_size / 3;
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_setup]
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_call]
		// Declare a pointer to Graph.
		//
		// Notice that we pass the address of VG to CreateVisibilityAllToAll.
		// CreateVisibilityAllToAll will assign the deferenced address to a pointer that points
		// to memory on the free store. We will call DestroyGraph to release the memory resources later on.
		HF::SpatialStructures::Graph* VG = nullptr;

		status = CreateVisibilityGraphAllToAll(bvh, points, points_count, &VG, height);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateVisibilityGraphAllToAll, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_call]
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_compress]
		// Always compress the graph after generating a graph/adding new edges
		status = Compress(VG);

		if (status != 1) {
			// Error!
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_compress]
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_destroy]
		//
		// VG, the visibility graph, is now ready for use.
		//

		// destroy VG (visibility graph)
		status = DestroyGraph(VG);

		if (status != 1) {
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_destroy]
		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}

	TEST(_visibility_graph_cinterface, CreateVisibilityGraphAllToAllUndirected) {
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected]
		// TODO snippet
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected]
	}

	TEST(_visibility_graph_cinterface, CreateVisibilityGraphGroupToGroup) {
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup]
		// TODO snippet
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup]
	}
}
