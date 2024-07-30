#include "gdwg_graph.h"

#include <catch2/catch.hpp>

TEST_CASE("basic test") {
	auto g = gdwg::graph<int, std::string>{};
	auto n = 5;
	g.insert_node(n);
	REQUIRE(g.is_node(n) == true);
}

TEST_CASE("Insert nodes") {
	auto g = gdwg::graph<std::string, int>{};
	REQUIRE(g.insert_node("A") == true);
	REQUIRE(g.insert_node("B") == true);
	REQUIRE(g.insert_node("A") == false);
}

TEST_CASE("Insert edges") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	REQUIRE(g.insert_edge("A", "B", 5) == true);
	REQUIRE(g.insert_edge("A", "B", 10) == true);
	REQUIRE(g.insert_edge("A", "B", 5) == false);
}

TEST_CASE("Insert edge with non-existent nodes throws runtime_error") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	REQUIRE_THROWS_WITH(g.insert_edge("X", "B", 5),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_THROWS_WITH(g.insert_edge("A", "Y", 10),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_THROWS_WITH(g.insert_edge("X", "Y", 15),
	                    "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");

	REQUIRE_NOTHROW(g.insert_edge("A", "B", 5));
}

TEST_CASE("replace non-existing node") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3};

	REQUIRE_THROWS_WITH(g.replace_node(4, 5), "Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
}

TEST_CASE("replace existing node and update edges") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3};
	g.insert_edge(1, 2, "edge1");
	g.insert_edge(2, 3, "edge2");
	g.insert_edge(1, 3, "edge3");

	REQUIRE(g.replace_node(2, 4) == true);
	REQUIRE(g.is_node(2) == false);
	REQUIRE(g.is_node(4) == true);
	REQUIRE(g.is_connected(1, 4) == true);
	REQUIRE(g.is_connected(4, 3) == true);
}

TEST_CASE("replace node with existing node") {
	auto g = gdwg::graph<int, std::string>{1, 2, 3};

	REQUIRE(g.replace_node(1, 2) == false);
}

TEST_CASE("merge_replace_node: old_data or new_data does not exist") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);

	REQUIRE_THROWS_WITH(g.merge_replace_node("C", "A"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they don't exist in "
	                    "the graph");

	REQUIRE_THROWS_WITH(g.merge_replace_node("A", "C"),
	                    "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they don't exist in "
	                    "the graph");
}

TEST_CASE("merge_replace_node: duplicate edges") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C", "D"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);
	g.insert_edge("D", "B", 1);

	g.merge_replace_node("A", "D");

	REQUIRE(g.is_connected("D", "B") == true);
	REQUIRE(g.is_connected("D", "C") == true);

	auto edges = g.edges("D", "B");
	REQUIRE(edges.size() == 1);
}

TEST_CASE("merge_replace_node: no duplicate edges") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C", "D"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);
	g.insert_edge("D", "B", 2);

	g.merge_replace_node("A", "D");

	REQUIRE(g.is_connected("D", "B") == true);
	REQUIRE(g.is_connected("D", "C") == true);

	auto edges = g.edges("D", "B");
	REQUIRE(edges.size() == 2);
	REQUIRE(edges[0]->get_weight().value() == 1);
	REQUIRE(edges[1]->get_weight().value() == 2);
}

TEST_CASE("Erase a weighted edge that exists") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);

	REQUIRE(g.erase_edge("A", "B", 1) == true);
	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("Erase an unweighted edge that exists") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};
	g.insert_edge("A", "B");

	REQUIRE(g.erase_edge("A", "B") == true);
	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("Erase an unweighted edge that does not exist") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	REQUIRE(g.erase_edge("A", "B") == false);
}

TEST_CASE("Erase an edge when either src or dst node does not exist", "[graph][erase_edge]") {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("A");

	REQUIRE_THROWS_WITH(g.erase_edge("A", "B"),
	                    "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in the graph");
	REQUIRE_THROWS_WITH(g.erase_edge("B", "A"),
	                    "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in the graph");
}

TEST_CASE("erase_edge(iterator): Erase an edge using iterator") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);

	auto it = g.find("A", "B", 1);
	auto next_it = g.erase_edge(it);

	REQUIRE((*next_it).from == "A");
	REQUIRE((*next_it).to == "C");
	REQUIRE((*next_it).weight == 2);

	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("erase_edge(iterator): Erase the last edge") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);

	auto it = g.find("A", "B", 1);
	auto next_it = g.erase_edge(it);

	REQUIRE(next_it == g.end());

	REQUIRE(g.is_connected("A", "B") == false);
}

