﻿using DHARTAPI.Exceptions;
using DHARTAPI.NativeUtils;
using DHARTAPI.NativeUtils.CommonNativeArrays;
using DHARTAPI.SpatialStructures;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;

/*! 
    \brief Calculate the shortest path between points in a Graph.

    \see ShortestPath for a list of pathfinding functions.
    \see Path for information on the fundamental path datatype. 

    \see SpatialStructures.Graph for information about the graph itself
    \see GraphGenerator to automatically generate a graph of accessible space on a mesh.

*/
namespace DHARTAPI.Pathfinding
{

	/*!
        \brief Functions for finding the shortest path between two nodes in a graph.

        \remarks
        The shortest path functions will use Dijkstra's algorithm to find the shortest path between
        one or more nodes in a Graph. Since the graph can hold multiple costs for each contained edge,
        the type of cost used in this calculation can be specified for each function. This allows for
        paths to be generated using costs generated by the CostAlgorithms in SpatialStructures such as
        cross slope, or energy expenditure.

        \see ShortestPath for generating a single path between two nodes.
        \see DijkstraShortestPath for generating multiple paths at once. 
        \see SpatialStructures.Graph for more information on how it stores multiple costs.

        \internal
        \note There are several functions that check the output of a C++ path function then insert
              null values where paths couldn't be generated into an output array, and create C# path
              wrappers for paths that could be generated. this could be consolidated in a single
              function, instead of scattered throughout the code. 
        \endinternal

    */
	public static class ShortestPath
	{

		/*!
            \brief Perform Dijkstra's shortest path algorithm to find a path between two nodes.
            
            \param graph The graph to conduct the search on.
            \param start_id The ID of the node to start at.
            \param end_id The ID of the node to find a path to. 
            \param cost_type The type of cost to use for generating the path. If left blank, will use
            the cost that the graph was created with. In the case of the graph generator, the default
            cost is distance. 

            \returns The shortest path from start_node to end_node or null if no path could be found.
            
            \pre 1) `start_id` and `end_id` must be the X,Y,Z position of nodes that already exist in `graph`.
            \pre 2) If `cost_type` is not left as the default, then it must be the name of a valid cost already
            defined in `graph`.

            \throws KeyNotFoundException `cost_type` wasn't left as blank, and didn't
            refer to the name of any cost that already exists in the graph.

            \see DijkstraShortestPathMulti for efficently generating multiple paths in parallel.
            
            \par Example
            \snippet base\overall_examples.cs EX_PathFinding_Graph
            \snippet base\overall_examples.cs EX_Pathfinding_Setup
            \snippet base\overall_examples.cs EX_Pathfinding_IDS
            \snippet base\overall_examples.cs EX_Pathfinding_Print

            ```
            [(1, 1.415028), (12, 1.417536), (26, 1.417887), (39, 1.418485), (50, 1.000265), (63, 1.000128), (80, 1.000098), (105, 0)]
            [(1, 4.559175), (12, 5.759251), (26, 5.889585), (39, 6.100943), (50, 2.978094), (63, 2.826927), (80, 2.784634), (105, 0)]
            ```
        */
		public static Path DijkstraShortestPath(Graph graph, int start_id, int end_id, string cost_type = "")
		{
			// Get CVectorAndData from native code
			CVectorAndData cvad = NativeMethods.C_CreatePath(graph.Pointer, start_id, end_id, cost_type);

			// If the path is invalid, that means that no path could be found between start and end
			// so return a null value. 
			if (!cvad.IsValid())
				return null;
			// Otherwise construct a new path with this CVectorAndData
			else
				return new Path(cvad);
		}

		/*!
            \brief Perform Dijkstra's shortest path algorithm to find a path between two nodes.
            
            \param graph The graph to conduct the search on.
            \param start_node The X,Y,Z of a node in the graph node to start at.
            \param end_node The X,Y,Z of a node in the graph node to end at.
            \param cost_type The type of cost to use for generating the path. If left blank, will use
            the cost that the graph was created with. In the case of the graph generator, the default
            cost is distance. 
            
            \returns The shortest path from start_node to end_node or null if no path could be found.
            
            \pre 1) `start_node` and `end_node` must be the X,Y,Z position of nodes that already exist in `graph`.
            \pre 2) If `cost_type` is not left as the default, then it must be the name of a valid cost already
            defined in `graph`.

            \remarks Gets the start id and end of both nodes, then calls the ID overload. 
            
            \throws KeyNotFoundException `cost_type` wasn't left as blank, and didn't
            refer to the name of any cost that already exists in the graph.

            \see DijkstraShortestPathMulti for efficently generating multiple paths in parallel.

            \par Example
            \snippet base\overall_examples.cs EX_PathFinding_Graph
            \snippet base\overall_examples.cs EX_Pathfinding_Setup
            \snippet base\overall_examples.cs EX_Pathfinding_Nodes
            \snippet base\overall_examples.cs EX_Pathfinding_Print

            ```
            [(1, 1.415028), (12, 1.417536), (26, 1.417887), (39, 1.418485), (50, 1.000265), (63, 1.000128), (80, 1.000098), (105, 0)]
            [(1, 4.559175), (12, 5.759251), (26, 5.889585), (39, 6.100943), (50, 2.978094), (63, 2.826927), (80, 2.784634), (105, 0)]
            ```
        */
		public static Path DijkstraShortestPath(Graph graph, Vector3D start_node, Vector3D end_node, string cost_type = "")
		{
			// Get the parent and child of the node with this ID
			int parent_id = graph.GetNodeID(start_node);
			int child_id = graph.GetNodeID(end_node);

			// Call the other overload for node IDs
			return DijkstraShortestPath(graph, parent_id, child_id, cost_type);
		}

