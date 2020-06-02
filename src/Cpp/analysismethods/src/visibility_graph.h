#include <vector>

// Forward Declares
namespace HF {
	namespace SpatialStructures {
		class Graph;
		struct Node;
		struct Edge;
	}
	namespace RayTracer {
		class EmbreeRayTracer;
	}
}

namespace HF {
	namespace AnalysisMethods {
		namespace VisibilityGraph {
			/// <summary>
			/// All variations for generating the visibility graph 
			/// </summary>
			

			///<summary>
			/// Generate a visibility between all nodes in input nodes
			///</summary>
			HF::SpatialStructures::Graph AllToAll(
				HF::RayTracer::EmbreeRayTracer& ert,
				const std::vector<HF::SpatialStructures::Node> & input_nodes,
				float height = 1.7f
			);

			///<summary>
			/// Generate a visibility graph between every node in set a and every node in set b
			///</summary> 
			HF::SpatialStructures::Graph GroupToGroup(
				HF::RayTracer::EmbreeRayTracer& ert,
				const std::vector<HF::SpatialStructures::Node> & from,
				const std::vector<HF::SpatialStructures::Node> & to,
				float height = 1.7f
			);

			///<summary>
			/// Parallel AllToAll Algorithm for an undirected visibility graph
			/// </summary>
			HF::SpatialStructures::Graph AllToAllUndirected(
				HF::RayTracer::EmbreeRayTracer& ert,
				const std::vector<HF::SpatialStructures::Node>& nodes,
				float height,
				int cores = -1
			);
		}
	}
}