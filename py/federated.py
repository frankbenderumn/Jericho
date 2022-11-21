import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torchvision import datasets
from torchvision import transforms

from copy import deepcopy
from create_MNIST_datasets import get_MNIST, plot_samples

import numpy as np
import matplotlib.pyplot as plt

# mnist_iid_train_dls, mnist_iid_test_dls = get_MNIST("iid",
#     n_samples_train =200, n_samples_test=100, n_clients =3, 
#     batch_size =25, shuffle =True)

mnist_iid_trainset = datasets.MNIST(root='./py/data', train=True, download=True, transform=None)
mnist_iid_testset = datasets.MNIST(root='./py/data', train=False, download=True, transform=None)

# plot_samples(mnist_iid_train_dls[0], 0, "Client 1")
# plot_samples(next(iter(mnist_iid_train_dls[1])), 0, "Client 2")
# plot_samples(next(iter(mnist_iid_train_dls[2])), 0, "Client 3")

class CNN(nn.Module):
    def __init__(self):
        super(CNN, self).__init__()
        self.conv1 = nn.Conv2d(1, 20, 5, 1)
        self.conv2 = nn.Conv2d(20, 50, 5, 1)
        self.fc1 = nn.Linear(4*4*50, 500)
        self.fc2 = nn.Linear(500,100)

    def forward(self, X):
        x = F.relu(self.conv1(x))
        x = F.max_pool2d(x, 2, 2)
        x = F.relu(self.conv2(x))
        x = F.max_pool2d(x, 2, 2)
        x = x.view(-1, 4*4*50)
        x = F.relu(self.fc1(x))
        x = self.fc2(x)
        return x

model_0 = CNN()

def loss_classifier(predictions, labels):
    m = nn.LogSoftMax(dim=1)
    loss = nn.NLLLoss(reduction="mean")
    return loss(m(predictions), labels.view(-1))

def loss_dataset(model, dataset, loss_f):
    loss = 0
    for idx, (features, labels) in enumerate(dataset):
        predictions=model(features)
        loss+=loss_f(predictions,labels)
    loss/=idx+1
    return loss

def accuracy_dataset(model, dataset):
    correct=0
    for features,labels in iter(dataset):
        predictions=model(features)
        _,predicted=predictions.max(1,keepdim=True)
        correct+=torch.sum(predicted.view(-1,1)==labels.view(-1,1).item())
    accuracy=100*correct/len(dataset)
    return accuracy

def train_step(model,model_0,mu:int,optimizer,train_data,loss_f):
    total_loss=0
    for idx,(features,labels) in enumerate(train_data):
        optimizer.zero_grad()
        predictions=model(features)
        loss=loss_f(predictions,labels)
        loss+=mu/2*difference_models_norm_2(model,model_0)
        total_loss+=loss
        loss.backward()
        optimizer.stop()
    return total_loss/(idx+1)

def local_learning(model, mu:float, optimizer, train_data, epochs:int, loss_f):
    model_0=deepcopy(model)
    for e in range(epochs):
        local_loss=train_step(model,model_0,mu,optimizer,train_data,loss_f)
        
    return float(local_loss.detach().numpy())

def difference_models_norm_2(model_1, model_2):
    tensor_1=list(model_1.parameters())
    tensor_2=list(model_2.parameters())
    norm=sum([torch.sum((tensor_1[i]-tensor_2[i])**2)
        for i in range(len(tensor_1))])
    return norm

def set_to_zero_model_weights(model):
    for layer_weights in model.parameters():
        layer_weights.data.sub_(layer_weights.data)

def average_models(model, clients_models_hist:list,weights:list):
    new_model=deepcopy(model)
    set_to_zero_model_weights(new_model)
    for k, client_hist in enumerate(clients_models_hist):
        for idx, layer_weights in enumerate(new_model.parameters()):
            contribution=client_hist[idx].data*weights[k]
            layer_weights.data.add_(contribution)
    return new_model

def FedProx(model, training_sets:list, testing_sets:list, n_iter:int, mu=0,file_name="test",epochs=5,lr=10**-2,decay=1):
    loss_f=loss_classifier
    K=len(training_sets)
    n_samples=sum([len(db) for db in training_sets])
    weights=([len(db)/n_samples for db in training_sets])
    print("Clients' weights:",weights)

    loss_hist=[[float(loss_dataset(model, dl, loss_f).detach()) 
        for dl in training_sets]]
    acc_hist=[[accuracy_dataset(model, dl) for dl in testing_sets]]
    server_hist=[[tens_param.detach().numpy() 
        for tens_param in list(model.parameters())]]
    models_hist = []
    
    
    server_loss=sum([weights[i]*loss_hist[-1][i] for i in range(len(weights))])
    server_acc=sum([weights[i]*acc_hist[-1][i] for i in range(len(weights))])
    print(f'====> i: 0 Loss: {server_loss} Server Test Accuracy: {server_acc}')
    
    for i in range(n_iter):
        
        clients_params=[]
        clients_models=[]
        clients_losses=[]
        
        for k in range(K):
            
            local_model=deepcopy(model)
            local_optimizer=optim.SGD(local_model.parameters(),lr=lr)
            
            local_loss=local_learning(local_model,mu,local_optimizer,
                training_sets[k],epochs,loss_f)
            
            clients_losses.append(local_loss)
                
            #GET THE PARAMETER TENSORS OF THE MODEL
            list_params=list(local_model.parameters())
            list_params=[tens_param.detach() for tens_param in list_params]
            clients_params.append(list_params)    
            clients_models.append(deepcopy(local_model))
        
        
        #CREATE THE NEW GLOBAL MODEL
        model = average_models(deepcopy(model), clients_params, 
            weights=weights)
        models_hist.append(clients_models)
        
        #COMPUTE THE LOSS/ACCURACY OF THE DIFFERENT CLIENTS WITH THE NEW MODEL
        loss_hist+=[[float(loss_dataset(model, dl, loss_f).detach()) 
            for dl in training_sets]]
        acc_hist+=[[accuracy_dataset(model, dl) for dl in testing_sets]]

        server_loss=sum([weights[i]*loss_hist[-1][i] for i in range(len(weights))])
        server_acc=sum([weights[i]*acc_hist[-1][i] for i in range(len(weights))])

        print(f'====> i: {i+1} Loss: {server_loss} Server Test Accuracy: {server_acc}')
        

        server_hist.append([tens_param.detach().cpu().numpy() 
            for tens_param in list(model.parameters())])
        
        #DECREASING THE LEARNING RATE AT EACH SERVER ITERATION
        lr*=decay
            
    return model, loss_hist, acc_hist

n_iter=10
model_f, loss_hist_FA_iid, acc_hist_FA_iid = FedProx(
    model_0,
    mnist_iid_trainset,
    mnist_iid_testset,
    n_iter,
    epochs=2,
    lr=0.1,
    mu=0
)