using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

/*!
    \brief Standard fundamental data structures for representing space used throughout HumanFactors. 

    \remarks
    The datatypes in the SpatialStructures namespace are used throughout the HumanFactors.
    For example, the GraphGenerator and VisibilityGraph both produce a Graph as output,
    allowing for the code to manage the Graph's internal CSR to be centralized in a single 
    location. 
*/
namespace HumanFactors.SpatialStructures
{
	/// \brief  Methods for aggregating edge costs per node from the graph.
	/// \see Graph.AggregateEdgeCosts for usage of this enum.
	public enum GraphEdgeAggregation
	{
		SUM = 0, ///< Add the cost of all edges
		AVERAGE = 1,///< Average the cost of all edges
		COUNT = 2 ///< Count the number of edges.
	}

    /*! \brief Node to use for calculating the cost of an edge when converting node attributes to edge costs. */
    public enum Direction
    {
        INCOMING = 0, //< Use the child node's attribute for the cost.
        OUTGOING = 1, //< Use the parent node's attribute as the cost.
        BOTH = 2 	//< Add the parent and child's attributes for the cost.
    }


	/*!
        \brief Contains names for the costs of the cost algorithms in the CostAlgrorithms namespace
        
        \details Upon running their respective algorithm, the name defined here can be used
        to index the cost type created for that graph.

        \internal
            \warning DO NOT TOUCH EXISTING NAMES. Changing these DOES NOT CHANGE THEM IN C++. Make sure any changes
            are carried over to native code.
        \endinternal
   */
    public static class CostAlgorithmNames
	{
		public static string CROSS_SLOPE = "CrossSlope"; ///< The key for the cost set generated by CostAlgorithms.GenerateAndStoreCrossSlope()
		public static string ENERGY_EXPENDITURE = "EnergyExpenditure"; ///< The key for the cost set generated by CostAlgorithms.GenerateAndStoreEnergyExpenditure()  
	}

	/*!
        \brief A graph representing connections between points in space.

        \details
        Every Node in the graph contains an X,Y,Z coordinate which can be
        used to represent a specific point in space. This graph internally
        is stored as a Sparse Matrix for space efficency. Nodes are stored
        in a hashmap containing X,Y, and Z coordinates, allowing for quick 
        indexing of specific nodes by location alone. Access to this graph's
        internal CSR is available through Graph.CompressToCSR().

        \par Cost Types
        This Graph is capable of holding multiple cost types for any of it's edges.
        Each cost type has a distinct key as it's name, such as "CrossSlope" or
        "EnergyExpenditure". Upon creation, the graph is assigned a default cost
        type, `Distance` which can be accessed explicitly by the key "Distance" or
        leaving the cost_type field blank. Alternate costs have corresponding edges 
        in the default cost set, but different costs to traverse from the parent 
        to the child node. 

        \par NodeAttributes
        The graph is able to store an arbitrary amount of information about the nodes
        it contains as strings. Similar to alternate cost types, node attributes are
        each have a distinct key as their name, but instead of conatining information
        about edges in the graph, node attributes contain information about nodes.
        Unlike the cost algorithms in edgecosts,  right now there is no functionality 
        within HumanFactors that populates the node attributes of the graph with any
        kind of metric, however the methods to add and clear node attributes are
        made available so you are free to add your own node attributes. 

        \note 
        To get the XYZ coordinates of a node from it's ID, use the ID as an index
        into the graph's nodes array returned by getNodes(); For example, if you want
        to get the node with an ID of 1 from the graph, you'd access the element at
        index 1 in the nodes array. 
        
        \invariant 1) The CSR maintained by this graph will always be valid. 
        \invariant
        2) Every unique unique node in the graph will be assigned a unique id. A Node
        is considered unique if it has an X,Y,Z location that is not within
        0.0001 units of any other node in the graph.

        \note
        The graph offers some basic functionality to add edges and nodes but it's main use
        is to provide access to the output of the GraphGenerator and VisibilityGraph. If
        adding edges or alternate cost types please make sure to read the documentation
        for these functions and that all preconditions are followed. 

        \internal
            \todo Functions to access edges in the graph like numpy. Users shouldn't have
            to use unsafe functions to get the edges of a node from the CSR.
        \endinternal

        \see CompressToCSR to get a CSR representation of the graph.
    */
	public class Graph : NativeObject
	{
		/// \brief This graph's CSR pointers.
		private CSRInfo CSRPointers;

