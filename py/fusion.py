import torch
import torchvision
import numpy as np
from copy import deepcopy
import sys
import os

args = sys.argv
n = len(args[1])
v= sys.argv[1][1:n-1]
a = v.split(',')
round = args[2]
id = args[3]
dir = args[4]
mode = args[5]
model_script = args[6]

print("CLIENTS FUSING ARE")
print(a)

models = []

model_name = model_script

for i in a:
    model = torch.jit.load(model_name)
    path = dir + i + ".wt"
    print(path)
    if not os.path.exists(path):
        print("Path does not exist for ",i,". Has model been sent?", sep='')
        exit()
    model.load_state_dict(torch.load(path), strict=False)
    models.append(model)

def features(txt_file):
    all_data = np.loadtxt(txt_file, delimiter=",",
    usecols=range(0,785), dtype=np.int64)
    return all_data

def zero(model):
    for layer in model.parameters():
        layer.data.sub_(layer.data)

def average(model, clients, weights):
    avg = deepcopy(model)
    zero(avg)

    for client_idx, client in enumerate(clients):
        for layer_idx, layer in enumerate(avg.parameters()):
            contribution = client[layer_idx].data * weights[client_idx]
            layer.data.add_(contribution)

    return avg

# def FedProx(model, train_sets, iters, test_sets, mu=0, epochs=5, lr=10**-2, decay=1):
def FedProx(model, models):
    client_params = []

    for mod in models:
        client_params.append(list(mod.parameters())) 

    weights = [1/len(models)] * len(models)
    model = average(deepcopy(model), client_params, weights)
    print(weights)

    return model

accuracy = 0

def test(model, features):
    correct_count, all_count = 0, 0
    for i in range(features.shape[0]):
        data = features[i]
        img = data[1:].reshape(-1, 784)
        label = data[0]
        
        with torch.no_grad():
            logps = model(img)

        ps = torch.exp(logps)
        probab = list(ps.detach().numpy()[0])
        pred_label = probab.index(max(probab))
        true_label = label
        if(true_label == pred_label):
            correct_count += 1
        all_count += 1

    print("Number Of Images Tested =", all_count)
    global accuracy
    accuracy = (correct_count/all_count)
    print("\nModel Accuracy =", accuracy)
    
model0 = torch.jit.load("./py/scripts/torch.pt")
fused = FedProx(model0, models)
# torch.save(model.state_dict(), "./py/aggregator/aggegate-"+str(round)+".pt")

valid = features("./py/data/MNIST/test.txt")
valid = np.stack(valid, axis=0)
valid = torch.tensor(valid, dtype=torch.float32)

test(fused, valid)

if (mode == "main"): 
    fused_script = torch.jit.script(fused) # Export to TorchScript
    fused_script.save(dir + "model-"+str(round)+".pt") # Save
else:
    torch.save(fused.state_dict(), dir + "agg-"+str(round)+".wt")

fp = open(dir + "/log.txt", "a")
fp.write("Fed Id: "+str(id)+", Round - " + str(round) + ", Clients Trained: ["+str(v)+"], Accuracy: " + str(accuracy) + "\n")
fp.close()



