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

train_dataset=torchvision.datasets.MNIST('./py/data/MNIST/raw',train=True,
                transform=torchvision.transforms.ToTensor(),download=True)

test_dataset=torchvision.datasets.MNIST('./py/data/MNIST/raw',train=False,     
                transform=torchvision.transforms.ToTensor(),download=True)