		/// \brief Wrap a graph that already exists in unmanaged memory.
		/// <param name="GraphPtr"> Pointer to the grpah in unmanaged memory </param>
		internal Graph(IntPtr GraphPtr) : base(GraphPtr, -1) { }

		/// <summary> Construct a new empty graph in unmanaged memory. </summary>
		public Graph() : base(NativeMethods.C_CreateGraph(), -1) { }

		/*! 
            \brief Create a new edge between parent and child with cost.
            
            \param parent The X,Y,Z location for the parent node.
            \param child x,y,z location for the child 
            \param cost cost for parent to child 
            \param cost_type The type of cost to add the edge to. 


            \attention
            This overload is meant for debugging. There are many issues that can occur with
            adding integers to the graph that don't already have nodes assigned. Instead
            use the overload of this function deals with vector 3.

            \pre 1) If adding edges to a new cost type, the graph has been compressed
            by calling CompressToCSR().
            \pre 2) If adding a cost to a type that isn't the default cost type, the edge must
            already exist in the default cost type.
            \pre 3) If the graph has already been compressed and alternate costs exist, then
            both `parent` and `child` already exist in the graph.

            \post
            1) If the ID of either parent or child does not exist in the graph
            then New IDs will be assigned to them. 
            \post 2) Existing representations of the this graph's CSR will be invalidated. 
            \post 3) If `cost_type` is not blank, and does not refer to the default cost type
            or any other cost that already exists in the graph, a new cost type will be created. 
            
            \throws LogicError tried to add an alternate cost to the graph before it was compressed
            \throws InvalidCostOperation Tried to add an alternate cost that doesn't exist in the graph's
            default cost type.

            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.
            \warning 
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.

            \par Example
            \snippet spatialstructures\test_spatialstructures.cs EX_AddEdge_V3
            `39`

        */
		public void AddEdge(Vector3D parent, Vector3D child, float cost, string cost_type = "")
			=> NativeMethods.C_AddEdge(handle, parent, child, cost, cost_type);
		/*!
            \brief Create a new edge between parent and child with cost.
            
            \param parent_id The ID of the parent node.
            \param child_id The ID of the child node. 
            \param cost cost from parent to child.
            \param cost_type The type of cost to add this edge to. If left blank
            will add the edge to the graph's default cost type.

            \pre 1) If adding edges to a new cost type, the graph must first be compressed
            by calling CompressToCSR()
            \pre 2) If adding a cost to a type that isn't the default cost type, the edge must
            already exist in the default cost type.
            \pre 3) If the graph has already been compressed and alternate costs exist, then
            both `parent` and `child` already exist in the graph.

            \post
            1) If the ID of either parent or child does not exist in the graph
            then New IDs will be assigned to them. 
            \post 2) Existing representations of the this graph's CSR will be invalidated. 
            \post 3) If `cost_type` is not blank, and does not refer to the default cost type
            or any other cost that already exists in the graph, a new cost type will be created. 
            
            \throws LogicError tried to add an alternate cost to the graph before it was compressed
            \throws InvalidCostOperation Tried to add an alternate cost that doesn't exist in the graph's
            default cost type.

            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.
            \warning
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.

            \par Example
            \snippet spatialstructures\test_spatialstructures.cs EX_AddEdge_ID
            `39`
        */
		public void AddEdge(int parent_id, int child_id, float cost, string cost_type = "")
			=> NativeMethods.C_AddEdge(handle, parent_id, child_id, cost, cost_type);

