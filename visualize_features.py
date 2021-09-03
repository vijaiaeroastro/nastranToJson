import numpy as np
import polyscope as ps
import json

def vertexFromJson(v_obj):
    vArr = np.empty([len(v_obj), 3], dtype=np.float64)
    for v in v_obj:
        vArr[v["id"], 0] = v["x"]
        vArr[v["id"], 1] = v["y"]
        vArr[v["id"], 2] = v["z"]
    return vArr

def featuresFromJson(f_obj):
    feature_array = list()
    feature_colour = list()
    max_id = 0
    for f in f_obj:
        if (f["id"] > max_id):
            max_id = f["id"]
    max_id = max_id + 1
    unique_colours = np.random.rand(max_id, 3)
    for f in f_obj:
        chain = f["chain"]
        for fpair in list(zip(chain, chain[1:])):
            feature_array.append(fpair)
            feature_colour.append(unique_colours[f["id"]])
    return np.array(feature_array), np.array(feature_colour)

def jsonToNumpy(input_file):
    f = open(input_file)
    data = json.load(f)
    vertex_data = data["vertices"]
    vertices = vertexFromJson(vertex_data)
    feature_data = data["feature_curves"]
    edges, colours = featuresFromJson(feature_data)
    return vertices, edges, colours

if __name__ == "__main__":
    ps.init()
    v, e, c = jsonToNumpy("/home/vijai.kumar/Code/nastranToJson/cmake-build-release/bracket.fjson")
    ps_net = ps.register_curve_network("Feature Curve", v, e)
    ps_net.add_color_quantity("f_id", c, defined_on='edges')
    ps.show()