TEST_CASE("erase_edge(iterator): Erase an edge with multiple edges existing") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("B", "C", 3);
	g.insert_edge("C", "A", 4);

	auto it = g.find("B", "C", 3);
	auto next_it = g.erase_edge(it);

	REQUIRE((*next_it).from == "C");
	REQUIRE((*next_it).to == "A");
	REQUIRE((*next_it).weight == 4);

	REQUIRE(g.is_connected("B", "C") == false);
}

TEST_CASE("erase_edge(iterator): Erase an edge when no such edge exists") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);

	auto it = g.find("A", "B", 1);
	g.erase_edge(it);

	REQUIRE(g.is_connected("A", "B") == false);

	auto invalid_it = g.find("A", "B", 1);
	REQUIRE(invalid_it == g.end());
}

TEST_CASE("erase_edge(iterator, iterator) test") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C", "D"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "C", 2);
	g.insert_edge("A", "D", 3);
	g.insert_edge("B", "C", 4);
	g.insert_edge("C", "D", 5);

	auto it1 = g.find("A", "B", 1);
	auto it2 = g.find("B", "C", 4);

	REQUIRE(it1 != g.end());
	REQUIRE(it2 != g.end());

	auto next_it = g.erase_edge(it1, it2);

	REQUIRE(next_it == it2);

	auto edges = g.edges("A", "B");
	REQUIRE(edges.size() == 0);

	edges = g.edges("A", "D");
	REQUIRE(edges.size() == 0);

	edges = g.edges("B", "C");
	REQUIRE(edges.size() == 1);

	edges = g.edges("C", "D");
	REQUIRE(edges.size() == 1);
}

TEST_CASE("find edge function test: Edge not found") {
	auto g = gdwg::graph<std::string, int>{};
	REQUIRE(g.find("A", "B") == g.end());
}

TEST_CASE("find edge function test: Single unweighted edge") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B");

	auto it = g.find("A", "B");
	REQUIRE(it != g.end());
	REQUIRE((*it).from == "A");
	REQUIRE((*it).to == "B");
	REQUIRE(not(*it).weight.has_value());
}

TEST_CASE("find edge function test: Single weighted edge") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 5);

	auto it = g.find("A", "B", 5);
	REQUIRE(it != g.end());
	REQUIRE((*it).from == "A");
	REQUIRE((*it).to == "B");
	REQUIRE((*it).weight == 5);
}

TEST_CASE("find edge function test: Multiple edges") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);
	g.insert_edge("A", "B", 2);

	auto it1 = g.find("A", "B", 1);
	REQUIRE(it1 != g.end());
	REQUIRE((*it1).from == "A");
	REQUIRE((*it1).to == "B");
	REQUIRE((*it1).weight == 1);

	auto it2 = g.find("A", "B", 2);
	REQUIRE(it2 != g.end());
	REQUIRE((*it2).from == "A");
	REQUIRE((*it2).to == "B");
	REQUIRE((*it2).weight == 2);
}

TEST_CASE("connections() test: Connections from a node that exists and Connections are sorted in ascending order") {
	auto g = gdwg::graph<std::string, int>{"A", "B", "C", "D"};

	g.insert_edge("A", "D", 3);
	g.insert_edge("A", "C", 2);
	g.insert_edge("A", "B", 1);

	std::vector<std::string> expected = {"B", "C", "D"};
	auto result = g.connections("A");

	REQUIRE(result == expected);
}

TEST_CASE("connections() test: Connections from a node with no outgoing edges") {
	auto g = gdwg::graph<std::string, int>{"A", "B"};

	g.insert_edge("A", "B", 1);

	std::vector<std::string> expected = {};
	auto result = g.connections("B");

	REQUIRE(result == expected);
}

TEST_CASE("connections() test: Connections from a node that does not exist") {
	auto g = gdwg::graph<std::string, int>{"A"};

	REQUIRE_THROWS_WITH(g.connections("B"),
	                    "Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the graph");
}

TEST_CASE("Nodes function returns all stored nodes sorted in ascending order", "[graph][nodes]") {
	SECTION("Empty graph") {
		auto g = gdwg::graph<std::string, int>{};
		std::vector<std::string> expected = {};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}

	SECTION("Single node") {
		auto g = gdwg::graph<std::string, int>{"A"};
		std::vector<std::string> expected = {"A"};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}

	SECTION("Multiple nodes, sorted in ascending order") {
		auto g = gdwg::graph<std::string, int>{"D", "C", "A", "B"};
		std::vector<std::string> expected = {"A", "B", "C", "D"};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}

	SECTION("Nodes with edges") {
		auto g = gdwg::graph<std::string, int>{"C", "A", "B"};
		g.insert_edge("A", "B", 1);
		g.insert_edge("B", "C", 2);
		std::vector<std::string> expected = {"A", "B", "C"};
		auto result = g.nodes();
		REQUIRE(result == expected);
	}
}
