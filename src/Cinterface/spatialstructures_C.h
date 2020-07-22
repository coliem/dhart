#include <cinterface_utils.h>
#include <vector>
#include <array>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures {
		class Graph;
		struct Subgraph;
		enum class COST_AGGREGATE : int;
		struct Node; // Careful with these forward declares!
		struct Edge; // Spent like 2 hours reading mangled names because I accidentally
	}				 // defined these as classes (V) instead of structs (U)
}

C_INTERFACE ExampleFloat(float* out_float);

/**
* @defgroup SpatialStructures
* Contains the Graph and Node datatypes.
* @{
*/

/// <summary> Get a vector of every node in the given graph pointer. </summary>
/// <param name="graph"> Graph to retrieve nodes from. </param>
/// <param name="out_vector_ptr"> Output parameter for the new vector. </param>
/// <param name="out_data_ptr"> Output parameter for the vector's data. </param>
/// <returns>
/// HF_STATUS::INVALID_PTR if the given pointer was invalid. HF::GENERIC_ERROR if the graph is not
/// valid. HF::OK if successful.
/// </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		auto out_vec = new std::vector<HF::SpatialStructures::Node>;
		HF::SpatialStructures::Node* out_data = nullptr;

		GetAllNodesFromGraph(g, &out_vec, &out_data);

		DestroyGraph(g);
	\endcode
*/
C_INTERFACE GetAllNodesFromGraph(
	const HF::SpatialStructures::Graph* graph,
	std::vector<HF::SpatialStructures::Node>** out_vector_ptr,
	HF::SpatialStructures::Node** out_data_ptr
);

// DOn't document this because it's not ready yet.
C_INTERFACE GetEdgesForNode(
	const HF::SpatialStructures::Graph* graph,
	const HF::SpatialStructures::Node* Node,
	std::vector<HF::SpatialStructures::Edge>** out_vector_ptr,
	HF::SpatialStructures::Edge** out_edge_list_ptr,
	int* out_edge_list_size
);

/// <summary> Get the size of a node vector. </summary>
/// <param name="node_list"> Node vector to get the size from. </param>
/// <param name="out_size"> Size of the vector will be written to this int. </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		std::vector<HF::SpatialStructures::Node> node_vec{ n0, n1, n2, n3 };

		int node_vec_size = -1;
		GetSizeOfNodeVector(&node_vec, &node_vec_size);

		DestroyNodes(node_vec);
	\endcode
*/
C_INTERFACE GetSizeOfNodeVector(
	const std::vector<HF::SpatialStructures::Node>* node_list,
	int* out_size
);

// This is never used. Don't include.
C_INTERFACE GetSizeOfEdgeVector(
	const std::vector<HF::SpatialStructures::Edge>* edge_list,
	int* out_size
);

/// <summary> Get an ordered array of costs for each node aggregatted by the desired method. </summary>
/// <param name="graph"> Graph to aggregare edges from. </param>
/// <param name="agg"> Aggregation type to use. </param>
/// <param name="directed">
/// If true, only count outgoing edges for a node, otherwise count both outgoing and incoming edges.
/// </param>
/// <param name="out_vector_ptr"> Output parameter for the vector. </param>
/// <param name="out_vector_ptr"> Output parameter for the vector's held data. </param>
/// <returns> HF_STATUS::OK if successful. If the graph wasn't valid HF_STATUS::NO_GRAPH. </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		std::vector<float>* out_vector = nullptr;
		float* out_data = nullptr;

		int aggregation_type = 0;
		AggregateCosts(g, aggregation_type, false, &out_vector, &out_data);

		DestroyGraph(g);
	\endcode
*/
C_INTERFACE AggregateCosts(
	const HF::SpatialStructures::Graph* graph,
	int agg,
	bool directed,
	std::vector<float>** out_vector_ptr,
	float** out_data_ptr
);

/// <summary> Create a new empty graph. </summary>
/// <param name="nodes"> Unused. </param>
/// <param name="num_nodes"> Unused. </param>
/// <param name="out_graph"> Output parameter to store the graph in. </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		// use Graph

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE CreateGraph(
	const float* nodes,
	int num_nodes,
	HF::SpatialStructures::Graph** out_graph
);

