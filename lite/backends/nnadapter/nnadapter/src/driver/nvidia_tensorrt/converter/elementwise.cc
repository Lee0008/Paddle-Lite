// Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "operation/elementwise.h"
#include "driver/nvidia_tensorrt/converter/converter.h"
#include "utility/debug.h"
#include "utility/logging.h"

namespace nnadapter {
namespace nvidia_tensorrt {

int ConvertElementwise(Converter* converter, core::Operation* operation) {
  ELEMENTWISE_OPERATION_EXTRACT_INPUTS_OUTPUTS
  NNADAPTER_CHECK_EQ(fuse_code, NNADAPTER_FUSED_NONE);
  // Convert to trt tensors and node
  auto input0_tensor = converter->GetMappedTensor(input0_operand);
  if (!input0_tensor) {
    input0_tensor = converter->ConvertOperand(input0_operand);
  }
  auto input1_tensor = converter->GetMappedTensor(input1_operand);
  if (!input1_tensor) {
    input1_tensor = converter->ConvertOperand(input1_operand);
  }
  std::map<NNAdapterOperationType, nvinfer1::ElementWiseOperation>
      elementwise_type_map{
          {NNADAPTER_ADD, nvinfer1::ElementWiseOperation::kSUM},
          {NNADAPTER_MUL, nvinfer1::ElementWiseOperation::kPROD},
          {NNADAPTER_SUB, nvinfer1::ElementWiseOperation::kSUB},
      };
  auto operation_type = operation->type;
  NNADAPTER_CHECK(elementwise_type_map.count(operation_type))
      << "Not support operation_type: "
      << OperationTypeToString(operation_type);
  auto elementwise_layer = converter->network()->addElementWise(
      *input0_tensor, *input1_tensor, elementwise_type_map.at(operation_type));
  NNADAPTER_CHECK(elementwise_layer);
  auto output_tensor = elementwise_layer->getOutput(0);
  converter->UpdateTensorMap(output_operand, output_tensor);
  return NNADAPTER_NO_ERROR;
}

}  // namespace nvidia_tensorrt
}  // namespace nnadapter
