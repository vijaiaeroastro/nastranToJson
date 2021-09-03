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

def trianglesFromJson(f_obj):
    fArr = np.empty([len(f_obj), 3], dtype=np.int64)
    for f in f_obj:
        fArr[f["id"], 0] = f["v1"]
        fArr[f["id"], 1] = f["v2"]
        fArr[f["id"], 2] = f["v3"]
    return fArr

def jsonToNumpy(input_file):
    f = open(input_file)
    data = json.load(f)
    vertices = vertexFromJson(data["vertices"])
    edges, colours = featuresFromJson(data["feature_curves"])
    triangles = trianglesFromJson(data["triangles"])
    return vertices, triangles, edges, colours

if __name__ == "__main__":
    ps.init()
    v, t, e, c = jsonToNumpy("/home/vijai.kumar/Code/nastranToJson/cmake-build-release/rotor_shaft.fjson")
    os_mesh = ps.register_surface_mesh("mesh", v, t)
    ps_net = ps.register_curve_network("Feature Curve", v, e)
    ps_net.add_color_quantity("f_id", c, defined_on='edges')
    ps.show()