		/// \brief Get an array containing the graph's current nodes.
		/// \returns An array of the graph's current nodes ordered by ID.
		/*!
            \par Example
            \snippet spatialstructures\test_spatialstructures.cs EX_GetNodes
            `[(0, 0, 2), (0, 0, 1)]`
        */
		public NodeList getNodes() => new NodeList(NativeMethods.C_GetNodes(handle));

		/*! 
            \brief Compress the graph into a CSR representation, and get pointers to it.

            \param cost_type Change the type of cost that's carried in the CSR's 
            values array. If left blank, will use the graph's default cost type. 

            \remarks 
            The CSR pointers can be mapped to after retrieved from C++ using spans, or
            can be copied out of managed memory.

            \throws KeyNotFoundException `cost_type` is not blank, the name of the graph's
            default cost type, or the name of any already defined cost type in
            the graph.

            \see CSRPtrs for more info on the CSR type and how to access it.

            \par Example
            \snippet spatialstructures\test_spatialstructures.cs EX_CompressToCSR
            `(nnz: 1, rows: 3, cols: 3)`
        */
		public CSRInfo CompressToCSR(string cost_type = "")
		{
			this.CSRPointers = NativeMethods.C_GetCSRPointers(handle, cost_type);
			return this.CSRPointers;
		}

		/*! 
            \brief Summarize the edgecosts of every node in the graph. 
            
            \param type The type of aggregation method to use.
            
            \param directed
            Whether or not the graph is directed. If set to true then each nodes's 
            score will only consider incomning edges. Otherwise, each node's score will consider
            both outgoing and incoming edges.
            
            \param cost_type The type of cost to use for aggregating the edges.
            
            \returns
            An array of scores, in which each element corresponds to a node in the graph sorted by ID.

            \pre If not left blank, cost_type must point to a valid cost in the graph. 

            \throws KeyNotFoundException Cost specified didn't match the default cost, or any other
            cost type defined in the graph.

            \warning 
            This will compress the graph if it is not compressed already. If any edges
            were added between lat call to CompressToCSR and now, then any active CSRPtrs
            will be invalidated.

            \remarks
            The order of the scores returned bythis function match the order of the scores returned from
            Graph.getNodes. This can be especially useful for summarizing the results of a VisibilityGraph.

            \par Example
            \snippet spatialstructures\test_spatialstructures.cs Example_CreateSampleGraph

            \snippet spatialstructures\test_spatialstructures.cs Example_AggregateEdgeCosts
            `[150, 20, 0]`
        */
		public ManagedFloatArray AggregateEdgeCosts(
			GraphEdgeAggregation type,
			bool directed = true,
			string cost_type = ""
		)
		{

			// Comress this to a CSR befor continuing
			this.CompressToCSR();

			// Get Results from C++
			CVectorAndData cvad = NativeMethods.C_AggregateEdgeCosts(handle, directed, type, cost_type);

			// Set the size of the return to our number of nodes.
			cvad.size = this.NumNodes();

			// Wrap these pointers in a ManagedFloatArray
			return new ManagedFloatArray(cvad);
		}


		/*!
            \brief Get the cost of traversing between `parent` and `child`

            \param parent Node that's being traversed from
            \param child Node that's being traversed to
            \param cost_type The cost type to retrieve. If blank, the graph's
            default cost will be used.

            \returns The cost of traversing from `parent` to `child`.

			\pre 1) cost_type must be the name of a cost that already exists in the graph,
			or blank. 
            \pre 2) The graph must first have been compressed using Graph.CompressToCSR().
            
            \throws LogicError The graph wasn't compressed before calling this function.
			\throws KeyNotFoundExeption The cost_type specified was not the default cost, blank, or 
			the name of any cost that currently belongs to the graph.

            \note This is not a high performance function, since each index into the CSR requires
            an indexing operation. If multiple values are required, it is suggested to iterate through
            the pointers from Graph.CompressToCSR().

            \par Example
            \snippet base\overall_examples.cs Example_GetCost
            `100`

        */
		public float GetCost(int parent, int child, string cost_type = "")
			=> NativeMethods.C_GetEdgeCost(this.Pointer, parent, child, cost_type);