		/*! 
            \brief Find the shortest paths between each pair of start_id and end_id in order. 
            
            \param graph The graph to generate paths in.
            \param start_ids Ids for the start points to generate paths from. 
            \param end_ids Ids for the end points to generate paths to
            \param cost_type The type of cost to use for generating the path. If left blank, will use
            the cost that the graph was created with. In the case of the graph generator, the default
            cost is distance. 

            \returns
            A list of paths in order from start_ids to end_ids. If a path could not be generated by a set of points,
            then the path at that location will be null. 
            
            \pre 1) The length of start_ids must match the length of end_ids.
            \pre 2) Every ID in `start_ids` and `end_ids` must be the ID of some node in `graph`. 
            \pre 3) If `cost_type` is not left as the default, then it must be the name of a valid cost already
            defined in `graph`.

            \details Uses all available cores for parallel calculation. 

            \pre The length of start_ids must match the length of end_ids.

            \throws System.ArgumentException Length of start_ids didn't equal length of end_ids
            \throws KeyNotFoundException `cost_type` wasn't left as blank, and didn't
            refer to the name of any cost that already exists in the graph.
            
            \par Example
            \snippet base\overall_examples.cs EX_PathFinding_Graph
            \snippet base\overall_examples.cs EX_MultiPathFinding

            ```
            1 to 101 Energy  : [(1, 2.461), (11, 2.5), (24, 2.5), (36, 4.491), (47, 5.402), (60, 5.302), (77, 5.129), (101, 0)]
			1 to 101 Distance: [(1, 1), (11, 1), (24, 1), (36, 1.415), (47, 1.417), (60, 1.416), (77, 1.416), (101, 0)]
			2 to 102 Energy  : [(2, 2.5), (1, 2.461), (11, 2.5), (24, 4.536), (37, 5.528), (48, 5.452), (61, 5.605), (78, 5.837), (102, 0)]
			2 to 102 Distance: [(2, 1), (1, 1), (11, 1), (24, 1.415), (37, 1.417), (48, 1.417), (61, 1.417), (78, 1.418), (102, 0)]
			3 to 103 Energy  : [(3, 2.52), (2, 2.5), (1, 4.559), (12, 2.48), (25, 5.708), (38, 5.656), (49, 5.916), (62, 6.644), (79, 5.08), (103, 0)]
			3 to 103 Distance: [(3, 1), (2, 1), (1, 1.415), (12, 1), (25, 1.417), (38, 1.417), (49, 1.418), (62, 1.42), (79, 1.416), (103, 0)]
			4 to 104 Energy  : [(4, 2.48), (12, 5.759), (26, 5.89), (39, 6.101), (50, 7.008), (64, 5.863), (83, 3.827), (104, 0)]
			4 to 104 Distance: [(4, 1), (12, 1.418), (26, 1.418), (39, 1.418), (50, 1.421), (64, 1.418), (83, 1.002), (104, 0)]
            ```
        */
		public static Path[] DijkstraShortestPathMulti(
			Graph graph,
			int[] start_ids,
			int[] end_ids,
			string cost_type = ""
		)
		{
			// If our precondition fails, throw
			if (start_ids.Length != end_ids.Length)
				throw new ArgumentException("Length of start_ids didn't equal length of end_ids");

			// Call the native function to generate cvectors and data
			CVectorAndData[] cvads = NativeMethods.C_CreatePaths(graph.Pointer, start_ids, end_ids, cost_type);

			// Create an array of paths to hold all paths created by the call to native code.
			Path[] paths = new Path[start_ids.Length];

			// Iterate through each CVectorAndData in the returned array
			int size = start_ids.Length;
			for (int i = 0; i < size; i++)
			{

				// If this path is valid, then jsut 
				if (cvads[i].IsValid())
					paths[i] = new Path(cvads[i]);

				// If this CVectorAndData isn't valid that indicates a path
				// couldn't be created between the start and end node at this 
				// position, insert a null element at this index.
				else
					paths[i] = null;
			}

			return paths;
		}