/// <summary>
/// Add an edge between parent and child. If parent or child doesn't already exist in the graph,
/// they will be added and automatically assigned new IDS.
/// </summary>
/// <param name="graph"> Graph to add the new edge to. </param>
/// <param name="parent">
/// A 3 element float array containing the x, y, and z coordinate of the parent node.
/// </param>
/// <param name="child">
/// A 3 element float array containing the x, y, and z coordinate of the child node,
/// </param>
/// <param name="score"> The cost from parent to child </param>
/// <returns>
/// HF_STATUS::OK on success. HF_STATUS::INVALID_PTR on an invalidparent or child node.
/// </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		AddEdgeFromNodes(g, n0, n1, distance);

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE AddEdgeFromNodes(
	HF::SpatialStructures::Graph* graph,
	const float* parent,
	const float* child,
	float score
);

/// <summary>
/// Create a new edge between parent_id and child_id. If these IDs don't yet exist in the graph they
/// will be added.
/// </summary>
/// <param name="graph"> Graph to create the new edge in. </param>
/// <param name="parent"> The parent's id in the graph. </param>
/// <param name="child"> The child's id in the graph. </param>
/// <param name="score"> The cost from parent to child. </param>
/// <param name="returns"> HF_STATUS::OK on completion. </param>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}
		const int id0 = 0;
		const int id1 = 1;
		const float distance = 3;

		AddEdgeFromNodeIDs(g, id0, id1, distance);

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE AddEdgeFromNodeIDs(
	HF::SpatialStructures::Graph* graph,
	int parent_id,
	int child_id,
	float score
);

/// <summary>
/// Retrieve all information for a graph's CSR representation. This will compress the graph if it
/// was not yet already compressed.
/// </summary>
/// <param name="out_nnz"> Number of non-zeros contained within the graph. </param>
/// <param name="out_num_rows"> Number of rows contained within the graph. </param>
/// <param name="out_num_rows"> Number of columns contained within the graph. </param>
/// <param name="out_data_ptr"> Pointer to the graph's data array. </param>
/// <param name="out_inner_indices_ptr"> Pointer to the graph's inner indices array. </param>
/// <param name="out_inner_indices_ptr"> Pointer to the graph's outer indices array. </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		Compress(g);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		// Retrieve the CSR from the graph
		CSRPtrs csr;
		GetCSRPointers(g, &csr.nnz, &csr.rows, &csr.cols, &csr.data, &csr.inner_indices, &csr.outer_indices);

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE GetCSRPointers(
	HF::SpatialStructures::Graph* graph,
	int* out_nnz,
	int* out_num_rows,
	int* out_num_cols,
	float** out_data_ptr,
	int** out_inner_indices_ptr,
	int** out_outer_indices_ptr
);

/// <summary>
/// Get the id of the given node in the graph. If the node does not exist, <paramref name="out_id"
/// /> will be set to -1.
/// </summary>
/// <param name="graph"> The graph to get the ID from. </param>
/// <param name="point">
/// A 3 float array containing the x,y, and z coordinates of the point to get the ID for.
/// </param>
/// <param name="out_id">
/// Output parameter for the id. Set to -1 if that point couldn't be found in <paramref name="graph" />
/// </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		AddEdgeFromNodes(g, n0, n1, distance);

		float point[] = { 0, 1, 2 };
		int result_id = -1;

		GetNodeID(g, point, &result_id);

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE GetNodeID(
	HF::SpatialStructures::Graph* graph,
	const float* point,
	int* out_id
);

/// <summary> Compress the given graph into a csr representation. </summary>
/// <returns> HF_STATUS::OK on completion. </returns>
/// <remarks>
/// This will reduce the memory footprint of the graph, and invalidate all existing CSR
/// representations of it. If the graph is already compressed, this will be a no-op.
/// </remarks>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		Compress(g);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE Compress(
	HF::SpatialStructures::Graph* graph
);

