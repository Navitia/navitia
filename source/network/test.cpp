#include "network.h"
#include "boost/property_map/property_map.hpp"
#include <boost/graph/graphviz.hpp>

using namespace network;

struct label_name {
    NW g;
    navitia::type::Data &data;
    map_tc_t map_tc;

    label_name(NW &g, navitia::type::Data &data, map_tc_t map_tc): g(g), data(data), map_tc(map_tc){}

    void operator()(std::ostream& oss,vertex_t v) {
        oss << "[label=\"";
        switch(get_n_type(v, data)) {
        case SA : oss << "SA " << get_idx(v, data, map_tc); break;
        case SP : oss << "SP " << get_idx(v, data, map_tc); break;
        case RP : oss << "RP " << get_idx(v, data, map_tc); break;
        case TA : oss << "TA " << get_idx(v, data, map_tc) << " " << get_time(v, data, g, map_tc); break;
        case TD : oss << "TD " << get_idx(v, data, map_tc) << " "<< get_time(v, data, g, map_tc); break;
        case TC : oss << "TC " << get_idx(v, data, map_tc) << " "<< get_time(v, data, g, map_tc); break;
        }
        oss << " " << v << " " << get_saidx(v, data, g, map_tc) << "\"]";
    }

};






int main(int , char** argv) {
    navitia::type::Data data;
    std::cout << "Debut chargement des données ... " << std::flush;
    data.load_flz("/home/vlara/navitia/jeu/IdF/IdF.nav");
    std::cout << " Fin chargement des données" << std::endl;


    std::cout << "Création et chargement du graph ..." << std::flush;

    NW g(data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size() + data.pt_data.stop_times.size() * 2);
    map_tc_t map_tc;
    charger_graph(data, g, map_tc);
    std::cout << " Fin de création et chargement du graph" << std::endl;
    std::cout << "Nombre d'arêtes : " << num_edges(g) << std::endl;

    vertex_t v1, v2;
    v1 = atoi(argv[1]);
    v2 = atoi(argv[2]);
    std::vector<vertex_t> predecessors(boost::num_vertices(g));
    std::vector<etiquette> distances(boost::num_vertices(g));
    etiquette etdebut;
    etdebut.temps = 0;
    etdebut.date_arrivee = data.pt_data.validity_patterns.at(0).slide(boost::gregorian::from_undelimited_string(argv[4]));
    etdebut.heure_arrivee = atoi(argv[3]);


    std::cout  << "Recherche du chemin entre : " << data.pt_data.stop_areas.at(v1).name << " et " << data.pt_data.stop_areas.at(v2).name << std::endl;

    boost::typed_identity_property_map<edge_t> identity;

    boost::posix_time::ptime start, end;



    start = boost::posix_time::microsec_clock::local_time();
    try {
        boost::dijkstra_shortest_paths(g, v1,
                                       boost::predecessor_map(&predecessors[0])
                                       .weight_map(identity)
                                       .distance_map(&distances[0])
                                       .distance_combine(combine(data, g, map_tc, v1, v2, etdebut.date_arrivee))
                                       .distance_zero(etdebut)
                                       .distance_inf(etiquette::max())
                                       .distance_compare(edge_less2())
                                       .visitor(dijkstra_goal_visitor(v2, data, g, map_tc))
                                       );
    } catch(found_goal fg) { v2 = fg.v; }
    end = boost::posix_time::microsec_clock::local_time();

    if(predecessors[v2] == v2) {
        std::cout << "Pas de chemin trouve" << std::endl;
    } else {
        std::cout << "Chemin trouve en " << (end-start).total_milliseconds() << std::endl;

        for(vertex_t v = v2; (v!=v1); v = predecessors[v]) {
            if(get_n_type(v, data) == TD || get_n_type(v, data) == TA) {
                std::cout << data.pt_data.stop_areas.at(data.pt_data.stop_points.at(data.pt_data.route_points.at(data.pt_data.stop_times.at(get_idx(v, data, map_tc)).route_point_idx).stop_point_idx).stop_area_idx).name << " ";
                std::cout << data.pt_data.stop_points.at(data.pt_data.route_points.at(data.pt_data.stop_times.at(get_idx(v, data, map_tc)).route_point_idx).stop_point_idx).stop_area_idx;
                std::cout << " " << data.pt_data.stop_times.at(get_idx(v, data, map_tc)).arrival_time;
                std::cout << " " << data.pt_data.lines.at(data.pt_data.routes.at(data.pt_data.route_points.at(data.pt_data.stop_times.at(get_idx(v, data, map_tc)).route_point_idx).route_idx).line_idx).name;
                std::cout << " t : " << distances[v].temps << " d : " << distances[v].date_arrivee << " " << data.pt_data.route_points.at(data.pt_data.stop_times.at(get_idx(v, data, map_tc)).route_point_idx).route_idx << std::endl;
            }
        }
    }


    //    label_name ln(g, data, map_tc);
    //    std::ofstream f("test.z");
    //    write_graphviz(f, g, ln);



    return 0;
}