		/*! 
            \brief Find the shortest paths between each pair of start_id and end_id in order. 
            
            \param graph The graph to generate paths in.
            \param start_nodes Locations of the start points to generate paths from.
            \param end_nodes Locations of the end nodes to generate paths to.
            \param cost_type The type of cost to use for generating the path. If left blank, will use
            the cost that the graph was created with. In the case of the graph generator, the default
            cost is distance. 

            \returns
            A list of paths in order from `start_ids` to `end_ids`. If a path could not be generated by a set of points,
            then the path at that location will be null. 

            \details Determines the IDs of nodes, then calls the other overload. Uses all available cores for parallel calculation. 

            \pre 1) The length of `start_ids` must match the length of `end_ids`.
            \pre 2) Each node in `start_nodes` and end_nodes must contain the x,y,z position of an existing node in `graph`
            \pre 3) If `cost_type` is not left as the default, then it must be the name of a valid cost already
            defined in `graph`.

            \throws System.ArgumentException Length of `start_ids` didn't equal length of `end_ids`
            \throws KeyNotFoundException `cost_type` wasn't left as blank, and didn't
                     refer to the name of any cost that already exists in `graph`.

            \par Example
            \snippet base\overall_examples.cs EX_PathFinding_Graph
            \snippet base\overall_examples.cs EX_MultiPathFinding_Nodes

            ```
			(-30, 0, 1.068) to (-27, -8, 1.295) Energy  : [(0, 2.48), (4, 2.48), (12, 2.48), (25, 2.461), (37, 2.461), (47, 5.402), (60, 5.302), (77, 5.129), (101, 0)]
			(-30, 0, 1.068) to (-27, -8, 1.295) Distance: [(0, 1), (4, 1), (12, 1), (25, 1), (37, 1), (47, 1.417), (60, 1.416), (77, 1.416), (101, 0)]
			(-31, -1, 1.018) to (-26, -8, 1.427) Energy  : [(1, 2.461), (11, 2.5), (24, 4.536), (37, 5.528), (48, 5.452), (61, 5.605), (78, 5.837), (102, 0)]
			(-31, -1, 1.018) to (-26, -8, 1.427) Distance: [(1, 1), (11, 1), (24, 1.415), (37, 1.417), (48, 1.417), (61, 1.417), (78, 1.418), (102, 0)]
			(-31, 0, 1.018) to (-25, -8, 1.556) Energy  : [(2, 2.5), (1, 4.559), (12, 2.48), (25, 5.708), (38, 5.656), (49, 5.916), (62, 6.644), (79, 5.08), (103, 0)]
			(-31, 0, 1.018) to (-25, -8, 1.556) Distance: [(2, 1), (1, 1.415), (12, 1), (25, 1.417), (38, 1.417), (49, 1.418), (62, 1.42), (79, 1.416), (103, 0)]
			(-31, 1, 1.017) to (-25, -6, 1.678) Energy  : [(3, 2.52), (2, 2.5), (1, 4.559), (12, 5.759), (26, 5.89), (39, 6.101), (50, 7.008), (64, 5.863), (83, 3.827), (104, 0)]
			(-31, 1, 1.017) to (-25, -6, 1.678) Distance: [(3, 1), (2, 1), (1, 1.415), (12, 1.418), (26, 1.418), (39, 1.418), (50, 1.421), (64, 1.418), (83, 1.002), (104, 0)]
           ```
        */
		public static Path[] DijkstraShortestPathMulti(
			Graph graph,
			IEnumerable<Vector3D> start_nodes,
			IEnumerable<Vector3D> end_nodes,
			string cost_type = ""
		)
		{
			// If the size of start and end nodes differ, then return false
			if (start_nodes.Count() != end_nodes.Count())
				throw new ArgumentException("Length of start_nodes didn't equal length of end_nodes");

			// Create ID arrays
			int size = start_nodes.Count();
			int[] start_ids = new int[size];
			int[] end_ids = new int[size];

			// Get the IDS of every start and end node using a foreach loop here. 
			// Note: that foreach is slightly faster than int loops due to .net optimizations. This
			// may not matter much for this function, but it's an example
			// of how some procedures can be sped up. 

			/*! 
                \internal
                \note The process of finding IDS may fit better in the graph itself through an overload on 
                      Get ID instead of being implemented here. 
                \endinternal
            */
			int i = 0;
			foreach (var start_end in start_nodes.Zip(end_nodes, (start, end) => new Tuple<Vector3D, Vector3D>(start, end)))
			{
				// Insert these elements into start_ids and end_ids at index i
				start_ids[i] = graph.GetNodeID(start_end.Item1);
				end_ids[i] = graph.GetNodeID(start_end.Item2);
				i++;
			}

			// Return the results of the ID overload.
			return DijkstraShortestPathMulti(graph, start_ids, end_ids, cost_type);
		}