/// <summary> Clear the nodes/edges for the given graph. </summary>
/// <param name="graph"> Graph to clear nodes from. </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		AddEdgeFromNodes(g, n0, n1, distance);

		ClearGraph(g);

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE ClearGraph(
	HF::SpatialStructures::Graph* graph
);

/// <summary> Delete the vector of nodes at the given pointer. </summary>
/// <param name="nodelist_to_destroy"> Vector of nodes to destroy. </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		auto node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

		// Use node_vec

		DestroyNodes(node_vec);
	\endcode
*/
C_INTERFACE DestroyNodes(
	std::vector<HF::SpatialStructures::Node>* nodelist_to_destroy
);

/// <summary> Delete the vector of edges at the given pointer. </summary>
/// <param name="edgelist_to_destroy"> Vector of nodes to destroy. </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		HF::SpatialStructures::Edge e0(n1); // parent is n0
		HF::SpatialStructures::Edge e1(n3); // parent is n2

		auto edge_vec = new std::vector<HF::SpatialStructures::Edge>{ e0, e1 };

		// Use edge_vec

		DestroyEdges(edge_vec);
	\endcode
*/
C_INTERFACE DestroyEdges(
	std::vector<HF::SpatialStructures::Edge>* edgelist_to_destroy
);

/// <summary> Delete a graph. </summary>
/// <param name="graph_to_destroy"> Graph to delete. </param>
/// <returns> HF_STATUS::OK on completion. </returns>

/*!
	\code
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		// use Graph

		// Release memory for g after use
		DestroyGraph(g);
	\endcode
*/
C_INTERFACE DestroyGraph(
	HF::SpatialStructures::Graph* graph_to_destroy
);

/*!
	\summary Calculates cross slope for all subgraphs in *g
	\param g The address of a Graph
	\returns HF::STATUS::OK on completion

	\code
		// Requires #include "graph.h"

		// Create 7 nodes
		Node n0(0, 0, 0);
		Node n1(1, 3, 5);
		Node n2(3, -1, 2);
		Node n3(1, 2, 1);
		Node n4(4, 5, 7);
		Node n5(5, 3, 2);
		Node n6(-2, -5, 1);

		Graph g;

		// Adding 9 edges
		g.addEdge(n0, n1);
		g.addEdge(n1, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n5, n6);
		g.addEdge(n4, n6);

		// Always compress the graph after adding edges!
		g.Compress();

		// Within CalculateAndStoreCrossSlope,
		// std::vector<std::vector<IntEdge>> CostAlgorithms::CalculateAndStoreCrossSlope(Graph& g)
		// will be called, along with a call to the member function
		// void Graph::AddEdges(std::vector<std::vector<IntEdge>>& edges).
		CalculateAndStoreCrossSlope(&g);
	\endcode
*/
C_INTERFACE CalculateAndStoreCrossSlope(HF::SpatialStructures::Graph* g);

/*!
	\summary Calculates energy expenditure for all subgraphs in *g
	\param g The address of a Graph
	\returns HF::STATUS::OK on completion

	\code
		// Requires #include "graph.h"

		// Create 7 nodes
		Node n0(0, 0, 0);
		Node n1(0, 0, 1);
		Node n2(5, 5, 4);
		Node n3(2, 2, 2);
		Node n4(5, 3, 2);
		Node n5(6, 6, 7);
		Node n6(2, 5, 1);

		Graph g;

		// Adding 8 edges
		g.addEdge(n0, n1);
		g.addEdge(n1, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n3, n5);
		g.addEdge(n4, n2);
		g.addEdge(n6, n4);
		g.addEdge(n6, n5);

		// Always compress the graph after adding edges!
		g.Compress();

		// Within CalculateAndStoreEnergyExpenditure,
		// std::vector<std::vector<EdgeSet>> CostAlgorithms::CalculateAndStoreEnergyExpenditure(Graph& g)
		// will be called, along with a call to the member function
		// void Graph::AddEdges(std::vector<std::vector<EdgeSet>>& edges).
		CalculateAndStoreEnergyExpenditure(&g);
	\endcode
*/
C_INTERFACE CalculateAndStoreEnergyExpenditure(HF::SpatialStructures::Graph* g);

