#include <stdio.h>
#include <tensorflow/c/c_api.h>

int main(void) {
  printf("Hello from TensorFlow C library version %s\n", TF_Version());
  
  TF_Graph* Graph = TF_NewGraph();
  TF_Status* Status = TF_NewStatus();
  TF_SessionOptions* SessionOpts = TF_NewSessionOptions();
  TF_Buffer* RunOpts = NULL;

  return 0;
}