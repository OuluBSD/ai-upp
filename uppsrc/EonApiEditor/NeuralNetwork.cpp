#include "EonApiEditor.h"

NAMESPACE_UPP

void InterfaceBuilder::AddNeuralNetwork() {
    Package("NeuralNetwork", "Nn");
    SetColor(200, 0, 150);
    Dependency("ParallelLib");
    Dependency("ports/tensorflow", "BUILTIN_TENSORFLOW");
    Dependency("ports/pytorch", "BUILTIN_PYTORCH");
    Library("tensorflow", "TENSORFLOW");
    Library("pytorch", "PYTORCH");
    
    Interface("Model");
    Interface("Tensor");
    Interface("Layer");
    Interface("Optimizer");
    Interface("LossFunction");
    Interface("NeuralNetworkEngine");
    Interface("TrainingSession");
    Interface("InferenceSession");
    Interface("DataLoader");
    Interface("NeuralNetworkContext");
    
    Vendor("TensorFlow", "BUILTIN_TENSORFLOW|TENSORFLOW");
    Vendor("PyTorch", "BUILTIN_PYTORCH|PYTORCH");
    Vendor("OnnxRuntime", "ONNXRUNTIME");
    Vendor("Nnapi", "NNAPI");
}

END_UPP_NAMESPACE