		/*! 
            \brief Gets the ID of a node in the graph.
            
            \param node The X,Y,Z position of a node to get the ID for.

            \returns The ID of the node, or -1 if the node isn't in the graph.

            \par Examples
            \snippet spatialstructures\test_spatialstructures.cs  EX_GetNodeID_1
            `0`\n
            `1`\n
            \snippet spatialstructures\test_spatialstructures.cs  EX_GetNodeID_2
            `-1`
            
        */
		public int GetNodeID(Vector3D node) => NativeMethods.C_GetNodeID(handle, node.x, node.y, node.z);

		/*!
		 \brief Free the native memory managed by this class. 
		 \note the garbage collector will handle this automatically
		 \warning Do not attempt to use this class after freeing it!
		 \returns True. This is guaranteed to execute properly.  
		*/
		protected override bool ReleaseHandle()
		{
			NativeMethods.C_DestroyGraph(handle);
			return true;
		}

		/*!
            \brief  Define a node attribute for the node at id. 
            
            \param id          The ID of the node that will receive attribute.
            \param attribute   The name of the attribute to use. 
            \param score       The score for `attribute` to store for this node. 

            \par Example
            \code
                // Create a graph and add two edges to create nodes
				Graph g = new Graph();
				g.AddEdge(0, 1, 150);
				g.AddEdge(0, 2, 100);
				g.AddEdge(0, 3, 2);

				// Add node attributes to the graph for the nodes
				// we just created
				g.AddNodeAttribute(2, "Attr", "200");
				g.AddNodeAttribute(1, "Attr", "100");
				g.AddNodeAttribute(0, "Attr", "0");

				// Get scores for this attribute from the graph
				var attr = g.GetNodeAttributes("Attr");

				// Print results
				foreach (var attribute in attr)
					Debug.WriteLine(attribute);
            \endcode
            `>>> 0`\n
            `>>> 100`\n
            `>>> 200`\n
         */
		public void AddNodeAttribute(int id, string attribute, string score)
		{
			// Call the other overload with these as lists.
			AddNodeAttribute(attribute, new int[] { id }, new string[] { score });
		}

		/*! 
            \brief  Add attribute to all node in ids, with their respective score in scores

            \param  ids         IDs of nodes to assign scores to for `attribute`
            \param  attribute   Name of the attribute to assigns cores to for each node in `ids`
            \param  scores      Ordered ids of scores to add to the node at the id in `ids` at the same index

            \pre the length of `ids` and `scores` must match

            \par Example
            \code
			    // Create a graph and add two edges to create nodes
				Graph g = new Graph();
				g.AddEdge(0, 1, 150);
				g.AddEdge(0, 2, 100);

				// Create arrays for ids and scores
				int[] ids = { 0, 1, 2 };
				string[] scores = { "0", "100", "200" };

				// Add them to the graph
				g.AddNodeAttribute("Attr", ids, scores);

				// Get scores for this attribute from the graph
				var attr = g.GetNodeAttributes("Attr");

				foreach (var attribute in attr)
					Debug.WriteLine(attribute);
            \endcode

            `>>> 0`\n
            `>>> 100`\n
            `>>> 200`\n
        */
		public void AddNodeAttribute(string attribute, int[] ids, string[] scores)
		{
			// Ensure we're keeping our precondition. The length of these
			// must match for proper behavior
			if (ids.Count() != scores.Count())
				throw new ArgumentException("The length of scores" + ids.Count().ToString() + "did not match" +
					"the length of scores " + scores.Count().ToString());

			// Call into native code
			SpatialStructures.NativeMethods.C_AddNodeAttributes(
				this.Pointer,
				attribute,
				ids,
				scores
			); ;
		}
    
