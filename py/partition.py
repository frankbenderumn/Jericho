from generate_mnist import *
import torch
import torchvision.datasets as datasets
import numpy as np
import random
                                
def features(txt_file):
    all_data = np.loadtxt(txt_file, delimiter=",",
    usecols=range(0,785), dtype=np.int64)
    return all_data

def tensor_list(tensor):
    list = []
    for i in range(tensor.shape[0]):
        list.append(tensor[i][:])
    return list

def write(tensor_list, path):
    fp = open(path, "w")
    for i in tensor_list:
        for j in range(len(i) - 1):
            fp.write(str(i[j])+",")
        fp.write(str(i[783])+"\n")
    fp.close()

def shuffler():
    tensor = features("./py/data/MNIST/train.txt")
    shuffled = random.shuffle(tensor_list(tensor))
    write(shuffled, "./py/data/MNIST/shuffle.txt")
    print("DONE 1")
    random.shuffle(shuffled)
    print("DONE 2")
    write(shuffled, "./py/data/MNIST/shuffle2.txt")
    random.shuffle(shuffled)
    print("DONE 3")
    write(shuffled, "./py/data/MNIST/shuffle3.txt")
    random.shuffle(shuffled)
    print("DONE 4")
    write(shuffled, "./py/data/MNIST/shuffle4.txt")

def distribute(data, train=True, batch_size=200, client_list=[]):
    if (client_list==[]):
        client_list=[
            "./public/cluster/8081",
            "./public/cluster/8082",
            "./public/cluster/8083",
            "./public/cluster/8084",
            "./public/cluster/8085",
            "./public/cluster/8086",
            "./public/cluster/8087",
            "./public/cluster/8088",
            "./public/cluster/8089",
            "./public/cluster/8090",
        ]
    idx = 0
    file = "/train.txt"
    if not train:
        file = "/test.txt"
    for dir in client_list:
        partition = []
        path = dir + file
        for i in range(batch_size):
            partition.append(data[idx+i][:])
        idx += batch_size
        write(partition, path)
        partition = []

data = tensor_list(features("./py/data/MNIST/train.txt"))
data2 = tensor_list(features("./py/data/MNIST/test.txt"))
random.shuffle(data)
random.shuffle(data2)
distribute(data, 5000)
distribute(data2, False, 500)