/// <summary> Add a new node attribute in the graph for the node at id.
/// Returns an error code if the specified node doesn't exist in the graph.
/// score and ids both point to arrays allocated by the caller which should be
/// able to be passed to AddNodeAttributes.</summary>
/// <param name="g">The graph that will receive attributes</param>
/// <param name="ids">Array of int, allocated by caller, represents node IDs</param>
/// <param name="attribute">The attribute to add to the graph</param>
/// <param name="scores">Array of (char *), allocated by caller</param>
/// <param name="num_nodes">Block count of scores</param>
/// <returns> HF_STATUS::OK on completion </returns>
/*!
	\code
		
	\endcode
*/
C_INTERFACE AddNodeAttributes(HF::SpatialStructures::Graph* g, const int* ids, const char* attribute, const char** scores, int num_nodes);

/// <summary> Attribute should be the name of the attribute to get from the graph.
/// Memory shall be allocated in *out_scores to hold the char arrays.
/// out_scores is a pointer to an array of pointers to char arrays, which will be allocated by the caller.
/// The caller must call DeleteScores to deallocate these node scores.
/// out_score_size should be updated to the length of the score array.</summary>
/// <param name="g">The graph that will be used to retrieve attributes</param>
/// <param name="attribute">The attribute to retrieve from g</param>
/// <param name="out_scores">Pointer to array of pointers to char arrays, allocated by the caller</param>
/// <param name="out_score_size">Keeps track of the length of the out_scores buffer, updated as needed</param>
/// <returns> HF_STATUS::OK on completion </returns>
/*!
	\code

	\endcode
*/
C_INTERFACE GetNodeAttributes(const HF::SpatialStructures::Graph* g, const char* attribute, char*** out_scores, int* out_score_size);

/// <summary> Free the memory of every char array in scores_to_delete.
/// The length of each contained char array should be determined by using strlen on the given array
/// unless they are not delimited.</summary>
/// <param name="scores_to_delete">Pointer to array of pointers to char arrays, allocated by caller</param>
/// <param name="num_char_arrays">Block count of scores_to_delete</param>
/// <returns> HF_STATUS::OK on completion </returns>

/*!
	\code

	\endcode
*/
C_INTERFACE DeleteScoreArray(char*** scores_to_delete, int num_char_arrays);

/// <summary> Deletes the contents of that attribute type in the graph </summary>
/// <param name="g"> The graph from which attributes of type s will be delete</param>
/// <param name="s"> The attribute to be cleared from within g</param>
/// <returns> HF_STATUS::OK on completion </returns>

/*!
	\code

	\endcode
*/
C_INTERFACE ClearAttributeType(HF::SpatialStructures::Graph* g, const char* s);

/**@}*/
/*!
	\summary Calculates cross slope for all subgraphs in *g
	\param g The address of a Graph
	\returns HF::STATUS::OK on completion

	\code
		// Requires #include "graph.h"

		// Create 7 nodes
		Node n0(0, 0, 0);
		Node n1(1, 3, 5);
		Node n2(3, -1, 2);
		Node n3(1, 2, 1);
		Node n4(4, 5, 7);
		Node n5(5, 3, 2);
		Node n6(-2, -5, 1);

		Graph g;

		// Adding 9 edges
		g.addEdge(n0, n1);
		g.addEdge(n1, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n5, n6);
		g.addEdge(n4, n6);

		// Always compress the graph after adding edges!
		g.Compress();

		// Within CalculateAndStoreCrossSlope,
		// std::vector<std::vector<IntEdge>> CostAlgorithms::CalculateAndStoreCrossSlope(Graph& g)
		// will be called, along with a call to the member function
		// void Graph::AddEdges(std::vector<std::vector<IntEdge>>& edges).
		CalculateAndStoreCrossSlope(&g);
	\endcode
*/
C_INTERFACE CalculateAndStoreCrossSlope(HF::SpatialStructures::Graph* g);