        /*  
            \brief Convert input array to strings
            \param in_array Array of objects to convert to strings
            \returns String representation of all elements in in_array
        */
        private string[] MakeStringArray<T>(T[] in_array) where T : IFormattable => in_array.Select(x => x.ToString()).ToArray();


        /*! 
            \brief  Add attribute to all node in ids, with their respective score in scores

            \param  attribute   Name of the attribute to assign scores to for each node in the graph
            \param  scores      Ordered ids of scores to add to the node at the id in `ids` at the same index

            \pre the length of scores must equal the number of nodes in the graph.

            \throws ArgumentException The number of scores in scores isn't equal to the nubmer of nodes in the graph

        */
        public void AddNodeAttribute<T>(string attribute, IEnumerable<T> scores) where T : IFormattable
        {
            // If the length of this array is less than the number of nodes in the graph, throw
            int num_nodes = this.NumNodes();
            if (scores.Count() < num_nodes)
                throw new ArgumentException("Didn't provide a score for every node in the graph");
            
            // Create an ID array and convert all the input scores to strings. 
            AddNodeAttribute(attribute, Enumerable.Range(0, num_nodes).ToArray(), MakeStringArray(scores.ToArray()));
        }

        /*! 
            \brief Get the score of every node for a given attribute.

            \param attribute The unique name of the attribute type to get from the graph fopr every node

            \returns 
            If an attribute with the name of `attribute`, type was found in the graph, then an array of scores
            for each node is returned in order of ID. For example, the score of the node with id 10 would be stored
            at index 10, id 12 stored at index 12, etc. Nodes without scores for `attribute` will have empty
            strings at their indexes. 
            
            \returns
            If `attribute` didn't exist in the graph, then an empty array of strings will be returned. 

            \par Example
            \snippet spatialstructures\test_spatialstructures.cs Example_CreateSampleGraph
            \snippet spatialstructures\test_spatialstructures.cs EX_AddNodeAttribute
            `0, 100, 200,`
            \snippet spatialstructures\test_spatialstructures.cs EX_AddNodeAttribute_2
            `0, 100, 200, ,`

        */
        public string[] GetNodeAttributes(string attribute)
			=> NativeMethods.C_GetNodeAttributes(Pointer, attribute, this.NumNodes());

		/*!
            \brief Clear an attribute and all of its scores from the graph.

            \param  attribute The unique key of the attribute to clear from the graph.
            
            \par Example
            \code
				// Create a graph and add two edges to create nodes
				Graph g = new Graph();
				g.AddEdge(0, 1, 150);
				g.AddEdge(0, 2, 100);

				// Create arrays for ids and scores
				int[] ids = { 0, 1, 2 };
				string[] scores = { "0", "100", "200" };

				// Add them to the graph
				g.AddNodeAttribute("Attr", ids, scores);

				// Now try to delete
				g.ClearNodeAttributes("Attr");

				// check that this is truly gone
				var node_attrs = g.GetNodeAttributes("Attr");
                Debug.WriteLine(node_attrs.size());
            \endcode
            `>>> 0`
            
         */
		public void ClearNodeAttributes(string attribute) => NativeMethods.C_ClearAttributeType(this.Pointer, attribute);

		/*!
            \brief Get the number of nodes in this graph
            \returns The number of currently defined nodes in this graph

            \remarks
            This is used multiple times internally to get the size of the graph without
            needing to get its nodes.

            \par Example
            \snippet spatialstructures\test_spatialstructures.cs Example_CreateSampleGraph
            \snippet spatialstructures\test_spatialstructures.cs EX_NumNodes
            `3`
        */
		public int NumNodes() => NativeMethods.C_GetGraphSize(this.Pointer);

