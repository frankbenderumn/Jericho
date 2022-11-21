import torch
import torchvision
import torchvision.datasets as datasets
import numpy as np
import matplotlib as plt
import torch.nn as nn
import torch.optim as optim
import time
import torchvision.transforms as transforms
import torch.nn.functional as F

def features(txt_file):
    all_data = np.loadtxt(txt_file, delimiter=",",
    usecols=range(0,785), dtype=np.int64)
    x_data = all_data[:,1:785]
    y_data = all_data[:,0]
    return x_data, y_data

class MNIST:
    def __init__(self, file="./py/data/MNIST/", train=False):
        self.train=train
        self.tensors=[]
        if (self.train):
            self.file = file + "train.txt"
        else:
            self.file = file + "test.txt"
        self.pixels, self.labels = features(self.file)
        for i in self.pixels:
            i = i / 255
            t = torch.tensor(i, dtype=torch.float32)
            self.tensors.append(t)
    
    def dump(self, idx):
        pixels = self.pixels[idx]
        label = self.labels[idx]
        pixels = pixels.reshape((28,28))
        print("Label is:",label)
        for i in range(28):
            for j in range(28):
                print("%3d" % pixels[i,j], end="")
                print(" ", end="")
            print("")

        # plt.tight_layout()
        # plt.imshow(pixels, cmap=plt.get_cmap('gray_r'))
        # plt.show()

data = MNIST()

data.dump(0)
data.dump(1)
data.dump(2)
data.dump(3)
data.dump(4)
data.dump(5)
data.dump(6)
data.dump(7)
dataset = torch.stack(data.tensors)
labels = torch.tensor(data.labels)
print(torch.stack(data.tensors).shape)
print(labels.shape)
print(dataset[9])

input_size = 784
hidden_sizes = [128, 64]
output_size = 10

model = nn.Sequential(
    nn.Linear(input_size, hidden_sizes[0]),
    nn.ReLU(),
    nn.Linear(hidden_sizes[0], hidden_sizes[1]),
    nn.ReLU(),
    nn.Linear(hidden_sizes[1], output_size),
    nn.LogSoftmax(dim=1)
)

print(model)

criterion = nn.NLLLoss()
logps = model(dataset)
loss = criterion(logps, torch.tensor(data.labels))
print(logps)

print('Before backward pass: \n', model[0].weight.grad)
loss.backward()
print('After backward pass: \n', model[0].weight.grad)

optimizer = optim.SGD(model.parameters(), lr=0.003, momentum=0.9)
time0 = time.time()
epochs = 2
for e in range(epochs):
    running_loss = 0
    for i in range(len(data.tensors)):
        pixels = torch.stack(data.tensors)
        label = data.labels
        # Flatten MNIST images into a 784 long vector
        # images = images.view(images.shape[0], -1)
    
        # Training pass
        optimizer.zero_grad()
        
        # print("\033[1;34m",torch.tensor(pixels).shape,"\033[0m")

        output = model(pixels)

        pred = output.data.max(1, keepdim=True)[1]
        print(pred[7])
        loss = criterion(output, torch.tensor(label))
        
        #This is where the model learns by backpropagating
        loss.backward()
        
        #And optimizes its weights here
        optimizer.step()
        
        running_loss += loss.item()
    else:
        print("Epoch {} - Training loss: {}".format(e, running_loss/1000))
print("\nTraining Time (in minutes) =",(time.time()-time0)/60)

class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(1, 10, kernel_size=5)
        self.conv2 = nn.Conv2d(10, 20, kernel_size=5)
        self.conv2_drop = nn.Dropout2d()
        self.fc1 = nn.Linear(320, 50)
        self.fc2 = nn.Linear(50, 10)

    def forward(self, x):
        x = F.relu(F.max_pool2d(self.conv1(x), 2))
        x = F.relu(F.max_pool2d(self.conv2_drop(self.conv2(x)), 2))
        x = x.view(-1, 320)
        x = F.relu(self.fc1(x))
        x = F.dropout(x, training=self.training)
        x = self.fc2(x)
        return F.log_softmax(x, dim=1)


def f():
    n_epochs = 3
    batch_size_train = 64
    batch_size_test = 1000
    learning_rate = 0.01
    momentum = 0.5
    log_interval = 10

    random_seed = 1
    torch.backends.cudnn.enabled = False
    torch.manual_seed(random_seed)

    network = Net()
    optimizer = optim.SGD(network.parameters(), lr=learning_rate,
                        momentum=momentum)

    train_dataset=datasets.MNIST('./py/data/MNIST/raw',train=True,
                    transform=transforms.ToTensor(),download=True)

    test_dataset=datasets.MNIST('./py/data/MNIST/raw',train=False,     
                    transform=transforms.ToTensor(),download=False)

    train_dataloader=torch.utils.data.DataLoader(dataset=train_dataset, 
                                    batch_size=10,shuffle=True)

    train_dataloader=torch.utils.data.DataLoader(dataset=test_dataset, 
                                    batch_size=10,shuffle=True)

    data=iter(train_dataloader)
    samples,labels=next(data)
    print(f"number of samples {samples.shape}")
    print(f"number of labels {labels.shape}")

    examples = enumerate(train_dataloader)
    batch_idx, (example_data, example_targets) = next(examples)

    print(example_data.shape)
    # print(samples[0].view(-1).shape)
    # logps = model(samples[0].view(-1))
    example_data = example_data.view(example_data.size(0), -1)
    print(example_data.shape)
    output = model(example_data)
    print(output)
    pred = output.data.max(1, keepdim=True)[1]
    print(pred)
    # print(logps)
# f()