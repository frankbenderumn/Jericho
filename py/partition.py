import tensorflow_federated as tff

emnist_train, emnist_test = tff.simulation.datasets.emnist.load_data()

num_clients = len(emnist_train.client_ids)

print("Number of clients: ",num_clients)

print("Structure: ",emnist_train.element_type_structure)