        /*! 
            \brief Generate a path from every node in the graph to every other node in a graph.
        
            \param g The graph to generate paths in. 
            \param cost_type Type of cost to use for path generation. If left blank will use the
                              default cost of the graph
             
            \returns 
            An array of paths with a length equal to the number of nodes in `g` squared. Paths will
            be returned in order starting with all paths from node 0, then all paths from node 1, etc.
            If a path could not be generated between a set of nodes, then path at that index will be null.

            \pre If `cost_type` is not left as the default, then it must be the name of a valid cost already
            defined in `graph`.

            \throws KeyNotFoundException `cost_type` wasn't left as blank, and didn't
                     refer to the name of any cost that already exists in `graph`.

            \par Example
            \snippet base\overall_examples.cs EX_Pathfinding_AllToAll

            ```
            0 -> 1 : [(0, 10), (1, 0)]
			0 -> 2 : [(0, 10), (1, 15), (2, 0)]
			0 -> 3 : [(0, 30), (3, 0)]
			1 -> 0 : [None]
			1 -> 2 : [(1, 15), (2, 0)]
			1 -> 3 : [(1, 15), (2, 5), (3, 0)]
			2 -> 0 : [None]
			2 -> 1 : [(2, 5), (3, 15), (1, 0)]
			2 -> 3 : [(2, 5), (3, 0)]
			3 -> 0 : [None]
			3 -> 1 : [(3, 15), (1, 0)]
			3 -> 2 : [(3, 15), (1, 15), (2, 0)]
           ```
        */
        public static Path[] DijkstraAllToAll(Graph g, string cost_type = "")
        {
            // Generate paths in native code
            var cvads = NativeMethods.C_AllToAllPaths(g.Pointer, g.NumNodes(), cost_type);

            // Create an array of paths and only copy over the paths that have a length
            // greater than one.
            Path[] paths = new Path[cvads.Length];
            for (int i = 0; i < cvads.Length; i++)
            {

                if (cvads[i].IsValid())
                    paths[i] = new Path(cvads[i]);

                // If the validity check fails, then no path 
                // could be found between these nodes and we must
                // set the path in to null in our output
                else
                    paths[i] = null;
            }
            return paths;
        }

		/*! \brief Calculate Predecessor and Distance Matricies for a graph 
         
            \param g Graph to calculate predeessor and distance matricies for
            \param out_dist Output parameter for the distance matrix
            \param out_predecessor Output parameter for the predecessor matrix.
            \param cost_type Type of cost to use for the distance and predecessor
                             matricies. If left blank will use the default cost of 
                             the graph.

            \throws KeyNotFoundException If cost_type isn't left to the default, and
                                         does not match the key of any cost that already
                                         exists in the graph.
        

            \post out_dist and out_predecessor are updated to contain the distance and predecessor
            matricies for `g`.

            \internal
            \note
            This SHOULD by all means be returning a tuple, as it would allow for 
            returning multiple parameters without needing to resort to ou tparameters
            however, that support isn't actually built into the language, but instead
            is accessible through a nuget package from microsoft like Span. This is 
            not enough of a case to add another DLL that we need to ship with, so 
            i'm doing output parameters.

            \endinternal

            \par Example
            \snippet pathfinding\test_pathfinding.cs EX_CreateDistPred
            `[0, 15, 10, NaN, 0, NaN, NaN, 5, 0]`\n
            `[0, 2, 0, -1, 1, -1, -1, 2, 2]`
        */
		public static void GeneratePredecessorAndDistanceMatricies(
			Graph g,
			out UnmanagedFloatArray2D out_dist,
			out UnmanagedIntArray2D out_predecessor,
			string cost_type = ""
		)
		{
			// Define output parameters
			CVectorAndData predecessor_ptrs = new CVectorAndData();
			CVectorAndData distance_ptrs = new CVectorAndData();

			// Set both both sets of pointers' sizes to the correct sizes
			int num_nodes = g.NumNodes();
			predecessor_ptrs.size = num_nodes; distance_ptrs.size = num_nodes;
			predecessor_ptrs.size2 = num_nodes; distance_ptrs.size2 = num_nodes;

			// Call NativeMethods. This will update the output parameters
			// for distance and predecessor 
			NativeMethods.C_GeneratePredecessorAndDistanceMatricies(
				g.Pointer,
				ref distance_ptrs,
				ref predecessor_ptrs,
				cost_type
			);

			// Update output parameters 
			out_dist = new UnmanagedFloatArray2D(distance_ptrs);
			out_predecessor = new UnmanagedIntArray2D(predecessor_ptrs);
		}
	}

}