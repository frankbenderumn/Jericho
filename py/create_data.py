import os
import torchvision.datasets as datasets
import numpy as np
import torch

# Load MNIST test set
mnist_test = datasets.MNIST(root='./data', train=False, download=True)

# Create directory to store data files if it doesn't exist
if not os.path.exists('./py/data/MNIST'):
    os.makedirs('./py/data/MNIST')

# Save test set to file
with open('./py/data/MNIST/test.txt', 'w') as f:
    for i in range(len(mnist_test)):
        image, label = mnist_test[i]
        f.write(','.join([str(label)] + [str(pixel) for pixel in np.array(image).flatten().tolist()]) + '\n')

# Load test set from file
mnist_test_loaded = []
with open('./py/data/MNIST/test.txt', 'r') as f:
    for line in f:
        data = line.strip().split(',')
        label = int(data[0])
        image = torch.tensor([int(pixel) for pixel in data[1:]], dtype=torch.uint8).view(28, 28)
        mnist_test_loaded.append((image, label))

# Print the number of examples in the loaded test set
print("Loaded test set size:", len(mnist_test_loaded))