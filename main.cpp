#include <vector>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <map>
/*
* code style. members will be m_Var
* functions will be snake case
* globals will be CAPS
* local vars will be camel case
* types will be capatalized
*/

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
constexpr int INF = std::numeric_limits<int>::max();


struct Edge {
	int to;
	int weight;
};

std::vector<int> Dijkstra(std::vector<std::vector<Edge>> graph, int source) {
	// distance[node] = shortest distance from start to that node so far
	// set all variable names to something that makes sense... wtf is u#
	// comment each line

	int n = graph.size();
	std::vector<bool> visited(n, 0);
	std::vector<int> distance(n, INF); // set rest to inf
	distance[source] = 0; // start -> start = 0
	
	for (int i = 0; i < n; ++i)
	{
		// 1. Find the closest unvisited node.
		int u = -1;

		for (int j = 0; j < n; ++j)
		{
			if (!visited[j] &&
				(u == -1 || distance[j] < distance[u]))
			{
				u = j;
			}
		}

		// No more reachable nodes.
		if (u == -1 || distance[u] == INF)
			break;

		visited[u] = true;

		// 2. Try improving its neighbours.
		for (const Edge& edge : graph[u])
		{
			int v = edge.to;
			int weight = edge.weight;

			if (!visited[v] &&
				weight < INF - distance[u] &&
				distance[u] + weight < distance[v])
			{
				distance[v] = distance[u] + weight;
			}
		}
	}

	return distance;
}


struct WeightedGraph {
	// each vector is a vertex, and contains each connection with its weight
	// .first = dest, .second = weight of connection to dest
	std::vector<std::vector<std::pair<int, int>>> graph;

	WeightedGraph(std::size_t capacity) : graph(capacity) {};

		
	void Insert(int src_node,int dest_node, int weight) {
		graph[src_node].push_back({ dest_node,weight });
	};

	std::vector<std::pair<int, int>> Neighbours(int src_node) {
		return graph[src_node];
	}

	void PrintGraph() {
		for (int i = 0; i < graph.capacity(); i++) {
			std::cout << "connections from : "  << i << "\n";
			for (const auto& edge : graph[i]) {
				std::cout << i << "--> " << edge.first << " weight : " << edge.second << "\n";
			}
		}
	}
};


struct Graph {
	// each vector is a vertex, and contains each connection with its weight
	// .first = dest, .second = weight of connection to dest
	std::vector<std::vector<int>> graph;
	bool directed;
	Graph(std::size_t capacity, bool isDirected = true) : graph(capacity), directed(isDirected) {};


	void Insert(int src_node, int dest_node) {
		graph[src_node].push_back(dest_node);
		if (!directed) {
			graph[dest_node].push_back(src_node);
		}
	};

	std::vector<int> Neighbours(int src_node) const {
		return graph[src_node];
	}

	void PrintGraph() {
		for (int i = 0; i < graph.size(); i++) {
			std::cout << "connections from : " << i << "\n";
			for (const auto& edge : graph[i]) {
				std::cout << i << "--> " << edge << "\n";
			}
		}
	}
};


#include <queue>


// for unweighted
auto BFS(Graph graph, int src_node) {

	// visited bool array
	std::vector<bool> visited(graph.graph.capacity());
	// min-heap
	std::queue<int> queue;
	queue.push(src_node);
	visited[src_node] = true;

	// start from the src, expand out equal distance at a time..

	while (!queue.empty()) {
		int vertex = queue.front(); queue.pop();
		if (!visited[vertex]) {
			//visit(vertex)
			visited[vertex] = true;
			std::vector<int> neighbours = graph.Neighbours(vertex);
			for (const auto& neighbour : neighbours) {
				if (!visited[neighbour]) {
					queue.push(neighbour);
				}
			}
		}
	}
}

struct VisitedNode {
	int node;
	int parent;
};
std::vector<int> BuildPath(std::vector<int>& parents, int dest) {
	std::vector<int> path;
	path.push_back(dest);
	while (parents[dest] != -1) {
		path.push_back(parents[dest]);
		dest = parents[dest];
	}
	std::reverse(path.begin(), path.end());
	return path;
}
namespace Pathfinder {
	std::vector<int> BFS(const Graph& graph, int src_node, int target_node) {

		std::vector<bool> visited(graph.graph.size());
		std::vector<int> parents(graph.graph.size(), -1);
		std::queue<int> queue;
		queue.push(src_node);
		visited[src_node] = true;
		

		// start from the src, expand out equal distance at a time..
		
		while (!queue.empty()) {
			int vertex = queue.front(); queue.pop();
			if (vertex == target_node) {
				return BuildPath(parents,vertex);
			}
			
				//visit(vertex)
				//visited[vertex] = true;
				for (int neighbour : graph.Neighbours(vertex)) {
					
					if (!visited[neighbour]) {
						visited[neighbour] = true;
						queue.push(neighbour);
						parents[neighbour] = vertex;
						
					}
				}
			
		}
		return {-1};
	}
}


int main() {

	Graph graph(12, true); 

	// Path 1: cost 5
	graph.Insert(0, 4);
	graph.Insert(4, 2);
	graph.Insert(2, 9);
	graph.Insert(9, 6);
	graph.Insert(6, 7);

	// Path 2: cost 7
	graph.Insert(0, 3);
	graph.Insert(3, 8);
	graph.Insert(8, 1);
	graph.Insert(1, 10);
	graph.Insert(10, 5);
	graph.Insert(5, 11);
	graph.Insert(11, 7);


	auto p = Pathfinder::BFS(graph,0, 7);


	for (auto a : p) {
		std::cout << a;
	}
}
