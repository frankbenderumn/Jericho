import torch
import torch.nn as nn
import torch.optim as optim
import torch.functional as F
import numpy as np
import time
import torchvision
import sys

args = sys.argv[1:]

print("WE MADE IT! READING FROM FILE:",args[0])
client_dir = args[0]

train_dataset=torchvision.datasets.MNIST('./py/data',train=True,
                transform=torchvision.transforms.ToTensor(),download=True)

test_dataset=torchvision.datasets.MNIST('./py/data',train=False,     
                transform=torchvision.transforms.ToTensor(),download=True)

train_subset = torch.utils.data.Subset(train_dataset, torch.arange(20000))
test_subset = torch.utils.data.Subset(test_dataset, torch.arange(1000))

train_dataloader=torch.utils.data.DataLoader(dataset=train_subset, 
                                batch_size=32,shuffle=True)

train_dataloader=torch.utils.data.DataLoader(dataset=test_subset, 
                                batch_size=32,shuffle=True)

def features(txt_file):
    all_data = np.loadtxt(txt_file, delimiter=",",
    usecols=range(0,785), dtype=np.int64)
    return all_data

def train(model, features, epochs, batch_size):
    optimizer = optim.Adam(model.parameters(), lr=0.0001)
    time0 = time.time()
    criterion = nn.CrossEntropyLoss()

    losses = []
    for e in range(epochs):
        running_loss = 0
        for i, (data, label) in enumerate(train_dataloader):
            # print(i, "to", i+batch_size)
            # data = torch.tensor(features[i:i+100])
            img = data.view(data.size(0), -1)
            # label = data[:,0]
            # label = torch.tensor(label, dtype=torch.long)

            optimizer.zero_grad()

            output = model(img)

            loss = criterion(output, label)
            
            loss.backward()
            
            optimizer.step()
            
            running_loss += loss.item()
            losses.append(loss.item())

        if (running_loss/600 < .0001):
            break

        print("Epoch {} - Training loss: {}".format(e, running_loss/600))

    print("\nTraining Time (in minutes) =",(time.time()-time0)/60)
    print("Saving weights...")
    print("Model's state_dict:")
    for param_tensor in model.state_dict():
        print(param_tensor, "\t", model.state_dict()[param_tensor].size())

    # Print optimizer's state_dict
    print("Optimizer's state_dict:")
    for var_name in optimizer.state_dict():
        print(var_name, "\t", optimizer.state_dict()[var_name])
    torch.save(model.state_dict(), client_dir + "/mnist_train.wt")


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

# plot(losses, "Pytorch Adam")

# load full model
# model = torch.load("./py/models/ok_full")

# load torch script
# model = torch.jit.load("./py/scripts/torch.pt")
model = torch.jit.load(client_dir + "/model_init.pt")
# model.load_state_dict(torch.load("./py/models/ok"))
# model = torch.load("./py/models/ok_full")

# model.eval()
# print("Model's state_dict:")
# for param_tensor in model.state_dict():
#     print(param_tensor, "\t", model.state_dict()[param_tensor].size())

# mnist = features("./py/data/MNIST/train.txt")
# valid = features("./py/data/MNIST/test.txt")

mnist = features(client_dir + "/train.txt")
valid = features(client_dir + "/test.txt")
mnist = np.stack(mnist, axis=0)
valid = np.stack(valid, axis=0)
mnist = torch.tensor(mnist, dtype=torch.float32)
valid = torch.tensor(valid, dtype=torch.float32)

train(model, mnist, 5, 100)
test(model, valid)