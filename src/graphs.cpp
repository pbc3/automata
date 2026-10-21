#include <stack>
#include <queue>
#include <iostream>

struct WeightedGraph {
	// each vector is a vertex, and contains each connection with its weight
	// .first = dest, .second = weight of connection to dest
	std::vector<std::vector<std::pair<int, int>>> graph;

	WeightedGraph(std::size_t capacity) : graph(capacity) {};


	void Insert(int src_node, int dest_node, int weight) {
		graph[src_node].push_back({ dest_node,weight });
	};

	std::vector<std::pair<int, int>> Neighbours(int src_node) {
		return graph[src_node];
	}

	void PrintGraph() {
		for (int i = 0; i < graph.capacity(); i++) {
			std::cout << "connections from : " << i << "\n";
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
	int size;
	bool directed;
	Graph(std::size_t capacity, bool isDirected = true) : graph(capacity), directed(isDirected), size(capacity){};


	void Insert(int src_node, int dest_node) {
		graph[src_node].push_back(dest_node);
		if (!directed) {
			graph[dest_node].push_back(src_node);
		}
		size++;
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
	std::vector<int> BFS(const Graph& graph, int src_node, int target_node);
	std::vector<int> DFS(const Graph& graph, int src_node, int target_node);
	std::vector<int> Dijkstra(const WeightedGraph& graph, int src_node, int target_node);
}

//std::vector<int> Pathfinder::Dijkstra(const WeightedGraph& graph, int src_node, int target_node) {
//	// Minheap<distance_from_src,node> - > automatically keeps track of the shortest path
//
//}

std::vector<int> Pathfinder::BFS(const Graph& graph, int src_node, int target_node) {

	std::vector<bool> visited(graph.graph.size());
	std::vector<int> parents(graph.graph.size(), -1);
	std::queue<int> queue;
	queue.push(src_node);
	visited[src_node] = true;


	while (!queue.empty()) {
		int vertex = queue.front(); queue.pop();
		if (vertex == target_node) {
			return BuildPath(parents, vertex);
		}

		for (int neighbour : graph.Neighbours(vertex)) {
			if (!visited[neighbour]) {
				visited[neighbour] = true;
				queue.push(neighbour);
				parents[neighbour] = vertex;

			}
		}
	}
	return { -1 };
};


//void tree_search(Graph& graph, int start_node, int goal, strategy) {
//	std::queue<int> fringe;
//	fringe.push(start_node);
//
//	while (fringe != empty) {
//		int chosen_node = fringe[some strategy to choose a node, i.e. this could be a minheap]
//		if(graph[chosen_node] == goal){return build_path(graph, chosen_node)}
//		else {
//			for (int neighbour : graph.neighbours(chosen_node)) {
//				fringe.push(neighbour)
//			}
//		}
//	}
//	return failure
//}

std::vector<int> build_path(std::vector<int> parents, int node) {
	std::vector<int> path;
	while (node != -1) {
		path.push_back(node);
		node = parents[node];
	}
	std::reverse(path.begin(), path.end());
	return path;
}
std::vector<int> bfs(Graph& graph, int start_node, int goal) {

	std::queue<int> fringe;
	std::vector<int> visited(graph.size, false);
	std::vector<int> parents(graph.size, -1);
	fringe.push(start_node);
	visited[start_node] = true;

	while (!fringe.empty()) {
		int chosen_node = fringe.front(); fringe.pop();
			if (chosen_node == goal) { return build_path(parents, chosen_node) }
			else {
				for (int neighbour : graph.Neighbours(chosen_node)) {
					if (!visited[neighbour]) {
					visited[neighbour] = true;
					parents[neighbour] = chosen_node;
					fringe.push(neighbour);
				}
			}
		}
	}
	return { -1 }
}
std::vector<int> Pathfinder::DFS(const Graph& graph, int src_node, int target_node) {
	
	std::stack<int> fringe;
	std::vector<int> visited(graph.size, false);
	std::vector<int> parents(graph.size, -1);
	fringe.push(src_node);
	visited[src_node] = true;

	while (!fringe.empty()) {
		int chosen_node = fringe.top(); fringe.pop();
		if (chosen_node == target_node) { return build_path(parents, chosen_node) }
		else {
			for (int neighbour : graph.Neighbours(chosen_node)) {
				if (!visited[neighbour]) {
					visited[neighbour] = true;
					parents[neighbour] = chosen_node;
					fringe.push(neighbour);
				}
			}
		}
	}
	return { -1 }
}

std::vector<int> greedy();
std::vector<int> cfs();

std::vector<int> astar(WeightedGraph& graph,int start_node, int target_node) {
	std::priority_queue<
		std::pair<int, int>, // type of entry
		std::vector<std::pair<int, int>>, // container storing the entries
		std::greater<std::pair<int, int>> // min heap
	> fringe;
	
	std::vector<int> parents(graph.size, -1);
	std::unordered_map<int, int> cost_so_far;

	fringe.push({ herusitic(start_node,target_node),start_node});
	parents[start_node] = -1;
	cost_so_far[start_node] = 0;

	while (!fringe.empty()) {
		auto [priority, node] = fringe.top(); fringe.pop();

		if (priority != cost_so_far[node] + heuristic(node, target_node)) {
			continue; // there can be >1 pairs with the same node, but different costs. this confirms its still the best one, else its skipped
		}

		if (node == target_node) {
			return build_path(parents, node);
		}

		for (auto [edge_cost, candidate] : graph.Neighbours(node)) {
			int new_cost = edge_cost + cost_so_far.at(node); // g(n);

			auto it = cost_so_far.find(candidate);
			if (it == cost_so_far.end() || new_cost < it->second) {
				cost_so_far[candidate] = new_cost;
				parents[candidate] = node;
				int new_priority = new_cost + herustic(candidate, target_node); // h(n)
				fringe.push({ new_priority,candidate });
			}
		}
	}
	return { -1 };
}
//int main() {
//

//}
