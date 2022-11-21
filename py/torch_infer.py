import numpy as np
import torch
import torch.nn as nn

def features(txt_file):
    all_data = np.loadtxt(txt_file, delimiter=",",
    usecols=range(0,785), dtype=np.int64)
    valid = np.stack(all_data, axis=0)
    valid = torch.tensor(valid, dtype=torch.float32)
    return valid

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
    print("\nModel Accuracy =", (correct_count/all_count))

valid = features("./py/data/MNIST/test.txt")
model = torch.jit.load("./py/scripts/torch.pt")
model.load_state_dict(torch.load("./py/models/mnist_train.wt"))
# model.eval()

test(model, valid)
