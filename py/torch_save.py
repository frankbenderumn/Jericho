import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F

# Define model
class MNet(nn.Module):
    def __init__(self, input_size, hidden_size, output_size):
        super(MNet, self).__init__()
        self.linear = nn.Sequential(
            nn.Linear(input_size, hidden_size[0]),
            nn.ReLU(),
            nn.Linear(hidden_size[0], hidden_size[1]),
            nn.ReLU(),
            nn.Linear(hidden_size[1], output_size),
            nn.LogSoftmax(dim=1)
        )

    def forward(self, x):
        return self.linear(x)

model = MNet(784, [128, 64], 10)

# Initialize optimizer
optimizer = optim.Adam(model.parameters(), lr=0.001)

# Print model's state_dict
print("Model's state_dict:")
for param_tensor in model.state_dict():
    print(param_tensor, "\t", model.state_dict()[param_tensor].size())

# Print optimizer's state_dict
print("Optimizer's state_dict:")
for var_name in optimizer.state_dict():
    print(var_name, "\t", optimizer.state_dict()[var_name])

# save just model
torch.save(model.state_dict(), "./py/models/ok.mod")

# save everything
torch.save(model, "./py/models/ok_full.fmod")

# save as torch script
model_script = torch.jit.script(model)
model_script.save("./py/scripts/torch.pt", _use_new_zipfile_serialization=False)

#   local_header_magic_number = [b'P', b'K', b'\x03', b'\x04']