        /*! 
			\brief Generate a cost set based on a set of node parameters

			\param attribute_key Attribute to create a new cost set from.
			\param cost_key Key for the newly generated cost set. 
			\param dir Direction to use for calculating the cost of any edge. For example
					   INCOMING will use the cost of the node being traveled to by the edge. 
            
            \throws KeyNotFoundException The parameter assinged by parameter_name is not
                                         the key of any node parameter in the graph. 

            \pre An attribute of `attribute_key` already exists in the graph
            \post A new cost set will be created in the graph with a key of `cost_key`.

            \par Example
            \snippet spatialstructures\test_spatialstructures.cs EX_ConvertAttributes
            `0->1 = 100, 0->2 = 200`
        */
        public void AttrsToCosts(string attribute_key, string cost_key, Direction dir) =>
            NativeMethods.C_AttrsToCosts(this.Pointer, attribute_key, cost_key, dir);
	}


	/*! 
        \brief Several cost algorithms that generate alternate costs for edges in a Graph.
        
        \details
        All algorithms in this class behave in a similar manner. They all accept a graph, 
        perform some calculation then store their results as an alternate set of costs
        in the graph. The names used as keys to access each specific cost in the
        graph are available in the static CostAlgorithmNames class.
        
        \see Graph for more information about how alternate costs can be used. 
    */
	public static class CostAlgorithms
	{
		/*!
            \brief  Calculate the cross slope for every edge in a graph and store it as
            a new cost type within the graph.

            \param g The graph of nodes and edges to calculate this score for. 

            \post The results of the cross slope between every edge in `g` is stored
            within g as an alternate cost accessible with CostAlgorithmNames.CrossSlope.

            \code
                // Create the graph
                Graph g = new Graph();

                // Create 7 nodes
                Vector3D n0 = new Vector3D(2, 6, 6);
                Vector3D n1 = new Vector3D(0, 0, 0);
                Vector3D n2 = new Vector3D(-5, 5, 4);
                Vector3D n3 = new Vector3D(-1, 1, 1);
                Vector3D n4 = new Vector3D(2, 2, 2);
                Vector3D n5 = new Vector3D(5, 3, 2);
                Vector3D n6 = new Vector3D(-2, -5, 1);

                // Add 9 edges
                g.AddEdge(n0, n1, 0); // [ -2, -6, -6 ]
                g.AddEdge(n1, n2, 0); // [ -5,  5,  4 ]
                g.AddEdge(n1, n3, 0); // [ -1,  1,  1 ]
                g.AddEdge(n1, n4, 0); // [  2,  2,  2 ]
                g.AddEdge(n2, n4, 0); // [ -9, -3, -2 ]
                g.AddEdge(n3, n5, 0); // [ -6,  2,  1 ]
                g.AddEdge(n5, n6, 0); // [ -7, -8, -1 ]
                g.AddEdge(n4, n6, 0); // [ -6, -7, -1 ]

                // Compress the graph after adding edges
                g.CompressToCSR();

                // Calculate and store edge type in g: cross slope
                CostAlgorithms.CalculateAndStoreCrossSlope(g);
            \endcode
        */
		public static void CalculateAndStoreCrossSlope(Graph g)
			=> NativeMethods.C_CalculateAndStoreCrossSlope(g.Pointer);

		/*!
            \brief  Calculate the energy expenditure for every edge in a graph and store it as
            a new cost type within the graph.

            \param g The graph of nodes and edges to calculate this score for. 

            \post The results of the cross slope between every edge in `g` is stored
            within g as an alternate cost accessible with CostAlgorithmNames.EnergyExpenditure

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

                // Calculate and store edge type in g: energy expenditure
		        CalculateAndStoreEnergyExpenditure(&g);
            \endcode
         */
		public static void CalculateAndStoreEnergyExpenditure(Graph g)
			=> NativeMethods.C_CalculateAndStoreEnergyExpenditure(g.Pointer);

        /*! \brief 
        */
	}
}