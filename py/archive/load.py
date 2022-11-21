import tensorflow as tf
 
def load_graph(trained_model):   
    with tf.compat.v2.io.gfile.GFile(trained_model, "rb") as f:
        graph_def = tf.compat.v1.GraphDef()
        graph_def.ParseFromString(f.read())
 
    with tf.Graph().as_default() as graph:
        tf.import_graph_def(
            graph_def,
            input_map=None,
            return_elements=None,
            name=""
            )
    return graph

graph = load_graph("./py/models/aggregator/saved_model.pb")
graph.summary()