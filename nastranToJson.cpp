#include "nastranToJson.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "json.hpp"
#include "pystring.h"

template<typename T>
std::string nastranToJson::convertVectorToDelimitedString(std::vector<T> &inputString) {
    std::string result;
    result.append("{");
    for (auto iter: inputString) {
        result.append(boost::lexical_cast<std::string>(iter));
        result.append(",");
    }
    result.pop_back();
    result.append("}");
    return result;
}

template<typename T>
std::string nastranToJson::convertDequeToDelimitedString(std::deque<T> &inputString) {
    std::string result;
    result.append("{");
    for (auto iter: inputString) {
        result.append(boost::lexical_cast<std::string>(iter));
        result.append(",");
    }
    result.pop_back();
    result.append("}");
    return result;
}

void nastranToJson::saveJson(const std::string _json_file_name, unsigned int _file_mode) {
    json_file_name = _json_file_name;
    json_output_mode = _file_mode;
    convertNastranToJson();
}

void nastranToJson::convertNastranToJson() {
    std::fstream newfile;
    newfile.open(nastran_file_name, std::ios::in);
    nlohmann::json feature_json;
    if (newfile.is_open()) {
        std::string tp;
        unsigned int line_count = 0;
        int first_blseg = -1;
        int first_pshell = -1;
        int first_vertex = -1;
        int first_triangle = -1;
        std::string vertex_master_string;
        std::string blseg_master_string;
        std::string triangle_master_string;
        while (getline(newfile, tp)) {
            if (boost::starts_with(tp, "BLSEG")) {
                if (first_blseg == -1) {
                    if (line_count < first_blseg) {
                        first_blseg = line_count;
                    }
                }
            }
            if (boost::starts_with(tp, "GRID*")) {
                if (first_vertex == -1) {
                    if (line_count < first_vertex) {
                        first_vertex = line_count;
                    }
                }
            }
            if (boost::starts_with(tp, "CTRIA3")) {
                if (first_triangle == -1) {
                    if (line_count < first_triangle) {
                        first_triangle = line_count;
                    }
                }
            }
            if (boost::starts_with(tp, "PSHELL")) {
                if (first_pshell == -1) {
                    if (line_count < first_pshell) {
                        first_pshell = line_count;
                    }
                }
            }
            if ((first_vertex > 0) && (first_triangle < 0)) {
                vertex_master_string.append(tp.c_str());
            }
            if( (first_triangle > 0) && (first_blseg < 0) ) {
                triangle_master_string.append(tp.c_str());
            }
            if ((first_blseg > 0) && (first_pshell < 0)) {
                blseg_master_string.append(tp.c_str());
            }
            line_count = line_count + 1;
        }
        // Vertex Collection Begins Here
        std::vector<std::string> vertex_strings;
        pystring::split(vertex_master_string, vertex_strings, "GRID*");
        std::map<unsigned int, std::deque<std::string> > vertex_id_to_vertex_string_map;
        for (auto vString: vertex_strings) {
            std::string current_vertex_string = vString;
            boost::trim(current_vertex_string);
            boost::trim_if(current_vertex_string, boost::is_any_of("*"));
            if (current_vertex_string.size() > 0) {
                std::vector<std::string> single_vertex_split;
                boost::split(single_vertex_split, current_vertex_string, boost::is_any_of(" "));
                std::deque<std::string> new_split_strings;
                for (auto iter: single_vertex_split) {
                    std::string current_token = iter;
                    boost::trim(current_token);
                    boost::trim_if(current_token, boost::is_any_of("*"));
                    if (current_token.size() > 0) {
                        new_split_strings.push_back(current_token);
                    }
                }
                unsigned int vertex_id = boost::lexical_cast<unsigned int>(new_split_strings.at(0));
                new_split_strings.pop_front();
                vertex_id_to_vertex_string_map[vertex_id] = new_split_strings;
            }
        }
        std::map<unsigned int, std::deque<double> > cleaned_up_vertex_data;
        for (auto iter: vertex_id_to_vertex_string_map) {
            std::deque<std::string> current_vertex_uncleaned = iter.second;
            std::deque<double> cleaned_vertex_data;
            if (current_vertex_uncleaned.size() == 3) {
                for (auto vIter: current_vertex_uncleaned) {
                    cleaned_vertex_data.push_back(boost::lexical_cast<double>(vIter));
                }
            } else {
                std::string v1 = current_vertex_uncleaned.at(0).substr(0, 16);
                std::string v2 = current_vertex_uncleaned.at(0).substr(16, current_vertex_uncleaned.size() - 17);
                std::string v3 = current_vertex_uncleaned.at(1);
                cleaned_vertex_data.push_back(boost::lexical_cast<double>(v1));
                cleaned_vertex_data.push_back(boost::lexical_cast<double>(v2));
                cleaned_vertex_data.push_back(boost::lexical_cast<double>(v3));
            }
            cleaned_up_vertex_data[iter.first] = cleaned_vertex_data;
        }
        nlohmann::json vertexCollection = nlohmann::json::array();
        for (auto iter: cleaned_up_vertex_data) {
            nlohmann::json currentJVertex = nlohmann::json::object();
            currentJVertex["id"] = iter.first;
            currentJVertex["x"] = iter.second[0];
            currentJVertex["y"] = iter.second[1];
            currentJVertex["z"] = iter.second[2];
            vertexCollection.push_back(currentJVertex);
        }
        feature_json["vertices"] = vertexCollection;
        // Vertex Collection Ends Here
        // Triangle Collection Begins Here
        std::vector<std::string> triangle_strings;
        pystring::split(triangle_master_string, triangle_strings, "CTRIA3");
        nlohmann::json triangleCollection = nlohmann::json::array();
        for(auto tString: triangle_strings) {
            std::string current_triangle_string = tString;
            boost::trim(current_triangle_string);
            std::vector< std::string > current_tria_split;
            boost::split(current_tria_split, current_triangle_string, boost::is_any_of(" "));
            std::deque< std::string > cleaned_up_triangles;
            if(current_triangle_string.size() == 0)
                continue;
            for(auto iter: current_tria_split) {
                std::string current_triangle_token = iter;
                boost::trim(current_triangle_token);
                if(current_triangle_token.size() > 0) {
                    cleaned_up_triangles.push_back(current_triangle_token);
                }
            }
            if( cleaned_up_triangles.size() == 5)  {
                nlohmann::json currentJTriangle = nlohmann::json::object();
                currentJTriangle["id"] = boost::lexical_cast<unsigned int>(cleaned_up_triangles.at(0));
                currentJTriangle["v1"] = boost::lexical_cast<unsigned int>(cleaned_up_triangles.at(2));
                currentJTriangle["v2"] = boost::lexical_cast<unsigned int>(cleaned_up_triangles.at(3));
                currentJTriangle["v3"] = boost::lexical_cast<unsigned int>(cleaned_up_triangles.at(4));
                triangleCollection.push_back(currentJTriangle);
            }
        }
        feature_json["triangles"] = triangleCollection;
        // Triangle Collection Ends Here
        // Feature Line Collection Begins Here
        std::vector<std::string> feature_curve_strings;
        pystring::split(blseg_master_string, feature_curve_strings, "BLSEG");
        std::map<unsigned int, std::deque<unsigned int> > feature_curve_map;
        unsigned int feature_curve = 0;
        for (auto fCurveString: feature_curve_strings) {
            std::string current_feature_curve_string = fCurveString;
            boost::trim(current_feature_curve_string);
            if (current_feature_curve_string.size() == 0) {
                feature_curve = feature_curve - 1;
            } else {
                std::deque<unsigned int> current_feature_curve;
                std::vector<std::string> split_strings;
                boost::split(split_strings, current_feature_curve_string, boost::is_any_of(" "));
                for (auto fSecondary: split_strings) {
                    std::string current_secondary = fSecondary;
                    boost::trim(current_secondary);
                    if (current_secondary.size() > 0) {
                        current_feature_curve.push_back(boost::lexical_cast<unsigned int>(current_secondary));
                    }
                }
                if (current_feature_curve.size() > 0) {
                    unsigned int nastran_feature_id = current_feature_curve[0];
                    current_feature_curve.pop_front();
                    feature_curve_map[nastran_feature_id] = current_feature_curve;
                }
            }
            feature_curve = feature_curve + 1;
        }
        nlohmann::json featureCurveCollection = nlohmann::json::array();
        for (auto iter: feature_curve_map) {
            nlohmann::json currentJCurve = nlohmann::json::object();
            currentJCurve["id"] = iter.first;
            nlohmann::json featJIds = nlohmann::json::array();
            for (auto fIter: iter.second) {
                featJIds.push_back(fIter);
            }
            currentJCurve["chain"] = featJIds;
            featureCurveCollection.push_back(currentJCurve);
        }
        newfile.close();
        feature_json["feature_curves"] = featureCurveCollection;
        if(json_output_mode == 1) {
            std::vector<std::uint8_t> v_bson = nlohmann::json::to_bson(feature_json);
            std::ofstream mesh_output(json_file_name, std::ios::out | std::ios::binary);
            mesh_output.write((char *) &v_bson[0], v_bson.size() * sizeof(std::uint8_t));
            mesh_output.close();
        }
        if(json_output_mode == 2) {
            std::string final_json_string = feature_json.dump(4);
            std::ofstream mesh_output;
            mesh_output.open(json_file_name);
            mesh_output << final_json_string;
            mesh_output.close();
        }
    } else {
        std::cout << "Unable to open input nastran file" << std::endl;
    }
}